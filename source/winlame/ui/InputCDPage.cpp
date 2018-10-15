//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
#include "ClassicModeStartPage.hpp"
#include <ulib/IoCContainer.hpp>
#include "UISettings.hpp"
#include <ulib/DynamicLibrary.hpp>
#include <ulib/win32/IniFile.hpp>
#include "basscd.h"
#include "CommonStuff.hpp"
#include "FreedbInfo.hpp"
#include "FreeDbDiscListDlg.hpp"
#include "RedrawLock.hpp"
#include <ulib/UTF8.hpp>
#include "CoverArtArchive.hpp"
#include "CoverArtDlg.hpp"
#include "App.hpp"

using namespace UI;

const DWORD INVALID_DRIVE_ID = 0xffffffff;

InputCDPage::InputCDPage(WizardPageHost& pageHost)
   :WizardPage(pageHost, IDD_PAGE_INPUT_CD,
      pageHost.IsClassicMode() ? WizardPage::typeCancelBackNext : WizardPage::typeCancelNext),
   m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
   m_pageWidth(0),
   m_editedTrack(false),
   m_driveActive(false),
   m_acquiredDiscInfo(false)
{
}

bool InputCDPage::IsCDExtractionAvail()
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
   std::vector<CString> genreList = Encoder::TrackInfo::GetGenreList();
   for (const CString& genre : genreList)
      m_comboGenre.AddString(genre);

   m_buttonPlay.EnableWindow(false);
   m_buttonStop.EnableWindow(false);

   m_editedTrack = false;

   PostMessage(WM_TIMER, IDT_CDRIP_CHECK);

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

LRESULT InputCDPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   KillTimer(IDT_CDRIP_CHECK);

   StoreSettings();

   if (m_pageHost.IsClassicMode())
      m_pageHost.SetWizardPage(std::make_shared<ClassicModeStartPage>(m_pageHost));

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

LRESULT InputCDPage::OnDriveSelEndOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   RefreshCDList();
   return 0;
}

LRESULT InputCDPage::OnListDoubleClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
   return OnClickedButtonPlay(0, 0, 0, bHandled);
}

LRESULT InputCDPage::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
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
   if (m_listTracks.GetItemState(nmListView.iItem, LVIS_SELECTED) != 0)
   {
      s_ignoreChangedMessage = true;

      for (int itemIndex = 0; itemIndex < m_listTracks.GetItemCount(); itemIndex++)
      {
         if (nmListView.iItem == itemIndex)
            continue; // ignore currently changed list item

         // only change selected lines
         if (m_listTracks.GetItemState(itemIndex, LVIS_SELECTED) != 0)
            m_listTracks.SetCheckState(itemIndex, isChecked);
      }

      s_ignoreChangedMessage = false;
   }

   return 0;
}

LRESULT InputCDPage::OnClickedButtonPlay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   DWORD driveIndex = GetCurrentDrive();
   if (driveIndex == INVALID_DRIVE_ID)
      return 0;

   int itemIndex = m_listTracks.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

   if (itemIndex < 0) itemIndex = 0; // no track selected? play first one

   DWORD trackIndex = m_listTracks.GetItemData(itemIndex);

   BASS_CD_Analog_Play(driveIndex, trackIndex, 0);

   m_buttonStop.EnableWindow(true);

   return 0;
}

LRESULT InputCDPage::OnClickedButtonStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   DWORD driveIndex = GetCurrentDrive();
   if (driveIndex == INVALID_DRIVE_ID)
      return 0;

   BASS_CD_Analog_Stop(driveIndex);

   m_buttonStop.EnableWindow(false);

   return 0;
}

LRESULT InputCDPage::OnClickedButtonFreedb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   FreedbLookup();

   return 0;
}

LRESULT InputCDPage::OnClickedButtonAlbumArt(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   DWORD driveIndex = GetCurrentDrive();

   const char* cdid = BASS_CD_GetID(driveIndex, BASS_CDID_MUSICBRAINZ);
   if (cdid == nullptr || strlen(cdid) == 0)
      return 0;

   RetrieveAlbumCoverArt(cdid);

   return 0;
}

LRESULT InputCDPage::OnClickedStaticAlbumArt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   UI::CoverArtDlg dlg(m_coverArtImage);
   dlg.DoModal(m_hWnd);
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
   UpdateVariousArtistsCheck();
   return 0;
}

LRESULT InputCDPage::OnChangedEditCtrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   m_editedTrack = true;

   return 0;
}

LRESULT InputCDPage::OnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   NMLVDISPINFO* pLvDispInfo = reinterpret_cast<NMLVDISPINFO*>(pnmh);
   if (pLvDispInfo->item.iItem == -1)
      return 0;

   m_editedTrack = true;

   return 1;
}

void InputCDPage::SetupDriveCombobox()
{
   unsigned int driveCount = 0;

   for (DWORD drive = 0; drive < 26; drive++)
   {
      BASS_CD_INFO info = { 0 };
      BOOL ret = BASS_CD_GetInfo(drive, &info);
      if (ret == FALSE)
         break;

      CString driveDescription(info.product);
      DWORD letter = info.letter;

      if (!driveDescription.IsEmpty())
      {
         CString text;

         if (letter == -1)
            text = driveDescription; // couldn't get drive letter; restricted user account
         else
            text.Format(_T("[%c:] %s"), static_cast<char>(_T('A') + letter), driveDescription.GetString());

         int itemIndex = m_comboDrives.AddString(text);
         m_comboDrives.SetItemData(itemIndex, drive);

         driveCount++;
      }
   }

   m_comboDrives.SetCurSel(0);

   // hide drive combobox when only one drive present
   if (driveCount <= 1)
      HideDriveCombobox();
}

void InputCDPage::HideDriveCombobox()
{
   CRect rcCombo, rcList;
   m_comboDrives.GetWindowRect(rcCombo);
   m_listTracks.GetWindowRect(rcList);
   rcList.top = rcCombo.top;
   ScreenToClient(rcList);
   m_listTracks.MoveWindow(rcList);

   m_comboDrives.ShowWindow(SW_HIDE);
   m_listTracks.SetFocus();
}

void InputCDPage::SetupTracksList()
{
   m_listTracks.SetExtendedListViewStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT, 0);

   CString text(MAKEINTRESOURCE(IDS_CDRIP_COLUMN_NR));
   m_listTracks.InsertColumn(0, text, LVCFMT_LEFT, 40, 0);
   text.LoadString(IDS_CDRIP_COLUMN_TRACK);
   m_listTracks.InsertColumn(1, text, LVCFMT_LEFT, 240, 0);
   text.LoadString(IDS_CDRIP_COLUMN_LENGTH);
   m_listTracks.InsertColumn(2, text, LVCFMT_LEFT, 60, 0);

   text.LoadString(IDS_CDRIP_UNKNOWN_ARTIST);
   SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, text);
   text.LoadString(IDS_CDRIP_UNKNOWN_TITLE);
   SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, text);
}

void InputCDPage::ResizeListCtrlColumns(int dx)
{
   int column1Width = m_listTracks.GetColumnWidth(1);
   column1Width += dx;

   m_listTracks.SetColumnWidth(1, column1Width);
}

DWORD InputCDPage::GetCurrentDrive()
{
   int nSel = m_comboDrives.GetCurSel();
   if (nSel == -1)
      return INVALID_DRIVE_ID;

   return m_comboDrives.GetItemData(nSel);
}

void InputCDPage::RefreshCDList()
{
   CWaitCursor waitCursor;

   RedrawLock lock(m_listTracks);
   m_listTracks.DeleteAllItems();

   m_buttonPlay.EnableWindow(false);

   DWORD driveIndex = GetCurrentDrive();
   if (driveIndex == INVALID_DRIVE_ID)
      return;

   m_editedTrack = false;

   if (FALSE == BASS_CD_IsReady(driveIndex))
   {
      m_driveActive = false;
      return;
   }

   m_driveActive = true;

   DWORD maxCDTracks = BASS_CD_GetTracks(driveIndex);
   if (maxCDTracks == DWORD(-1))
   {
      return;
   }

   for (DWORD n = 0; n < maxCDTracks; n++)
   {
      CString text;
      text.Format(_T("%lu"), n + 1);

      DWORD nLength = BASS_CD_GetTrackLength(driveIndex, n);
      bool bDataTrack = (nLength == 0xFFFFFFFF && BASS_ERROR_NOTAUDIO == BASS_ErrorGetCode());

      int nItem = m_listTracks.InsertItem(m_listTracks.GetItemCount(), text);
      m_listTracks.SetItemData(nItem, n);
      m_listTracks.SetCheckState(nItem, true);

      text.Format(IDS_CDRIP_TRACK_U, n + 1);
      m_listTracks.SetItemText(nItem, 1, text);

      if (!bDataTrack)
      {
         if (nLength != 0xFFFFFFFF)
         {
            nLength /= 176400;

            text.Format(_T("%lu:%02lu"), nLength / 60, nLength % 60);
            m_listTracks.SetItemText(nItem, 2, text);
         }
         else
         {
            text.LoadString(IDS_CDRIP_TRACKTYPE_UNKNOWN);
            m_listTracks.SetItemText(nItem, 2, text);
         }
      }
      else
      {
         text.LoadString(IDS_CDRIP_TRACKTYPE_DATA);
         m_listTracks.SetItemText(nItem, 2, text);
      }
   }

   if (maxCDTracks > 0)
      m_buttonPlay.EnableWindow(true);

   bool variousArtists = false;
   if (!ReadCachedCDDB(variousArtists))
      if (!ReadCdplayerIni(variousArtists))
         ReadCDText(variousArtists);

   // check or uncheck "various artists"
   m_checkVariousArtists.SetCheck(variousArtists ? BST_CHECKED : BST_UNCHECKED);
   UpdateVariousArtistsCheck();

   m_editedTrack = false;
}

bool InputCDPage::ReadCachedCDDB(bool& variousArtists)
{
   DWORD driveIndex = GetCurrentDrive();

   const char* entry = BASS_CD_GetID(driveIndex, BASS_CDID_CDDB_READ_CACHE);
   if (!entry || strlen(entry) == 0)
      return false;

   FreedbInfo info(UTF8ToString(entry));
   FillListFreedbInfo(info, variousArtists);

   m_acquiredDiscInfo = true;

   return true;
}

bool InputCDPage::ReadCdplayerIni(bool& variousArtists)
{
   // retrieve info from cdplayer.ini
   CString cdplayerIniFilename = Path::Combine(Path::WindowsFolder(), _T("cdplayer.ini")).ToString();

   DWORD driveIndex = GetCurrentDrive();

   DWORD maxCDTracks = BASS_CD_GetTracks(driveIndex);

   const char* cdplayer_id_raw = BASS_CD_GetID(driveIndex, BASS_CDID_CDPLAYER);

   if (cdplayer_id_raw == nullptr)
      return false;

   CString cdplayer_id(cdplayer_id_raw);

   IniFile ini(cdplayerIniFilename);

   unsigned int numTracks = ini.GetInt(cdplayer_id, _T("numtracks"), 0);
   if (numTracks == 0)
      return false;

   CString text;
   // title
   text = ini.GetString(cdplayer_id, _T("title"), _T("[]#"));

   if (text != _T("[]#"))
      SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, text);

   // artist
   text = ini.GetString(cdplayer_id, _T("artist"), _T("[]#"));

   if (text != _T("[]#"))
      SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, text);

   CString textLower(text);
   textLower.MakeLower();
   if (-1 != textLower.Find(_T("various")))
      variousArtists = true;

   // year
   text = ini.GetString(cdplayer_id, _T("year"), _T("[]#"));

   if (text != _T("[]#"))
      SetDlgItemText(IDC_CDSELECT_EDIT_YEAR, text);

   // genre
   text = ini.GetString(cdplayer_id, _T("genre"), _T("[]#"));

   if (text != _T("[]#"))
   {
      int nItem = m_comboGenre.FindStringExact(-1, text);
      if (nItem == CB_ERR)
         nItem = m_comboGenre.AddString(text);

      m_comboGenre.SetCurSel(nItem);
   }

   // limit to actual number of tracks in list
   if (numTracks > maxCDTracks)
      numTracks = maxCDTracks;

   // tracks
   CString numTrackText;
   for (unsigned int n = 0; n < numTracks; n++)
   {
      numTrackText.Format(_T("%u"), n);

      text = ini.GetString(cdplayer_id, numTrackText, _T("[]#"));

      if (text != _T("[]#"))
      {
         m_listTracks.SetItemText(n, 1, text);

         if (!variousArtists && text.Find(_T(" / ")) != -1)
            variousArtists = true;
      }
   }

   m_acquiredDiscInfo = true;

   return true;
}

void InputCDPage::ReadCDText(bool& variousArtists)
{
   DWORD driveIndex = GetCurrentDrive();

   DWORD maxCDTracks = BASS_CD_GetTracks(driveIndex);

   const CHAR* cdtext = BASS_CD_GetID(driveIndex, BASS_CDID_TEXT);
   if (cdtext != nullptr)
   {
      std::vector<CString> titlesList(maxCDTracks + 1);
      std::vector<CString> performerList(maxCDTracks + 1);

      const CHAR* endpos = cdtext;
      do
      {
         while (*endpos++ != 0);

         CString text(cdtext);
         if (cdtext == strstr(cdtext, "TITLE"))
         {
            LPSTR pNext = nullptr;
            unsigned long uTrack = strtoul(cdtext + 5, &pNext, 10);
            if (uTrack < maxCDTracks + 1)
            {
               titlesList[uTrack] = pNext + 1;
               titlesList[uTrack].Trim();
            }
         }
         if (cdtext == strstr(cdtext, "PERFORMER"))
         {
            LPSTR pNext = nullptr;
            unsigned long uPerf = strtoul(cdtext + 9, &pNext, 10);
            if (uPerf < maxCDTracks + 1)
            {
               performerList[uPerf] = pNext + 1;
               performerList[uPerf].Trim();
            }

            if (uPerf > 0 &&
               !performerList[uPerf].IsEmpty() &&
               performerList[0] != performerList[uPerf])
            {
               variousArtists = true;
            }
         }

         cdtext = endpos;
      } while (*endpos != 0);

      // set title and artist
      SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, titlesList[0]);
      SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, performerList[0]);

      CString format;
      for (DWORD n = 1; n < maxCDTracks + 1; n++)
      {
         if (performerList[n].IsEmpty() ||
            performerList[n] == performerList[0])
            format = titlesList[n];
         else
            format.Format(_T("%s / %s"), performerList[n].GetString(), titlesList[n].GetString());

         m_listTracks.SetItemText(n - 1, 1, format);
      }

      m_acquiredDiscInfo = true;
   }
}

void InputCDPage::CheckCD()
{
   DWORD driveIndex = GetCurrentDrive();
   if (driveIndex == INVALID_DRIVE_ID)
      return;

   // check if current track still plays
   bool bPlaying = BASS_ACTIVE_PLAYING == BASS_CD_Analog_IsActive(driveIndex);
   m_buttonStop.EnableWindow(bPlaying);

   // check for new cd in drive
   bool isReady = BASS_CD_IsReady(driveIndex) == TRUE;
   if (m_driveActive != isReady)
   {
      RefreshCDList();
   }
}

void InputCDPage::StoreSettings()
{
   DWORD driveIndex = GetCurrentDrive();
   if (driveIndex == INVALID_DRIVE_ID)
      return;

   if (m_editedTrack)
      StoreInCdplayerIni(driveIndex);

   UpdateCDReadJobList(driveIndex);

   UpdatePlaylistFilename(driveIndex);
}

void InputCDPage::UpdateVariousArtistsCheck()
{
   bool isChecked = BST_CHECKED != m_checkVariousArtists.GetCheck();

   GetDlgItem(IDC_CDSELECT_EDIT_ARTIST).EnableWindow(isChecked);
   GetDlgItem(IDC_CDSELECT_STATIC_ARTIST).EnableWindow(isChecked);
}

void InputCDPage::UpdateCDReadJobList(unsigned int driveIndex)
{
   CDRipDiscInfo discInfo = ReadDiscInfo(driveIndex);

   // get all track infos
   std::vector<DWORD> tracksList;

   for (int itemIndex = 0; itemIndex < m_listTracks.GetItemCount(); itemIndex++)
   {
      if (!m_listTracks.GetCheckState(itemIndex))
         continue;

      tracksList.push_back(m_listTracks.GetItemData(itemIndex));
   }

   unsigned int maxTrack = tracksList.size();

   for (unsigned int n = 0; n < maxTrack; n++)
   {
      unsigned int numTrack = tracksList[n];

      DWORD nLength = BASS_CD_GetTrackLength(driveIndex, numTrack);
      bool isDataTrack = (nLength == 0xFFFFFFFF && BASS_ERROR_NOTAUDIO == BASS_ErrorGetCode());
      if (isDataTrack)
         continue;

      CDRipTrackInfo trackInfo = ReadTrackInfo(driveIndex, numTrack, discInfo);

      Encoder::CDReadJob cdReadJob(discInfo, trackInfo);
      cdReadJob.FrontCoverArtImage(m_covertArtImageData);

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
         discInfo.m_discTitle.GetString());
   }
   else
   {
      m_uiSettings.playlist_filename.Format(
         _T("%s - %s.m3u"),
         discInfo.m_discArtist.GetString(), discInfo.m_discTitle.GetString());
   }
}

CDRipDiscInfo InputCDPage::ReadDiscInfo(DWORD driveIndex)
{
   CDRipDiscInfo discInfo;

   discInfo.m_discDrive = driveIndex;

   GetDlgItemText(IDC_CDSELECT_EDIT_TITLE, discInfo.m_discTitle);

   GetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, discInfo.m_discArtist);

   discInfo.m_year = GetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, NULL, FALSE);

   int nItem = m_comboGenre.GetCurSel();
   if (nItem == CB_ERR)
   {
      m_comboGenre.GetWindowText(discInfo.m_genre);
   }
   else
      m_comboGenre.GetLBText(nItem, discInfo.m_genre);

   discInfo.m_variousArtists = BST_CHECKED == m_checkVariousArtists.GetCheck();

   discInfo.m_CDID = BASS_CD_GetID(driveIndex, BASS_CDID_CDDB);

   discInfo.m_numTracks = BASS_CD_GetTracks(driveIndex);
   if (discInfo.m_numTracks == (DWORD)-1)
      discInfo.m_numTracks = 0;

   return discInfo;
}

CDRipTrackInfo InputCDPage::ReadTrackInfo(DWORD driveIndex, unsigned int trackNum, const CDRipDiscInfo& discInfo)
{
   CDRipTrackInfo trackInfo;

   trackInfo.m_numTrackOnDisc = trackNum;

   CString entry;
   m_listTracks.GetItemText(trackNum, 1, entry);

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

void InputCDPage::StoreInCdplayerIni(unsigned int driveIndex)
{
   if (!m_uiSettings.store_disc_infos_cdplayer_ini)
      return;

   CString cdplayerIniFilename = Path::Combine(Path::WindowsFolder(), _T("cdplayer.ini")).ToString();

   const char* cdplayer_id_raw = BASS_CD_GetID(driveIndex, BASS_CDID_CDPLAYER);

   CString cdplayer_id(cdplayer_id_raw);

   CDRipDiscInfo discinfo = ReadDiscInfo(driveIndex);

   IniFile ini(cdplayerIniFilename);

   // numtracks
   unsigned int numTracks = m_listTracks.GetItemCount();
   CString format;
   format.Format(_T("%u"), numTracks);
   ini.WriteString(cdplayer_id, _T("numtracks"), format);

   // artist
   ini.WriteString(cdplayer_id, _T("artist"), discinfo.m_discArtist);

   // title
   ini.WriteString(cdplayer_id, _T("title"), discinfo.m_discTitle);

   // year
   if (discinfo.m_year > 0)
   {
      format.Format(_T("%u"), discinfo.m_year);
      ini.WriteString(cdplayer_id, _T("year"), format);
   }

   // genre
   ini.WriteString(cdplayer_id, _T("genre"), discinfo.m_genre);

   // tracks
   for (unsigned int trackIndex = 0; trackIndex < numTracks; trackIndex++)
   {
      format.Format(_T("%u"), trackIndex);

      CDRipTrackInfo trackInfo = ReadTrackInfo(driveIndex, trackIndex, discinfo);

      ini.WriteString(cdplayer_id, format, trackInfo.m_trackTitle);
   }
}

void InputCDPage::FreedbLookup()
{
   CString serverAddress = m_uiSettings.freedb_server;

   BOOL retConfig = BASS_SetConfigPtr(
      BASS_CONFIG_CD_CDDB_SERVER,
      CStringA(serverAddress).GetString());
   ATLASSERT(TRUE == retConfig);
   UNUSED(retConfig);

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
      AtlMessageBox(m_hWnd, IDS_CDRIP_ERROR_NOCDINFO, IDS_APP_CAPTION, MB_OK | MB_ICONSTOP);
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

      m_acquiredDiscInfo = true;
   }
}

void InputCDPage::FillListFreedbInfo(const FreedbInfo& info, bool& variousArtists)
{
   CString text;
   unsigned int maxIndex = info.TrackTitles().size();
   for (unsigned int index = 0; index < maxIndex; index++)
   {
      text = info.TrackTitles()[index];
      m_listTracks.SetItemText(index, 1, text);
   }

   SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, info.DiscArtist());

   SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, info.DiscTitle());

   if (!info.Year().IsEmpty())
      SetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, _ttoi(info.Year()), FALSE);

   CString genre = info.Genre();
   if (!genre.IsEmpty())
   {
      int nItem = m_comboGenre.FindStringExact(-1, genre);
      if (nItem == CB_ERR)
         nItem = m_comboGenre.AddString(genre);

      m_comboGenre.SetCurSel(nItem);
   }

   CString discArtistLower = info.DiscArtist();
   discArtistLower.MakeLower();

   if (discArtistLower.Find(_T("various")) != -1)
      variousArtists = true;
}

void InputCDPage::RetrieveAlbumCoverArt(const std::string& discId)
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
         AtlMessageBox(m_hWnd, IDS_CDRIP_COVERART_ERROR_NOART, IDS_APP_CAPTION, MB_OK | MB_ICONSTOP);
      }
   }
   // NOSONAR
   catch (const std::exception& ex)
   {
      errorText = ex.what();
      ATLTRACE(_T("Error retrieving cover art: %hs"), errorText.c_str());

      AtlMessageBox(m_hWnd, IDS_CDRIP_COVERART_ERROR_NOART, IDS_APP_CAPTION, MB_OK | MB_ICONSTOP);
   }
}

void InputCDPage::SetFrontCoverArt(const ATL::CImage& image)
{
   if (image.IsNull())
      return;

   m_staticAlbumArtImage.ShowWindow(SW_SHOW);

   m_staticAlbumArtImage.ModifyStyle(SS_BLACKFRAME, SS_BITMAP | SS_REALSIZECONTROL);
   m_staticAlbumArtImage.SetBitmap(image);
}
