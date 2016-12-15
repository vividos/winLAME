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
/// \file CDExtractTask.cpp
/// \brief CD extract task class

// includes
#include "StdAfx.h"
#include "CDExtractTask.hpp"
#include "WaveOutputModule.h"
#include "UISettings.h"
#include "resource.h"
#include <basscd.h>
#include "CDRipTitleFormatManager.hpp"

CDExtractTask::CDExtractTask(unsigned int dependentTaskId, const CDRipDiscInfo& discinfo, const CDRipTrackInfo& trackinfo)
:Task(dependentTaskId),
m_discinfo(discinfo),
m_trackinfo(trackinfo),
m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
m_bStopped(false),
m_running(false),
m_finished(false),
m_uiProgress(0)
{
   m_title = CDRipTitleFormatManager::FormatTitle(
      m_discinfo.m_bVariousArtists ? m_uiSettings.cdrip_format_various_track : m_uiSettings.cdrip_format_album_track,
      m_discinfo, m_trackinfo);

   if (m_trackinfo.m_cszRippedFilename.IsEmpty())
   {
      CString cszDiscTrackTitle = CDRipTitleFormatManager::GetFilenameByTitle(m_title);

      CString cszTempFilename = GetTempFilename(cszDiscTrackTitle);

      m_trackinfo.m_cszRippedFilename = cszTempFilename;
   }
}

TaskInfo CDExtractTask::GetTaskInfo()
{
   TaskInfo info(Id(), TaskInfo::taskCdExtraction);

   info.Name(m_title);

   CString desc;
   desc.Format(IDS_CDEXTRACT_DESC_US,
      m_trackinfo.m_nTrackOnDisc,
      m_title);
   info.Description(desc);

   info.Status(
      m_finished ? TaskInfo::statusCompleted :
      m_running ? TaskInfo::statusRunning :
      TaskInfo::statusWaiting);

   info.Progress(m_finished ? 100 : m_uiProgress);

   return info;
}

void CDExtractTask::Run()
{
   m_running = true;
   ExtractTrack(m_trackinfo.m_cszRippedFilename);
   m_running = false;
   m_finished = true;
}

void CDExtractTask::Stop()
{
   m_bStopped = true;
}

CString CDExtractTask::GetTempFilename(const CString& cszDiscTrackTitle) const
{
   CString cszGuid;
   cszGuid.Format(_T("{%08x-%08x}"), ::GetCurrentProcessId(), ::GetCurrentThreadId());

   // add slash to temp folder if necessary
   CString cszTempFolder(m_uiSettings.cdrip_temp_folder);
   Path::AddEndingBackslash(cszTempFolder);

   CString cszTempFilename;
   cszTempFilename.Format(_T("%s\\(%02u) %s%s.wav"),
      cszTempFolder,
      m_trackinfo.m_nTrackOnDisc + 1,
      cszDiscTrackTitle, cszGuid);

   return cszTempFilename;
}

bool CDExtractTask::ExtractTrack(const CString& cszTempFilename)
{
   // check disc ID
   CString cszCDID(BASS_CD_GetID(m_discinfo.m_nDiscDrive, BASS_CDID_CDDB));
   if (cszCDID != m_discinfo.m_cszCDID)
   {
      SetTaskError(IDS_CDRIP_PAGE_ERROR_WRONG_CD);
      return false;
   }

   DWORD nTrackLength = BASS_CD_GetTrackLength(m_discinfo.m_nDiscDrive, m_trackinfo.m_nTrackOnDisc);
   DWORD nCurLength = 0;

   BASS_Init(0, 44100, 0, NULL, NULL);

   HSTREAM hStream = BASS_CD_StreamCreate(m_discinfo.m_nDiscDrive, m_trackinfo.m_nTrackOnDisc,
      BASS_STREAM_DECODE);

   DWORD nError = BASS_ErrorGetCode();

   if (hStream == 0 || nError != BASS_OK)
      return false;

   WaveOutputModule outmod;

   if (!outmod.isAvailable())
   {
      SetTaskError(IDS_CDRIP_PAGE_WAVE_OUTPUT_NOT_AVAIL);
      return false;
   }

   SettingsManager mgr;
   TrackInfo outtrackinfo;
   SampleContainer samplecont;
   {
      samplecont.setInputModuleTraits(16, SamplesInterleaved, 44100, 2);
      outtrackinfo.ResetInfos();
      mgr.setValue(SndFileFormat, SF_FORMAT_WAV);
      mgr.setValue(SndFileSubType, SF_FORMAT_PCM_16);

      outmod.prepareOutput(mgr);
      int nRet = outmod.initOutput(cszTempFilename, mgr, outtrackinfo, samplecont);
      if (nRet < 0)
      {
         CString text;
         text.Format(IDS_CDRIP_PAGE_ERROR_CREATE_OUTPUT_FILE_S, cszTempFilename);
         SetTaskError(text);
         return false;
      }
   }

   const unsigned int nBufferSize = 65536;

   std::vector<signed short> vecBuffer(nBufferSize);

   bool bFinished = true;

   while (BASS_ACTIVE_STOPPED != BASS_ChannelIsActive(hStream))
   {
      DWORD nAvail = BASS_ChannelGetData(hStream, &vecBuffer[0], nBufferSize*sizeof(vecBuffer[0]));

      if (nAvail == 0)
         break; // couldn't read more samples

      samplecont.putSamplesInterleaved(&vecBuffer[0], nAvail / sizeof(vecBuffer[0]) / 2);

      nCurLength += nAvail;

      if (m_bStopped)
      {
         bFinished = false;
         break;
      }

      m_uiProgress = nCurLength * 100 / nTrackLength;

      int nRet = outmod.encodeSamples(samplecont);
      if (nRet < 0)
         break;
   }

   BASS_StreamFree(hStream);
   BASS_Free();

   outmod.doneOutput();

   return bFinished;
}

void CDExtractTask::SetTrackInfoFromCDTrackInfo(TrackInfo& encodeTrackInfo, const CDReadJob& cdReadJob)
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
