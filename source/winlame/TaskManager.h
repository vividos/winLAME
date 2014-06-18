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
/// \file TaskManager.h
/// \brief Task manager

// include guard
#pragma once

// includes
#include <vector>
#include <deque>
#include <memory>
#include <boost/thread/recursive_mutex.hpp>
#include <thread>
#include <boost/asio.hpp>
#include "TaskInfo.h"
#include "TaskManagerConfig.h"

// forward references
class Task;

/// manages all background tasks
class TaskManager
{
public:
   /// ctor
   TaskManager();
   /// dtor
   ~TaskManager();

   /// returns a snapshot of current tasks
   std::vector<TaskInfo> CurrentTasks();

   /// returns a copy of the current configuration
   TaskManagerConfig CurrentConfig() const;

   /// sets new config
   void SetNewConfig(const TaskManagerConfig& config);

   /// adds a task to the queue
   void AddTask(std::shared_ptr<Task> spTask);

   /// returns if task queue is empty
   bool IsQueueEmpty() const throw();

   /// stops all tasks
   void StopAll();

private:
   /// thread function
   static void RunThread(boost::asio::io_service& ioService);

   /// runs single task
   void RunTask(std::shared_ptr<Task> spTask);

   /// removes task from queue
   void RemoveTask(std::shared_ptr<Task> spTask);

   /// sets busy flag for thread
   void SetBusyFlag(DWORD dwThreadId, bool bBusy);

private:
   // config

   /// mutex protecting config
   boost::recursive_mutex m_mutexConfig;

   /// task manager config
   TaskManagerConfig m_config;


   // task queue

   /// mutex protecting task queue
   boost::recursive_mutex m_mutexQueue;

   /// task queue typedef
   typedef std::deque<std::shared_ptr<Task>> T_deqTaskQueue;

   /// task queue
   T_deqTaskQueue m_deqTaskQueue;


   // thread pool

   /// io service
   boost::asio::io_service m_ioService;

   /// default work for io service
   std::unique_ptr<boost::asio::io_service::work> m_upDefaultWork;

   /// thread pool
   std::vector<std::shared_ptr<std::thread>> m_vecThreadPool;


   // busy flags

   /// mutex protecting busy flag map
   boost::recursive_mutex m_mutexBusyFlagMap;

   /// busy flag map type
   typedef std::map<DWORD, bool> T_mapBusyFlagMap;

   /// busy flag map
   T_mapBusyFlagMap m_mapBusyFlagMap;
};
