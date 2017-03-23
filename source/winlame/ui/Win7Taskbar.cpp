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
/// \file Win7Taskbar.cpp
/// \brief Windows 7 Taskbar classes
//
#include "StdAfx.h"
#include "Win7Taskbar.hpp"
#include <ShObjIdl.h>

using Win7::TaskbarProgressBar;
using Win7::TaskbarImpl;
using Win7::Taskbar;

/// task bar implementation
class TaskbarImpl
{
public:
   HWND m_hwnd;   ///< window handle of window
   CComPtr<ITaskbarList3> m_taskBarList;  ///< task bar list interface 3
};

void TaskbarProgressBar::SetState(TaskbarProgressBarState state)
{
   ATLASSERT(
      state == TBPF_NOPROGRESS ||
      state == TBPF_INDETERMINATE ||
      state == TBPF_NORMAL ||
      state == TBPF_ERROR ||
      state == TBPF_PAUSED);

   m_impl->m_taskBarList->SetProgressState(
      m_impl->m_hwnd,
      static_cast<TBPFLAG>(state));
}

void TaskbarProgressBar::SetPos(UINT currentPos, UINT maxPos)
{
   m_impl->m_taskBarList->SetProgressValue(m_impl->m_hwnd, currentPos, maxPos);
}

Taskbar::Taskbar(HWND hwnd)
:m_impl(new TaskbarImpl)
{
   m_impl->m_hwnd = hwnd;
   HRESULT hr = m_impl->m_taskBarList.CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL);
   ATLVERIFY(SUCCEEDED(hr));
}

bool Taskbar::IsAvailable() const
{
   return m_impl->m_taskBarList != nullptr;
}
