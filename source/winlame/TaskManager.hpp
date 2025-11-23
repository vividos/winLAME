//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file TaskManager.hpp
/// \brief Task manager
//
#pragma once

#include <vector>
#include <deque>
#include <set>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <boost/asio.hpp>
#include "TaskInfo.hpp"
#include "TaskManagerConfig.hpp"

class Task;

/// manages all background tasks
class TaskManager
{
public:
   /// ctor
   explicit TaskManager(const TaskManagerConfig& config);
   /// dtor
   ~TaskManager();

   /// returns a snapshot of current tasks
   std::vector<TaskInfo> CurrentTasks();

   /// adds a task to the queue
   void AddTask(std::shared_ptr<Task> spTask);

   /// checks if there are tasks that are now runnable and starts them
   void CheckRunnableTasks();

   /// returns if task queue is empty
   bool IsQueueEmpty() const;

   /// returns if there are running tasks
   bool AreRunningTasksAvail() const;

   /// returns if there are completed tasks
   bool AreCompletedTasksAvail() const;

   /// returns if there are CD Extract tasks running
   bool AreCDExtractTasksRunning() const;

   /// returns task list state infos
   void GetTaskListState(bool& hasActiveTasks, bool& hasErrorTasks, unsigned int& percentComplete) const;

   /// stops all tasks
   void StopAll();

   /// removes all completed tasks from queue
   void RemoveCompletedTasks();

private:
   /// thread function
   static void RunThread(boost::asio::io_context& ioContext, unsigned int threadNumber);

   /// returns if a task is runnable
   bool IsTaskRunnable(std::shared_ptr<Task> spTask) const;

   /// runs single task
   void RunTask(std::shared_ptr<Task> spTask);

   /// stores task info for completed (or stopped) task
   void StoreCompletedTaskInfo(std::shared_ptr<Task> spTask, CString& errorText);

   /// removes task from queue
   void RemoveTask(std::shared_ptr<Task> spTask);

   /// sets busy flag for thread
   void SetBusyFlag(DWORD dwThreadId, bool bBusy);

private:
   /// task manager configuration
   TaskManagerConfig m_config;


   // task queue

   /// next task id
   std::atomic<unsigned int> m_nextTaskId;

   /// mutex protecting task queue
   mutable std::recursive_mutex m_mutexQueue;

   /// task queue typedef
   typedef std::deque<std::shared_ptr<Task>> T_deqTaskQueue;

   /// task queue, protected by queue mutex
   T_deqTaskQueue m_deqTaskQueue;


   // task bookkeeping

   /// task infos of all completed tasks, protected by queue mutex
   std::map<unsigned int, TaskInfo> m_mapCompletedTaskInfos;

   /// set with all finished task ids
   std::set<unsigned int> m_setFinishedTaskIds;


   // thread pool

   /// io context
   boost::asio::io_context m_ioContext;

   /// default work for io context
   boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_defaultWork;

   /// thread pool
   std::vector<std::shared_ptr<std::thread>> m_vecThreadPool;


   // busy flags

   /// mutex protecting busy flag map
   std::recursive_mutex m_mutexBusyFlagMap;

   /// busy flag map type
   typedef std::map<DWORD, bool> T_mapBusyFlagMap;

   /// busy flag map
   T_mapBusyFlagMap m_mapBusyFlagMap;
};
