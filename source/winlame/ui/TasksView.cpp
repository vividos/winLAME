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
/// \file TasksView.cpp
/// \brief Tasks view window
//
#include "stdafx.h"
#include "resource.h"
#include "TasksView.hpp"
#include "TaskManager.hpp"
#include "TaskInfo.hpp"
#include "RedrawLock.hpp"
#include <set>

using UI::TasksView;

/// timer id for updating the list
const UINT c_timerIdUpdateList = 128;

/// update cycle time in milliseconds
const UINT c_updateCycleInMilliseconds = 2 * 100;

/// item ID that represents "no data" entry
const UINT c_itemIdNoData = 0xffffffff;

/// index of name column
const int c_nameColumn = 0;

/// index of progress column
const int c_progressColumn = 1;

/// index of status column
const int c_statusColumn = 2;

BOOL TasksView::PreTranslateMessage(MSG* /*msg*/)
{
   return FALSE;
}

LRESULT TasksView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

   noFlickerClass::Initialize(this->GetHeader());

   // already called DefWindowProc
   bHandled = TRUE;
   return lRet;
}

LRESULT TasksView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   noFlickerClass::Uninitialize();

   KillTimer(c_timerIdUpdateList);

   return 0;
}

LRESULT TasksView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if (wParam == c_timerIdUpdateList)
      UpdateTasks();

   return 0;
}

LRESULT TasksView::OnItemClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
   if (m_fnOnClickedTask != nullptr)
   {
      LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pnmh;

      unsigned int itemIndex = static_cast<unsigned int>(lpnmitem->iItem);

      m_fnOnClickedTask(itemIndex);

      if (lpnmitem->iItem >= 0)
         SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
   }

   return 0;
}

void TasksView::Init(bool useLargeNameColumn)
{
   InsertColumn(c_nameColumn, CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_VIEW_COLUMN_TRACK)), LVCFMT_LEFT, useLargeNameColumn ? 500 : 350);
   InsertColumn(c_progressColumn, CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_VIEW_COLUMN_PROGRESS)), LVCFMT_LEFT, 100);
   InsertColumn(c_statusColumn, CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_VIEW_COLUMN_STATUS)), LVCFMT_LEFT, 100);

   DWORD dwExStyle = LVS_EX_FULLROWSELECT;
   SetExtendedListViewStyle(dwExStyle, dwExStyle);

   // task images
   m_taskImages.Create(16, 16, ILC_MASK | ILC_COLOR32, 0, 0);
   CBitmap bmpImages;
   // load bitmap, but always from main module (bmp not in translation dlls)
   bmpImages.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP_TASKS)));
   m_taskImages.Add(bmpImages, RGB(0, 0, 0));

   SetImageList(m_taskImages, LVSIL_SMALL);

   SetTimer(c_timerIdUpdateList, c_updateCycleInMilliseconds);
}

void TasksView::UpdateTasks()
{
   int topIndex = GetTopIndex();

   std::set<int> selectedTaskIds;
   {
      int selectedItemIndex = GetNextItem(-1, LVNI_SELECTED);
      while (selectedItemIndex != -1)
      {
         selectedTaskIds.insert(static_cast<int>(GetItemData(selectedItemIndex)));

         selectedItemIndex = GetNextItem(selectedItemIndex, LVNI_SELECTED);
      }
   }

   std::set<int> itemIndicesToSelect;

   RedrawLock lock(*this);
   DeleteAllItems();

   std::vector<TaskInfo> taskInfoList = m_taskManager.CurrentTasks();

   if (taskInfoList.empty())
   {
      int itemIndex = InsertItem(0, CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_VIEW_NO_TASK)));
      SetItemData(itemIndex, c_itemIdNoData);

      return;
   }

   for (size_t infoIndex = 0, maxInfoIndex = taskInfoList.size(); infoIndex < maxInfoIndex; infoIndex++)
   {
      const TaskInfo& info = taskInfoList[infoIndex];

      int itemIndex = InsertItem(GetItemCount(), info.Name(), IconFromTaskType(info));
      SetItemData(itemIndex, info.Id());

      CString progressText;
      progressText.Format(IDS_MAIN_TASKS_PERCENT_DONE_U, info.Progress());

      SetItemText(itemIndex, c_progressColumn, progressText);

      CString statusText = StatusTextFromStatus(info.Status());
      SetItemText(itemIndex, c_statusColumn, statusText);

      // select item when previously selected
      if (selectedTaskIds.find(info.Id()) != selectedTaskIds.end())
      {
         itemIndicesToSelect.insert(itemIndex);
      }
   }

   for (auto iter = itemIndicesToSelect.begin(); iter != itemIndicesToSelect.end(); iter++)
   {
      int itemIndex = *iter;
      SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
   }

   if (topIndex > 0 && topIndex < GetItemCount())
   {
      // first scroll to the end, then back up to the previous top index
      EnsureVisible(GetItemCount()-1, false);
      EnsureVisible(topIndex, false);
   }
}

CString TasksView::StatusTextFromStatus(TaskInfo::TaskStatus status)
{
   switch (status)
   {
   case TaskInfo::statusWaiting:
      return CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_STATUS_WAITING));
   case TaskInfo::statusRunning:
      return CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_STATUS_RUNNING));
   case TaskInfo::statusCompleted:
      return CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_STATUS_COMPLETED));
   case TaskInfo::statusError:
      return CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_STATUS_ERROR));
   default:
      ATLASSERT(false);
      return CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_STATUS_UNKNOWN));
   }
}

int TasksView::IconFromTaskType(const TaskInfo& info)
{
   switch (info.Type())
   {
   case TaskInfo::taskEncoding:     return 1;
   case TaskInfo::taskCdExtraction: return 2;
   case TaskInfo::taskWritePlaylist: return 3;
   case TaskInfo::taskUnknown:
   default:
      ATLASSERT(false);
      break;
   }

   return 0;
}
