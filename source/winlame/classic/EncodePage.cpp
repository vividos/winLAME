//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
// Copyright (c) 2004 DeXT
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
/// \file EncodePage.cpp
/// \brief contains the encode page implementation
//
#include "stdafx.h"
#include "EncodePage.hpp"
#include "ErrorDlg.hpp"
#include "CDRipTrackManager.hpp"
#include "EncoderSettings.hpp"
#include "EncoderState.hpp"

using ClassicUI::EncodePage;

#define IDR_TRAYICON 64
#define IDM_ABOUTBOX 16 // from MainDlg.cpp

// global functions

static HWND mainwnd = NULL;

/// warns about transcoding
bool WarnAboutTranscode()
{
   return AppMessageBox(::GetActiveWindow(), IDS_WARN_TRANSCODE, MB_YESNO | MB_ICONEXCLAMATION) == IDYES;
}


// EncodePage methods

LRESULT EncodePage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_ENC_BEVEL1));
   bevel2.SubclassWindow(GetDlgItem(IDC_ENC_BEVEL2));
   bevel3.SubclassWindow(GetDlgItem(IDC_ENC_BEVEL3));

   // create the image list
   ilIcons.Create(MAKEINTRESOURCE(IDB_BITMAP_BTNICONS), 16, 0, RGB(192, 192, 192));

   // set icons on buttons
   SendDlgItemMessage(IDC_ENC_START, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(2));

   SendDlgItemMessage(IDC_ENC_STOP, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(4));

   SendDlgItemMessage(IDC_ENC_TOTRAY, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(5));

   ::EnableWindow(GetDlgItem(IDC_ENC_STOP), FALSE);

   // set info text strings
   CString text;
   text.LoadString(IDS_ENC_STOPPED);
   ::SetWindowText(GetDlgItem(IDC_ENC_INFO1), text);
   ::SetWindowText(GetDlgItem(IDC_ENC_INFO2), _T(""));
   ::SetWindowText(GetDlgItem(IDC_ENC_INFO3), _T(""));
   ::SetWindowText(GetDlgItem(IDC_ENC_INFO4), _T(""));

   UISettings &settings = pui->getUISettings();

   CProgressBarCtrl progressAll(GetDlgItem(IDC_ENC_PROGRESS_FILES));
   progressAll.SetRange(0, settings.encoderjoblist.size() * 100);
   progressAll.SetPos(0);

   CProgressBarCtrl progressCurrent(GetDlgItem(IDC_ENC_PROGRESS_CURRFILE));
   progressCurrent.SetRange(0, 100);
   progressCurrent.SetPos(0);

   newfile = true;

   encoder->SetSettingsManager(&settings.settings_manager);

   // load tray icons
   idle_icon = (HICON)LoadImage(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDI_ICON_IDLE), IMAGE_ICON, 16, 16, 0);

   working_icon = (HICON)LoadImage(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDI_ICON_WORKING), IMAGE_ICON, 16, 16, 0);

   mainwnd = GetParent();

   // autostart encoding when:
   // - last page was rip page
   // - there are files in the filename list
   // - there were cd tracks to rip
   // - check was activated on rip page
   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

   if (settings.last_page_was_cdrip_page &&
      settings.encoderjoblist.size() > 0 &&
      pManager->GetMaxTrackInfo() > 0 &&
      settings.cdrip_autostart_encoding)
      ::PostMessage(GetDlgItem(IDC_ENC_START), BM_CLICK, 0, 0);

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT EncodePage::OnClickedStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   if (encoder->IsRunning())
   {
      // update pause timer
      if (encoder->IsPaused())
      {
         // encoding continues; read out pause counter
         pausetime += GetTickCount() - startpause;
      }
      else
      {
         // not paused yet; start pause counter
         startpause = GetTickCount();
      }

      // toggles pausing
      encoder->PauseEncoding();

      SendDlgItemMessage(IDC_ENC_START, BM_SETIMAGE, IMAGE_ICON,
         (LPARAM)ilIcons.ExtractIcon(encoder->IsPaused() ? 2 : 3));

      // set appropriate tray icon
      if (intray)
         trayicon.SetIcon(encoder->IsPaused() ? idle_icon : working_icon);
   }
   else
   {
      newfile = true;

      // lock wizard back button
      pui->lockWizardButtons(true);

      // start encoding
      ::EnableWindow(GetDlgItem(IDC_ENC_START), TRUE);
      ::EnableWindow(GetDlgItem(IDC_ENC_STOP), TRUE);

      // set pause icon on first button
      SendDlgItemMessage(IDC_ENC_START, BM_SETIMAGE, IMAGE_ICON,
         (LPARAM)ilIcons.ExtractIcon(3));

      // set some encoder options
      UISettings &settings = pui->getUISettings();

      // set param if this is the last file
      settings.settings_manager.setValue(GeneralIsLastFile,
         unsigned(curfile + 1) >= settings.encoderjoblist.size() ? 1 : 0);

      // set encoder settings
      Encoder::EncoderSettings encoderSettings;

      // set input path
      encoderSettings.m_inputFilename = settings.encoderjoblist[curfile].InputFilename();

      // set output path
      if (settings.out_location_use_input_dir)
      {
         Path outputPath(settings.encoderjoblist[curfile].InputFilename());

         encoderSettings.m_outputFolder = outputPath.FolderName();
      }
      else
         encoderSettings.m_outputFolder = settings.m_defaultSettings.outputdir;

      if (settings.create_playlist)
         encoderSettings.m_playlistFilename = settings.playlist_filename;

      Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
      encoderSettings.m_outputModuleID = moduleManager.GetOutputModuleID(settings.output_module);

      encoderSettings.m_overwriteExisting = settings.m_defaultSettings.overwrite_existing;
      encoderSettings.m_deleteInputAfterEncode = settings.m_defaultSettings.delete_after_encode;

      encoderSettings.m_warnLossyTranscoding = settings.warn_lossy_transcoding;

      encoder->SetEncoderSettings(encoderSettings);

      encoder->SetErrorHandler(this);

      encoder->StartEncode();

      // update info every 100 ms
      SetTimer(IDT_ENC_UPDATEINFO, 100);

      UpdateInfo();

      // init timers
      starttimer = GetTickCount();
      pausetime = 0;

      // set tray icon for working
      if (intray)
         trayicon.SetIcon(working_icon);
   }

   return 0;
}

LRESULT EncodePage::OnClickedStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // stop timer
   KillTimer(IDT_ENC_UPDATEINFO);

   // prevent UpdateInfo() from possibly deleting the input file
   noupdate = true;

   // stop encoding
   encoder->StopEncode();

   // change the icons on the buttons
   SendDlgItemMessage(IDC_ENC_START, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(2));
   ::EnableWindow(GetDlgItem(IDC_ENC_STOP), FALSE);

   if (pui->getUISettings().encoderjoblist.size() != 0)
   {
      CString text;
      text.LoadString(IDS_ENC_STOPPED);
      ::SetWindowText(GetDlgItem(IDC_ENC_INFO1), text);
   }

   // wait for encoder to finish
   while (encoder->IsRunning())
      Sleep(0);

   // set idle tray icon
   if (intray)
      trayicon.SetIcon(idle_icon);

   // unlock wizard back button
   pui->lockWizardButtons(false);

   // set focus on start button
   ::SetFocus(GetDlgItem(IDC_ENC_START));

   noupdate = false;

   return 0;
}

LRESULT EncodePage::OnClickedToTray(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   HWND parent = GetParent();

   // get tooltop for tray

   TCHAR buffer[256]; buffer[255] = 0;
   ::GetWindowText(parent, buffer, 255);

   // init tray icon
   trayicon.Init(m_hWnd, IDR_TRAYICON,
      encoder->IsRunning() && !encoder->IsPaused() ? working_icon : idle_icon,
      WL_SYSTRAY_ACTIVE, buffer);
   ::ShowWindow(parent, SW_HIDE);

   intray = true;
   return 0;
}

LRESULT EncodePage::OnSystrayActive(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   HWND parent = GetParent();
   switch ((UINT)lParam)
   {
   case WM_LBUTTONDBLCLK:
   case WM_LBUTTONDOWN:
   {
      // show window again
      ::ShowWindow(parent, SW_SHOW);
      ::BringWindowToTop(parent);
      ::SetFocus(parent);

      // remove tray icon
      trayicon.RemoveIcon();
      intray = false;
   }
   break;
   case WM_CONTEXTMENU:
   case WM_RBUTTONDOWN:
   {
      // show context menu

      // load menu resource
      HMENU menu = ::LoadMenu(_Module.GetResourceInstance(),
         MAKEINTRESOURCE(IDM_TRAY_CONTEXTMENU));

      // remove either pause or start encoding item
      ::RemoveMenu(menu,
         (encoder->IsRunning() && !encoder->IsPaused() ?
            ID_TRAY_START : ID_TRAY_PAUSE), MF_BYCOMMAND);

      // check if there are files left to encode
      if (pui->getUISettings().encoderjoblist.empty())
      {
         unsigned int searchid =
            encoder->IsRunning() && !encoder->IsPaused() ? ID_TRAY_PAUSE : ID_TRAY_START;

         // search for entry pos
         HMENU submenu = GetSubMenu(menu, 0);
         int max = ::GetMenuItemCount(submenu);
         for (int i = 0; i < max; i++)
            if (searchid == ::GetMenuItemID(submenu, i))
            {
               // remove entry and separator
               ::RemoveMenu(submenu, i, MF_BYPOSITION);
               ::RemoveMenu(submenu, i, MF_BYPOSITION);
               break;
            }
      }

      // get cursor pos
      POINT pos;
      GetCursorPos(&pos);

      // show popup menu
      int ret = ::TrackPopupMenu(::GetSubMenu(menu, 0),
         TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
         pos.x, pos.y, 0, m_hWnd, NULL);
      PostMessage(WM_NULL);

      switch (ret)
      {
      case ID_TRAY_ABOUT:
         // show about box
         ::SendMessage(parent, WM_SYSCOMMAND, IDM_ABOUTBOX, 0);
         break;

      case ID_TRAY_PAUSE:
      case ID_TRAY_START:
         // simulate button click
         if (pui->getUISettings().encoderjoblist.size() != 0)
            OnClickedStart(0, 0, 0, bHandled);
         break;

      case ID_TRAY_QUIT:
         // quit winLAME
         trayicon.RemoveIcon();
         intray = false;
         OnClickedStop(0, 0, 0, bHandled);
         ::PostQuitMessage(0);
         break;

      case ID_TRAY_SHOW:
         // show window again
         ::ShowWindow(parent, SW_SHOW);
         ::BringWindowToTop(parent);
         ::SetFocus(parent);

         // remove tray icon
         trayicon.RemoveIcon();
         intray = false;
         break;
      }

      ::DestroyMenu(menu);
   }
   break;
   }
   return 0;
}

Encoder::EncoderErrorHandler::ErrorAction EncodePage::HandleError(LPCTSTR infilename,
   LPCTSTR modulename, int errnum, LPCTSTR errormsg, bool bSkipDisabled)
{
   // show error dialog
   ErrorDlg dlg(infilename, modulename, errnum, errormsg, bSkipDisabled);

   EncoderErrorHandler::ErrorAction ret = EncoderErrorHandler::Continue;
   switch (dlg.DoModal())
   {
   case IDC_ERR_BUTTON_CONTINUE:
      ret = EncoderErrorHandler::Continue;
      break;

   case IDC_ERR_BUTTON_SKIPFILE:
      ret = EncoderErrorHandler::SkipFile;
      break;

   case IDC_ERR_BUTTON_STOP:
      ret = EncoderErrorHandler::StopEncode;
      break;
   }
   return ret;
}

void EncodePage::OnEnterPage()
{
   // disable start button when no input files
   // (e.g. when, after encoding, the user pressed back and next again
   if (pui->getUISettings().encoderjoblist.empty())
   {
      ::EnableWindow(GetDlgItem(IDC_ENC_START), FALSE);
      ::SetWindowText(GetDlgItem(IDC_ENC_INFO1), _T(""));
   }
}

bool EncodePage::OnLeavePage()
{
   UISettings& settings = pui->getUISettings();

   settings.last_page_was_cdrip_page = false;

   // to prevent encoding of the files already encoded, delete them from the list
   if (curfile > 0)
   {
      if (settings.encoderjoblist.size() < (unsigned)curfile)
         settings.encoderjoblist.clear();
      else
         settings.encoderjoblist.erase(settings.encoderjoblist.begin(),
            settings.encoderjoblist.begin() + curfile);
   }

   // when no file is available anymore, go to first page again
   if (settings.encoderjoblist.size() == 0)
   {
      // delete all pages up to the output page (excluding)
      int i = pui->getCurrentWizardPage() - 1;
      for (; i > 1; i--)
         pui->deleteWizardPage(i);

      pui->setCurrentPage(0);

      // delete all files in track manager
      CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

      while (pManager->GetMaxTrackInfo() > 0)
         pManager->RemoveTrackInfo(0);

   }

   // resets current file number (when finished encoding, user could
   // leave the page again)
   curfile = 0;
   return true;
}

void EncodePage::UpdateInfo()
{
   UISettings& settings = pui->getUISettings();

   Encoder::EncoderState encoderState = encoder->GetEncoderState();

   if (encoderState.m_running)
   {
      CString text;

      // update some settings only when a new file has started
      if (newfile)
      {
         // number of file out of all files
         text.Format(IDS_ENC_U_OF_U, curfile + 1, settings.encoderjoblist.size());
         ::SetWindowText(GetDlgItem(IDC_ENC_INFO1), text);

         // current filename
         std::tstring filename(settings.encoderjoblist[curfile].InputFilename());
         filename = filename.c_str() + filename.find_last_of(_T("\\/")) + 1;
         text.Format(IDS_ENC_FILE_S, filename.c_str());
         ::SetWindowText(GetDlgItem(IDC_ENC_INFO2), text);

         // encoding description
         CString description = encoderState.m_encodingDescription;
         ::SetWindowText(GetDlgItem(IDC_ENC_ENCINFO), description);

         // force a second update when the description is not yet available
         if (!description.IsEmpty())
            newfile = false;
      }

      // show percent text
      float percent = encoderState.m_percent;
      text.Format(IDS_ENC_PERCENT_F, int(percent), int((percent - int(percent)) * 10));
      ::SetWindowText(GetDlgItem(IDC_ENC_INFO3), text);

      text = "";
      if (percent > 5.f)
      {
         // calculate seconds needed to complete encoding
         long elapsed = GetTickCount() - starttimer - pausetime;
         long wholeneeded = long(elapsed * 100 / percent);
         long stillneeded = long((wholeneeded - elapsed) / 1000) + 1;
         if (stillneeded < 1)
            stillneeded = 1;

         // calculate hours and minutes
         int seconds, minutes, hours;
         seconds = stillneeded % 60;
         minutes = (stillneeded / 60) % 60;
         hours = stillneeded / 3600;

         text.Format(IDS_ENC_REMAINING_UUU, hours, minutes, seconds);
      }
      if (!encoderState.m_paused)
         ::SetWindowText(GetDlgItem(IDC_ENC_INFO4), text);

      // set progress bar pos
      CProgressBarCtrl progressCurrent(GetDlgItem(IDC_ENC_PROGRESS_CURRFILE));
      progressCurrent.SetPos(static_cast<int>(percent));

      CProgressBarCtrl progressAll(GetDlgItem(IDC_ENC_PROGRESS_FILES));
      progressAll.SetPos(static_cast<int>(curfile * 100 + percent));

      // update tray icon tooltip, if user hid main window
      if (intray)
      {
         float allpercent = (curfile*100.f + percent) / settings.encoderjoblist.size();

         // format string
         text.Format(IDS_ENC_TRAY_TOOLTIP,
            curfile + 1, settings.encoderjoblist.size(),
            int(allpercent), int((allpercent - int(allpercent)) * 10));

         trayicon.SetTooltipText(text);
      }
   }
   else
   {
      // current encoding thread has stopped

      if (unsigned(++curfile) < settings.encoderjoblist.size() && encoderState.m_errorCode >= 0)
      {
         // start the next encode
         noupdate = true;

         // start new file
         BOOL dummy;
         OnClickedStart(0, 0, 0, dummy);
         while (!encoder->IsRunning())
            Sleep(0);

         noupdate = false;
      }
      else
      {
         // no more files to encode
         CString text;
         text.LoadString(IDS_ENC_FINISHED);
         ::SetWindowText(GetDlgItem(IDC_ENC_INFO1), text);

         text.Format(IDS_ENC_PERCENT_F, 100, 0);
         ::SetWindowText(GetDlgItem(IDC_ENC_INFO3), text);

         ::SetWindowText(GetDlgItem(IDC_ENC_INFO2), _T(""));
         ::SetWindowText(GetDlgItem(IDC_ENC_INFO4), _T(""));

         // set progress bar positions
         CProgressBarCtrl progressAll(GetDlgItem(IDC_ENC_PROGRESS_FILES));
         progressAll.SetPos(curfile * 100);

         CProgressBarCtrl progressCurrent(GetDlgItem(IDC_ENC_PROGRESS_CURRFILE));
         progressCurrent.SetPos(100);

         // change the icons on the buttons
         SendDlgItemMessage(IDC_ENC_START, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM)ilIcons.ExtractIcon(2));
         ::EnableWindow(GetDlgItem(IDC_ENC_START), FALSE);
         ::EnableWindow(GetDlgItem(IDC_ENC_STOP), FALSE);

         // set tray icon tooltip when finished
         if (intray)
         {
            text.LoadString(IDS_ENC_TRAY_FINISHED);
            trayicon.SetTooltipText(text);
            trayicon.SetIcon(idle_icon);
         }

         // stop timer
         KillTimer(IDT_ENC_UPDATEINFO);

         // delete all files in list
         settings.encoderjoblist.clear();

         // unlock back and next buttons
         pui->lockWizardButtons(false);

         // set encoding description
         ::SetWindowText(GetDlgItem(IDC_ENC_ENCINFO), encoderState.m_encodingDescription);

         // check if we should perform an action when finished encoding
         if (encoderState.m_errorCode == 0 && settings.after_encoding_action > 0)
            ShutdownWindows(settings.after_encoding_action);
      }
   }
}

/// action codes:
/// 1: just quit winLAME
/// 2: shutdown windows and switch power off
/// 3: log off current user
/// 4: hibernate (write to disk); only available with > windows 2000
/// 5: suspend (to ram); not available in windows NT
void EncodePage::ShutdownWindows(int action)
{
   if (action == 2 || action == 3)
   {
      // adjust windows NT / 2000 privilege to shutdown the system
      HANDLE token = NULL;

      if (TRUE == OpenProcessToken(GetCurrentProcess(),
         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
      {
         TOKEN_PRIVILEGES tpriv;

         // get luid of shutdown privilege
         if (TRUE == LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tpriv.Privileges[0].Luid))
         {
            // enable privilege
            tpriv.PrivilegeCount = 1;
            tpriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            AdjustTokenPrivileges(token, FALSE, &tpriv, 0, NULL, NULL);
         }
         CloseHandle(token);
      }
   }

   switch (action)
   {
   case 1: // exit winLAME
      PostQuitMessage(0);
      break;

   case 2: // shutdown
      ::ExitWindowsEx(EWX_SHUTDOWN, 0);
      ::PostQuitMessage(0);
      break;

   case 3: // logoff
      ::ExitWindowsEx(EWX_LOGOFF, 0);
      ::PostQuitMessage(0);
      break;

   case 4: // hibernate
      ::SetSystemPowerState(FALSE, FALSE);
      ::PostQuitMessage(0);
      break;

   case 5: // suspend
      ::SetSystemPowerState(TRUE, FALSE);
      ::PostQuitMessage(0);
      break;
   }
}
