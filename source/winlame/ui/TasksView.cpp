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
/// \file TasksView.cpp
/// \brief Tasks view window
//
#include "stdafx.h"
#include "resource.h"
#include "TasksView.hpp"
#include "TaskManager.h"
#include "TaskInfo.h"

using UI::TasksView;

BOOL TasksView::PreTranslateMessage(MSG* pMsg)
{
   pMsg;
   return FALSE;
}

void TasksView::Init()
{
   // TODO translate
   InsertColumn(0, _T("Track"), LVCFMT_LEFT, 400);
   InsertColumn(1, _T("Progress"), LVCFMT_LEFT, 200);

   SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}

void TasksView::UpdateTasks()
{
   SetRedraw(FALSE);

   DeleteAllItems();

   std::vector<TaskInfo> vecTaskInfos = m_taskManager.CurrentTasks();

   if (vecTaskInfos.empty())
   {
      // TODO translate
      InsertItem(0, _T("<No Task>"));
      return;
   }

   for (size_t i=0, iMax=vecTaskInfos.size(); i<iMax; i++)
   {
      const TaskInfo& info = vecTaskInfos[i];

      int iItem = InsertItem(IconFromTaskType(info), info.Name());

      CString cszProgress;
      // TODO translate
      cszProgress.Format(_T("%u%% done"), info.Progress());
      SetItemText(iItem, 1, cszProgress);
   }

   SetRedraw(TRUE);
}

int TasksView::IconFromTaskType(const TaskInfo& /*info*/)
{
   // TODO
   return 1;
}
