//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2015 Michael Fink
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

// forward references
class TaskManager;
class TaskInfo;

namespace UI
{
/// win traits for tasks view
typedef CWinTraitsOR<LVS_REPORT | LVS_SHOWSELALWAYS, 0, CControlWinTraits>
   TasksViewWinTraits;

/// tasks view; shows all currently running tasks
class TasksView : public CWindowImpl<TasksView, CListViewCtrl, TasksViewWinTraits>
{
public:
   /// ctor
   TasksView(TaskManager& taskManager)
      :m_taskManager(taskManager)
   {
   }

   /// initialize tasks view list
   void Init();

   /// update tasks list
   void UpdateTasks();

   DECLARE_WND_SUPERCLASS(NULL, CListViewCtrl::GetWndClassName())

   BOOL PreTranslateMessage(MSG* pMsg);

private:
   BEGIN_MSG_MAP(TasksView)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_TIMER, OnTimer)
   END_MSG_MAP()

   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
   void InsertNewItem(const TaskInfo& info);
   void UpdateExistingItem(int iItem, const TaskInfo& info);

   /// determines icon from task type
   static int IconFromTaskType(const TaskInfo& info);

private:
   /// ref to task manager
   TaskManager& m_taskManager;
};

} // namespace UI
