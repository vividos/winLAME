//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2023 Michael Fink
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
/// \file EjectCDTask.cpp
/// \brief CD extract task class
//
#include "stdafx.h"
#include "EjectCDTask.hpp"
#include "resource.h"
#include <basscd.h>

using Encoder::EjectCDTask;

extern std::atomic<unsigned int> s_bassApiusageCount;

EjectCDTask::EjectCDTask(unsigned int dependentTaskId, unsigned int discDrive)
   :Task(dependentTaskId),
   m_discDrive(discDrive),
   m_finished(false),
   m_stopped(false)
{
}

TaskInfo EjectCDTask::GetTaskInfo()
{
   TaskInfo info(Id(), TaskInfo::taskEjectCD);

   CString title;
   title.LoadString(IDS_EJECT_CD_TASK_TITLE);
   info.Name(title);

   CString description;
   description.LoadString(IDS_EJECT_CD_TASK_DESCRIPTION);
   info.Description(description);

   info.Progress(m_finished || m_stopped ? 100 : 0);
   info.Status(
      !m_errorText.IsEmpty() ? TaskInfo::statusError :
      m_finished || m_stopped ? TaskInfo::statusCompleted : TaskInfo::statusWaiting);

   return info;
}

void EjectCDTask::Run()
{
   if (m_stopped)
      return;

   m_finished = false;

   if (s_bassApiusageCount++ == 0)
   {
      BASS_Init(0, 44100, 0, nullptr, nullptr);
   }

   if (BASS_CD_DoorIsLocked(m_discDrive) == FALSE &&
      BASS_CD_DoorIsOpen(m_discDrive) == FALSE)
   {
      BOOL result = BASS_CD_Door(m_discDrive, BASS_CD_DOOR_OPEN);
      if (result == FALSE)
      {
         int errorCode = BASS_ErrorGetCode();

         m_errorText.Format(_T("BASS error: %i"), errorCode);
      }
   }

   if (--s_bassApiusageCount == 0)
   {
      BASS_Free();
   }

   m_finished = true;
}

void EjectCDTask::Stop()
{
   m_stopped = true;
}
