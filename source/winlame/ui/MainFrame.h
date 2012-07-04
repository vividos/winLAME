//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2012 Michael Fink
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
/// \file MainFrame.h
/// \brief Main frame window

// include guard
#pragma once

// includes
#include "TasksView.h"

// forward references
class TaskManager;

/// \brief application main frame
/// \details uses ribbon for commands
/// \see http://www.codeproject.com/Articles/54116/Relook-your-Old-and-New-Native-Applications-with-a
class MainFrame :
   public CRibbonFrameWindowImpl<MainFrame>,
   public CMessageFilter,
   public CIdleHandler
{
   /// base class typedef
   typedef CRibbonFrameWindowImpl<MainFrame> BaseClass;

public:
   /// ctor
   MainFrame(TaskManager& taskManager) throw()
      :m_taskManager(taskManager),
       m_view(taskManager),
       m_bRefreshActive(false)
   {
   }

   DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

   /// command bar
   CCommandBarCtrl m_CmdBar;

   virtual BOOL PreTranslateMessage(MSG* pMsg);
   virtual BOOL OnIdle();

   // update map
   BEGIN_UPDATE_UI_MAP(MainFrame)
      UPDATE_ELEMENT(ID_VIEW_RIBBON, UPDUI_MENUPOPUP)
   END_UPDATE_UI_MAP()

   // message map
   BEGIN_MSG_MAP(MainFrame)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_CLOSE, OnClose)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_TIMER, OnTimer)
      MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
      COMMAND_ID_HANDLER(ID_APP_EXIT, OnAppExit)
      COMMAND_ID_HANDLER(ID_ENCODE_FILES, OnEncodeFiles)
      COMMAND_ID_HANDLER(ID_ENCODE_CD, OnEncodeCD)
      COMMAND_ID_HANDLER(ID_SETTINGS_GENERAL, OnSettingsGeneral)
      COMMAND_ID_HANDLER(ID_SETTINGS_CDREAD, OnSettingsCDRead)
      COMMAND_ID_HANDLER(ID_VIEW_RIBBON, OnToggleRibbon)
      COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
      CHAIN_MSG_MAP(BaseClass)
   END_MSG_MAP()

private:
// Handler prototypes (uncomment arguments if needed):
// LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
// LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
// LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

   LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
   LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
   LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnAppExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnEncodeFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnEncodeCD(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnSettingsGeneral(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnSettingsCDRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnToggleRibbon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
   /// enables tasks list refresh
   void EnableRefresh(bool bEnable = true);

   /// parses files dropped on window
   void ParseDroppedFiles(HDROP hDropInfo);

   /// returns filter string
   CString GetFilterString();

   /// opens the file dialog
   void OpenFileDialog();

   /// parse buffer from multi selection from open file dialog
   void ParseMultiSelectionFiles(LPCTSTR pszBuffer);

private:
   /// ref to task manager
   TaskManager& m_taskManager;

   /// tasks view
   TasksView m_view;

   /// indicates if tasks list refresh is active
   bool m_bRefreshActive;

   /// filter string
   CString m_cszFilterString;
};
