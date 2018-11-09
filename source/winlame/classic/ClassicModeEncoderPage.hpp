//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2018 Michael Fink
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
/// \file ClassicModeEncoderPage.hpp
/// \brief Start page for Classic UI
//
#pragma once

#include "WizardPage.hpp"
#include "TasksView.hpp"
#include "TaskDetailsView.hpp"
#include <ulib/win32/Win7Taskbar.hpp>
#include <boost/optional.hpp>
#include "resource.h"

class TaskManager;

namespace UI
{
   /// \brief Encoder page for classic mode
   /// \details Lets the user encode the files or CD tracks that were selected. Tasks were already added in
   /// the FinishPage. The page shows a tasks list and a task details view, separated by a
   /// splitter. Also a toolbar is shown with the two commands "stop all" and "remove completed".
   /// When exiting the page and there are still tasks running, a message box is shown, asking if
   /// the user wants to stop encoding. Navigating back when all tasks are finished returns to the
   /// classic mode start page.
   class ClassicModeEncoderPage :
      public WizardPage,
      public CWinDataExchange<ClassicModeEncoderPage>,
      public CDialogResize<ClassicModeEncoderPage>,
      public CIdleHandler
   {
   public:
      /// ctor
      explicit ClassicModeEncoderPage(WizardPageHost& pageHost);
      /// dtor
      ~ClassicModeEncoderPage()
      {
      }

   private:
      friend CDialogResize<ClassicModeEncoderPage>;

      BEGIN_DDX_MAP(ClassicModeEncoderPage)
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(ClassicModeEncoderPage)
         DLGRESIZE_CONTROL(IDC_CLASSIC_ENCODE_PAGE_TOOLBAR, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_CLASSIC_ENCODE_PAGE_SPLITTER, DLSZ_SIZE_X | DLSZ_SIZE_Y)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(ClassicModeEncoderPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
         COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
         COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
         COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
         COMMAND_ID_HANDLER(ID_TASKS_STOP_ALL, OnTasksStopAll)
         COMMAND_ID_HANDLER(ID_TASKS_REMOVE_COMPLETED, OnTasksRemoveCompleted)
         CHAIN_MSG_MAP(CDialogResize<ClassicModeEncoderPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

      /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when dialog is about to be destroyed
      LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when page is left with the Finish button
      LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with the Cancel  button
      LRESULT OnButtonCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with the Back button
      LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when the user clicks on the "stop all" toolbar button
      LRESULT OnTasksStopAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when the user clicks on the "remove completed" toolbar button
      LRESULT OnTasksRemoveCompleted(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when dialog is idle
      virtual BOOL OnIdle() override;

      /// sets up toolbar
      void SetupToolbar();

      /// sets up dialog view
      void SetupView();

      /// checks if tasks are currently running and ask user if he really wants to quit
      bool CheckRunningTasksAndQuit();

      /// checks task manager and updates Win7 task bar
      void UpdateWin7TaskBar();

      /// called when user clicked on a task list view item
      void OnClickedTaskItem(size_t clickedIndex);

      /// checks if all tasks have finished
      void CheckAllTasksFinished();

   private:
      // model

      /// task manager instance
      TaskManager& m_taskManager;

      /// possible actions when encoding has finished
      enum T_enEncodingFinishAction
      {
         doNothing = 0, ///< do nothing after finished encoding
         closeApp = 1,  ///< close app after encoding
         standbyPC = 2, ///< switches PC to standby
      };

      /// current encoding finish action
      T_enEncodingFinishAction m_encodingFinishAction;

      /// indicates if at the last check time, there were tasks running
      bool m_areTasksRunningPreviously;

      // controls

      /// toolbar showing commands to deal with tasks
      CToolBarCtrl m_toolbar;

      /// splitter window to show tasks view and details
      CHorSplitterWindow m_splitter;

      /// list view showing all tasks
      TasksView m_tasksView;

      /// pane for task details view
      CPaneContainer m_paneTaskDetails;

      /// task details view
      TaskDetailsView m_taskDetailsView;

      /// access to task bar
      boost::optional<Win32::Taskbar> m_win7TaskBar;

      /// access to task bar progress bar
      boost::optional<Win32::TaskbarProgressBar> m_win7TaskBarProgressBar;
   };

} // namespace UI
