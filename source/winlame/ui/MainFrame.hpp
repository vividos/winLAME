//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2020 Michael Fink
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
/// \file MainFrame.hpp
/// \brief Main frame window
//
#pragma once

#include "TasksView.hpp"
#include <ulib/win32/Win7Taskbar.hpp>
#include "HtmlHelper.hpp"
#include "TaskDetailsView.hpp"
#include <optional>

#define WM_CHECK_COMMAND_LINE WM_APP+2

class TaskManager;

/// Modern UI classes
namespace UI
{
   /// \brief application main frame
   /// \details uses ribbon for commands
   /// \see https://www.codeproject.com/Articles/54116/Relook-your-Old-and-New-Native-Applications-with-a
   class MainFrame :
      public CRibbonFrameWindowImpl<MainFrame>,
      public CMessageFilter,
      public CIdleHandler
   {
      /// base class typedef
      typedef CRibbonFrameWindowImpl<MainFrame> BaseClass;

   public:
      /// ctor
      explicit MainFrame(TaskManager& taskManager)
         :m_taskManager(taskManager),
         m_tasksView(taskManager),
         m_isAppModeChanged(false),
         m_encodingFinishAction(T_enEncodingFinishAction::doNothing),
         m_areTasksRunningPreviously(false)
      {
      }

      /// returns if the dialog has been closed to change the app mode to classic mode
      bool IsAppModeChanged() const { return m_isAppModeChanged; }

      DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

      /// command bar
      CCommandBarCtrl m_CmdBar;

      /// ribbon item gallery for settings "finish action" selection
      CRibbonItemGalleryCtrl<ID_SETTINGS_FINISH_ACTION, 3> m_cbSettingsFinishAction;

      virtual BOOL PreTranslateMessage(MSG* pMsg);
      virtual BOOL OnIdle();

      BEGIN_RIBBON_CONTROL_MAP(MainFrame)
         RIBBON_CONTROL(m_cbSettingsFinishAction)
      END_RIBBON_CONTROL_MAP()

      BEGIN_UPDATE_UI_MAP(MainFrame)
         UPDATE_ELEMENT(ID_TASKS_STOP_ALL, UPDUI_RIBBON)
         UPDATE_ELEMENT(ID_TASKS_REMOVE_COMPLETED, UPDUI_RIBBON)
         UPDATE_ELEMENT(ID_SETTINGS_FINISH_ACTION, UPDUI_RIBBON)
      END_UPDATE_UI_MAP()

      BEGIN_MSG_MAP(MainFrame)
         MESSAGE_HANDLER(WM_CREATE, OnCreate)
         MESSAGE_HANDLER(WM_CLOSE, OnClose)
         MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
         MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
         MESSAGE_HANDLER(WM_CHECK_COMMAND_LINE, OnCheckCommandLine)
         COMMAND_ID_HANDLER(ID_APP_EXIT, OnAppExit)
         COMMAND_ID_HANDLER(ID_ENCODE_FILES, OnEncodeFiles)
         COMMAND_ID_HANDLER(ID_ENCODE_CD, OnEncodeCD)
         COMMAND_ID_HANDLER(ID_TASKS_STOP_ALL, OnTasksStopAll)
         COMMAND_ID_HANDLER(ID_TASKS_REMOVE_COMPLETED, OnTasksRemoveCompleted)
         COMMAND_ID_HANDLER(ID_SETTINGS_GENERAL, OnSettingsGeneral)
         COMMAND_ID_HANDLER(ID_SETTINGS_CDREAD, OnSettingsCDRead)
         RIBBON_GALLERY_CONTROL_HANDLER(ID_SETTINGS_FINISH_ACTION, OnSettingsFinishActionSelChanged)
         COMMAND_RANGE_HANDLER(ID_SETTINGS_FINISH_ACTION_NONE, ID_SETTINGS_FINISH_ACTION_STANDBY, OnSettingsFinishActionRange)
         COMMAND_ID_HANDLER(ID_VIEW_SWITCH_CLASSIC, OnViewSwitchToClassic)
         COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
         COMMAND_ID_HANDLER(ID_HELP, OnHelpCommand)
         MESSAGE_HANDLER(WM_HELP, OnHelp)
         CHAIN_MSG_MAP(BaseClass)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

   private:
      // Handler prototypes (uncomment arguments if needed):
      // LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
      // LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
      // LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

      LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
      LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
      LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
      LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
      LRESULT OnCheckCommandLine(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      LRESULT OnAppExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
      LRESULT OnEncodeFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
      LRESULT OnEncodeCD(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
      LRESULT OnTasksStopAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
      LRESULT OnTasksRemoveCompleted(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
      LRESULT OnSettingsGeneral(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
      LRESULT OnSettingsCDRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

      /// called when a selection in the ribbon combobox for "finish action" settings was made
      LRESULT OnSettingsFinishActionSelChanged(UI_EXECUTIONVERB verb, WORD wID, UINT uSel, BOOL& bHandled);
      /// called when an entry in "Settings | Finish action" submenu entry is being selected
      LRESULT OnSettingsFinishActionRange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      LRESULT OnViewSwitchToClassic(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

      /// called when the help command has been invoked
      LRESULT OnHelpCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when pressing F1
      LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   private:
      /// sets up command bar
      void SetupCmdBar();

      /// sets up ribbon bar
      bool SetupRibbonBar();

      /// sets up view
      void SetupView();

      /// collects command line files and opens input files page when necessary
      void GetCommandLineFiles();

      /// checks task manager and updates Win7 task bar
      void UpdateWin7TaskBar();

      /// called when user clicked on a task list view item
      void OnClickedTaskItem(size_t clickedIndex);

      /// checks if all tasks have finished
      void CheckAllTasksFinished();

   private:
      /// ref to task manager
      TaskManager& m_taskManager;

      /// splitter window to show tasks view and details
      CHorSplitterWindow m_splitter;

      /// tasks view
      TasksView m_tasksView;

      /// pane for task details view
      CPaneContainer m_paneTaskDetails;

      /// task details view
      TaskDetailsView m_taskDetailsView;

      /// access to task bar
      std::optional<Win32::Taskbar> m_win7TaskBar;

      /// access to task bar progress bar
      std::optional<Win32::TaskbarProgressBar> m_win7TaskBarProgressBar;

      /// indicates if the dialog has been closed to change the app mode to classic mode
      bool m_isAppModeChanged;

      /// possible actions when encoding has finished
      enum T_enEncodingFinishAction
      {
         doNothing = 0, ///< do nothing after finished encoding
         closeApp = 1,  ///< close app after encoding
         standbyPC = 2, ///< switches PC to standby
         hibernatePC = 3, ///< switches PC to standby
         logoffUser = 4,///< logs off current user
         shutdownPC = 5,///< shuts down PC
      };

      /// current encoding finish action
      T_enEncodingFinishAction m_encodingFinishAction;

      /// indicates if at the last check time, there were tasks running
      bool m_areTasksRunningPreviously;

      /// html help object
      HtmlHelper m_htmlHelper;
   };

} // namespace UI
