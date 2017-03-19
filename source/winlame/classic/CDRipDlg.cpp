//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2005-2017 Michael Fink
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file CDRipDlg.cpp
/// \brief contains the dialog for cd ripping
//
#include "stdafx.h"
#include "CDRipDlg.hpp"
#include "CDRipTrackManager.hpp"
#include "OptionsDlg.hpp"
#include "basscd.h"
#include "encoder/TrackInfo.hpp"
#include <shellapi.h>
#include <atlctrlx.h> // CWaitCursor
#include "DynamicLibrary.hpp"
#include "IniFile.hpp"
#include "App.hpp"
#include "FreedbInfo.hpp"
#include "FreeDbDiscListDlg.hpp"
#include "UTF8.hpp"
#include "CoverArtArchive.hpp"
#include "CoverArtDlg.hpp"
#include "App.hpp"

using ClassicUI::CDRipDlg;

const DWORD INVALID_DRIVE_ID = 0xffffffff;

CDRipDlg::CDRipDlg(UISettings& uiSettings, ClassicUI::UIinterface& UIinterface)
   :m_uiSettings(uiSettings),
   m_bDriveActive(false),
   m_bAcquiredDiscInfo(false),
   m_bEditedTrack(false),
   m_UIinterface(UIinterface)
{
}

bool CDRipDlg::IsCDExtractionAvail() throw()
{
   return DynamicLibrary(_T("basscd.dll")).IsLoaded();
}

LRESULT CDRipDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // center the dialog on the screen
   CenterWindow();

   // set icons
   HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME),
      IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
   SetIcon(hIcon, TRUE);
   HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME),
      IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
   SetIcon(hIconSmall, FALSE);

   DoDataExchange(DDX_LOAD);

   // drive combobox
   unsigned int nDriveCount = 0;
   {
      for (DWORD n = 0; n < 26; n++)
      {
         BASS_CD_INFO info = { 0 };
         BOOL bRet = BASS_CD_GetInfo(n, &info);
         if (bRet == FALSE)
            break;

         CString cszDriveDescription(info.product);
         DWORD nLetter = info.letter;

         if (!cszDriveDescription.IsEmpty())
         {
            CString cszText;

            if (nLetter == -1)
               cszText = cszDriveDescription; // couldn't get drive letter; restricted user account
            else
               cszText.Format(_T("[%c:] %s"), static_cast<char>(_T('A') + nLetter), cszDriveDescription.GetString());

            int nItem = m_cbDrives.AddString(cszText);
            m_cbDrives.SetItemData(nItem, n);

            nDriveCount++;
         }
      }
   }

   // hide drive combobox when only one drive present
   if (nDriveCount <= 1)
   {
      CRect rcCombo, rcList;
      m_cbDrives.GetWindowRect(rcCombo);
      m_lcTracks.GetWindowRect(rcList);
      rcList.top = rcCombo.top;
      ScreenToClient(rcList);
      m_lcTracks.MoveWindow(rcList);

      m_cbDrives.ShowWindow(SW_HIDE);
      m_lcTracks.SetFocus();
   }

   // tracks list
   m_lcTracks.SetExtendedListViewStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT, 0);

   CString cszText(MAKEINTRESOURCE(IDS_CDRIP_COLUMN_NR));
   m_lcTracks.InsertColumn(0, cszText, LVCFMT_LEFT, 40, 0);
   cszText.LoadString(IDS_CDRIP_COLUMN_TRACK);
   m_lcTracks.InsertColumn(1, cszText, LVCFMT_LEFT, 240, 0);
   cszText.LoadString(IDS_CDRIP_COLUMN_LENGTH);
   m_lcTracks.InsertColumn(2, cszText, LVCFMT_LEFT, 60, 0);

   cszText.LoadString(IDS_CDRIP_UNKNOWN_ARTIST);
   SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, cszText);
   cszText.LoadString(IDS_CDRIP_UNKNOWN_TITLE);
   SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, cszText);

   // genre combobox
   LPCTSTR* apszGenre = Encoder::TrackInfo::GetGenreList();
   for (unsigned int i = 0, iMax = Encoder::TrackInfo::GetGenreListLength(); i < iMax; i++)
      m_cbGenre.AddString(apszGenre[i]);

   // misc.
   m_cbDrives.SetCurSel(0);

   m_buttonPlay.EnableWindow(false);
   m_buttonStop.EnableWindow(false);

   m_bEditedTrack = false;

   PostMessage(WM_TIMER, IDT_CDRIP_CHECK);

   SetTimer(IDT_CDRIP_CHECK, 2 * 1000);

   DlgResize_Init(true, true);

   return 1;
}

LRESULT CDRipDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   if (wParam == IDT_CDRIP_CHECK)
      CheckCD();
   return 0;
}

LRESULT CDRipDlg::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   KillTimer(IDT_CDRIP_CHECK);

   if (wID == IDOK)
   {
      if (m_bEditedTrack)
         StoreInCdplayerIni(GetCurrentDrive());

      UpdateTrackManager();

      UpdatePlaylistFilename();
   }

   // ends the modal dialog
   EndDialog(wID);

   return 0;
}

LRESULT CDRipDlg::OnDriveSelEndOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)//(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
   RefreshCDList();
   return 0;
}

LRESULT CDRipDlg::OnListDoubleClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
   return OnClickedButtonPlay(0, 0, 0, bHandled);
}

LRESULT CDRipDlg::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
   static bool s_ignoreChangedMessage = false;

   if (s_ignoreChangedMessage)
      return 0;

   NM_LISTVIEW& nmListView = *reinterpret_cast<NM_LISTVIEW*>(pnmh);

   if (nmListView.uOldState == 0 && nmListView.uNewState == 0)
      return 0;

   BOOL prevState = (BOOL)(((nmListView.uOldState & LVIS_STATEIMAGEMASK) >> 12) - 1);
   if (prevState < 0) // on startup there's no previous state
      prevState = 0;

   BOOL isChecked = (BOOL)(((nmListView.uNewState & LVIS_STATEIMAGEMASK) >> 12) - 1);
   if (isChecked < 0) // on non-checkbox notifications assume false
      isChecked = 0;

   if (prevState == isChecked) // no change in check box
      return 0;

   // when a check was changed of a selected item, change the check boxes of
   // the selected items, too
   if (m_lcTracks.GetItemState(nmListView.iItem, LVIS_SELECTED) != 0)
   {
      s_ignoreChangedMessage = true;

      for (int itemIndex = 0; itemIndex < m_lcTracks.GetItemCount(); itemIndex++)
      {
         if (nmListView.iItem == itemIndex)
            continue; // ignore currently changed list item

                      // only change selected lines
         if (m_lcTracks.GetItemState(itemIndex, LVIS_SELECTED) != 0)
            m_lcTracks.SetCheckState(itemIndex, isChecked);
      }

      s_ignoreChangedMessage = false;
   }

   return 0;
}

LRESULT CDRipDlg::OnClickedButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   DWORD nDrive = GetCurrentDrive();

   int nItem = m_lcTracks.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

   if (nItem < 0) nItem = 0; // no track selected? play first one

   DWORD nTrack = m_lcTracks.GetItemData(nItem);

   BASS_CD_Analog_Play(nDrive, nTrack, 0);
   m_buttonStop.EnableWindow(true);

   return 0;
}

LRESULT CDRipDlg::OnClickedButtonStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   BASS_CD_Analog_Stop(GetCurrentDrive());

   m_buttonStop.EnableWindow(false);

   return 0;
}

LRESULT CDRipDlg::OnClickedButtonFreedb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   FreedbLookup();
   return 0;
}

LRESULT CDRipDlg::OnClickedButtonAlbumArt(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   DWORD driveIndex = GetCurrentDrive();

   const char* cdid = BASS_CD_GetID(driveIndex, BASS_CDID_MUSICBRAINZ);
   if (cdid == nullptr || strlen(cdid) == 0)
      return 0;

   RetrieveAlbumCoverArt(cdid);

   return 0;
}

LRESULT CDRipDlg::OnClickedStaticAlbumArt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   UI::CoverArtDlg dlg(m_coverArtImage);
   dlg.DoModal(m_hWnd);
   return 0;
}

LRESULT CDRipDlg::OnClickedButtonOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   OptionsDlg dlg(m_uiSettings, m_UIinterface.GetLanguageResourceManager());
   dlg.DoModal(m_hWnd);
   return 0;
}

LRESULT CDRipDlg::OnClickedCheckVariousArtists(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   UpdateVariousArtistsCheck();
   return 0;
}

LRESULT CDRipDlg::OnChangedEditCtrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   m_bEditedTrack = true;

   return 0;
}

DWORD CDRipDlg::GetCurrentDrive()
{
   int nSel = m_cbDrives.GetCurSel();
   if (nSel == -1)
      return INVALID_DRIVE_ID;

   return m_cbDrives.GetItemData(nSel);
}

void CDRipDlg::RefreshCDList()
{
   CWaitCursor waitCursor;

   UI::RedrawLock lock(m_lcTracks);
   m_lcTracks.DeleteAllItems();

   m_buttonPlay.EnableWindow(false);

   DWORD nDrive = GetCurrentDrive();
   if (nDrive == INVALID_DRIVE_ID)
      return;

   m_bEditedTrack = false;

   if (FALSE == BASS_CD_IsReady(nDrive))
   {
      m_bDriveActive = false;
      return;
   }

   m_bDriveActive = true;

   DWORD uMaxCDTracks = BASS_CD_GetTracks(nDrive);
   if (uMaxCDTracks == DWORD(-1))
   {
      return;
   }

   for (DWORD n = 0; n < uMaxCDTracks; n++)
   {
      CString cszText;
      cszText.Format(_T("%lu"), n + 1);

      DWORD nLength = BASS_CD_GetTrackLength(nDrive, n);
      bool bDataTrack = (nLength == 0xFFFFFFFF && BASS_ERROR_NOTAUDIO == BASS_ErrorGetCode());

      int nItem = m_lcTracks.InsertItem(m_lcTracks.GetItemCount(), cszText);
      m_lcTracks.SetItemData(nItem, n);
      m_lcTracks.SetCheckState(nItem, true);

      cszText.Format(IDS_CDRIP_TRACK_U, n + 1);
      m_lcTracks.SetItemText(nItem, 1, cszText);

      if (!bDataTrack)
      {
         if (nLength != 0xFFFFFFFF)
         {
            nLength /= 176400;

            cszText.Format(_T("%lu:%02lu"), nLength / 60, nLength % 60);
            m_lcTracks.SetItemText(nItem, 2, cszText);
         }
         else
         {
            cszText.LoadString(IDS_CDRIP_TRACKTYPE_UNKNOWN);
            m_lcTracks.SetItemText(nItem, 2, cszText);
         }
      }
      else
      {
         cszText.LoadString(IDS_CDRIP_TRACKTYPE_DATA);
         m_lcTracks.SetItemText(nItem, 2, cszText);
      }
   }

   if (uMaxCDTracks > 0)
      m_buttonPlay.EnableWindow(true);

   bool bVarious = false;
   if (!ReadCachedCDDB(bVarious))
      if (!ReadCdplayerIni(bVarious))
         ReadCDText(bVarious);

   // check or uncheck "various artists"
   m_checkVariousArtists.SetCheck(bVarious ? BST_CHECKED : BST_UNCHECKED);
   UpdateVariousArtistsCheck();

   m_bEditedTrack = false;
}

bool CDRipDlg::ReadCachedCDDB(bool& bVarious)
{
   DWORD driveIndex = GetCurrentDrive();

   const char* entry = BASS_CD_GetID(driveIndex, BASS_CDID_CDDB_READ_CACHE);
   if (!entry || strlen(entry) == 0)
      return false;

   FreedbInfo info(UTF8ToString(entry));
   FillListFreedbInfo(info, bVarious);

   m_bAcquiredDiscInfo = true;

   return true;
}

bool CDRipDlg::ReadCdplayerIni(bool& bVarious)
{
   // retrieve info from cdplayer.ini
   CString cszCDPlayerIniFilename = Path::Combine(Path::WindowsFolder(), _T("cdplayer.ini"));

   DWORD nDrive = GetCurrentDrive();

   DWORD uMaxCDTracks = BASS_CD_GetTracks(nDrive);

   const char* cdplayer_id_raw = BASS_CD_GetID(nDrive, BASS_CDID_CDPLAYER);

   CString cdplayer_id(cdplayer_id_raw);

   IniFile ini(cszCDPlayerIniFilename);

   unsigned int nNumTracks = 0;
   if (cdplayer_id_raw != NULL && 0 != (nNumTracks = ini.GetInt(cdplayer_id, _T("numtracks"), 0)))
   {
      CString cszText;
      // title
      cszText = ini.GetString(cdplayer_id, _T("title"), _T("[]#"));

      if (cszText != _T("[]#"))
         SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, cszText);

      // artist
      cszText = ini.GetString(cdplayer_id, _T("artist"), _T("[]#"));

      if (cszText != _T("[]#"))
         SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, cszText);

      CString textLower(cszText);
      textLower.MakeLower();
      if (-1 != textLower.Find(_T("various")))
         bVarious = true;

      // year
      cszText = ini.GetString(cdplayer_id, _T("year"), _T("[]#"));

      if (cszText != _T("[]#"))
         SetDlgItemText(IDC_CDSELECT_EDIT_YEAR, cszText);

      // genre
      cszText = ini.GetString(cdplayer_id, _T("genre"), _T("[]#"));

      if (cszText != _T("[]#"))
      {
         int nItem = m_cbGenre.FindStringExact(-1, cszText);
         if (nItem == CB_ERR)
            nItem = m_cbGenre.AddString(cszText);

         m_cbGenre.SetCurSel(nItem);
      }

      // limit to actual number of tracks in list
      if (nNumTracks > uMaxCDTracks)
         nNumTracks = uMaxCDTracks;

      // tracks
      CString cszNumTrack;
      for (unsigned int n = 0; n < nNumTracks; n++)
      {
         cszNumTrack.Format(_T("%u"), n);

         cszText = ini.GetString(cdplayer_id, cszNumTrack, _T("[]#"));

         if (cszText != _T("[]#"))
         {
            m_lcTracks.SetItemText(n, 1, cszText);

            if (!bVarious && cszText.Find(_T(" / ")) != -1)
               bVarious = true;
         }
      }

      m_bAcquiredDiscInfo = true;

      return true;
   }
   else
      return false;
}

void CDRipDlg::ReadCDText(bool& bVarious)
{
   DWORD nDrive = GetCurrentDrive();

   DWORD uMaxCDTracks = BASS_CD_GetTracks(nDrive);

   const CHAR* cdtext = BASS_CD_GetID(nDrive, BASS_CDID_TEXT);
   if (cdtext != NULL)
   {
      std::vector<CString> vecTitles(uMaxCDTracks + 1);
      std::vector<CString> vecPerformer(uMaxCDTracks + 1);

      CString cszOutput;
      const CHAR* endpos = cdtext;
      do
      {
         while (*endpos++ != 0);

         CString cszText(cdtext);
         if (cdtext == strstr(cdtext, "TITLE"))
         {
            LPSTR pNext = NULL;
            unsigned long uTrack = strtoul(cdtext + 5, &pNext, 10);
            if (uTrack < uMaxCDTracks + 1)
            {
               vecTitles[uTrack] = pNext + 1;
               vecTitles[uTrack].Trim();
            }
         }
         if (cdtext == strstr(cdtext, "PERFORMER"))
         {
            LPSTR pNext = NULL;
            unsigned long uPerf = strtoul(cdtext + 9, &pNext, 10);
            if (uPerf < uMaxCDTracks + 1)
            {
               vecPerformer[uPerf] = pNext + 1;
               vecPerformer[uPerf].Trim();
            }

            if (uPerf > 0 &&
               !vecPerformer[uPerf].IsEmpty() &&
               vecPerformer[0] != vecPerformer[uPerf])
            {
               bVarious = true;
            }
         }

         cdtext = endpos;
      } while (*endpos != 0);

      // set title and artist
      SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, vecTitles[0]);
      SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, vecPerformer[0]);

      CString cszFormat;
      for (DWORD n = 1; n < uMaxCDTracks + 1; n++)
      {
         if (vecPerformer[n].IsEmpty() ||
            vecPerformer[n] == vecPerformer[0])
            cszFormat = vecTitles[n];
         else
            cszFormat.Format(_T("%s / %s"), vecPerformer[n].GetString(), vecTitles[n].GetString());

         m_lcTracks.SetItemText(n - 1, 1, cszFormat);
      }

      m_bAcquiredDiscInfo = true;
   }
}

void CDRipDlg::CheckCD()
{
   DWORD dwDrive = GetCurrentDrive();
   if (dwDrive == INVALID_DRIVE_ID)
      return;

   // check if current track still plays
   BOOL fPlaying = BASS_ACTIVE_PLAYING == BASS_CD_Analog_IsActive(dwDrive) ? TRUE : FALSE;
   m_buttonStop.EnableWindow(fPlaying);

   // check for new cd in drive
   bool bIsReady = BASS_CD_IsReady(dwDrive) == TRUE;
   if (m_bDriveActive != bIsReady)
   {
      RefreshCDList();
   }
}

void CDRipDlg::UpdateVariousArtistsCheck()
{
   bool isChecked = BST_CHECKED != m_checkVariousArtists.GetCheck();

   GetDlgItem(IDC_CDSELECT_EDIT_ARTIST).EnableWindow(isChecked);
   GetDlgItem(IDC_CDSELECT_STATIC_ARTIST).EnableWindow(isChecked);
}

void CDRipDlg::UpdateTrackManager()
{
   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

   DWORD nDrive = GetCurrentDrive();

   CDRipDiscInfo& discinfo = pManager->GetDiscInfo();
   discinfo.m_discDrive = nDrive;

   GetDlgItemText(IDC_CDSELECT_EDIT_TITLE, discinfo.m_discTitle);

   GetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, discinfo.m_discArtist);

   discinfo.m_year = GetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, NULL, FALSE);

   int nItem = m_cbGenre.GetCurSel();
   if (nItem == CB_ERR)
   {
      m_cbGenre.GetWindowText(discinfo.m_genre);
   }
   else
      m_cbGenre.GetLBText(nItem, discinfo.m_genre);

   discinfo.m_variousArtists = BST_CHECKED == m_checkVariousArtists.GetCheck();

   discinfo.m_CDID = BASS_CD_GetID(nDrive, BASS_CDID_CDDB);

   pManager->ResetTrackInfo();

   // write all selected items into manager
   std::vector<DWORD> vecTracks;

   for (int itemIndex = 0; itemIndex < m_lcTracks.GetItemCount(); itemIndex++)
   {
      if (!m_lcTracks.GetCheckState(itemIndex))
         continue;

      vecTracks.push_back(m_lcTracks.GetItemData(itemIndex));
   }

   unsigned int nMax = vecTracks.size();

   for (unsigned int n = 0; n < nMax; n++)
   {
      unsigned int nTrack = vecTracks[n];

      DWORD nLength = BASS_CD_GetTrackLength(nDrive, nTrack);
      bool isDataTrack = (nLength == 0xFFFFFFFF && BASS_ERROR_NOTAUDIO == BASS_ErrorGetCode());
      if (isDataTrack)
         continue;

      CDRipTrackInfo trackInfo = ReadTrackInfo(nDrive, nTrack, discinfo);

      pManager->AddTrackInfo(trackInfo);
   }

   pManager->SetFrontCoverArtImage(m_covertArtImageData);

   // when acquired freedb info, store  in cdplayer.ini, too
   if (m_bAcquiredDiscInfo)
      StoreInCdplayerIni(nDrive);
}

void CDRipDlg::StoreInCdplayerIni(unsigned int nDrive)
{
   if (!m_uiSettings.store_disc_infos_cdplayer_ini)
      return;

   CString cszCDPlayerIniFilename = Path::Combine(Path::WindowsFolder(), _T("cdplayer.ini"));

   const char* cdplayer_id_raw = BASS_CD_GetID(nDrive, BASS_CDID_CDPLAYER);

   CString cdplayer_id(cdplayer_id_raw);

   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();
   CDRipDiscInfo& discinfo = pManager->GetDiscInfo();

   CString cszFormat;

   IniFile ini(cszCDPlayerIniFilename);

   // numtracks
   unsigned int nNumTracks = m_lcTracks.GetItemCount();
   cszFormat.Format(_T("%u"), nNumTracks);
   ini.WriteString(cdplayer_id, _T("numtracks"), cszFormat);

   // artist
   ini.WriteString(cdplayer_id, _T("artist"), discinfo.m_discArtist);

   // title
   ini.WriteString(cdplayer_id, _T("title"), discinfo.m_discTitle);

   // year
   if (discinfo.m_year > 0)
   {
      cszFormat.Format(_T("%u"), discinfo.m_year);
      ini.WriteString(cdplayer_id, _T("year"), cszFormat);
   }

   // genre
   ini.WriteString(cdplayer_id, _T("genre"), discinfo.m_genre);

   // tracks
   CString cszTrackText;
   for (unsigned int n = 0; n < nNumTracks; n++)
   {
      cszFormat.Format(_T("%u"), n);

      m_lcTracks.GetItemText(n, 1, cszTrackText);

      ini.WriteString(cdplayer_id, cszFormat, cszTrackText);
   }
}

void CDRipDlg::UpdatePlaylistFilename()
{
   DWORD nDrive = GetCurrentDrive();
   if (nDrive == INVALID_DRIVE_ID)
      return;

   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();
   CDRipDiscInfo& discInfo = pManager->GetDiscInfo();

   if (discInfo.m_variousArtists)
   {
      m_uiSettings.playlist_filename.Format(
         _T("%s.m3u"),
         discInfo.m_discTitle.GetString());
   }
   else
   {
      m_uiSettings.playlist_filename.Format(
         _T("%s - %s.m3u"),
         discInfo.m_discArtist.GetString(),
         discInfo.m_discTitle.GetString());
   }
}

LRESULT CDRipDlg::OnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   NMLVDISPINFO* pLvDispInfo = reinterpret_cast<NMLVDISPINFO*>(pnmh);
   if (pLvDispInfo->item.iItem == -1)
      return 0;

   m_bEditedTrack = true;

   return 1;
}

void CDRipDlg::FreedbLookup()
{
   CString serverAddress = m_uiSettings.freedb_server;

   BOOL retConfig = BASS_SetConfigPtr(
      BASS_CONFIG_CD_CDDB_SERVER,
      CStringA(serverAddress).GetString());
   ATLASSERT(TRUE == retConfig); retConfig;

   CWaitCursor waitCursor;

   DWORD driveIndex = GetCurrentDrive();

   BASS_CD_GetID(driveIndex, BASS_CDID_CDDB_QUERY);

   std::vector<FreedbInfo> entriesList;

   for (int entryIndex = 0; entryIndex < 100; entryIndex++)
   {
      const char* entry = BASS_CD_GetID(driveIndex, BASS_CDID_CDDB_READ + entryIndex);
      if (!entry || strlen(entry) == 0)
         break;

      entriesList.push_back(FreedbInfo(UTF8ToString(entry)));
   }

   if (entriesList.empty())
   {
      AppMessageBox(m_hWnd, IDS_CDRIP_ERROR_NOCDINFO, MB_OK | MB_ICONSTOP);
      return;
   }

   // show dialog when it's more than one item
   unsigned int selectedIndex = 0;

   if (entriesList.size() > 1)
   {
      UI::FreeDbDiscListDlg dlg(entriesList);

      waitCursor.Restore();
      bool ret = IDOK == dlg.DoModal();
      waitCursor.Set();

      if (!ret)
         return;

      // select which one to take
      selectedIndex = dlg.GetSelectedItem();
   }

   if (selectedIndex < entriesList.size())
   {
      bool variousArtists = false;
      FillListFreedbInfo(entriesList[selectedIndex], variousArtists);

      m_checkVariousArtists.SetCheck(variousArtists ? BST_CHECKED : BST_UNCHECKED);
      UpdateVariousArtistsCheck();

      m_bAcquiredDiscInfo = true;
   }
}

void CDRipDlg::FillListFreedbInfo(const FreedbInfo& info, bool& variousArtists)
{
   CString cszText;
   unsigned int nMax = info.TrackTitles().size();
   for (unsigned int n = 0; n < nMax; n++)
   {
      cszText = info.TrackTitles()[n];
      m_lcTracks.SetItemText(n, 1, cszText);
   }

   SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, info.DiscArtist());

   SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, info.DiscTitle());

   if (!info.Year().IsEmpty())
      SetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, _ttoi(info.Year()), FALSE);

   CString cszGenre = info.Genre();
   if (cszGenre.GetLength() > 0)
   {
      int nItem = m_cbGenre.FindStringExact(-1, cszGenre);
      if (nItem == CB_ERR)
         nItem = m_cbGenre.AddString(cszGenre);

      m_cbGenre.SetCurSel(nItem);
   }

   CString discArtistLower = info.DiscArtist();
   discArtistLower.MakeLower();

   if (discArtistLower.Find(_T("various")) != -1)
      variousArtists = true;
}

CDRipTrackInfo CDRipDlg::ReadTrackInfo(DWORD driveIndex, unsigned int trackNum, const CDRipDiscInfo& discInfo)
{
   CDRipTrackInfo trackInfo;

   trackInfo.m_numTrackOnDisc = trackNum;

   CString entry;
   m_lcTracks.GetItemText(trackNum, 1, entry);

   // try to split text by "/" or "-"
   int pos = entry.Find(_T('/'));
   if (pos == -1)
      pos = entry.Find(_T('-'));

   if (pos == -1)
   {
      trackInfo.m_trackTitle = entry;
      trackInfo.m_trackArtist = discInfo.m_discArtist;
   }
   else
   {
      trackInfo.m_trackTitle = entry.Mid(pos + 1).Trim();
      trackInfo.m_trackArtist = entry.Left(pos).Trim();
   }

   trackInfo.m_trackLengthInSeconds = BASS_CD_GetTrackLength(driveIndex, trackNum) / 176400L;

   return trackInfo;
}


void CDRipDlg::RetrieveAlbumCoverArt(const std::string& discId)
{
   CWaitCursor waitCursor;

   std::string errorText;
   try
   {
      CString userAgent;
      userAgent.Format(_T("winLAME/%s ( https://winlame.sourceforge.io )"), App::Version().GetString());
      CoverArtArchive archive(CStringA(userAgent).GetString());

      std::vector<CoverArtResult> response = archive.Request(discId, true);

      if (!response.empty())
      {
         std::vector<unsigned char> imageData = response[0].FrontCover();

         if (CoverArtArchive::ImageFromJpegByteArray(imageData, m_coverArtImage))
         {
            SetFrontCoverArt(m_coverArtImage);

            m_covertArtImageData = imageData;
         }
      }
      else
      {
         AppMessageBox(m_hWnd, IDS_CDRIP_COVERART_ERROR_NOART, MB_OK | MB_ICONSTOP);
      }
   }
   catch (const std::exception& ex)
   {
      errorText = ex.what();
      ATLTRACE(_T("Error retrieving cover art: %hs"), errorText.c_str());

      AppMessageBox(m_hWnd, IDS_CDRIP_COVERART_ERROR_NOART, MB_OK | MB_ICONSTOP);
   }
}

void CDRipDlg::SetFrontCoverArt(const ATL::CImage& image)
{
   if (image.IsNull())
      return;

   m_staticAlbumArtImage.ShowWindow(SW_SHOW);

   m_staticAlbumArtImage.ModifyStyle(SS_BLACKFRAME, SS_BITMAP | SS_REALSIZECONTROL);
   m_staticAlbumArtImage.SetBitmap(image);
}
