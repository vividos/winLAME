//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2012 Michael Fink
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
/// \file TaskManagerConfig.h
/// \brief TaskManagerConfig class

// include guard
#pragma once

/// task manager config
struct TaskManagerConfig
{
   /// ctor
   TaskManagerConfig() throw()
      :m_bAutoTasksPerCpu(true),
       m_uiUseNumTasks(2)
   {
   }

   /// when enabled, as many tasks as CPU cores are available are run
   bool m_bAutoTasksPerCpu;

   /// when m_bAutoTasksPerCpu is false, TaskManager uses this many concurrent threads
   /// to run tasks
   unsigned int m_uiUseNumTasks;
};
