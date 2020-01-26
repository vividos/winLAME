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
/// \file MainFrame.cpp
/// \brief Main frame window
//
#include "stdafx.h"
#include "resource.h"
#include "MainFrame.hpp"
#include "App.hpp"
#include "AboutDlg.hpp"
#include "TasksView.hpp"
#include "TaskManager.hpp"
#include "res/MainFrameRibbon.h"
#include "WizardPageHost.hpp"
#include "GeneralSettingsPage.hpp"
#include "CDReadSettingsPage.hpp"
#include "InputFilesPage.hpp"
#include "InputCDPage.hpp"
#include "ResourceInstanceSwitcher.hpp"
#include "DropFilesManager.hpp"
#include <ulib/CommandLineParser.hpp>
#include <boost/foreach.hpp>

using namespace UI;

/// tasks list refresh cycle in ms
const UINT c_uiTasksListRefreshCycleInMs = 500;

/// ribbon registry key (subkey "Ribbon" is used)
LPCTSTR c_pszRibbonRegkey = _T("Software\\winLAME");

/// URL for positive feedback
LPCTSTR c_urlFeedbackPositive = _T("https://winlame.sourceforge.io/feedback_positive.html");

/// URL for negative feedback
LPCTSTR c_urlFeedbackNegative = _T("https://winlame.sourceforge.io/feedback_negative.html");


BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
   if (m_tasksView.PreTranslateMessage(pMsg))
      return TRUE;

   return BaseClass::PreTranslateMessage(pMsg);
}

BOOL MainFrame::OnIdle()
{
   UIEnable(ID_ENCODE_CD, !m_taskManager.AreCDExtractTasksRunning());
   UIEnable(ID_TASKS_STOP_ALL, m_taskManager.AreRunningTasksAvail());
   UIEnable(ID_TASKS_REMOVE_COMPLETED, m_taskManager.AreCompletedTasksAvail());

   m_taskManager.CheckRunnableTasks();

   UpdateWin7TaskBar();

   CheckAllTasksFinished();

   return FALSE;
}

/// enables toolbar button text
void EnableButtonText(CToolBarCtrl& tb, UINT uiId)
{
   // load second part of text (the shorter one)
   CString cszText;
   cszText.LoadString(uiId);
   int iPos = cszText.ReverseFind(_T('\n'));
   if (iPos != -1)
      cszText = cszText.Mid(iPos+1);

   // get infos
   TBBUTTONINFO tbbi = {0};
   tbbi.cbSize = sizeof(tbbi);
   tbbi.dwMask = TBIF_STYLE;

   tb.GetButtonInfo(uiId, &tbbi);

   // set new infos
   tbbi.dwMask |= TBIF_TEXT;
   tbbi.fsStyle |= BTNS_SHOWTEXT;
   tbbi.pszText = (LPTSTR)static_cast<LPCTSTR>(cszText);
   tbbi.cchText = cszText.GetLength();

   tb.SetButtonInfo(uiId, &tbbi);
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   SetupCmdBar();
   if (!SetupRibbonBar())
      return -1;

   CreateSimpleStatusBar();
   SetupView();

   // set caption
   CString cszCaption;
   cszCaption.LoadString(IDS_APP_CAPTION);
   cszCaption += _T(" ") + App::Version();
   SetWindowText(cszCaption);

   // register object for message filtering and idle updates
   CMessageLoop* pLoop = _Module.GetMessageLoop();
   ATLASSERT(pLoop != NULL);
   pLoop->AddMessageFilter(this);
   pLoop->AddIdleHandler(this);

   ShowRibbonUI(true);

   UISetCheck(ID_SETTINGS_FINISH_ACTION_NONE + m_encodingFinishAction, true);
   m_cbSettingsFinishAction.Select(m_encodingFinishAction);

   // enable dropping files
   DragAcceptFiles(TRUE);

   m_win7TaskBar = Win32::Taskbar(m_hWnd);

   PostMessage(WM_CHECK_COMMAND_LINE);

   // check if help is available
   if (App::Current().IsHelpAvailable())
      m_htmlHelper.Init(m_hWnd, App::Current().HelpFilename());
   else
      m_CmdBar.GetMenu().RemoveMenu(ID_HELP, MF_BYCOMMAND);

   return 0;
}

void MainFrame::SetupCmdBar()
{
   CMenuHandle menu = GetMenu();

   // remove feedback buttons in release
   if (App::Version().Find(_T("release")) != -1)
   {
      menu.DeleteMenu(ID_FEEDBACK_POSITIVE, MF_BYCOMMAND);
      menu.DeleteMenu(ID_FEEDBACK_NEGATIVE, MF_BYCOMMAND);

      menu.DeleteMenu(menu.GetMenuItemCount() - 2, MF_BYPOSITION);
   }

   // create the command bar
   m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);

   m_CmdBar.AttachMenu(GetMenu());

   // remove old menu
   SetMenu(nullptr);
}

bool MainFrame::SetupRibbonBar()
{
   // check if ribbon is available
   bool ribbonUI = RunTimeHelper::IsRibbonUIAvailable();

   if (!ribbonUI)
   {
      AtlMessageBox(m_hWnd,
         _T("This Windows version doesn't support ribbon UI; quitting winLAME"),
         IDS_APP_CAPTION, MB_OK | MB_ICONEXCLAMATION);

      PostQuitMessage(0);
      return false;
   }

   UIAddMenu(m_CmdBar.GetMenu(), true);

   CRibbonPersist(c_pszRibbonRegkey).Restore(ribbonUI, m_hgRibbonSettings);

   return true;
}

void MainFrame::SetupView()
{
   m_splitter.Create(*this, rcDefault);

   m_tasksView.Create(m_splitter, rcDefault);
   m_tasksView.SetFont(AtlGetDefaultGuiFont());

   m_tasksView.Init();
   m_tasksView.SetClickedTaskHandler(std::bind(&MainFrame::OnClickedTaskItem, this, std::placeholders::_1));

   m_paneTaskDetails.Create(m_splitter, IDS_MAIN_TASKS_PANE_CONTAINER);
   m_paneTaskDetails.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);

   m_taskDetailsView.Create(m_paneTaskDetails, rcDefault);

   m_paneTaskDetails.SetClient(m_taskDetailsView);

   m_splitter.SetSplitterPanes(m_tasksView, m_paneTaskDetails);
   m_splitter.SetSplitterExtendedStyle(SPLIT_BOTTOMALIGNED);
   m_splitter.SetActivePane(SPLIT_PANE_TOP);
   m_splitter.SetDefaultActivePane(SPLIT_PANE_TOP);

   m_hWndClient = m_splitter;
   UpdateLayout();

   CRect rectFrame;
   m_splitter.GetWindowRect(rectFrame);

   m_splitter.SetSplitterPos(rectFrame.Height() - 146);
}

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
   bHandled = false;

   // there may still be running tasks
   if (m_taskManager.AreRunningTasksAvail())
   {
      int iRet = AtlMessageBox(m_hWnd, IDS_MAIN_TASKS_STILL_RUNNING, IDS_APP_CAPTION, MB_YESNO | MB_ICONQUESTION);

      if (iRet == IDNO)
      {
         bHandled = true;
         m_isAppModeChanged = false;
         return 0;
      }
   }

   if (RunTimeHelper::IsRibbonUIAvailable())
   {
      bool bRibbonUI = IsRibbonUI();
      if (bRibbonUI)
         SaveRibbonSettings();

      CRibbonPersist(c_pszRibbonRegkey).Save(bRibbonUI, m_hgRibbonSettings);
   }

   return 0;
}

LRESULT MainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
   // unregister message filtering and idle updates
   CMessageLoop* pLoop = _Module.GetMessageLoop();
   ATLASSERT(pLoop != NULL);
   pLoop->RemoveMessageFilter(this);
   pLoop->RemoveIdleHandler(this);

   bHandled = FALSE;
   return 1;
}

LRESULT MainFrame::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DropFilesManager dropMgr((HDROP)wParam);

   bool cdaFiles = false;
   const std::vector<CString>& filenamesList = dropMgr.Filenames();

   for (auto filename : filenamesList)
   {
      if (filename.Find(_T(".cda")) != -1)
      {
         cdaFiles = true;
         break;
      }
   }

   // show input files or input cd page
   WizardPageHost host;

   if (cdaFiles)
      host.SetWizardPage(std::make_shared<InputCDPage>(host));
   else
      host.SetWizardPage(std::make_shared<InputFilesPage>(host, filenamesList));

   host.Run(m_hWnd);

   return 0;
}

LRESULT MainFrame::OnCheckCommandLine(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   App& app = App::Current();

   if (app.StartInputCD())
   {
      if (app.StartInputCD())
      {
         app.ResetStartInputCD();
         PostMessage(WM_COMMAND, MAKEWPARAM(ID_ENCODE_CD, BN_CLICKED));
      }
   }

   if (!app.AlreadyReadCommandLine())
   {
      GetCommandLineFiles();
      app.SetAlreadyReadCommandLine();
   }

   return 0;
}

LRESULT MainFrame::OnAppExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   PostMessage(WM_CLOSE);

   return 0;
}

LRESULT MainFrame::OnEncodeFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   // open files
   std::vector<CString> vecFilenames;
   if (!InputFilesPage::OpenFileDialog(m_hWnd, vecFilenames))
      return 0;

   // show input files page
   WizardPageHost host;
   host.SetWizardPage(std::shared_ptr<WizardPage>(new InputFilesPage(host, vecFilenames)));
   host.Run(m_hWnd);

   return 0;
}

LRESULT MainFrame::OnEncodeCD(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   // show input cd page
   WizardPageHost host;
   host.SetWizardPage(std::shared_ptr<WizardPage>(new InputCDPage(host)));
   host.Run(m_hWnd);

   return 0;
}

LRESULT MainFrame::OnTasksStopAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_taskManager.StopAll();

   return 0;
}

LRESULT MainFrame::OnTasksRemoveCompleted(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_taskManager.RemoveCompletedTasks();

   return 0;
}

LRESULT MainFrame::OnSettingsGeneral(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   WizardPageHost host;
   host.SetWizardPage(std::shared_ptr<WizardPage>(
      new GeneralSettingsPage(host,
         IoCContainer::Current().Resolve<UISettings>(),
         IoCContainer::Current().Resolve<LanguageResourceManager>())));
   host.Run(m_hWnd);

   return 0;
}

LRESULT MainFrame::OnSettingsCDRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   WizardPageHost host;
   host.SetWizardPage(std::shared_ptr<WizardPage>(
      new CDReadSettingsPage(host,
         IoCContainer::Current().Resolve<UISettings>())));
   host.Run(m_hWnd);

   return 0;
}

LRESULT MainFrame::OnSettingsFinishActionSelChanged(UI_EXECUTIONVERB verb, WORD /*wID*/, UINT uSel, BOOL& /*bHandled*/)
{
   if (verb == UI_EXECUTIONVERB_EXECUTE &&
      uSel != UI_COLLECTION_INVALIDINDEX)
   {
      m_encodingFinishAction = static_cast<T_enEncodingFinishAction>(uSel);

      UISetCheck(ID_SETTINGS_FINISH_ACTION_NONE, false);
      UISetCheck(ID_SETTINGS_FINISH_ACTION_CLOSE, false);
      UISetCheck(ID_SETTINGS_FINISH_ACTION_STANDBY, false);

      UISetCheck(ID_SETTINGS_FINISH_ACTION_NONE + uSel, true);
      m_cbSettingsFinishAction.Select(uSel);
   }

   return 0;
}

LRESULT MainFrame::OnSettingsFinishActionRange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_encodingFinishAction = static_cast<T_enEncodingFinishAction>(wID - ID_SETTINGS_FINISH_ACTION_NONE);

   UISetCheck(ID_SETTINGS_FINISH_ACTION_NONE, false);
   UISetCheck(ID_SETTINGS_FINISH_ACTION_CLOSE, false);
   UISetCheck(ID_SETTINGS_FINISH_ACTION_STANDBY, false);

   UISetCheck(wID, true);
   m_cbSettingsFinishAction.Select(wID - ID_SETTINGS_FINISH_ACTION_NONE);

   return 0;
}

LRESULT MainFrame::OnViewSwitchToClassic(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_isAppModeChanged = true;

   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();

   settings.m_appMode = UISettings::classicMode;
   settings.StoreSettings();

   PostMessage(WM_CLOSE);

   return 0;
}

LRESULT MainFrame::OnFeedbackPositive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   ShellExecute(m_hWnd, _T("open"), c_urlFeedbackPositive, nullptr, nullptr, SW_SHOWNORMAL);
   return 0;
}

LRESULT MainFrame::OnFeedbackNegative(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   ShellExecute(m_hWnd, _T("open"), c_urlFeedbackNegative, nullptr, nullptr, SW_SHOWNORMAL);
   return 0;
}

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   AboutDlg dlg;

   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
   dlg.SetPresetsXmlFilename(settings.presets_filename);

   dlg.DoModal();
   return 0;
}

LRESULT MainFrame::OnHelpCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (!App::Current().IsHelpAvailable())
      return 0;

   m_htmlHelper.DisplayTopic(_T("/html/pages/modernui.html"));

   return 0;
}

LRESULT MainFrame::OnHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
   // same as pressing the help icon
   return OnHelpCommand(0, 0, 0, bHandled);
}

void MainFrame::GetCommandLineFiles()
{
   std::vector<CString> filenames;

   CommandLineParser parser(::GetCommandLine());

   // skip first string; it's the program's name
   CString param;
   parser.GetNext(param);

   while (parser.GetNext(param))
   {
      filenames.push_back(param);
   }

   // show input files page
   if (!filenames.empty())
   {
      WizardPageHost host;
      host.SetWizardPage(std::shared_ptr<WizardPage>(new InputFilesPage(host, filenames)));
      host.Run(m_hWnd);
   }
}

void MainFrame::UpdateWin7TaskBar()
{
   if (!m_win7TaskBar.has_value() ||
      !m_win7TaskBar.value().IsAvailable())
      return;

   bool hasActiveTasks = false;
   bool hasCompletedTasks = m_taskManager.AreCompletedTasksAvail();
   bool hasErrorTasks = false;
   unsigned int percentComplete = 0;

   m_taskManager.GetTaskListState(hasActiveTasks, hasErrorTasks, percentComplete);

   if (hasActiveTasks || hasCompletedTasks)
   {
      if (!m_win7TaskBarProgressBar.has_value())
         m_win7TaskBarProgressBar.emplace(m_win7TaskBar.value().OpenProgressBar());

      m_win7TaskBarProgressBar.value().SetState(
         hasErrorTasks ? Win32::TaskbarProgressBar::TBPF_ERROR : Win32::TaskbarProgressBar::TBPF_NORMAL);

      m_win7TaskBarProgressBar.value().SetPos(percentComplete, 100);
   }
   else
      m_win7TaskBarProgressBar.reset();
}

void MainFrame::OnClickedTaskItem(size_t clickedIndex)
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


void MainFrame::CheckAllTasksFinished()
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
