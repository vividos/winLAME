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
#include "TaskManager.h"
#include "Task.h"
#include <boost/foreach.hpp>

TaskManager::TaskManager()
:m_scpDefaultWork(new boost::asio::io_service::work(m_ioService))
{
   // find out number of threads to start
   unsigned int uiNumThreads = m_config.m_uiUseNumTasks;
   if (m_config.m_bAutoTasksPerCpu)
   {
      uiNumThreads = boost::thread::hardware_concurrency();
      if (uiNumThreads == 0)
         uiNumThreads = m_config.m_uiUseNumTasks;
   }

   // start up threads
   for (unsigned int i=0; i<uiNumThreads; i++)
   {
      boost::shared_ptr<boost::thread> spThread(
         new boost::thread(
            boost::bind(&TaskManager::RunThread, boost::ref(m_ioService))
      ));

      SetBusyFlag(GetThreadId(spThread->native_handle()), false);

      m_vecThreadPool.push_back(spThread);
   }
}

TaskManager::~TaskManager()
{
   // stop all tasks
   StopAll();

   // stop threads
   m_scpDefaultWork.reset();

   for (unsigned int i=0, iMax=m_vecThreadPool.size(); i<iMax; i++)
      m_vecThreadPool[i]->join();

   m_vecThreadPool.clear();
}

std::vector<TaskInfo> TaskManager::CurrentTasks()
{
   std::vector<TaskInfo> vecTaskInfos;

   {
      boost::recursive_mutex::scoped_lock lock(m_mutexQueue);
      BOOST_FOREACH(boost::shared_ptr<Task> spTask, m_deqTaskQueue)
      {
         // query task for info
         vecTaskInfos.push_back(spTask->GetTaskInfo());
      }
   } // lock release

   return vecTaskInfos;
};

TaskManagerConfig TaskManager::CurrentConfig() const
{
   boost::recursive_mutex::scoped_lock lock(m_mutexConfig);
   TaskManagerConfig config = m_config;
   return config;
}

void TaskManager::SetNewConfig(const TaskManagerConfig& config)
{
   boost::recursive_mutex::scoped_lock lock(m_mutexConfig);

   m_config = config;

   // TODO reduce/start new threads
}

void TaskManager::AddTask(boost::shared_ptr<Task> spTask)
{
   {
      boost::recursive_mutex::scoped_lock lock(m_mutexQueue);
      m_deqTaskQueue.push_back(spTask);
   }

   m_ioService.post(
      boost::bind(&TaskManager::RunTask, this, spTask));
}

bool TaskManager::IsQueueEmpty() const
{
   bool bIsEmpty;
   {
      boost::recursive_mutex::scoped_lock lock(
         const_cast<boost::recursive_mutex&>(m_mutexQueue));
      bIsEmpty = m_deqTaskQueue.empty();
   }

   return bIsEmpty;
}

void TaskManager::StopAll()
{
   boost::recursive_mutex::scoped_lock lock(m_mutexQueue);

   BOOST_FOREACH(boost::shared_ptr<Task> spTask, m_deqTaskQueue)
   {
      spTask->Stop();
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
      ATLTRACE(_T("system_error: %hs\n"), error.what());
      ATLASSERT(false);
   }
}

void TaskManager::RunTask(boost::shared_ptr<Task> spTask)
{
   SetBusyFlag(GetCurrentThreadId(), true);

   spTask->Run();

   SetBusyFlag(GetCurrentThreadId(), false);

   // remove from queue
   RemoveTask(spTask);
}

void TaskManager::RemoveTask(boost::shared_ptr<Task> spTask)
{
   boost::recursive_mutex::scoped_lock lock(m_mutexQueue);

   // search for task
   for (T_deqTaskQueue::iterator iter = m_deqTaskQueue.begin(),
      stop = m_deqTaskQueue.end(); iter != stop; iter++)
   {
      if (spTask == *iter)
      {
         m_deqTaskQueue.erase(iter);
         return;
      }

      ATLASSERT(false); // task not in queue anymore? should not happen
   }
}

void TaskManager::SetBusyFlag(DWORD dwThreadId, bool bBusy)
{
   boost::recursive_mutex::scoped_lock lock(m_mutexBusyFlagMap);

   m_mapBusyFlagMap[dwThreadId] = bBusy;
}
