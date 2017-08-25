//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file TaskCreationHelper.cpp
/// \brief Task creation helper class
//
#include "stdafx.h"
#include "TaskCreationHelper.hpp"
#include "TaskManager.hpp"
#include "EncoderTask.hpp"
#include "CreatePlaylistTask.hpp"
#include "CDExtractTask.hpp"
#include "CDRipTitleFormatManager.hpp"
#include "LameNogapInstanceManager.hpp"
#include <sndfile.h>

TaskCreationHelper::TaskCreationHelper()
   :m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
   m_lastTaskId(0)
{
}

bool TaskCreationHelper::IsLossyTranscoding() const
{
   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();

   bool in_lossy = false;

   // only have to check when from input page; CD reading is always lossless
   if (m_uiSettings.m_bFromInputFilesPage)
   {
      Encoder::ModuleManagerImpl& modImpl = reinterpret_cast<Encoder::ModuleManagerImpl&>(moduleManager);

      for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
      {
         Encoder::EncoderJob& job = m_uiSettings.encoderjoblist[i];

         CString filename = job.InputFilename();

         std::unique_ptr<Encoder::InputModule> inmod(modImpl.ChooseInputModule(filename));
         if (inmod == nullptr)
            continue;

         int in_id = inmod->GetModuleID();

         in_lossy |= Encoder::EncoderImpl::IsLossyInputModule(in_id);
      }
   }

   int out_id = moduleManager.GetOutputModuleID(m_uiSettings.output_module);

   bool out_lossy = Encoder::EncoderImpl::IsLossyOutputModule(out_id);

   return in_lossy && out_lossy;
}

bool TaskCreationHelper::IsOverwritingOriginalFiles() const
{
   // overwriting doesn't happen when encoding CD tracks
   if (!m_uiSettings.m_bFromInputFilesPage)
      return false;

   // overwriting doesn't happen when flag "overwrite existing" isn't active
   if (!m_uiSettings.m_defaultSettings.overwrite_existing)
      return false;

   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
   Encoder::ModuleManagerImpl& modImpl = reinterpret_cast<Encoder::ModuleManagerImpl&>(moduleManager);

   int out_module_id = moduleManager.GetOutputModuleID(m_uiSettings.output_module);

   for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
   {
      Encoder::EncoderJob& job = m_uiSettings.encoderjoblist[i];

      CString inputFilename = job.InputFilename();

      std::unique_ptr<Encoder::OutputModule> outputModule(modImpl.GetOutputModule(out_module_id));
      if (outputModule == nullptr)
         continue;

      outputModule->PrepareOutput(m_uiSettings.settings_manager);

      CString outputFilename = Encoder::EncoderImpl::GetOutputFilename(m_uiSettings.m_defaultSettings.outputdir, inputFilename, *outputModule.get());

      if (outputFilename.CompareNoCase(inputFilename) == 0)
      {
         return true;
      }
   }

   return false;
}

void TaskCreationHelper::AddTasks()
{
   if (m_uiSettings.m_bFromInputFilesPage)
      AddInputFilesTasks();
   else
      AddCDExtractTasks();

   if (m_uiSettings.create_playlist)
      AddPlaylistTask();

   m_uiSettings.encoderjoblist.clear();
   m_uiSettings.cdreadjoblist.clear();
}

void TaskCreationHelper::AddInputFilesTasks()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   m_lastTaskId = 0;

   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();

   bool lameNogapEncoding =
      moduleManager.GetOutputModuleID(m_uiSettings.output_module) == ID_OM_LAME &&
      m_uiSettings.settings_manager.QueryValueInt(LameOptNoGap) == 1;

   int nogapInstanceId = -1; // valid values start at 0
   if (lameNogapEncoding)
   {
      Encoder::LameNogapInstanceManager& nogapInstanceManager =
         IoCContainer::Current().Resolve<Encoder::LameNogapInstanceManager>();

      nogapInstanceId = nogapInstanceManager.NextNogapInstanceId();
   }

   for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
   {
      Encoder::EncoderJob& job = m_uiSettings.encoderjoblist[i];

      Encoder::EncoderTaskSettings taskSettings;

      taskSettings.m_inputFilename = job.InputFilename();

      if (m_uiSettings.out_location_use_input_dir)
      {
         Path outputPath(job.InputFilename());

         taskSettings.m_outputFolder = outputPath.FolderName();
      }
      else
         taskSettings.m_outputFolder = m_uiSettings.m_defaultSettings.outputdir;

      taskSettings.m_title = Path(job.InputFilename()).FilenameAndExt();

      taskSettings.m_outputModuleID = moduleManager.GetOutputModuleID(m_uiSettings.output_module);

      taskSettings.m_settingsManager = m_uiSettings.settings_manager;
      taskSettings.m_trackInfo = job.GetTrackInfo();
      taskSettings.m_overwriteExisting = m_uiSettings.m_defaultSettings.overwrite_existing;
      taskSettings.m_deleteInputAfterEncode = m_uiSettings.m_defaultSettings.delete_after_encode;

      // set previous task id when encoding with LAME and using nogap encoding
      unsigned int dependentTaskId = 0;
      if (lameNogapEncoding)
      {
         dependentTaskId = m_lastTaskId;

         taskSettings.m_settingsManager.setValue(LameNoGapInstanceId, nogapInstanceId);

         if (i == iMax - 1)
            taskSettings.m_settingsManager.setValue(GeneralIsLastFile, 1);
      }

      std::shared_ptr<Encoder::EncoderTask> spTask(new Encoder::EncoderTask(dependentTaskId, taskSettings));

      taskMgr.AddTask(spTask);

      job.OutputFilename(spTask->GenerateOutputFilename(job.InputFilename()));

      m_lastTaskId = spTask->Id();
   }
}

void TaskCreationHelper::AddCDExtractTasks()
{
   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();

   bool outputWaveFile16bit =
      moduleManager.GetOutputModuleID(m_uiSettings.output_module) == ID_OM_WAVE &&
      m_uiSettings.settings_manager.QueryValueInt(SndFileFormat) == SF_FORMAT_WAV &&
      m_uiSettings.settings_manager.QueryValueInt(SndFileSubType) == SF_FORMAT_PCM_16;

   bool lameNogapEncoding =
      moduleManager.GetOutputModuleID(m_uiSettings.output_module) == ID_OM_LAME &&
      m_uiSettings.settings_manager.QueryValueInt(LameOptNoGap) == 1;

   int nogapInstanceId = -1; // valid values start at 0
   if (lameNogapEncoding)
   {
      Encoder::LameNogapInstanceManager& nogapInstanceManager =
         IoCContainer::Current().Resolve<Encoder::LameNogapInstanceManager>();

      nogapInstanceId = nogapInstanceManager.NextNogapInstanceId();
   }

   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   unsigned int lastCDReadTaskId = 0;

   unsigned int maxJobIndex = m_uiSettings.cdreadjoblist.size();
   for (unsigned int jobIndex = 0; jobIndex < maxJobIndex; jobIndex++)
   {
      Encoder::CDReadJob& cdReadJob = m_uiSettings.cdreadjoblist[jobIndex];

      const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();
      CDRipTrackInfo& trackInfo = cdReadJob.TrackInfo();

      if (!trackInfo.m_isActive)
         continue;

      if (outputWaveFile16bit)
      {
         // when outputting to CD format, we can store the wave file directly without writing
         // to temp storage first
         CString title = CDRipTitleFormatManager::FormatTitle(m_uiSettings, discInfo, trackInfo);

         CString titleFilename = CDRipTitleFormatManager::GetFilenameByTitle(title);

         trackInfo.m_rippedFilename = Path::Combine(
            m_uiSettings.m_defaultSettings.outputdir,
            titleFilename + _T(".wav")).ToString();
      }

      std::shared_ptr<Encoder::CDExtractTask> spCDExtractTask(new Encoder::CDExtractTask(lastCDReadTaskId, discInfo, trackInfo));
      taskMgr.AddTask(spCDExtractTask);

      m_lastTaskId = spCDExtractTask->Id();

      cdReadJob.OutputFilename(spCDExtractTask->OutputFilename());
      cdReadJob.Title(spCDExtractTask->Title());

      unsigned int cdReadTaskId = spCDExtractTask->Id();
      lastCDReadTaskId = cdReadTaskId;

      if (!outputWaveFile16bit)
      {
         bool isLastTrack = jobIndex == maxJobIndex - 1;

         // also add encode task
         std::shared_ptr<Encoder::EncoderTask> spEncoderTask =
            CreateEncoderTaskForCDReadJob(cdReadTaskId, cdReadJob, nogapInstanceId, isLastTrack);

         CString titleFilename = CDRipTitleFormatManager::GetFilenameByTitle(cdReadJob.Title());

         CString inputFilenameFromTitle =
            Path::Combine(
               Path(cdReadJob.OutputFilename()).FolderName(),
               titleFilename + _T(".wav")).ToString();

         cdReadJob.OutputFilename(spEncoderTask->GenerateOutputFilename(inputFilenameFromTitle));

         taskMgr.AddTask(spEncoderTask);

         m_lastTaskId = spEncoderTask->Id();
      }
   }
}

std::shared_ptr<Encoder::EncoderTask> TaskCreationHelper::CreateEncoderTaskForCDReadJob(
   unsigned int cdReadTaskId, const Encoder::CDReadJob& cdReadJob,
   int nogapInstanceId, bool isLastTrack)
{
   Encoder::EncoderTaskSettings taskSettings;

   taskSettings.m_inputFilename = cdReadJob.OutputFilename();
   taskSettings.m_outputFolder = m_uiSettings.m_defaultSettings.outputdir;

   taskSettings.m_title = cdReadJob.Title();

   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
   taskSettings.m_outputModuleID = moduleManager.GetOutputModuleID(m_uiSettings.output_module);

   taskSettings.m_settingsManager = m_uiSettings.settings_manager;

   if (nogapInstanceId >= 0)
      taskSettings.m_settingsManager.setValue(LameNoGapInstanceId, nogapInstanceId);

   Encoder::TrackInfo encodeTrackInfo;
   Encoder::CDExtractTask::SetTrackInfoFromCDTrackInfo(encodeTrackInfo, cdReadJob);

   taskSettings.m_trackInfo = encodeTrackInfo;
   taskSettings.m_useTrackInfo = true;
   taskSettings.m_overwriteExisting = m_uiSettings.m_defaultSettings.overwrite_existing;
   taskSettings.m_deleteInputAfterEncode = true; // temporary file created by CDExtractTask

   if (isLastTrack)
      taskSettings.m_settingsManager.setValue(GeneralIsLastFile, 1);

   return std::make_shared<Encoder::EncoderTask>(cdReadTaskId, taskSettings);
}

void TaskCreationHelper::AddPlaylistTask()
{
   TaskManager& taskMgr = IoCContainer::Current().Resolve<TaskManager>();

   CString playlistFilename =
      Path::Combine(m_uiSettings.m_defaultSettings.outputdir, m_uiSettings.playlist_filename).ToString();

   std::shared_ptr<Task> spTask;
   if (m_uiSettings.m_bFromInputFilesPage)
      spTask.reset(new Encoder::CreatePlaylistTask(m_lastTaskId, playlistFilename, m_uiSettings.encoderjoblist));
   else
      spTask.reset(new Encoder::CreatePlaylistTask(m_lastTaskId, playlistFilename, m_uiSettings.cdreadjoblist));

   taskMgr.AddTask(spTask);
}
