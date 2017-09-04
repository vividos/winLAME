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
/// \file Task.hpp
/// \brief Task class

// include guard
#pragma once

// includes
#include "TaskInfo.hpp"

/// task interface
class Task
{
public:
   /// ctor
   Task(unsigned int dependentTaskId = 0)
      :m_id(0),
      m_dependentTaskId(dependentTaskId),
      m_isStarted(false)
   {
   }
   /// dtor
   virtual ~Task() {}

   /// returns task id
   unsigned int Id() const { return m_id; }

   /// returns current task info; must return immediately
   virtual TaskInfo GetTaskInfo() = 0;

   /// runs task; may take longer
   virtual void Run() = 0;

   /// task should be aborted, e.g. when program is closed
   virtual void Stop() = 0;

   /// returns if task was already started
   bool IsStarted() const { return m_isStarted; }

protected:
   friend class TaskManager;

   /// sets task id
   void Id(unsigned int id) { m_id = id; }

   /// sets the "is started" flag
   void IsStarted(bool isStarted) { m_isStarted = isStarted; }

   /// sets task error text
   void SetTaskError(UINT stringResourceId)
   {
      m_errorText.LoadString(stringResourceId);

      // must be non-empty to be recognized as error
      ATLASSERT(!m_errorText.IsEmpty());
   }

   /// sets task error text
   void SetTaskError(const CString& errorText)
   {
      m_errorText = errorText;

      // must be non-empty to be recognized as error
      ATLASSERT(!m_errorText.IsEmpty());
   }

   /// returns dependent task id
   unsigned int DependentTaskId() const { return m_dependentTaskId; }

   /// returns error text, if any
   const CString& ErrorText() const { return m_errorText; }

private:
   /// task id
   unsigned int m_id;

   /// task id this task depends on; may be 0 for no task
   unsigned int m_dependentTaskId;

   /// flag that indicates if the task already has been started
   std::atomic<bool> m_isStarted;

   /// error text, or empty when not set yet
   CString m_errorText;
};
