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
/// \file CreatePlaylistTask.hpp
/// \brief Create playlist task class
//
#pragma once

#include "Task.hpp"
#include "UISettings.hpp"
#include <atomic>

namespace Encoder
{
   /// Task to create .m3u playlist file
   class CreatePlaylistTask : public Task
   {
   public:
      /// ctor, taking list of EncoderJobs
      CreatePlaylistTask(unsigned int dependentTaskId, const CString& playlistFilename, const EncoderJobList& encoderjoblist);
      /// ctor, taking list of CDReadJobs
      CreatePlaylistTask(unsigned int dependentTaskId, const CString& playlistFilename, const std::vector<::Encoder::CDReadJob>& cdreadjoblist);
      /// dtor
      virtual ~CreatePlaylistTask() {}

      /// returns current task info; must return immediately
      virtual TaskInfo GetTaskInfo();

      /// runs task; may take longer
      virtual void Run();

      /// task should be aborted, e.g. when program is closed
      virtual void Stop();

   private:
      /// indicates if an extended playlist is created
      bool m_extendedPlaylist;

      /// filename of playlist to write
      CString m_playlistFilename;

      /// single playlist entry
      struct PlaylistEntry
      {
         /// ctor
         PlaylistEntry()
            :m_trackLengthInSeconds(0)
         {
         }

         /// filename of playlist entry
         CString m_filename;

         /// title of entry
         CString m_title;

         /// track length, in seconds
         unsigned int m_trackLengthInSeconds;
      };

      /// list of playlist entries
      std::vector<PlaylistEntry> m_playlistEntries;

      /// indicates if task is already finished
      std::atomic<bool> m_finished;

      /// indicates if task was stopped
      std::atomic<bool> m_stopped;
   };

} // namespace Encoder
