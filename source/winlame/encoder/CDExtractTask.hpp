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

// includes
#include "Task.h"
#include "CDRipTrackManager.h"
#include <atomic>

// forward references
struct UISettings;

/// task to extract CD audio track
class CDExtractTask : public Task
{
public:
   /// ctor
   CDExtractTask(const CDRipDiscInfo& discinfo, const CDRipTrackInfo& trackinfo);
   /// dtor
   virtual ~CDExtractTask() throw() {}

   /// returns current task info; must return immediately
   virtual TaskInfo GetTaskInfo();

   /// runs task; may take longer
   virtual void Run();

   /// task should be aborted, e.g. when program is closed
   virtual void Stop();

private:
   /// generates title from disc and track infos
   CString GetDiscTrackTitle() const;

   /// generates temporary filename
   CString GetTempFilename(const CString& cszDiscTrackTitle) const;

   bool ExtractTrack(const CString& cszTempFilename);

private:
   /// CD disc info
   CDRipDiscInfo m_discinfo;

   /// CD track info
   CDRipTrackInfo m_trackinfo;

   /// settings
   UISettings& m_uiSettings;

   /// indicates if task was stopped
   std::atomic<bool> m_bStopped;

   /// progress in percent
   std::atomic<unsigned int> m_uiProgress;
};