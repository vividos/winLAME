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
/// \file TaskCreationHelper.hpp
/// \brief Task creation helper class
//
#pragma once

// forward references
struct UISettings;

namespace Encoder
{
   class EncoderTask;
   class CDReadJob;
}

/// helper class to help with creating tasks for encoding, CD readout and playlist writing
class TaskCreationHelper
{
public:
   /// ctor
   TaskCreationHelper();

   ///  determines if any files would be transcoded from lossy to lossy format
   bool IsLossyTranscoding() const;

   /// determines if any output files would overwrite original input files
   bool IsOverwritingOriginalFiles() const;

   /// adds tasks to task manager, depending on the options of the global UISettings object
   void AddTasks();

private:
   /// adds tasks for input files to task manager
   void AddInputFilesTasks();

   /// adds tasks for CD extraction to task manager
   void AddCDExtractTasks();

   /// creates encoder task for a CD Extract task
   std::shared_ptr<Encoder::EncoderTask> CreateEncoderTaskForCDReadJob(
      unsigned int cdReadTaskId, const Encoder::CDReadJob& cdReadJob,
      int nogapInstanceId, bool isLastTrack);

   /// finds playlist output folder that is common to all files on the playlist
   CString FindCommonPlaylistOutputFolder() const;

   /// adds task to create a playlist to task manager
   void AddPlaylistTask();

private:
   /// settings
   UISettings& m_uiSettings;

   /// last task id used for an encoding task or a CD extract task
   unsigned int m_lastTaskId;
};
