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
/// \file CDExtractTask.cpp
/// \brief CD extract task class
//
#include "stdafx.h"
#include "CDExtractTask.hpp"
#include "SndFileOutputModule.hpp"
#include "UISettings.hpp"
#include "resource.h"
#include <basscd.h>
#include "CDRipTitleFormatManager.hpp"

using Encoder::CDExtractTask;
using Encoder::TrackInfo;

extern std::atomic<unsigned int> s_bassApiusageCount;

CDExtractTask::CDExtractTask(unsigned int dependentTaskId, const CDRipDiscInfo& discinfo, const CDRipTrackInfo& trackinfo)
   :Task(dependentTaskId),
   m_discinfo(discinfo),
   m_trackinfo(trackinfo),
   m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
   m_stopped(false),
   m_running(false),
   m_finished(false),
   m_progressInPercent(0)
{
   m_title = CDRipTitleFormatManager::FormatTitle(m_uiSettings, m_discinfo, m_trackinfo);

   if (m_trackinfo.m_rippedFilename.IsEmpty())
   {
      CString discTrackTitle = CDRipTitleFormatManager::GetFilenameByTitle(m_title);

      CString tempFilename = GetTempFilename(discTrackTitle);

      m_trackinfo.m_rippedFilename = tempFilename;
   }
}

TaskInfo CDExtractTask::GetTaskInfo()
{
   TaskInfo info(Id(), TaskInfo::taskCdExtraction);

   info.Name(m_title);

   CString desc;
   desc.Format(IDS_CDEXTRACT_DESC_US,
      m_trackinfo.m_numTrackOnDisc,
      m_title.GetString());
   info.Description(desc);

   info.Status(
      m_finished ? TaskInfo::statusCompleted :
      m_running ? TaskInfo::statusRunning :
      TaskInfo::statusWaiting);

   info.Progress(m_finished ? 100 : static_cast<unsigned int>(m_progressInPercent));

   return info;
}

void CDExtractTask::Run()
{
   m_running = true;
   ExtractTrack(m_trackinfo.m_rippedFilename);
   m_running = false;
   m_finished = true;
}

void CDExtractTask::Stop()
{
   m_stopped = true;
}

CString CDExtractTask::GetTempFilename(const CString& discTrackTitle) const
{
   CString guid;
   guid.Format(_T("{%08x-%08x}"), ::GetCurrentProcessId(), ::GetCurrentThreadId());

   CString tempFilename;
   tempFilename.Format(_T("(%02u) %s%s.wav"),
      m_trackinfo.m_numTrackOnDisc + 1,
      discTrackTitle.GetString(),
      guid.GetString());

   tempFilename.Replace(_T("\\"), _T("_"));

   return Path::Combine(m_uiSettings.cdrip_temp_folder, tempFilename);
}

bool CDExtractTask::ExtractTrack(const CString& tempFilename)
{
   if (m_stopped)
   {
      return false;
   }

   // check disc ID
   CString CDID(BASS_CD_GetID(m_discinfo.m_discDrive, BASS_CDID_CDDB));
   if (CDID != m_discinfo.m_CDID)
   {
      SetTaskError(IDS_CDRIP_PAGE_ERROR_WRONG_CD);
      return false;
   }

   DWORD trackLength = BASS_CD_GetTrackLength(m_discinfo.m_discDrive, m_trackinfo.m_numTrackOnDisc);
   DWORD currentLength = 0;

   if (s_bassApiusageCount++ == 0)
   {
      BASS_Init(0, 44100, 0, nullptr, nullptr);
   }

   HSTREAM hStream = BASS_CD_StreamCreate(m_discinfo.m_discDrive, m_trackinfo.m_numTrackOnDisc,
      BASS_STREAM_DECODE);

   DWORD error = BASS_ErrorGetCode();

   if (hStream == 0 || error != BASS_OK)
      return false;

   Encoder::SndFileOutputModule outputModule;

   if (!outputModule.IsAvailable())
   {
      SetTaskError(IDS_CDRIP_PAGE_WAVE_OUTPUT_NOT_AVAIL);
      return false;
   }

   SettingsManager mgr;
   TrackInfo outputTrackInfo;
   SampleContainer samples;
   {
      samples.SetInputModuleTraits(16, SamplesInterleaved, 44100, 2);
      mgr.setValue(SndFileFormat, SF_FORMAT_WAV);
      mgr.setValue(SndFileSubType, SF_FORMAT_PCM_16);

      outputTrackInfo.ResetInfos();
      CDReadJob cdReadJob(m_discinfo, m_trackinfo);
      SetTrackInfoFromCDTrackInfo(outputTrackInfo, cdReadJob);

      outputModule.PrepareOutput(mgr);

      // create folder when it doesn't exist
      CString tempOutputFolder = Path::FolderName(tempFilename);
      if (!Path::FolderExists(tempOutputFolder))
         Path::CreateDirectoryRecursive(tempOutputFolder);

      int ret = outputModule.InitOutput(tempFilename, mgr, outputTrackInfo, samples);
      if (ret < 0)
      {
         CString text;
         text.Format(IDS_CDRIP_PAGE_ERROR_CREATE_OUTPUT_FILE_S, tempFilename.GetString());
         SetTaskError(text);
         return false;
      }
   }

   const unsigned int bufferSize = 65536;

   std::vector<signed short> vecBuffer(bufferSize);

   bool isFinished = true;

   while (BASS_ACTIVE_STOPPED != BASS_ChannelIsActive(hStream))
   {
      DWORD availBytes = BASS_ChannelGetData(hStream, &vecBuffer[0], bufferSize * sizeof(vecBuffer[0]));

      if (availBytes < 0)
         break; // channel ended or other error

      if (availBytes == 0)
      {
         // buffer is empty; wait a bit to fill it
         Sleep(1);
         continue;
      }

      samples.PutSamplesInterleaved(&vecBuffer[0], availBytes / sizeof(vecBuffer[0]) / 2);

      currentLength += availBytes;

      if (m_stopped)
      {
         isFinished = false;
         break;
      }

      m_progressInPercent = currentLength * 100 / trackLength;

      int ret = outputModule.EncodeSamples(samples);
      if (ret < 0)
         break;
   }

   BASS_StreamFree(hStream);

   if (--s_bassApiusageCount == 0)
   {
      BASS_Free();
   }

   outputModule.DoneOutput();

   return isFinished;
}

void CDExtractTask::SetTrackInfoFromCDTrackInfo(TrackInfo& encodeTrackInfo, const Encoder::CDReadJob& cdReadJob)
{
   // disc info
   const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();

   encodeTrackInfo.SetTextInfo(TrackInfoDiscArtist, discInfo.m_discArtist);
   encodeTrackInfo.SetTextInfo(TrackInfoAlbum, discInfo.m_discTitle);

   if (discInfo.m_year != 0)
      encodeTrackInfo.SetNumberInfo(TrackInfoYear, discInfo.m_year);

   if (!discInfo.m_genre.IsEmpty())
      encodeTrackInfo.SetTextInfo(TrackInfoGenre, discInfo.m_genre);

   // add track info
   const CDRipTrackInfo& cdTrackInfo = cdReadJob.TrackInfo();

   encodeTrackInfo.SetTextInfo(TrackInfoTitle, cdTrackInfo.m_trackTitle);

   CString trackArist = cdTrackInfo.m_trackArtist;
   encodeTrackInfo.SetTextInfo(TrackInfoArtist, trackArist);

   encodeTrackInfo.SetNumberInfo(TrackInfoTrack, cdTrackInfo.m_numTrackOnDisc + 1);

   // cover art
   if (!cdReadJob.FrontCoverArtImage().empty())
   {
      encodeTrackInfo.SetBinaryInfo(TrackInfoFrontCover, cdReadJob.FrontCoverArtImage());
   }
}
