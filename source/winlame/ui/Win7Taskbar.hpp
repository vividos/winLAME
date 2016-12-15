//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file Win7Taskbar.hpp
/// \brief Windows 7 Taskbar classes
//
#pragma once

/// \brief Windows 7 specific classes
namespace Win7
{
// forward references
class TaskbarImpl;

/// Taskbar progress bar access
class TaskbarProgressBar
{
public:
   /// state of task bar progress bar
   enum TaskbarProgressBarState
   {
      TBPF_NOPROGRESS = 0,       ///< show no progress
      TBPF_INDETERMINATE = 0x1,  ///< show indeterminate progress
      TBPF_NORMAL = 0x2,         ///< show normal progress bar (green)
      TBPF_ERROR = 0x4,          ///< show error progress bar (red)
      TBPF_PAUSED = 0x8          ///< show paused progress bar
   };

   /// dtor; returns progress to "none"
   ~TaskbarProgressBar()
   {
      SetState(TBPF_NOPROGRESS);
   }

   /// sets new progress bar state
   void SetState(TaskbarProgressBarState state);

   /// sets new progress bar position
   void SetPos(UINT currentPos, UINT maxPos);

private:
   friend class Taskbar;

   /// ctor; can only be called from Taskbar
   TaskbarProgressBar(std::shared_ptr<TaskbarImpl> impl)
      :m_impl(impl)
   {
      SetState(TBPF_INDETERMINATE);
   }

private:
   /// implementation
   std::shared_ptr<TaskbarImpl> m_impl;
};

/// Windows 7 taskbar access
class Taskbar
{
public:
   /// accesses task bar; uses task bar icon associated with given window
   Taskbar(HWND hwnd) throw();

   /// returns if task bar is available (Windows 7 and higher)
   bool IsAvailable() const throw();

   /// dtor
   ~Taskbar() throw()
   {
   }

   /// opens progress bar
   TaskbarProgressBar OpenProgressBar()
   {
      ATLASSERT(IsAvailable());
      return TaskbarProgressBar(m_impl);
   }

private:
   std::shared_ptr<TaskbarImpl> m_impl;
};

} // namespace Win7
