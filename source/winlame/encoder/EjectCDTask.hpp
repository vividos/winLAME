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
/// \file EjectCDTask.hpp
/// \brief Eject CD task class
//
#pragma once

#include "Task.hpp"
#include <atomic>

namespace Encoder
{
   /// task to eject CD after reading
   class EjectCDTask : public Task
   {
   public:
      /// ctor
      EjectCDTask(unsigned int dependentTaskId, unsigned int discDrive);
      /// dtor
      virtual ~EjectCDTask() {}

      /// returns current task info; must return immediately
      virtual TaskInfo GetTaskInfo() override;

      /// runs task; may take longer
      virtual void Run() override;

      /// task should be aborted, e.g. when program is closed
      virtual void Stop() override;

   private:
      /// disc drive index
      unsigned int m_discDrive;

      /// indicates if task is already finished
      std::atomic<bool> m_finished;

      /// indicates if task was stopped
      std::atomic<bool> m_stopped;

      /// error text when an error occurred
      CString m_errorText;
   };

} // namespace Encoder
