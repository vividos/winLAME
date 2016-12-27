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
/// \file CDExtractTask.hpp
/// \brief CD extract task class
//
#pragma once

#include "Task.h"
#include "CDRipTrackManager.h"
#include <atomic>

// forward references
struct UISettings;

namespace Encoder
{
   class TrackInfo;
   class CDReadJob;

   /// task to extract CD audio track
   class CDExtractTask : public Task
   {
   public:
      /// ctor
      CDExtractTask(unsigned int dependentTaskId, const CDRipDiscInfo& discinfo, const CDRipTrackInfo& trackinfo);
      /// dtor
      virtual ~CDExtractTask() throw() {}

      /// returns current task info; must return immediately
      virtual TaskInfo GetTaskInfo();

      /// runs task; may take longer
      virtual void Run();

      /// task should be aborted, e.g. when program is closed
      virtual void Stop();

      /// output filename for this task
      const CString& OutputFilename() { return m_trackinfo.m_rippedFilename; }

      /// title for this task
      const CString& Title() { return m_title; }

      /// Sets track info properties from CD Read job infos
      static void SetTrackInfoFromCDTrackInfo(TrackInfo& encodeTrackInfo, const CDReadJob& cdReadJob);

   private:
      /// generates temporary filename
      CString GetTempFilename(const CString& discTrackTitle) const;

      /// extracts track from CD and stores it in temporary filename
      bool ExtractTrack(const CString& tempFilename);

   private:
      /// CD disc info
      CDRipDiscInfo m_discinfo;

      /// CD track info
      CDRipTrackInfo m_trackinfo;

      /// settings
      UISettings& m_uiSettings;

      /// title of track to extract
      CString m_title;

      /// indicates if task was stopped
      std::atomic<bool> m_stopped;

      /// indicates if task is running
      std::atomic<bool> m_running;

      /// indicates if task has finished
      std::atomic<bool> m_finished;

      /// progress in percent
      std::atomic<unsigned int> m_progressInPercent;
   };

} // namespace Encoder
