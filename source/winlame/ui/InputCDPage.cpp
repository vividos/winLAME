//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file InputCDPage.cpp
/// \brief Input CD page
//
#include "StdAfx.h"
#include "InputCDPage.hpp"
#include "WizardPageHost.hpp"
#include "OutputSettingsPage.hpp"
#include "CDReadSettingsPage.hpp"
#include "IoCContainer.hpp"
#include "UISettings.h"
#include "DynamicLibrary.h"
#include "IniFile.h"
#include "basscd.h"
#include "CommonStuff.h"
#include "FreedbInfo.hpp"
#include "FreeDbDiscListDlg.hpp"
#include "RedrawLock.hpp"
#include "UTF8.hpp"

using namespace UI;

const DWORD INVALID_DRIVE_ID = 0xffffffff;

InputCDPage::InputCDPage(WizardPageHost& pageHost) throw()
   :WizardPage(pageHost, IDD_PAGE_INPUT_CD, WizardPage::typeCancelNext),
   m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
   m_bEditedTrack(false),
   m_bDriveActive(false),
   m_bAcquiredDiscInfo(false)
{
}

bool InputCDPage::IsCDExtractionAvail() throw()
{
   return DynamicLibrary(_T("basscd.dll")).IsLoaded();
}

LRESULT InputCDPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   // enable resizing
   CRect rectPage;
   GetClientRect(&rectPage);
   m_pageWidth = rectPage.Width();

   DlgResize_Init(false, false);

   m_uiSettings.cdreadjoblist.clear();

   SetupDriveCombobox();

   SetupTracksList();

   // genre combobox
   LPCTSTR* apszGenre = Encoder::TrackInfo::GetGenreList();
   for (unsigned int i = 0, iMax = Encoder::TrackInfo::GetGenreListLength(); i < iMax; i++)
      m_cbGenre.AddString(apszGenre[i]);

   m_buttonPlay.EnableWindow(false);
   m_buttonStop.EnableWindow(false);

   m_bEditedTrack = false;

   CheckCD();

   SetTimer(IDT_CDRIP_CHECK, 2 * 1000);

   return 1;
}

LRESULT InputCDPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   KillTimer(IDT_CDRIP_CHECK);

   StoreSettings();

   m_uiSettings.m_bFromInputFilesPage = false;
   m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

LRESULT InputCDPage::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if (wParam == IDT_CDRIP_CHECK)
      CheckCD();
   return 0;
}

LRESULT InputCDPage::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   bHandled = false;

   // resize list view columns
   if (wParam != SIZE_MINIMIZED)
   {
      int dx = GET_X_LPARAM(lParam) - m_pageWidth;

      ResizeListCtrlColumns(dx);

      m_pageWidth += dx;
   }

   return 0;
}

LRESULT InputCDPage::OnDriveSelEndOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)//(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
   RefreshCDList();
   return 0;
}

LRESULT InputCDPage::OnListDoubleClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
   return OnClickedButtonPlay(0, 0, 0, bHandled);
}

LRESULT InputCDPage::OnClickedButtonPlay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   DWORD nDrive = GetCurrentDrive();
   if (nDrive == INVALID_DRIVE_ID)
      return 0;

   int nItem = m_lcTracks.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

   if (nItem < 0) nItem = 0; // no track selected? play first one

   DWORD nTrack = m_lcTracks.GetItemData(nItem);

   BASS_CD_Analog_Play(nDrive, nTrack, 0);

   m_buttonStop.EnableWindow(true);

   return 0;
}

LRESULT InputCDPage::OnClickedButtonStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   DWORD nDrive = GetCurrentDrive();
   if (nDrive == INVALID_DRIVE_ID)
      return 0;

   BASS_CD_Analog_Stop(nDrive);

   m_buttonStop.EnableWindow(false);

   return 0;
}

LRESULT InputCDPage::OnClickedButtonFreedb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   FreedbLookup();

   return 0;
}

LRESULT InputCDPage::OnClickedButtonOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   WizardPageHost host;
   host.SetWizardPage(std::shared_ptr<WizardPage>(
      new CDReadSettingsPage(host, m_uiSettings)));
   host.Run(m_hWnd);

   return 0;
}

LRESULT InputCDPage::OnClickedCheckVariousArtists(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   bool bCheck = BST_CHECKED != m_checkVariousArtists.GetCheck();

   GetDlgItem(IDC_CDSELECT_EDIT_ARTIST).EnableWindow(bCheck);
   GetDlgItem(IDC_CDSELECT_STATIC_ARTIST).EnableWindow(bCheck);

   return 0;
}

LRESULT InputCDPage::OnChangedEditCtrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   m_bEditedTrack = true;

   return 0;
}

LRESULT InputCDPage::OnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   NMLVDISPINFO* pLvDispInfo = reinterpret_cast<NMLVDISPINFO*>(pnmh);
   if (pLvDispInfo->item.iItem == -1)
      return 0;

   m_bEditedTrack = true;

   return 1;
}
void InputCDPage::SetupDriveCombobox()
{
   unsigned int nDriveCount = 0;

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
            cszText.Format(_T("[%c:] %s"), _T('A') + nLetter, cszDriveDescription);

         int nItem = m_cbDrives.AddString(cszText);
         m_cbDrives.SetItemData(nItem, n);

         nDriveCount++;
      }
   }

   m_cbDrives.SetCurSel(0);

   // hide drive combobox when only one drive present
   if (nDriveCount <= 1)
      HideDriveCombobox();
}

void InputCDPage::HideDriveCombobox()
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

void InputCDPage::SetupTracksList()
{
   // tracks list
   m_lcTracks.SetExtendedListViewStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT,
      LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT);

   CString cszText(MAKEINTRESOURCE(IDS_CDRIP_COLUMN_NR));
   m_lcTracks.InsertColumn(0, cszText, LVCFMT_LEFT, 30, 0);
   cszText.LoadString(IDS_CDRIP_COLUMN_TRACK);
   m_lcTracks.InsertColumn(1, cszText, LVCFMT_LEFT, 250, 0);
   cszText.LoadString(IDS_CDRIP_COLUMN_LENGTH);
   m_lcTracks.InsertColumn(2, cszText, LVCFMT_LEFT, 60, 0);

   cszText.LoadString(IDS_CDRIP_UNKNOWN_ARTIST);
   SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, cszText);
   cszText.LoadString(IDS_CDRIP_UNKNOWN_TITLE);
   SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, cszText);
}

void InputCDPage::ResizeListCtrlColumns(int dx)
{
   int column1Width = m_lcTracks.GetColumnWidth(1);
   column1Width += dx;

   m_lcTracks.SetColumnWidth(1, column1Width);
}

DWORD InputCDPage::GetCurrentDrive()
{
   int nSel = m_cbDrives.GetCurSel();
   if (nSel == -1)
      return INVALID_DRIVE_ID;

   return m_cbDrives.GetItemData(nSel);
}

void InputCDPage::RefreshCDList()
{
   CWaitCursor waitCursor;

   RedrawLock lock(m_lcTracks);
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
      cszText.Format(_T("%u"), n + 1);

      DWORD nLength = BASS_CD_GetTrackLength(nDrive, n);
      bool bDataTrack = (nLength == 0xFFFFFFFF && BASS_ERROR_NOTAUDIO == BASS_ErrorGetCode());

      int nItem = m_lcTracks.InsertItem(m_lcTracks.GetItemCount(), cszText);
      m_lcTracks.SetItemData(nItem, n);

      cszText.Format(IDS_CDRIP_TRACK_U, n + 1);
      m_lcTracks.SetItemText(nItem, 1, cszText);

      if (!bDataTrack)
      {
         if (nLength != 0xFFFFFFFF)
         {
            nLength /= 176400;

            cszText.Format(_T("%u:%02u"), nLength / 60, nLength % 60);
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

   BOOL bDummy = true;
   OnClickedCheckVariousArtists(0, 0, NULL, bDummy);

   m_bEditedTrack = false;
}

bool InputCDPage::ReadCachedCDDB(bool& bVarious)
{
   DWORD driveIndex = GetCurrentDrive();

   const char* entry = BASS_CD_GetID(driveIndex, BASS_CDID_CDDB_READ_CACHE);
   if (!entry || strlen(entry) == 0)
      return false;

   FreedbInfo info(UTF8ToString(entry));
   FillListFreedbInfo(info);
   m_bAcquiredDiscInfo = true;

   return true;
}

bool InputCDPage::ReadCdplayerIni(bool& bVarious)
{
   // retrieve info from cdplayer.ini
   CString cszCDPlayerIniFilename;
   ::GetWindowsDirectory(cszCDPlayerIniFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszCDPlayerIniFilename.ReleaseBuffer();
   cszCDPlayerIniFilename += _T("\\cdplayer.ini");

   DWORD nDrive = GetCurrentDrive();

   DWORD uMaxCDTracks = BASS_CD_GetTracks(nDrive);

   const char* cdplayer_id_raw = BASS_CD_GetID(nDrive, BASS_CDID_CDPLAYER);

   CString cdplayer_id(cdplayer_id_raw);

   IniFile ini(cszCDPlayerIniFilename);

   unsigned int nNumTracks = 0;
   if (cdplayer_id_raw != NULL &&
      0 != (nNumTracks = ini.GetInt(cdplayer_id, _T("numtracks"), 0)))
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

      if (-1 != cszText.Find(_T("various")))
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

   return false;
}

void InputCDPage::ReadCDText(bool& bVarious)
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
               vecTitles[uTrack] = pNext + 1;
         }
         if (cdtext == strstr(cdtext, "PERFORMER"))
         {
            LPSTR pNext = NULL;
            unsigned long uPerf = strtoul(cdtext + 9, &pNext, 10);
            if (uPerf < uMaxCDTracks + 1)
               vecPerformer[uPerf] = pNext + 1;

            if (uPerf > 0 &&
               strlen(pNext + 1) > 0 &&
               vecPerformer[0] != CString(pNext + 1))
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
            cszFormat.Format(_T("%s / %s"), vecPerformer[n], vecTitles[n]);

         m_lcTracks.SetItemText(n - 1, 1, cszFormat);
      }

      m_bAcquiredDiscInfo = true;
   }
}

void InputCDPage::CheckCD()
{
   DWORD dwDrive = GetCurrentDrive();
   if (dwDrive == INVALID_DRIVE_ID)
      return;

   // check if current track still plays
   bool bPlaying = BASS_ACTIVE_PLAYING == BASS_CD_Analog_IsActive(dwDrive);
   m_buttonStop.EnableWindow(bPlaying);

   // check for new cd in drive
   bool bIsReady = BASS_CD_IsReady(dwDrive) == TRUE;
   if (m_bDriveActive != bIsReady)
   {
      RefreshCDList();
   }
}

void InputCDPage::StoreSettings()
{
   DWORD dwDrive = GetCurrentDrive();
   if (dwDrive == INVALID_DRIVE_ID)
      return;

   if (m_bEditedTrack)
      StoreInCdplayerIni(dwDrive);

   UpdateCDReadJobList(dwDrive);

   UpdatePlaylistFilename(dwDrive);
}

void InputCDPage::UpdateCDReadJobList(unsigned int dwDrive)
{
   CDRipDiscInfo discInfo = ReadDiscInfo(dwDrive);

   // get all track infos
   std::vector<DWORD> vecTracks;
   {
      int nItem = m_lcTracks.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

      if (nItem >= 0)
      {
         // add all selected items
         do
         {
            vecTracks.push_back(m_lcTracks.GetItemData(nItem));

         } while (-1 != (nItem = m_lcTracks.GetNextItem(nItem, LVNI_ALL | LVNI_SELECTED)));
      }
      else
      {
         // add all items
         int nMax = m_lcTracks.GetItemCount();
         for (int n = 0; n < nMax; n++)
            vecTracks.push_back(m_lcTracks.GetItemData(n));
      }
   }

   unsigned int nMax = vecTracks.size();

   for (unsigned int n = 0; n < nMax; n++)
   {
      unsigned int nTrack = vecTracks[n];

      CDRipTrackInfo trackInfo = ReadTrackInfo(dwDrive, nTrack);

      Encoder::CDReadJob cdReadJob(discInfo, trackInfo);
      m_uiSettings.cdreadjoblist.push_back(cdReadJob);
   }
}

void InputCDPage::UpdatePlaylistFilename(DWORD driveIndex)
{
   CDRipDiscInfo discInfo = ReadDiscInfo(driveIndex);

   if (discInfo.m_variousArtists)
   {
      m_uiSettings.playlist_filename.Format(
         _T("%s.m3u"),
         discInfo.m_discTitle);
   }
   else
   {
      m_uiSettings.playlist_filename.Format(
         _T("%s - %s.m3u"),
         discInfo.m_discArtist, discInfo.m_discTitle);
   }
}

CDRipDiscInfo InputCDPage::ReadDiscInfo(DWORD driveIndex)
{
   CDRipDiscInfo discInfo;

   discInfo.m_discDrive = driveIndex;

   GetDlgItemText(IDC_CDSELECT_EDIT_TITLE, discInfo.m_discTitle);

   GetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, discInfo.m_discArtist);

   discInfo.m_year = GetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, NULL, FALSE);

   int nItem = m_cbGenre.GetCurSel();
   if (nItem == CB_ERR)
   {
      m_cbGenre.GetWindowText(discInfo.m_genre);
   }
   else
      m_cbGenre.GetLBText(nItem, discInfo.m_genre);

   discInfo.m_variousArtists = BST_CHECKED == m_checkVariousArtists.GetCheck();

   discInfo.m_CDID = BASS_CD_GetID(driveIndex, BASS_CDID_CDDB);

   return discInfo;
}

CDRipTrackInfo InputCDPage::ReadTrackInfo(DWORD driveIndex, unsigned int trackNum)
{
   CDRipTrackInfo trackInfo;

   trackInfo.m_numTrackOnDisc = trackNum;
   m_lcTracks.GetItemText(trackNum, 1, trackInfo.m_trackTitle);
   trackInfo.m_trackLengthInSeconds = BASS_CD_GetTrackLength(driveIndex, trackNum) / 176400L;

   return trackInfo;
}

void InputCDPage::StoreInCdplayerIni(unsigned int nDrive)
{
   if (!m_uiSettings.store_disc_infos_cdplayer_ini)
      return;

   DWORD dwDrive = GetCurrentDrive();
   if (dwDrive == INVALID_DRIVE_ID)
      return;

   CString cszCDPlayerIniFilename;
   ::GetWindowsDirectory(cszCDPlayerIniFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszCDPlayerIniFilename.ReleaseBuffer();
   cszCDPlayerIniFilename += _T("\\cdplayer.ini");

   const char* cdplayer_id_raw = BASS_CD_GetID(nDrive, BASS_CDID_CDPLAYER);

   CString cdplayer_id(cdplayer_id_raw);

   CDRipDiscInfo discinfo = ReadDiscInfo(dwDrive);

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

      CDRipTrackInfo trackInfo = ReadTrackInfo(dwDrive, n);

      ini.WriteString(cdplayer_id, cszFormat, trackInfo.m_trackTitle);
   }
}

void InputCDPage::FreedbLookup()
{
   CString serverAddress = m_uiSettings.freedb_server;

   ATLVERIFY(TRUE == BASS_SetConfigPtr(
      BASS_CONFIG_CD_CDDB_SERVER,
      CStringA(serverAddress).GetString()));

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
   unsigned int nSelected = 0;

   if (entriesList.size() > 1)
   {
      UI::FreeDbDiscListDlg dlg(entriesList);

      waitCursor.Restore();
      ATLVERIFY(IDOK == dlg.DoModal());
      waitCursor.Set();

      // select which one to take
      nSelected = dlg.GetSelectedItem();
   }

   if (nSelected >= 0 && nSelected < entriesList.size())
   {
      FillListFreedbInfo(entriesList[nSelected]);
      m_bAcquiredDiscInfo = true;
   }
}

void InputCDPage::FillListFreedbInfo(const FreedbInfo& info)
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
   if (!cszGenre.IsEmpty())
   {
      int nItem = m_cbGenre.FindStringExact(-1, cszGenre);
      if (nItem == CB_ERR)
         nItem = m_cbGenre.AddString(cszGenre);

      m_cbGenre.SetCurSel(nItem);
   }
}
