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
/// \file Task.h
/// \brief Task class

// include guard
#pragma once

// includes
#include "TaskInfo.h"

/// task interface
class Task
{
public:
   /// ctor
   Task(unsigned int dependentTaskId = 0)
      :m_dependentTaskId(dependentTaskId)
   {
   }
   /// dtor
   virtual ~Task() throw() {}

   /// returns task id
   unsigned int Id() const throw() { return m_id; }

   /// returns current task info; must return immediately
   virtual TaskInfo GetTaskInfo() = 0;

   /// runs task; may take longer
   virtual void Run() = 0;

   /// task should be aborted, e.g. when program is closed
   virtual void Stop() = 0;

protected:
   friend class TaskManager;

   /// sets task id
   void Id(unsigned int id) throw() { m_id = id; }

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
   unsigned int DependentTaskId() const throw() { return m_dependentTaskId; }

   /// returns error text, if any
   const CString& ErrorText() const throw() { return m_errorText; }

private:
   /// task id
   unsigned int m_id;

   /// task id this task depends on; may be 0 for no task
   unsigned int m_dependentTaskId;

   /// error text, or empty when not set yet
   CString m_errorText;
};
