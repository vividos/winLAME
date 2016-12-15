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
#include "RedrawLock.hpp"
#include <sndfile.h>

using namespace UI;

/// moves (or scales) up a window in Y direction
static void MoveUpWindow(CWindow& window, int deltaY, bool scaleUp)
{
   CRect rect;
   window.GetWindowRect(rect);

   rect.top -= deltaY;
   if (!scaleUp)
      rect.bottom -= deltaY;

   window.GetParent().ScreenToClient(rect);

   window.MoveWindow(rect);
}

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

   m_iconLossy.SetIcon(LoadIcon(NULL, IDI_EXCLAMATION));
   m_iconOverwrite.SetIcon(LoadIcon(NULL, IDI_EXCLAMATION));

   bool warnLossyTranscoding = IsTranscodingLossy();
   bool warnOverwriteOriginal = IsOverwritingOriginalFiles();

   MoveAndHideWarnings(warnLossyTranscoding, warnOverwriteOriginal);

   SetupInputTracksList();
   UpdateInputTracksList();

   UpdateOutputModule();

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

bool FinishPage::IsTranscodingLossy() const
{
   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();

   bool in_lossy = false;

   // only have to check when from input page; CD reading is always lossless
   if (m_uiSettings.m_bFromInputFilesPage)
   {
      ModuleManagerImpl& modImpl = reinterpret_cast<ModuleManagerImpl&>(moduleManager);

      for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
      {
         EncoderJob& job = m_uiSettings.encoderjoblist[i];

         CString filename = job.InputFilename();

         std::unique_ptr<InputModule> inmod(modImpl.chooseInputModule(filename));
         if (inmod == nullptr)
            continue;

         int in_id = inmod->getModuleID();

         in_lossy |= EncoderImpl::IsLossyInputModule(in_id);
      }
   }

   int out_id = moduleManager.getOutputModuleID(m_uiSettings.output_module);

   bool out_lossy = EncoderImpl::IsLossyOutputModule(out_id);

   return in_lossy && out_lossy;
}

bool FinishPage::IsOverwritingOriginalFiles() const
{
   // overwriting doesn't happen when encoding CD tracks
   if (!m_uiSettings.m_bFromInputFilesPage)
      return false;

   // overwriting doesn't happen when flag "overwrite existing" isn't active
   if (!m_uiSettings.m_defaultSettings.overwrite_existing)
      return false;

   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
   ModuleManagerImpl& modImpl = reinterpret_cast<ModuleManagerImpl&>(moduleManager);

   int out_module_id = moduleManager.getOutputModuleID(m_uiSettings.output_module);

   for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
   {
      EncoderJob& job = m_uiSettings.encoderjoblist[i];

      CString inputFilename = job.InputFilename();

      std::unique_ptr<OutputModule> outputModule(modImpl.getOutputModule(out_module_id));
      if (outputModule == nullptr)
         continue;

      outputModule->prepareOutput(m_uiSettings.settings_manager);

      CString outputFilename = EncoderImpl::GetOutputFilename(m_uiSettings.m_defaultSettings.outputdir, inputFilename, *outputModule.get());

      if (outputFilename.CompareNoCase(inputFilename) == 0)
      {
         return true;
      }
   }

   return false;
}

void FinishPage::MoveAndHideWarnings(bool warnLossyTranscoding, bool warnOverwriteOriginal)
{
   int deltaInputTracks = 0;

   if (!warnLossyTranscoding)
   {
      m_iconLossy.ShowWindow(SW_HIDE);
      m_staticLossy.ShowWindow(SW_HIDE);

      CRect lossyRect, overwriteRect;
      m_iconLossy.GetWindowRect(lossyRect);
      m_iconOverwrite.GetWindowRect(overwriteRect);

      deltaInputTracks += overwriteRect.top - lossyRect.top;
   }

   if (!warnOverwriteOriginal)
   {
      m_iconOverwrite.ShowWindow(SW_HIDE);
      m_staticOverwrite.ShowWindow(SW_HIDE);

      CRect overwriteRect, bevelRect;
      m_iconOverwrite.GetWindowRect(overwriteRect);
      m_bevel1.GetWindowRect(bevelRect);

      deltaInputTracks += bevelRect.top - overwriteRect.top;
   }
   else
   {
      if (!warnLossyTranscoding)
      {
         MoveUpWindow(m_iconOverwrite, deltaInputTracks, false);
         MoveUpWindow(m_staticOverwrite, deltaInputTracks, false);
      }
   }

   if (deltaInputTracks > 0)
   {
      MoveUpWindow(m_bevel1, deltaInputTracks, false);
      MoveUpWindow(m_listInputTracks, deltaInputTracks, true);
   }
}

void FinishPage::SetupInputTracksList()
{
   m_listInputTracks.InsertColumn(0, _T("Track"));

   // task images
   m_taskImages.Create(16, 16, ILC_MASK | ILC_COLOR32, 0, 0);
   CBitmap bmpImages;
   // load bitmap, but always from main module (bmp not in translation dlls)
   bmpImages.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP_TASKS)));
   m_taskImages.Add(bmpImages, RGB(0, 0, 0));

   m_listInputTracks.SetImageList(m_taskImages, LVSIL_SMALL);
}

void FinishPage::UpdateInputTracksList()
{
   RedrawLock lock(m_listInputTracks);

   if (m_uiSettings.m_bFromInputFilesPage)
   {
      for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
      {
         EncoderJob& job = m_uiSettings.encoderjoblist[i];

         CString filename = job.InputFilename();

         m_listInputTracks.InsertItem(i, filename, 1); // icon 1: encoding
      }
   }
   else
   {
      unsigned int maxJobIndex = m_uiSettings.cdreadjoblist.size();
      for (unsigned int jobIndex = 0; jobIndex < maxJobIndex; jobIndex++)
      {
         CDReadJob& cdReadJob = m_uiSettings.cdreadjoblist[jobIndex];

         const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();
         const CDRipTrackInfo& trackInfo = cdReadJob.TrackInfo();

         if (!trackInfo.m_bActive)
            continue;

         CString title = CDRipTitleFormatManager::FormatTitle(
            discInfo.m_bVariousArtists ? m_uiSettings.cdrip_format_various_track : m_uiSettings.cdrip_format_album_track,
            discInfo, trackInfo);

         m_listInputTracks.InsertItem(jobIndex, title, 2); // icon 2: CD extraction
      }
   }

   m_listInputTracks.SetColumnWidth(0, LVSCW_AUTOSIZE);
}

void FinishPage::UpdateOutputModule()
{
   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();

   ATLASSERT(m_uiSettings.output_module < moduleManager.getOutputModuleCount());
   CString outputModuleName = moduleManager.getOutputModuleName(m_uiSettings.output_module);

   m_editOutputModule.SetWindowText(outputModuleName);
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

      job.OutputFilename(spTask->GenerateOutputFilename(job.InputFilename()));

      m_lastTaskId = spTask->Id();
   }
}

void FinishPage::AddCDExtractTasks()
{
   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();

   bool outputWaveFile16bit =
      moduleManager.getOutputModuleID(m_uiSettings.output_module) == ID_OM_WAVE &&
      m_uiSettings.settings_manager.queryValueInt(SndFileFormat) == SF_FORMAT_WAV &&
      m_uiSettings.settings_manager.queryValueInt(SndFileSubType) == SF_FORMAT_PCM_16;

   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   unsigned int lastCDReadTaskId = 0;

   unsigned int maxJobIndex = m_uiSettings.cdreadjoblist.size();
   for (unsigned int jobIndex = 0; jobIndex < maxJobIndex; jobIndex++)
   {
      CDReadJob& cdReadJob = m_uiSettings.cdreadjoblist[jobIndex];

      const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();
      CDRipTrackInfo& trackInfo = cdReadJob.TrackInfo();

      if (!trackInfo.m_bActive)
         continue;

      if (outputWaveFile16bit)
      {
         // when outputting to CD format, we can store the wave file directly without writing
         // to temp storage first
         CString title = CDRipTitleFormatManager::FormatTitle(
            discInfo.m_bVariousArtists ? m_uiSettings.cdrip_format_various_track : m_uiSettings.cdrip_format_album_track,
            discInfo, trackInfo);

         trackInfo.m_cszRippedFilename =
            (const CString&)Path::Combine(
               m_uiSettings.m_defaultSettings.outputdir,
               title + _T(".wav"));
      }

      std::shared_ptr<CDExtractTask> spCDExtractTask(new CDExtractTask(lastCDReadTaskId, discInfo, trackInfo));
      taskMgr.AddTask(spCDExtractTask);

      m_lastTaskId = spCDExtractTask->Id();

      cdReadJob.OutputFilename(spCDExtractTask->OutputFilename());
      cdReadJob.Title(spCDExtractTask->Title());

      unsigned int cdReadTaskId = spCDExtractTask->Id();
      lastCDReadTaskId = cdReadTaskId;

      if (!outputWaveFile16bit)
      {
         // also add encode task
         std::shared_ptr<EncoderTask> spEncoderTask =
            CreateEncoderTaskForCDReadJob(cdReadTaskId, cdReadJob);

         cdReadJob.OutputFilename(spEncoderTask->GenerateOutputFilename(cdReadJob.Title()));

         taskMgr.AddTask(spEncoderTask);

         m_lastTaskId = spEncoderTask->Id();
      }
   }
}

std::shared_ptr<EncoderTask> FinishPage::CreateEncoderTaskForCDReadJob(unsigned int cdReadTaskId, const CDReadJob& cdReadJob)
{
   EncoderTaskSettings taskSettings;

   taskSettings.m_cszInputFilename = cdReadJob.OutputFilename();
   taskSettings.m_cszOutputPath = m_uiSettings.m_defaultSettings.outputdir;

   taskSettings.m_cszTitle = cdReadJob.Title();

   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
   taskSettings.m_iOutputModuleId = moduleManager.getOutputModuleID(m_uiSettings.output_module);

   taskSettings.m_settingsManager = m_uiSettings.settings_manager;

   TrackInfo encodeTrackInfo;
   CDExtractTask::SetTrackInfoFromCDTrackInfo(encodeTrackInfo, cdReadJob);

   taskSettings.m_trackInfo = encodeTrackInfo;
   taskSettings.m_useTrackInfo = true;
   taskSettings.m_pModuleManager = &moduleManager;
   taskSettings.m_bOverwriteFiles = m_uiSettings.m_defaultSettings.overwrite_existing;
   taskSettings.m_bDeleteAfterEncode = true; // temporary file created by CDExtractTask

   return std::make_shared<EncoderTask>(cdReadTaskId, taskSettings);
}

void FinishPage::AddPlaylistTask()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   CString playlistFilename =
      Path::Combine(m_uiSettings.m_defaultSettings.outputdir, m_uiSettings.playlist_filename);

   std::shared_ptr<Task> spTask;
   if (m_uiSettings.m_bFromInputFilesPage)
      spTask.reset(new CreatePlaylistTask(m_lastTaskId, playlistFilename, m_uiSettings.encoderjoblist));
   else
      spTask.reset(new CreatePlaylistTask(m_lastTaskId, playlistFilename, m_uiSettings.cdreadjoblist));

   taskMgr.AddTask(spTask);
}
