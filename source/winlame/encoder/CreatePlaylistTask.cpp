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
/// \file CreatePlaylistTask.cpp
/// \brief encoder task class
//
#include "StdAfx.h"
#include "CreatePlaylistTask.hpp"
#include "Path.hpp"

using Encoder::CreatePlaylistTask;

CreatePlaylistTask::CreatePlaylistTask(unsigned int dependentTaskId, const CString& playlistFilename, const EncoderJobList& encoderJobList)
   :Task(dependentTaskId),
   m_extendedPlaylist(false), // no length infos available
   m_playlistFilename(playlistFilename),
   m_finished(false),
   m_stopped(false)
{
   std::for_each(encoderJobList.begin(), encoderJobList.end(), [&](const EncoderJob& encoderJob)
   {
      PlaylistEntry entry;

      entry.m_filename = encoderJob.OutputFilename();
      // TODO title, length

      m_playlistEntries.push_back(entry);
   });
}

CreatePlaylistTask::CreatePlaylistTask(unsigned int dependentTaskId, const CString& playlistFilename, const std::vector<Encoder::CDReadJob>& cdReadJobList)
   :Task(dependentTaskId),
   m_extendedPlaylist(true),
   m_playlistFilename(playlistFilename),
   m_finished(false),
   m_stopped(false)
{
   std::for_each(cdReadJobList.begin(), cdReadJobList.end(), [&](const CDReadJob& cdReadJob)
   {
      PlaylistEntry entry;

      entry.m_filename = cdReadJob.OutputFilename();
      entry.m_title = cdReadJob.TrackInfo().m_trackTitle;
      entry.m_trackLengthInSeconds = cdReadJob.TrackInfo().m_trackLengthInSeconds;

      m_playlistEntries.push_back(entry);
   });
}

TaskInfo CreatePlaylistTask::GetTaskInfo()
{
   TaskInfo info(Id(), TaskInfo::taskWritePlaylist);

   info.Name(_T("Playlist: ") + Path(m_playlistFilename).FilenameAndExt());
   info.Progress(m_finished || m_stopped ? 100 : 0);
   info.Status(m_finished || m_stopped ? TaskInfo::statusCompleted : TaskInfo::statusWaiting);

   return info;
}

void CreatePlaylistTask::Run()
{
   if (m_stopped)
      return;

   m_finished = false;

   FILE* fd = _tfopen(m_playlistFilename, _T("wt"));
   if (fd == NULL)
   {
      SetTaskError(IDS_PLAYLIST_TASK_ERROR_CREATE_FILE);
      return;
   }

   std::shared_ptr<FILE> spFd(fd, fclose);

   if (m_extendedPlaylist)
      _ftprintf(fd, _T("#EXTM3U\n\n"));

   CString rootFolder = Path(m_playlistFilename).FolderName();

   for (size_t entryIndex = 0, maxEntryIndex = m_playlistEntries.size(); entryIndex < maxEntryIndex; entryIndex++)
   {
      const PlaylistEntry& entry = m_playlistEntries[entryIndex];

      if (m_extendedPlaylist)
         _ftprintf(fd, _T("#EXTINF:%u,%s\n"), entry.m_trackLengthInSeconds, entry.m_title.GetString());

      CString relativeFilename = Path(entry.m_filename).MakeRelativeTo(rootFolder);
      if (relativeFilename.IsEmpty())
         relativeFilename = entry.m_filename;

      _ftprintf(fd, _T("%s\n"), relativeFilename.GetString());

      if (m_extendedPlaylist)
         _ftprintf(fd, _T("\n"));
   }

   m_finished = true;
}

void CreatePlaylistTask::Stop()
{
   m_stopped = true;
}
