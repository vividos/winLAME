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
/// \file TaskManager.cpp
/// \brief Task manager

// includes
#include "stdafx.h"
#include "TaskManager.hpp"
#include "CDExtractTask.hpp"
#include "Task.hpp"
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <set>

TaskManager::TaskManager()
:m_nextTaskId(1),
 m_upDefaultWork(new boost::asio::io_service::work(m_ioService))
{
   // find out number of threads to start
   unsigned int uiNumThreads = m_config.m_uiUseNumTasks;
   if (m_config.m_bAutoTasksPerCpu)
   {
      uiNumThreads = std::thread::hardware_concurrency();
      if (uiNumThreads == 0)
         uiNumThreads = m_config.m_uiUseNumTasks;
   }

   // start up threads
   for (unsigned int i=0; i<uiNumThreads; i++)
   {
      std::shared_ptr<std::thread> spThread(
         new std::thread(
            std::bind(&TaskManager::RunThread, boost::ref(m_ioService))
      ));

      // note: GetThreadId() not available in XP
      //SetBusyFlag(GetThreadId(spThread->native_handle()), false);

      m_vecThreadPool.push_back(spThread);
   }
}

TaskManager::~TaskManager()
{
   // stop all tasks
   StopAll();

   // stop threads
   m_upDefaultWork.reset();

   for (unsigned int i=0, iMax=m_vecThreadPool.size(); i<iMax; i++)
      m_vecThreadPool[i]->join();

   m_vecThreadPool.clear();
}

std::vector<TaskInfo> TaskManager::CurrentTasks()
{
   std::vector<TaskInfo> vecTaskInfos;

   {
      boost::recursive_mutex::scoped_lock lock(m_mutexQueue);
      BOOST_FOREACH(std::shared_ptr<Task> spTask, m_deqTaskQueue)
      {
         auto iter = m_mapCompletedTaskInfos.find(spTask->Id());
         if (iter != m_mapCompletedTaskInfos.end())
         {
            // already completed
            vecTaskInfos.push_back(iter->second);
         }
         else
         {
            // still running, query task for info
            vecTaskInfos.push_back(spTask->GetTaskInfo());
         }
      }
   } // lock release

   return vecTaskInfos;
};

TaskManagerConfig TaskManager::CurrentConfig() const
{
   boost::recursive_mutex::scoped_lock lock(const_cast<boost::recursive_mutex&>(m_mutexConfig));
   TaskManagerConfig config = m_config;
   return config;
}

void TaskManager::SetNewConfig(const TaskManagerConfig& config)
{
   boost::recursive_mutex::scoped_lock lock(m_mutexConfig);

   m_config = config;

   // TODO reduce/start new threads
}

void TaskManager::AddTask(std::shared_ptr<Task> spTask)
{
   unsigned int taskId = m_nextTaskId++;
   spTask->Id(taskId);

   {
      boost::recursive_mutex::scoped_lock lock(m_mutexQueue);
      m_deqTaskQueue.push_back(spTask);
   }

   if (IsTaskRunnable(spTask))
   {
      m_ioService.post(
         std::bind(&TaskManager::RunTask, this, spTask));
   }
}

void TaskManager::CheckRunnableTasks()
{
   boost::recursive_mutex::scoped_lock lock(m_mutexQueue);

   std::for_each(m_deqTaskQueue.begin(), m_deqTaskQueue.end(),
      [&](std::shared_ptr<Task>& spTask)
   {
      auto iter = m_mapCompletedTaskInfos.find(spTask->Id());
      if (iter != m_mapCompletedTaskInfos.end())
         return; // already completed

      TaskInfo info = spTask->GetTaskInfo();
      if (m_mapCompletedTaskInfos.find(spTask->Id()) == m_mapCompletedTaskInfos.end() &&
         info.Status() == TaskInfo::statusWaiting &&
         IsTaskRunnable(spTask))
      {
         m_ioService.post(
            std::bind(&TaskManager::RunTask, this, spTask));
      }
   });
}

bool TaskManager::IsQueueEmpty() const throw()
{
   bool isEmpty = true;

   try
   {
      boost::recursive_mutex::scoped_lock lock(
         const_cast<boost::recursive_mutex&>(m_mutexQueue));
      isEmpty = m_deqTaskQueue.empty();
   }
   catch (...)
   {
   }

   return isEmpty;
}

bool TaskManager::AreRunningTasksAvail() const
{
   boost::recursive_mutex::scoped_lock lock(
      const_cast<boost::recursive_mutex&>(m_mutexQueue));

   return m_mapCompletedTaskInfos.size() < m_deqTaskQueue.size();
}

bool TaskManager::AreCompletedTasksAvail() const
{
   boost::recursive_mutex::scoped_lock lock(
      const_cast<boost::recursive_mutex&>(m_mutexQueue));

   return !m_mapCompletedTaskInfos.empty();
}

bool TaskManager::AreCDExtractTasksRunning() const
{
   boost::recursive_mutex::scoped_lock lock(
      const_cast<boost::recursive_mutex&>(m_mutexQueue));

   bool found = false;
   std::for_each(m_deqTaskQueue.begin(), m_deqTaskQueue.end(),
      [&](const std::shared_ptr<Task>& spTask)
   {
      if (std::dynamic_pointer_cast<Encoder::CDExtractTask>(spTask) == nullptr)
         return; // no CD extract task

      if (m_mapCompletedTaskInfos.find(spTask->Id()) != m_mapCompletedTaskInfos.end())
         return; // already completed

      TaskInfo info = spTask->GetTaskInfo();

      if (info.Status() == TaskInfo::statusRunning ||
         info.Status() == TaskInfo::statusWaiting)
      {
         found = true;
      }
   });

   return found;
}

void TaskManager::GetTaskListState(bool& hasActiveTasks, bool& hasErrorTasks, unsigned int& percentComplete) const
{
   boost::recursive_mutex::scoped_lock lock(
      const_cast<boost::recursive_mutex&>(m_mutexQueue));

   if (m_deqTaskQueue.empty())
   {
      hasActiveTasks = hasErrorTasks = false;
      percentComplete = 100;

      return;
   }

   hasActiveTasks = false;
   hasErrorTasks = false;

   unsigned int numTasks = 0;
   unsigned int taskPercentageSum = 0;

   std::for_each(m_deqTaskQueue.begin(), m_deqTaskQueue.end(),
      [&](const std::shared_ptr<Task>& spTask)
   {
      TaskInfo info = spTask->GetTaskInfo();

      switch (info.Status())
      {
      case TaskInfo::statusWaiting:
         numTasks++;
         break;

      case TaskInfo::statusRunning:
         taskPercentageSum += info.Progress();
         hasActiveTasks = true;
         numTasks++;
         break;

      case TaskInfo::statusError:
         hasErrorTasks = true;
         // fall-through
      case TaskInfo::statusCompleted:
         numTasks++;
         taskPercentageSum += 100;
         break;

      default:
         ATLASSERT(false);
         break;
      }
   });

   percentComplete = numTasks == 0 ? 0 : (taskPercentageSum / numTasks);
}

void TaskManager::StopAll()
{
   boost::recursive_mutex::scoped_lock lock(m_mutexQueue);

   BOOST_FOREACH(std::shared_ptr<Task> spTask, m_deqTaskQueue)
   {
      spTask->Stop();

      CString errorText;
      StoreCompletedTaskInfo(spTask, errorText);
   }

   m_setFinishedTaskIds.clear();
}

void TaskManager::RemoveCompletedTasks()
{
   boost::recursive_mutex::scoped_lock lock(m_mutexQueue);

   std::set<std::shared_ptr<Task>> setTasksToRemove;
   BOOST_FOREACH(std::shared_ptr<Task> spTask, m_deqTaskQueue)
   {
      if (m_mapCompletedTaskInfos.find(spTask->Id()) != m_mapCompletedTaskInfos.end())
         setTasksToRemove.insert(spTask);
   }

   BOOST_FOREACH(std::shared_ptr<Task> spTask, setTasksToRemove)
   {
      RemoveTask(spTask);
   }
}

void TaskManager::RunThread(boost::asio::io_service& ioService)
{
   try
   {
      ioService.run();
   }
   catch(boost::system::system_error& error)
   {
      error;
      ATLTRACE(_T("system_error: %hs\n"), error.what());
      ATLASSERT(false);
   }
}

bool TaskManager::IsTaskRunnable(std::shared_ptr<Task> spTask) const
{
   boost::recursive_mutex::scoped_lock lock(const_cast<boost::recursive_mutex&>(m_mutexQueue));

   // check dependent task ID
   unsigned int dependentTaskId = spTask->DependentTaskId();

   if (dependentTaskId == 0)
      return true;

   if (m_setFinishedTaskIds.find(dependentTaskId) == m_setFinishedTaskIds.end())
   {
      // task id wasn't reported as finished yet
      return false;
   }

   return true;
}

void TaskManager::RunTask(std::shared_ptr<Task> spTask)
{
   SetBusyFlag(GetCurrentThreadId(), true);

   CString errorText;
   try
   {
      spTask->Run();
   }
   catch (const std::exception& ex)
   {
      errorText.Format(_T("Exception: %hs"), ex.what());
   }
   catch (...)
   {
      errorText = _T("Unknown Exception");
   }

   SetBusyFlag(GetCurrentThreadId(), false);

   StoreCompletedTaskInfo(spTask, errorText);
}

void TaskManager::StoreCompletedTaskInfo(std::shared_ptr<Task> spTask, CString& errorText)
{
   // store the last task info for the completed task
   TaskInfo info = spTask->GetTaskInfo();

   if (errorText.IsEmpty())
      errorText = spTask->ErrorText();

   if (!errorText.IsEmpty())
   {
      info.Status(TaskInfo::statusError);

      if (info.Progress() > 100)
         info.Progress(100);
   }

   if (info.Status() == TaskInfo::statusCompleted)
      info.Progress(100);

   {
      boost::recursive_mutex::scoped_lock lock(m_mutexQueue);

      m_mapCompletedTaskInfos.insert(std::make_pair(spTask->Id(), info));

      m_setFinishedTaskIds.insert(spTask->Id());
   }
}

void TaskManager::RemoveTask(std::shared_ptr<Task> spTask)
{
   boost::recursive_mutex::scoped_lock lock(m_mutexQueue);

   // search for task
   for (T_deqTaskQueue::iterator iterTaskQueue = m_deqTaskQueue.begin(),
      stop = m_deqTaskQueue.end(); iterTaskQueue != stop; ++iterTaskQueue)
   {
      if (spTask == *iterTaskQueue)
      {
         m_deqTaskQueue.erase(iterTaskQueue);

         auto iterTaskInfos = m_mapCompletedTaskInfos.find(spTask->Id());
         if (iterTaskInfos != m_mapCompletedTaskInfos.end())
            m_mapCompletedTaskInfos.erase(iterTaskInfos);

         return;
      }
   }

   ATLASSERT(false); // task not in queue anymore? should not happen
}

void TaskManager::SetBusyFlag(DWORD dwThreadId, bool bBusy)
{
   boost::recursive_mutex::scoped_lock lock(m_mutexBusyFlagMap);

   m_mapBusyFlagMap[dwThreadId] = bBusy;
}
