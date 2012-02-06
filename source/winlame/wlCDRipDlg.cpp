/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005-2010 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: wlCDRipDlg.cpp,v 1.21 2010/01/08 18:27:28 vividos Exp $

*/
/*! \file wlCDRipDlg.cpp

   \brief contains the dialog for cd ripping

*/

// needed includes
#include "stdafx.h"
#include "wlCDRipDlg.h"
#include "wlCDRipTrackManager.h"
#include "OptionsDlg.h"
#include "basscd.h"
#include "encoder/wlTrackInfo.h"
#include <shellapi.h>
#include <atlctrlx.h> // CWaitCursor

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// wlCDRipFreedbListDlg methods

LRESULT wlCDRipFreedbListDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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

   m_list.SubclassWindow(GetDlgItem(IDC_FREEDB_LIST_TRACKS));

   m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

   m_list.InsertColumn(0, _T("Album Name"), LVCFMT_LEFT, 200, 0);
   m_list.InsertColumn(1, _T("Genre"), LVCFMT_LEFT, 50, 0);

   m_list.SelectItem(0);

   UpdateList();

   return TRUE;
}

void wlCDRipFreedbListDlg::UpdateList()
{
   CString cszText;
   unsigned int nMax = results.size();
   for(unsigned int n=0; n<nMax; n++)
   {
      cszText.Format(_T("%hs / %hs"), results[n].dartist.c_str(), results[n].dtitle.c_str());
      cszText.Replace(_T("\n"), _T("")); // libfreedb may add a linefeed character

      int nItem = m_list.InsertItem(m_list.GetItemCount(), cszText);
      cszText = results[n].category.c_str();
      m_list.SetItemText(nItem, 1, cszText);
   }
}

LRESULT wlCDRipFreedbListDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   int iItem = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
   if (iItem != -1)
   {
      EndDialog(wID);
      m_uSelectedItem = static_cast<unsigned int>(iItem);
   }

   return 0;
}


// wlCDRipDlg methods

wlCDRipDlg::wlCDRipDlg(wlUISettings& uiSettings, wlUIinterface& UIinterface)
:m_uiSettings(uiSettings),
 m_bDriveActive(false),
 m_bAcquiredDiscInfo(false),
 m_bEditedTrack(false),
 m_UIinterface(UIinterface)
{
}

bool wlCDRipDlg::IsCDExtractionAvail() throw()
{
   HMODULE hMod = ::LoadLibrary(_T("basscd.dll"));
   bool bAvail = hMod != NULL;
   FreeLibrary(hMod);

   return bAvail;
}

LRESULT wlCDRipDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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
      for (DWORD n=0; n<26; n++)
      {
         BASS_CD_INFO info = {0};
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

   // genre combobox
   LPCTSTR* apszGenre = wlTrackInfo::GetGenreList();
   for (unsigned int i=0, iMax=wlTrackInfo::GetGenreListLength(); i<iMax; i++)
      m_cbGenre.AddString(apszGenre[i]);

   // misc.
   m_cbDrives.SetCurSel(0);

   m_bEditedTrack = false;

   CheckCD();

   SetTimer(IDT_CDRIP_CHECK, 2*1000);

   DlgResize_Init(true, true);

   return 1;
}

LRESULT wlCDRipDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   if (wParam == IDT_CDRIP_CHECK)
      CheckCD();
   return 0;
}

LRESULT wlCDRipDlg::OnDriveSelEndOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)//(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
   RefreshCDList();
   return 0;
}

LRESULT wlCDRipDlg::OnListDoubleClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
   return OnClickedButtonPlay(0, 0, 0, bHandled);
}

LRESULT wlCDRipDlg::OnClickedButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   DWORD nDrive = GetCurrentDrive();

   int nItem = m_lcTracks.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

   if (nItem < 0) nItem = 0; // no track selected? play first one

   DWORD nTrack = m_lcTracks.GetItemData(nItem);

   BASS_CD_Analog_Play(nDrive, nTrack, 0);
   ::EnableWindow(GetDlgItem(IDC_CDSELECT_BUTTON_STOP), TRUE);

   return 0;
}

LRESULT wlCDRipDlg::OnClickedButtonStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   BASS_CD_Analog_Stop(GetCurrentDrive());

   ::EnableWindow(GetDlgItem(IDC_CDSELECT_BUTTON_STOP), FALSE);

   return 0;
}

LRESULT wlCDRipDlg::OnClickedButtonFreedb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   FreedbLookup();
   return 0;
}

LRESULT wlCDRipDlg::OnClickedButtonOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   COptionsDlg dlg(m_uiSettings, m_UIinterface.GetLanguageResourceManager());
   dlg.DoModal(m_hWnd);
   return 0;
}

LRESULT wlCDRipDlg::OnClickedCheckVariousArtists(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   bool bCheck = BST_CHECKED == SendDlgItemMessage(IDC_CDSELECT_CHECK_VARIOUS_ARTISTS, BM_GETCHECK);

   ::EnableWindow(GetDlgItem(IDC_CDSELECT_EDIT_ARTIST), bCheck ? FALSE : TRUE);
   ::EnableWindow(GetDlgItem(IDC_CDSELECT_STATIC_ARTIST), bCheck ? FALSE : TRUE);

   return 0;
}

LRESULT wlCDRipDlg::OnChangedEditCtrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   m_bEditedTrack = true;

   return 0;
}

DWORD wlCDRipDlg::GetCurrentDrive()
{
   int nSel = m_cbDrives.GetCurSel();
   ATLASSERT(nSel != -1);
   return m_cbDrives.GetItemData(nSel);
}

void wlCDRipDlg::RefreshCDList()
{
   CWaitCursor waitCursor;

   DWORD nDrive = GetCurrentDrive();

   m_lcTracks.SetRedraw(FALSE);
   m_lcTracks.DeleteAllItems();

   m_bEditedTrack = false;

   if (FALSE == BASS_CD_IsReady(nDrive))
   {
      m_bDriveActive = false;
      m_lcTracks.SetRedraw(TRUE);
      return;
   }

   m_bDriveActive = true;

   DWORD uMaxCDTracks = BASS_CD_GetTracks(nDrive);
   if (uMaxCDTracks == DWORD(-1))
   {
      m_lcTracks.SetRedraw(TRUE);
      return;
   }

   for(DWORD n=0; n<uMaxCDTracks; n++)
   {
      CString cszText;
      cszText.Format(_T("%u"), n+1);

      DWORD nLength = BASS_CD_GetTrackLength(nDrive, n);
      bool bDataTrack = (nLength == 0xFFFFFFFF && BASS_ERROR_NOTAUDIO == BASS_ErrorGetCode());

      int nItem = m_lcTracks.InsertItem(m_lcTracks.GetItemCount(), cszText);
      m_lcTracks.SetItemData(nItem, n);

      cszText.Format(IDS_CDRIP_TRACK_U, n+1);
      m_lcTracks.SetItemText(nItem, 1, cszText);

      if (!bDataTrack)
      {
         if (nLength != 0xFFFFFFFF)
         {
            nLength /= 176400;

            cszText.Format(_T("%u:%02u"), nLength/60, nLength%60);
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

   // retrieve info from cdplayer.ini
   CString cszCDPlayerIniFilename;
   ::GetWindowsDirectory(cszCDPlayerIniFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszCDPlayerIniFilename.ReleaseBuffer();
   cszCDPlayerIniFilename += _T("\\cdplayer.ini");

   bool bVarious = false;

   const char* cdplayer_id_raw = BASS_CD_GetID(nDrive, BASS_CDID_CDPLAYER);

   USES_CONVERSION;
   const TCHAR* cdplayer_id = A2CT(cdplayer_id_raw);

   unsigned int nNumTracks = 0;
   if (cdplayer_id_raw != NULL && 0 != (nNumTracks = ::GetPrivateProfileInt(cdplayer_id, _T("numtracks"), 0, cszCDPlayerIniFilename)))
   {
      CString cszText;
      // title
      ::GetPrivateProfileString(cdplayer_id, _T("title"), _T("[]#"), cszText.GetBuffer(512), 512, cszCDPlayerIniFilename);
      cszText.ReleaseBuffer();

      if (cszText != _T("[]#"))
         SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, cszText);

      // artist
      ::GetPrivateProfileString(cdplayer_id, _T("artist"), _T("[]#"), cszText.GetBuffer(512), 512, cszCDPlayerIniFilename);
      cszText.ReleaseBuffer();

      if (cszText != _T("[]#"))
         SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, cszText);

      if (-1 != cszText.Find(_T("various")))
         bVarious = true;

      // year
      ::GetPrivateProfileString(cdplayer_id, _T("year"), _T("[]#"), cszText.GetBuffer(512), 512, cszCDPlayerIniFilename);
      cszText.ReleaseBuffer();

      if (cszText != _T("[]#"))
         SetDlgItemText(IDC_CDSELECT_EDIT_YEAR, cszText);

      // genre
      ::GetPrivateProfileString(cdplayer_id, _T("genre"), _T("[]#"), cszText.GetBuffer(512), 512, cszCDPlayerIniFilename);
      cszText.ReleaseBuffer();

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
      for(unsigned int n=0; n<nNumTracks; n++)
      {
         cszNumTrack.Format(_T("%u"), n);

         ::GetPrivateProfileString(cdplayer_id, cszNumTrack, _T("[]#"), cszText.GetBuffer(512), 512, cszCDPlayerIniFilename);
         cszText.ReleaseBuffer();

         if (cszText != _T("[]#"))
         {
            m_lcTracks.SetItemText(n, 1, cszText);

            if (!bVarious && cszText.Find(_T(" / ")) != -1)
               bVarious = true;
         }
      }

      m_bAcquiredDiscInfo = true;
   }
   // retrieve CD-Text
   else
   {
      const CHAR* cdtext = BASS_CD_GetID(nDrive, BASS_CDID_TEXT);
      if (cdtext != NULL)
      {
         USES_CONVERSION;

         std::vector<CString> vecTitles(uMaxCDTracks+1);
         std::vector<CString> vecPerformer(uMaxCDTracks+1);

         CString cszOutput;
         const CHAR* endpos = cdtext;
         do
         {
            while(*endpos++ != 0);

            CString cszText(cdtext);
            if (cdtext == strstr(cdtext, "TITLE"))
            {
               LPSTR pNext = NULL;
               unsigned long uTrack = strtoul(cdtext+5, &pNext, 10);
               if (uTrack < uMaxCDTracks+1)
                  vecTitles[uTrack] = pNext+1;
            }
            if (cdtext == strstr(cdtext, "PERFORMER"))
            {
               LPSTR pNext = NULL;
               unsigned long uPerf = strtoul(cdtext+9, &pNext, 10);
               if (uPerf < uMaxCDTracks+1)
                  vecPerformer[uPerf] = pNext+1;

               if (uPerf > 0 && strlen(pNext+1) > 0)
                  bVarious = true;
            }

            cdtext = endpos;
         }
         while(*endpos != 0);

         // set title and artist
         SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, vecTitles[0]);
         SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, vecPerformer[0]);

         CString cszFormat;
         for (DWORD n=1; n<uMaxCDTracks+1; n++)
         {
            if (vecPerformer[n].GetLength()==0)
               cszFormat = vecTitles[n];
            else
               cszFormat.Format(_T("%s / %s"), vecPerformer[n], vecTitles[n]);

            m_lcTracks.SetItemText(n-1, 1, cszFormat);
         }

         m_bAcquiredDiscInfo = true;
      }
   }

   // check or uncheck "various artists"
   SendDlgItemMessage(IDC_CDSELECT_CHECK_VARIOUS_ARTISTS, BM_SETCHECK,
      bVarious ? BST_CHECKED : BST_UNCHECKED, 0);

   BOOL bDummy = true;
   OnClickedCheckVariousArtists(0, 0, NULL, bDummy);

   m_bEditedTrack = false;

   m_lcTracks.SetRedraw(TRUE);
}

void wlCDRipDlg::CheckCD()
{
   // check if current track still plays
   BOOL fPlaying = BASS_ACTIVE_PLAYING == BASS_CD_Analog_IsActive(GetCurrentDrive()) ? TRUE : FALSE;
   ::EnableWindow(GetDlgItem(IDC_CDSELECT_BUTTON_STOP), fPlaying);

   // check for new cd in drive
   DWORD nDrive = GetCurrentDrive();

   bool bIsReady = BASS_CD_IsReady(nDrive) == TRUE;
   if (m_bDriveActive != bIsReady)
   {
      RefreshCDList();
   }
}

void wlCDRipDlg::UpdateTrackManager()
{
   wlCDRipTrackManager* pManager = wlCDRipTrackManager::getCDRipTrackManager();

   DWORD nDrive = GetCurrentDrive();

   wlCDRipDiscInfo& discinfo = pManager->GetDiscInfo();
   discinfo.m_nDiscDrive = nDrive;

   GetDlgItemText(IDC_CDSELECT_EDIT_TITLE, discinfo.m_cszDiscTitle.GetBuffer(256), 256);
   discinfo.m_cszDiscTitle.ReleaseBuffer();

   GetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, discinfo.m_cszDiscArtist.GetBuffer(256), 256);
   discinfo.m_cszDiscArtist.ReleaseBuffer();

   discinfo.m_nYear = GetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, NULL, FALSE);

   int nItem = m_cbGenre.GetCurSel();
   if (nItem == CB_ERR)
   {
      m_cbGenre.GetWindowText(discinfo.m_cszGenre.GetBuffer(512), 512);
      discinfo.m_cszGenre.ReleaseBuffer();
   }
   else
      m_cbGenre.GetLBText(nItem, discinfo.m_cszGenre);

   discinfo.m_bVariousArtists = BST_CHECKED == SendDlgItemMessage(IDC_CDSELECT_CHECK_VARIOUS_ARTISTS, BM_GETCHECK);

   discinfo.m_cszCDID = BASS_CD_GetID(nDrive, BASS_CDID_CDDB);

   pManager->ResetTrackInfo();

   // write all selected items into manager
   nItem = m_lcTracks.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

   std::vector<DWORD> vecTracks;

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
      for(int n=0; n<nMax; n++)
         vecTracks.push_back(m_lcTracks.GetItemData(n));
   }

   unsigned int nMax = vecTracks.size();

   for(unsigned int n=0; n<nMax; n++)
   {
      wlCDRipTrackInfo trackinfo;

      unsigned int nTrack = vecTracks[n];
      trackinfo.m_nTrackOnDisc = nTrack;
      m_lcTracks.GetItemText(nTrack, 1, trackinfo.m_cszTrackTitle);
      trackinfo.m_nTrackLength = BASS_CD_GetTrackLength(nDrive, nTrack) / 176400L;

      pManager->AddTrackInfo(trackinfo);
   }

   // when acquired freedb info, store  in cdplayer.ini, too
   if (m_bAcquiredDiscInfo)
      StoreInCdplayerIni(nDrive);
}

void wlCDRipDlg::StoreInCdplayerIni(unsigned int nDrive)
{
   if (!m_uiSettings.store_disc_infos_cdplayer_ini)
      return;

   CString cszCDPlayerIniFilename;
   ::GetWindowsDirectory(cszCDPlayerIniFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszCDPlayerIniFilename.ReleaseBuffer();
   cszCDPlayerIniFilename += _T("\\cdplayer.ini");

   const char* cdplayer_id_raw = BASS_CD_GetID(nDrive, BASS_CDID_CDPLAYER);

   USES_CONVERSION;
   const TCHAR* cdplayer_id = A2CT(cdplayer_id_raw);

   wlCDRipTrackManager* pManager = wlCDRipTrackManager::getCDRipTrackManager();
   wlCDRipDiscInfo& discinfo = pManager->GetDiscInfo();

   CString cszFormat;

   // numtracks
   unsigned int nNumTracks = m_lcTracks.GetItemCount();
   cszFormat.Format(_T("%u"), nNumTracks);
   ::WritePrivateProfileString(cdplayer_id, _T("numtracks"), cszFormat, cszCDPlayerIniFilename);

   // artist
   ::WritePrivateProfileString(cdplayer_id, _T("artist"), discinfo.m_cszDiscArtist, cszCDPlayerIniFilename);

   // title
   ::WritePrivateProfileString(cdplayer_id, _T("title"), discinfo.m_cszDiscTitle, cszCDPlayerIniFilename);

   // year
   if (discinfo.m_nYear > 0)
   {
      cszFormat.Format(_T("%u"), discinfo.m_nYear);
      ::WritePrivateProfileString(cdplayer_id, _T("year"), cszFormat, cszCDPlayerIniFilename);
   }

   // genre
   ::WritePrivateProfileString(cdplayer_id, _T("genre"), discinfo.m_cszGenre, cszCDPlayerIniFilename);

   // tracks
   CString cszTrackText;
   for (unsigned int n=0; n<nNumTracks; n++)
   {
      cszFormat.Format(_T("%u"), n);

      m_lcTracks.GetItemText(n, 1, cszTrackText);

      ::WritePrivateProfileString(cdplayer_id, cszFormat, cszTrackText, cszCDPlayerIniFilename);
   }
}

LRESULT wlCDRipDlg::OnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   NMLVDISPINFO* pLvDispInfo = reinterpret_cast<NMLVDISPINFO*>(pnmh);
   if (pLvDispInfo->item.iItem == -1)
      return 0;

   m_bEditedTrack = true;

   return 1;
}

#include "freedb.hpp"

#if _MSC_VER < 1400
#pragma comment(linker, "/delayload:ws2_32.dll")
#endif

void wlCDRipDlg::FreedbLookup()
{
   HMODULE dll = LoadLibrary(_T("ws2_32.dll"));
   if (dll == NULL)
   {
      wlMessageBox(m_hWnd, IDS_CDRIP_NO_INTERNET_AVAIL, MB_OK | MB_ICONSTOP);
      return;
   }
   FreeLibrary(dll);

   DWORD nDrive = GetCurrentDrive();

   const char* cdtext = BASS_CD_GetID(nDrive, BASS_CDID_CDDB);
   if (!cdtext || strlen(cdtext) == 0)
   {
      wlMessageBox(m_hWnd, IDS_CDRIP_ERROR_NOCDINFO, MB_OK | MB_ICONSTOP);
      return;
   }

   WSADATA wsaData;
   WORD wVersionRequested = MAKEWORD( 2, 2 );
   WSAStartup(wVersionRequested, &wsaData);

   Freedb::CDInfo info;
   try
   {
      CWaitCursor waitCursor;

      USES_CONVERSION;
      std::string server(T2CA(m_uiSettings.freedb_server));
      Freedb::Remote remoteFreedb(server);

      extern void GetWinlameVersion(CString& cszVersion);

      CString cszWinlameVersion;
      GetWinlameVersion(cszWinlameVersion);
      cszWinlameVersion.Replace(_T(' '), _T('-'));

      std::string username(T2CA(m_uiSettings.freedb_username));
      std::string version(T2CA(cszWinlameVersion));
      remoteFreedb.doHandshake(username, "winLAME", version);

      vector<Freedb::SearchResult> results = remoteFreedb.query_cddb_raw(cdtext);

      unsigned int nMax = results.size();
      if (nMax == 0)
      {
         wlMessageBox(m_hWnd, IDS_CDRIP_NO_CDINFO_AVAIL, MB_OK | MB_ICONEXCLAMATION);
      }
      else
      {
         unsigned int nSelected = 0;

         if (nMax > 1)
         {
            wlCDRipFreedbListDlg dlg;

            dlg.results.assign(results.begin(), results.end());

            waitCursor.Restore();
            ATLVERIFY(IDOK == dlg.DoModal());
            waitCursor.Set();

            // select which one to take
            nSelected = dlg.GetSelectedItem();
         }

         Freedb::SearchResult& result = results[nSelected];

         info = remoteFreedb.read(result.category, result.discid);
      }

      m_bAcquiredDiscInfo = true;
   }
   catch(Freedb::Error& err)
   {
      CString cszText, cszTemp;
      cszText.LoadString(IDS_CDRIP_ERROR_FREEDB);
      cszTemp.Format(_T(" (code=%i text=\"%hs\")"), err.code, err.msg.c_str());
      cszTemp.Replace(_T("\n"), _T(""));
      cszTemp.Replace(_T("\r"), _T(""));
      cszText += cszTemp;
      wlMessageBox(m_hWnd, cszText, MB_OK | MB_ICONSTOP);
   }
   catch(const std::string& strError)
   {
      CString cszText;
      cszText.LoadString(IDS_CDRIP_ERROR_FREEDB);
      if (!strError.empty())
      {
         cszText += _T(" (");
         cszText += CString(strError.c_str());
         cszText += _T(")");
      }
      wlMessageBox(m_hWnd, cszText, MB_OK | MB_ICONSTOP);
   }
   catch(...)
   {
      wlMessageBox(m_hWnd, IDS_CDRIP_ERROR_FREEDB, MB_OK | MB_ICONSTOP);
   }

   WSACleanup();

   // set info to list
   {
      CString cszText;
      unsigned int nMax = info.tracktitles.size();
      for(unsigned int n=0; n<nMax; n++)
      {
         cszText = info.tracktitles[n].c_str();
         m_lcTracks.SetItemText(n, 1, cszText);
      }

      cszText = info.dartist.c_str();
      SetDlgItemText(IDC_CDSELECT_EDIT_ARTIST, cszText);

      cszText = info.dtitle.c_str();
      SetDlgItemText(IDC_CDSELECT_EDIT_TITLE, cszText);

      if (info.dyear.size() > 0)
         SetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, strtoul(info.dyear.c_str(), NULL, 10), FALSE);
      else
      {
         unsigned int nPos = info.dextinfo.find("YEAR: ");
         if (-1 != nPos)
         {
            unsigned long nYear = strtoul(info.dextinfo.c_str()+nPos+6, NULL, 10);
            SetDlgItemInt(IDC_CDSELECT_EDIT_YEAR, nYear, FALSE);
         }
      }

      CString cszGenre(info.dgenre.c_str());
      if (cszGenre.GetLength() > 0)
      {
         int nItem = m_cbGenre.FindStringExact(-1, cszGenre);
         if (nItem == CB_ERR)
            nItem = m_cbGenre.AddString(cszGenre);

         m_cbGenre.SetCurSel(nItem);
      }
   }
}
