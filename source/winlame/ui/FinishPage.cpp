//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2014 Michael Fink
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
/// \file FinishPage.cpp
/// \brief Finish page
//
#include "StdAfx.h"
#include "FinishPage.hpp"
#include "WizardPageHost.hpp"
#include "IoCContainer.hpp"
#include "PresetSelectionPage.hpp"
#include "OutputSettingsPage.hpp"
#include "UISettings.h"
#include "TaskManager.h"
#include "EncoderTask.h"
#include "CreatePlaylistTask.hpp"
#include "CDExtractTask.hpp"
#include "CDRipTrackManager.h"

using namespace UI;

FinishPage::FinishPage(WizardPageHost& pageHost) throw()
:WizardPage(pageHost, IDD_PAGE_FINISH, WizardPage::typeCancelBackFinish),
m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
{
}

LRESULT FinishPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);


   return 1;
}

LRESULT FinishPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   AddTasks();

   // TODO clear encoderjoblist

   return 0;
}

LRESULT FinishPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (m_uiSettings.preset_avail && m_uiSettings.m_iLastSelectedPresetIndex > 0)
   {
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   }
   else
   {
      ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
      int modid = moduleManager.getOutputModuleID(m_uiSettings.output_module);

      OutputSettingsPage::SetWizardPageByOutputModule(m_pageHost, modid);
   }

   return 0;
}

void FinishPage::AddTasks()
{
   // create playlist?
   if (m_uiSettings.create_playlist)
      AddPlaylistTask();

   if (m_uiSettings.m_bFromInputFilesPage)
      AddInputFilesTasks();
   else
      AddCDExtractTasks();
}

void FinishPage::AddInputFilesTasks()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
   {
      EncoderJob& job = m_uiSettings.encoderjoblist[i];

      EncoderTaskSettings taskSettings;

      taskSettings.m_cszInputFilename = job.InputFilename();
      taskSettings.m_cszOutputPath = m_uiSettings.m_defaultSettings.outputdir;
      taskSettings.m_cszTitle = Path(job.InputFilename()).FilenameAndExt();

      ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
      taskSettings.m_iOutputModuleId = moduleManager.getOutputModuleID(m_uiSettings.output_module);

      taskSettings.m_settingsManager = m_uiSettings.settings_manager;
      taskSettings.m_trackInfo = job.GetTrackInfo();
      taskSettings.m_pModuleManager = &moduleManager;
      taskSettings.m_bOverwriteFiles = m_uiSettings.m_defaultSettings.overwrite_existing;
      taskSettings.m_bDeleteAfterEncode = m_uiSettings.m_defaultSettings.delete_after_encode;

      std::shared_ptr<EncoderTask> spTask(new EncoderTask(taskSettings));

      taskMgr.AddTask(spTask);
   }
}

void FinishPage::AddCDExtractTasks()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   CDRipTrackManager& ripTrackMgr = *CDRipTrackManager::getCDRipTrackManager();

   const CDRipDiscInfo& discInfo = ripTrackMgr.GetDiscInfo();

   unsigned int uiMax = ripTrackMgr.GetMaxTrackInfo();

   unsigned int uiLastTaskId = 0;

   for (unsigned int ui = 0; ui < uiMax; ui++)
   {
      const CDRipTrackInfo& trackInfo = ripTrackMgr.GetTrackInfo(ui);

      if (!trackInfo.m_bActive)
         continue;

      std::shared_ptr<CDExtractTask> spTask(new CDExtractTask(discInfo, trackInfo));

      taskMgr.AddTask(spTask);

      uiLastTaskId = spTask->GetTaskInfo().Id();
   }
}

void FinishPage::AddPlaylistTask()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   std::shared_ptr<Task> spTask;
   if (m_uiSettings.m_bFromInputFilesPage)
      spTask.reset(new CreatePlaylistTask(m_uiSettings.playlist_filename, m_uiSettings.encoderjoblist));
   else
      spTask.reset(new CreatePlaylistTask(m_uiSettings.playlist_filename, m_uiSettings.cdreadjoblist));

   taskMgr.AddTask(spTask);
}
