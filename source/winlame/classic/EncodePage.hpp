/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2017 Michael Fink

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

*/
/// \file EncodePage.hpp
/// \brief contains the definition of the encode page
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "resource.h"
#include "PageBase.hpp"
#include "CommonStuff.hpp"
#include "EncoderInterface.hpp"
#include "EncoderErrorHandler.hpp"
#include "SystemTrayIcon.hpp"


/// systray activation message
#define WL_SYSTRAY_ACTIVE WM_APP+1

/// timer id for updating info
#define IDT_ENC_UPDATEINFO 67

/// encode page class
class EncodePage:
   public PageBase,
   public Encoder::EncoderErrorHandler,
   public CDialogResize<EncodePage>
{
public:
   /// ctor
   EncodePage()
      :newfile(false),
       curfile(0),
       starttimer(0),
       noupdate(false),
       idle_icon(nullptr),
       working_icon(nullptr),
       intray(false),
       startpause(0),
       pausetime(0)
   {
      IDD = IDD_DLG_ENCODE;
      captionID = IDS_DLG_CAP_ENCODE;
      descID = IDS_DLG_DESC_ENCODE;
      helpID = IDS_HTML_ENCODE;

      // create new encoder object
      encoder = Encoder::EncoderInterface::CreateEncoder();
   }

   /// dtor
   virtual ~EncodePage()
   {
      delete encoder;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(EncodePage)
   DLGRESIZE_CONTROL(IDC_ENC_BEVEL1, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_ENC_BEVEL2, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_ENC_PROGRESS_FILES, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_ENC_INFO2, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_ENC_PROGRESS_CURRFILE, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_ENC_BEVEL3, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_ENC_ENCINFO, DLSZ_SIZE_X | DLSZ_SIZE_Y)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(EncodePage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
   COMMAND_HANDLER(IDC_ENC_START, BN_CLICKED, OnClickedStart)
   COMMAND_HANDLER(IDC_ENC_STOP, BN_CLICKED, OnClickedStop)
   COMMAND_HANDLER(IDC_ENC_TOTRAY, BN_CLICKED, OnClickedToTray)
   MESSAGE_HANDLER(WL_SYSTRAY_ACTIVE, OnSystrayActive)
   MESSAGE_HANDLER(WM_TIMER, OnTimer)
   CHAIN_MSG_MAP(CDialogResize<EncodePage>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is about to be destroyed
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // simulate click on stop button
      if (encoder->IsRunning())
      {
         BOOL dummy;
         OnClickedStop(0,0,0,dummy);
      }

      // free resources
      ::DestroyIcon(idle_icon);
      ::DestroyIcon(working_icon);

      ilIcons.Destroy();
      return 0;
   }

   /// called when user clicked on start (or pause) button
   LRESULT OnClickedStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   /// called when user clicked on stop button
   LRESULT OnClickedStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   /// called when user clicked on "send to tray" button
   LRESULT OnClickedToTray(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   /// called when user clicked on systray image
   LRESULT OnSystrayActive(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called for each "update info" timer interval
   LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (wParam == IDT_ENC_UPDATEINFO && !noupdate)
         UpdateInfo();
      return 0;
   }

   // virtual function from EncoderErrorHandler

   /// error handler function
   virtual Encoder::EncoderErrorHandler::ErrorAction HandleError(LPCTSTR infilename,
      LPCTSTR modulename, int errnum, LPCTSTR errormsg, bool bSkipDisabled=false);

   // virtual function from PageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   /// updates encoding information
   void UpdateInfo();

   /// shuts down windows, depending on the action code
   void ShutdownWindows(int action);

protected:
   BevelLine bevel1;  ///< bevel line
   BevelLine bevel2;  ///< bevel line
   BevelLine bevel3;  ///< bevel line

   /// icon list for image buttons
   CImageList ilIcons;

   /// encoder instance
   Encoder::EncoderInterface *encoder;

   /// indicates if infos about a new file to encode are available
   bool newfile;

   /// current file to encode
   int curfile;

   /// indicates if UpdateInfo() should be called
   bool noupdate;

   /// millisecond the last file to encode was started
   DWORD starttimer;

   /// start of pausing encoding
   DWORD startpause;

   /// total time of paused encoding
   DWORD pausetime;

   /// system tray icon object
   SystemTrayIcon trayicon;

   /// idle tray icon
   HICON idle_icon;

   /// tray icon while working
   HICON working_icon;

   /// indicates if window is in systray
   bool intray;
};


/// @}
