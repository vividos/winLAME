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

/// timer id for updating the list
const UINT c_uiTimerIdUpdateList = 128;

/// update cycle time in milliseconds
const UINT c_uiUpdateCycleInMilliseconds = 2 * 100;

const UINT ITEM_ID_NODATA = 0xffffffff;

BOOL TasksView::PreTranslateMessage(MSG* pMsg)
{
   pMsg;
   return FALSE;
}

LRESULT TasksView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   KillTimer(c_uiTimerIdUpdateList);
   return 0;
}

LRESULT TasksView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if (wParam == c_uiTimerIdUpdateList)
      UpdateTasks();

   return 0;
}

void TasksView::Init()
{
   // TODO translate
   InsertColumn(0, _T("Track"), LVCFMT_LEFT, 400);
   InsertColumn(1, _T("Progress"), LVCFMT_LEFT, 200);

   DWORD dwExStyle = LVS_EX_FULLROWSELECT;
   SetExtendedListViewStyle(dwExStyle, dwExStyle);

   SetTimer(c_uiTimerIdUpdateList, c_uiUpdateCycleInMilliseconds);
}

void TasksView::UpdateTasks()
{
   SetRedraw(false);

   std::vector<TaskInfo> vecTaskInfos = m_taskManager.CurrentTasks();

   if (vecTaskInfos.empty())
   {
      DeleteAllItems();

      // TODO translate
      int iItem = InsertItem(0, _T("<No Task>"));
      SetItemData(iItem, ITEM_ID_NODATA);

      SetRedraw(true);
      return;
   }

   // this loop assumes that tasks don't get reordered, and new tasks get added at the end
   int iItem = 0;
   size_t iInfos = 0;
   for (size_t iMaxInfos = vecTaskInfos.size(); iInfos<iMaxInfos; )
   {
      if (iItem >= GetItemCount())
         break;

      const TaskInfo& info = vecTaskInfos[iInfos];

      unsigned int uiId = GetItemData(iItem);

      if (uiId == info.Id())
      {
         UpdateExistingItem(iItem, info);
         ++iItem;
         ++iInfos;
         continue;
      }

      if (iItem >= GetItemCount())
         break; // no more items in list

      // current item isn't in vector anymore; remove item
      DeleteItem(iItem);
   }

   // add all new items
   for (size_t iMaxInfos = vecTaskInfos.size(); iInfos < iMaxInfos; iInfos++)
      InsertNewItem(vecTaskInfos[iInfos]);

   SetRedraw(true);
}

void TasksView::InsertNewItem(const TaskInfo& info)
{
   int iItem = InsertItem(IconFromTaskType(info), info.Name());
   SetItemData(iItem, info.Id());

   UpdateExistingItem(iItem, info);
}

void TasksView::UpdateExistingItem(int iItem, const TaskInfo& info)
{
   // TODO translate
   CString cszProgress;
   cszProgress.Format(_T("%u%% done"), info.Progress());

   SetItemText(iItem, 1, cszProgress);
}

int TasksView::IconFromTaskType(const TaskInfo& info)
{
   switch (info.Type())
   {
   case TaskInfo::taskEncoding:     return 0;
   case TaskInfo::taskCdExtraction: return 1;
   case TaskInfo::taskOther:        return 2;
   default:
      ATLASSERT(false);
   }

   return 0;
}
