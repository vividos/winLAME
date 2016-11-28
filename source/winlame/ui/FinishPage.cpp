//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
#include "CDRipTitleFormatManager.hpp"

using namespace UI;

FinishPage::FinishPage(WizardPageHost& pageHost) throw()
   :WizardPage(pageHost, IDD_PAGE_FINISH, WizardPage::typeCancelBackFinish),
   m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
   m_lastTaskId(0)
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

   m_uiSettings.encoderjoblist.clear();
   m_uiSettings.cdreadjoblist.clear();

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
   if (m_uiSettings.m_bFromInputFilesPage)
      AddInputFilesTasks();
   else
      AddCDExtractTasks();

   if (m_uiSettings.create_playlist)
      AddPlaylistTask();
}

void FinishPage::AddInputFilesTasks()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   m_lastTaskId = 0;

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

      // set previous task id when encoding with LAME and using nogap encoding
      unsigned int dependentTaskId = 0;
      if (taskSettings.m_iOutputModuleId == ID_OM_LAME &&
         taskSettings.m_settingsManager.queryValueInt(LameOptNoGap) == 1)
      {
         dependentTaskId = m_lastTaskId;
      }

      std::shared_ptr<EncoderTask> spTask(new EncoderTask(dependentTaskId, taskSettings));

      taskMgr.AddTask(spTask);

      job.OutputFilename(spTask->OutputFilename());

      m_lastTaskId = spTask->Id();
   }
}

void FinishPage::AddCDExtractTasks()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   unsigned int lastCDReadTaskId = 0;

   unsigned int maxJobIndex = m_uiSettings.cdreadjoblist.size();
   for (unsigned int jobIndex = 0; jobIndex < maxJobIndex; jobIndex++)
   {
      CDReadJob& cdReadJob = m_uiSettings.cdreadjoblist[jobIndex];

      const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();
      const CDRipTrackInfo& trackInfo = cdReadJob.TrackInfo();

      if (!trackInfo.m_bActive)
         continue;

      std::shared_ptr<CDExtractTask> spCDExtractTask(new CDExtractTask(lastCDReadTaskId, discInfo, trackInfo));
      taskMgr.AddTask(spCDExtractTask);

      cdReadJob.OutputFilename(spCDExtractTask->OutputFilename());

      unsigned int cdReadTaskId = spCDExtractTask->Id();
      lastCDReadTaskId = cdReadTaskId;

      // also add encode task
      std::shared_ptr<EncoderTask> spEncoderTask =
         CreateEncoderTaskForCDReadJob(cdReadTaskId, cdReadJob);

      taskMgr.AddTask(spEncoderTask);

      m_lastTaskId = spEncoderTask->Id();
   }
}

std::shared_ptr<EncoderTask> FinishPage::CreateEncoderTaskForCDReadJob(unsigned int cdReadTaskId, const CDReadJob& cdReadJob)
{
   const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();
   const CDRipTrackInfo& trackInfo = cdReadJob.TrackInfo();

   EncoderTaskSettings taskSettings;

   taskSettings.m_cszInputFilename = cdReadJob.OutputFilename();
   taskSettings.m_cszOutputPath = m_uiSettings.m_defaultSettings.outputdir;

   taskSettings.m_cszTitle = CDRipTitleFormatManager::FormatTitle(
      discInfo.m_bVariousArtists ? m_uiSettings.cdrip_format_various_track : m_uiSettings.cdrip_format_album_track,
      discInfo, trackInfo);

   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
   taskSettings.m_iOutputModuleId = moduleManager.getOutputModuleID(m_uiSettings.output_module);

   taskSettings.m_settingsManager = m_uiSettings.settings_manager;

   TrackInfo encodeTrackInfo;
   SetTrackInfoFromCDTrackInfo(encodeTrackInfo, cdReadJob);

   taskSettings.m_trackInfo = encodeTrackInfo;
   taskSettings.m_pModuleManager = &moduleManager;
   taskSettings.m_bOverwriteFiles = m_uiSettings.m_defaultSettings.overwrite_existing;
   taskSettings.m_bDeleteAfterEncode = true; // temporary file created by CDExtractTask

   return std::make_shared<EncoderTask>(cdReadTaskId, taskSettings);
}

void FinishPage::SetTrackInfoFromCDTrackInfo(TrackInfo& encodeTrackInfo, const CDReadJob& cdReadJob)
{
   const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();
   const CDRipTrackInfo& cdTrackInfo = cdReadJob.TrackInfo();

   // add track info
   encodeTrackInfo.TextInfo(TrackInfoTitle, cdTrackInfo.m_cszTrackTitle);

   CString value = discInfo.m_cszDiscArtist;
   if (discInfo.m_bVariousArtists)
      value.LoadString(IDS_CDRIP_ARTIST_VARIOUS);

   encodeTrackInfo.TextInfo(TrackInfoArtist, value);

   encodeTrackInfo.TextInfo(TrackInfoAlbum, discInfo.m_cszDiscTitle);

   // year
   if (discInfo.m_nYear != 0)
      encodeTrackInfo.NumberInfo(TrackInfoYear, discInfo.m_nYear);

   // track number
   encodeTrackInfo.NumberInfo(TrackInfoTrack, cdTrackInfo.m_nTrackOnDisc + 1);

   // genre
   if (!discInfo.m_cszGenre.IsEmpty())
      encodeTrackInfo.TextInfo(TrackInfoGenre, discInfo.m_cszGenre);
}

void FinishPage::AddPlaylistTask()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   std::shared_ptr<Task> spTask;
   if (m_uiSettings.m_bFromInputFilesPage)
      spTask.reset(new CreatePlaylistTask(m_lastTaskId, m_uiSettings.playlist_filename, m_uiSettings.encoderjoblist));
   else
      spTask.reset(new CreatePlaylistTask(m_lastTaskId, m_uiSettings.playlist_filename, m_uiSettings.cdreadjoblist));

   taskMgr.AddTask(spTask);
}
