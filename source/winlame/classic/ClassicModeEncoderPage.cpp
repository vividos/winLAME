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
/// \file ClassicModeEncoderPage.cpp
/// \brief Encoder page for Classic UI
//
#include "stdafx.h"
#include "ClassicModeEncoderPage.hpp"
#include "WizardPageHost.hpp"
#include "ClassicModeStartPage.hpp"
#include <ulib/IoCContainer.hpp>
#include "TaskManager.hpp"

using namespace UI;

/// size of toolbar
const unsigned int c_toolbarSize = 28;

ClassicModeEncoderPage::ClassicModeEncoderPage(WizardPageHost& pageHost)
   :WizardPage(pageHost, IDD_PAGE_CLASSIC_ENCODE, WizardPage::typeCancelBackFinish),
   m_taskManager(IoCContainer::Current().Resolve<TaskManager>()),
   m_tasksView(m_taskManager),
   m_encodingFinishAction(T_enEncodingFinishAction::doNothing),
   m_areTasksRunningPreviously(false)
{
}

LRESULT ClassicModeEncoderPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   // register object for idle updates
   CMessageLoop* pLoop = _Module.GetMessageLoop();
   ATLASSERT(pLoop != NULL);
   pLoop->AddIdleHandler(this);

   m_win7TaskBar = Win32::Taskbar(m_hWnd);

   SetupToolbar();
   SetupView();

   DlgResize_Init(false, false);

   m_tasksView.UpdateTasks();
   m_tasksView.SelectItem(0);
   OnClickedTaskItem(0);

   return 1;
}

LRESULT ClassicModeEncoderPage::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
   // unregister idle updates
   CMessageLoop* pLoop = _Module.GetMessageLoop();
   ATLASSERT(pLoop != NULL);
   pLoop->RemoveIdleHandler(this);

   bHandled = FALSE;
   return 1;
}

LRESULT ClassicModeEncoderPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
   if (!CheckRunningTasksAndQuit())
   {
      bHandled = true;
      return 1; // cancel leaving dialog
   }

   m_taskManager.StopAll();
   m_taskManager.RemoveCompletedTasks();

   return 0;
}

LRESULT ClassicModeEncoderPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
   if (!CheckRunningTasksAndQuit())
   {
      bHandled = true;
      return 1; // cancel leaving dialog
   }

   m_taskManager.StopAll();
   m_taskManager.RemoveCompletedTasks();

   return 0;
}

LRESULT ClassicModeEncoderPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
   if (!CheckRunningTasksAndQuit())
   {
      bHandled = true;
      return 1; // cancel leaving dialog
   }

   m_taskManager.StopAll();
   m_taskManager.RemoveCompletedTasks();

   // since all files are finished, go back to start page
   m_pageHost.SetWizardPage(std::make_shared<ClassicModeStartPage>(m_pageHost));

   return 0;
}

LRESULT ClassicModeEncoderPage::OnTasksStopAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_taskManager.StopAll();

   return 0;
}

LRESULT ClassicModeEncoderPage::OnTasksRemoveCompleted(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_taskManager.RemoveCompletedTasks();

   return 0;
}

BOOL ClassicModeEncoderPage::OnIdle()
{
   m_toolbar.EnableButton(ID_TASKS_STOP_ALL, m_taskManager.AreRunningTasksAvail());
   m_toolbar.EnableButton(ID_TASKS_REMOVE_COMPLETED, m_taskManager.AreCompletedTasksAvail());

   m_taskManager.CheckRunnableTasks();

   UpdateWin7TaskBar();

   CheckAllTasksFinished();

   return FALSE;
}

void ClassicModeEncoderPage::SetupToolbar()
{
   CRect toolbarRect;
   GetClientRect(toolbarRect);
   toolbarRect.bottom = toolbarRect.top + c_toolbarSize;

   m_toolbar.Create(m_hWnd, toolbarRect, nullptr,
      ATL_SIMPLE_TOOLBAR_PANE_STYLE | BTNS_SHOWTEXT | TBSTYLE_LIST);

   CImageList icons;

   icons.Create(16, 16, ILC_MASK | ILC_COLOR32, 0, 0);
   CBitmap bmpIcons;
   // load bitmap, but always from main module (bmp not in translation dlls)
   bmpIcons.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDR_MAINFRAME)));
   icons.Add(bmpIcons, RGB(255, 255, 255));

   m_toolbar.SetImageList(icons, 0);

   m_toolbar.AddButton(ID_TASKS_STOP_ALL, BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT, TBSTATE_ENABLED, 2, ID_TASKS_STOP_ALL, 0);
   m_toolbar.AddButton(ID_TASKS_REMOVE_COMPLETED, BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT, TBSTATE_ENABLED, 3, ID_TASKS_REMOVE_COMPLETED, 0);

   extern void EnableButtonText(CToolBarCtrl& tb, UINT uiId);

   EnableButtonText(m_toolbar, ID_TASKS_STOP_ALL);
   EnableButtonText(m_toolbar, ID_TASKS_REMOVE_COMPLETED);

   m_toolbar.SetDlgCtrlID(IDC_CLASSIC_ENCODE_PAGE_TOOLBAR);

   m_toolbar.AutoSize();
}

void ClassicModeEncoderPage::SetupView()
{
   CRect splitterRect;
   GetClientRect(splitterRect);
   splitterRect.top += c_toolbarSize;

   m_splitter.Create(*this, splitterRect, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0);
   m_splitter.SetDlgCtrlID(IDC_CLASSIC_ENCODE_PAGE_SPLITTER);

   m_tasksView.Create(m_splitter, rcDefault);
   m_tasksView.SetFont(AtlGetDefaultGuiFont());

   m_tasksView.Init(false);
   m_tasksView.SetClickedTaskHandler(std::bind(&ClassicModeEncoderPage::OnClickedTaskItem, this, std::placeholders::_1));

   m_paneTaskDetails.Create(m_splitter, IDS_MAIN_TASKS_PANE_CONTAINER);
   m_paneTaskDetails.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);

   m_taskDetailsView.Create(m_paneTaskDetails, rcDefault);

   m_paneTaskDetails.SetClient(m_taskDetailsView);

   m_splitter.SetSplitterPanes(m_tasksView, m_paneTaskDetails);
   m_splitter.SetSplitterExtendedStyle(SPLIT_BOTTOMALIGNED);
   m_splitter.SetActivePane(SPLIT_PANE_TOP);
   m_splitter.SetDefaultActivePane(SPLIT_PANE_TOP);

   CRect rectFrame;
   m_splitter.GetWindowRect(rectFrame);

   m_splitter.SetSplitterPos(rectFrame.Height() - 146);
}

bool ClassicModeEncoderPage::CheckRunningTasksAndQuit()
{
   // there may still be running tasks
   if (m_taskManager.AreRunningTasksAvail())
   {
      int iRet = AtlMessageBox(m_hWnd, IDS_MAIN_TASKS_STILL_RUNNING, IDS_APP_CAPTION, MB_YESNO | MB_ICONQUESTION);

      if (iRet == IDNO)
         return false;
   }

   return true;
}

void ClassicModeEncoderPage::UpdateWin7TaskBar()
{
   if (!m_win7TaskBar.is_initialized() ||
      !m_win7TaskBar.get().IsAvailable())
      return;

   bool hasActiveTasks = false;
   bool hasCompletedTasks = m_taskManager.AreCompletedTasksAvail();
   bool hasErrorTasks = false;
   unsigned int percentComplete = 0;

   m_taskManager.GetTaskListState(hasActiveTasks, hasErrorTasks, percentComplete);

   if (hasActiveTasks || hasCompletedTasks)
   {
      if (!m_win7TaskBarProgressBar.is_initialized())
         m_win7TaskBarProgressBar = m_win7TaskBar.get().OpenProgressBar();

      m_win7TaskBarProgressBar.get().SetState(
         hasErrorTasks ? Win32::TaskbarProgressBar::TBPF_ERROR : Win32::TaskbarProgressBar::TBPF_NORMAL);

      m_win7TaskBarProgressBar.get().SetPos(percentComplete, 100);
   }
   else
      m_win7TaskBarProgressBar.reset();
}

void ClassicModeEncoderPage::OnClickedTaskItem(size_t clickedIndex)
{
   std::vector<TaskInfo> taskInfoList = m_taskManager.CurrentTasks();
   if (clickedIndex >= taskInfoList.size())
   {
      // index has become invalid between clicking and getting task info list
      m_taskDetailsView.ResetTaskDetails();
      return;
   }

   m_taskDetailsView.UpdateTaskDetails(taskInfoList[clickedIndex]);
}

void ClassicModeEncoderPage::CheckAllTasksFinished()
{
   bool areTasksRunning = m_taskManager.AreRunningTasksAvail();

   if (!areTasksRunning &&
      m_areTasksRunningPreviously &&
      m_encodingFinishAction != T_enEncodingFinishAction::doNothing)
   {
      switch (m_encodingFinishAction)
      {
      case T_enEncodingFinishAction::closeApp:
         PostMessage(WM_CLOSE);
         break;

      case T_enEncodingFinishAction::standbyPC:
         ::SetSystemPowerState(TRUE, FALSE);
         PostMessage(WM_CLOSE);
         break;

      default:
         ATLASSERT(false);
         break;
      }
   }

   m_areTasksRunningPreviously = areTasksRunning;
}
