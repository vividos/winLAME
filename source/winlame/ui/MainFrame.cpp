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
/// \file MainFrame.cpp
/// \brief Main frame window
//
#include "stdafx.h"
#include "resource.h"
#include "MainFrame.hpp"
#include "App.h"
#include "AboutDlg.hpp"
#include "TasksView.hpp"
#include "TaskManager.h"
#include "res/MainFrameRibbon.h"
#include "WizardPageHost.hpp"
#include "GeneralSettingsPage.hpp"
#include "CDReadSettingsPage.hpp"
#include "InputFilesPage.hpp"
#include "InputCDPage.hpp"
#include "ResourceInstanceSwitcher.hpp"
#include "DropFilesManager.hpp"
#include "CommandLineParser.hpp"
#include <boost/foreach.hpp>

using namespace UI;

/// tasks list refresh cycle in ms
const UINT c_uiTasksListRefreshCycleInMs = 500;

/// timer id for tasks list refresh
const UINT IDT_REFRESH_TASKS_LIST = 128;

/// ribbon registry key (subkey "Ribbon" is used)
LPCTSTR c_pszRibbonRegkey = _T("Software\\winLAME");


BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
   if (m_view.PreTranslateMessage(pMsg))
      return TRUE;

   return BaseClass::PreTranslateMessage(pMsg);
}

BOOL MainFrame::OnIdle()
{
   UIUpdateToolBar();

   UIEnable(ID_TASKS_STOP_ALL, m_taskManager.AreRunningTasksAvail());
   UIEnable(ID_TASKS_REMOVE_COMPLETED, m_taskManager.AreCompletedTasksAvail());

   m_taskManager.CheckRunnableTasks();

   UpdateWin7TaskBar();

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
   tbbi.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(cszText));
   tbbi.cchText = cszText.GetLength();

   tb.SetButtonInfo(uiId, &tbbi);
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   // create command bar window
   HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
   // attach menu
   m_CmdBar.AttachMenu(GetMenu());
   // load command bar images
   m_CmdBar.LoadImages(IDR_MAINFRAME);
   // remove old menu
   SetMenu(NULL);

   // set caption
   CString cszCaption;
   cszCaption.LoadString(IDS_APP_CAPTION);
   cszCaption += _T(" ") + App::Version();
   SetWindowText(cszCaption);

   // check if ribbon is available
   bool bRibbonUI = RunTimeHelper::IsRibbonUIAvailable();

   if (bRibbonUI)
   {
      UIAddMenu(m_CmdBar.GetMenu(), true);

      CRibbonPersist(c_pszRibbonRegkey).Restore(bRibbonUI, m_hgRibbonSettings);
   }
   else
      CMenuHandle(m_CmdBar.GetMenu()).DeleteMenu(ID_VIEW_RIBBON, MF_BYCOMMAND);

   // toolbar setup
   {
      HWND hWndToolBar;
      {
         // switch to module handle, since toolbar isn't in translated language dll
         ResourceInstanceSwitcher sw(::_Module.GetModuleInstance());

         hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE,
            ATL_SIMPLE_TOOLBAR_PANE_STYLE | BTNS_SHOWTEXT | TBSTYLE_LIST);
      }

      CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
      AddSimpleReBarBand(hWndCmdBar);
      AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

      CToolBarCtrl tb(hWndToolBar);
      tb.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

      EnableButtonText(tb, ID_ENCODE_FILES);
      EnableButtonText(tb, ID_ENCODE_CD);

      if (!InputCDPage::IsCDExtractionAvail())
         UIEnable(ID_ENCODE_CD, FALSE);

      UIAddToolBar(hWndToolBar);
   }

   CreateSimpleStatusBar();
/* TODO status bar panes
   m_statusBar.SubclassWindow(m_hWndStatusBar);
   int arrParts[] =
   {
      ID_DEFAULT_PANE,
      ID_ROW_PANE,
      ID_COL_PANE
   };
   m_statusBar.SetPanes(arrParts, sizeof(arrParts) / sizeof(int), false);
*/

   m_hWndClient = m_view.Create(m_hWnd, rcDefault);
   m_view.SetFont(AtlGetDefaultGuiFont());
   m_view.Init();

   // register object for message filtering and idle updates
   CMessageLoop* pLoop = _Module.GetMessageLoop();
   ATLASSERT(pLoop != NULL);
   pLoop->AddMessageFilter(this);
   pLoop->AddIdleHandler(this);

   ShowRibbonUI(bRibbonUI);
   UISetCheck(ID_VIEW_RIBBON, bRibbonUI);

   // enable dropping files
   DragAcceptFiles(TRUE);

   m_win7TaskBar = Win7::Taskbar(m_hWnd);

   PostMessage(WM_CHECK_COMMAND_LINE);

   return 0;
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

   EnableRefresh(false);

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

LRESULT MainFrame::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if (wParam == IDT_REFRESH_TASKS_LIST)
   {
      m_view.UpdateTasks();
   }

   return 0;
}

LRESULT MainFrame::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DropFilesManager dropMgr((HDROP)wParam);

   // show input files page
   WizardPageHost host;
   host.SetWizardPage(std::shared_ptr<WizardPage>(new InputFilesPage(host, dropMgr.Filenames())));
   host.Run(m_hWnd);

   return 0;
}

LRESULT MainFrame::OnCheckCommandLine(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if (!App::Current().AlreadyReadCommandLine())
   {
      GetCommandLineFiles();
      App::Current().SetAlreadyReadCommandLine();
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

LRESULT MainFrame::OnToggleRibbon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) 
{
    ShowRibbonUI(!IsRibbonUI());
    UISetCheck(ID_VIEW_RIBBON, IsRibbonUI());
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

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   AboutDlg dlg;

   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
   dlg.SetPresetsXmlFilename(settings.presets_filename);

   dlg.DoModal();
   return 0;
}

void MainFrame::EnableRefresh(bool bEnable)
{
   if (bEnable)
   {
      if (!m_bRefreshActive)
      {
         m_bRefreshActive = true;
         SetTimer(IDT_REFRESH_TASKS_LIST, c_uiTasksListRefreshCycleInMs);
      }
   }
   else
   {
      if (m_bRefreshActive)
      {
         m_bRefreshActive = false;
         KillTimer(IDT_REFRESH_TASKS_LIST);
      }
   }
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
         hasErrorTasks ? Win7::TaskbarProgressBar::TBPF_ERROR : Win7::TaskbarProgressBar::TBPF_NORMAL);

      m_win7TaskBarProgressBar.get().SetPos(percentComplete, 100);
   }
   else
      m_win7TaskBarProgressBar.reset();
}
