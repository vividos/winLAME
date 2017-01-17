//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file TasksView.hpp
/// \brief Tasks view window
//
#pragma once

// includes
#include "TaskInfo.hpp"

// forward references
class TaskManager;

namespace UI
{
/// win traits for tasks view
typedef CWinTraitsOR<LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER, 0, CControlWinTraits>
   TasksViewWinTraits;

/// tasks view; shows all currently running tasks
class TasksView : public CWindowImpl<TasksView, CListViewCtrl, TasksViewWinTraits>
{
public:
   /// function type of handler called when a task item was clicked
   typedef std::function<void(size_t clickedIndex)> T_fnOnClickedTask;

   /// ctor
   TasksView(TaskManager& taskManager)
      :m_taskManager(taskManager)
   {
   }

   /// initialize tasks view list
   void Init();

   /// sets "clicked task" handler
   void SetClickedTaskHandler(T_fnOnClickedTask fnOnClickedTask)
   {
      m_fnOnClickedTask = fnOnClickedTask;
   }

   /// update tasks list
   void UpdateTasks();

   DECLARE_WND_SUPERCLASS(NULL, CListViewCtrl::GetWndClassName())

   BOOL PreTranslateMessage(MSG* pMsg);

private:
   BEGIN_MSG_MAP(TasksView)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_TIMER, OnTimer)
      REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnItemClick)
   END_MSG_MAP()

   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when a task item has been clicked
   LRESULT OnItemClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
   /// returns status text from task status
   static CString StatusTextFromStatus(TaskInfo::TaskStatus status);

   /// determines icon from task type
   static int IconFromTaskType(const TaskInfo& info);

private:
   // model

   /// ref to task manager
   TaskManager& m_taskManager;

   /// "clicked task" handler
   T_fnOnClickedTask m_fnOnClickedTask;

   // UI

   /// task list images
   CImageList m_taskImages;
};

} // namespace UI
