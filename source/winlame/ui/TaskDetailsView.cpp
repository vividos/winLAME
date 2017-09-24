//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2017 Michael Fink
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
/// \brief Tasks detail view window
//
#include "stdafx.h"
#include "TaskDetailsView.hpp"
#include "TaskInfo.hpp"
#include "TasksView.hpp"

using UI::TaskDetailsView;

LRESULT TaskDetailsView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   DlgResize_Init(false);

   // task images
   m_taskImages.Create(16, 16, ILC_MASK | ILC_COLOR32, 0, 0);
   CBitmap bmpImages;
   // load bitmap, but always from main module (bmp not in translation dlls)
   bmpImages.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP_TASKS)));
   m_taskImages.Add(bmpImages, RGB(0, 0, 0));

   m_staticIconTaskType.ModifyStyle(SS_BLACKFRAME, SS_ICON | SS_REALSIZECONTROL);
   m_staticIconTaskType.SetWindowPos(nullptr, 0, 0, 32, 32, SWP_NOMOVE);

   CFontHandle font = AtlGetDefaultGuiFont();
   CLogFont logFont;
   font.GetLogFont(logFont);
   logFont.MakeBolder(1);
   logFont.MakeLarger(10);
   m_captionFont = logFont.CreateFontIndirect();

   ResetTaskDetails();

   return TRUE;
}

void TaskDetailsView::ResetTaskDetails()
{
   m_staticTextTaskType.SetFont(AtlGetDefaultGuiFont());

   m_staticTextTaskType.SetWindowText(CString(MAKEINTRESOURCE(IDS_MAIN_TASKS_TASK_DETAILS_SELECT_TASK)));

   m_staticIconTaskType.ShowWindow(SW_HIDE);

   m_editTextFilenameTrack.SetWindowText(_T(""));
   m_editTextTaskDescription.SetWindowText(_T(""));
}

void TaskDetailsView::UpdateTaskDetails(const TaskInfo& taskInfo)
{
   m_staticTextTaskType.SetFont(m_captionFont);

   m_staticIconTaskType.ShowWindow(SW_SHOW);

   CIconHandle icon = m_taskImages.GetIcon(TasksView::IconFromTaskType(taskInfo));
   m_staticIconTaskType.SetIcon(icon);

   m_staticTextTaskType.SetWindowText(TaskDetailsView::TaskTypeFromInfo(taskInfo));

   m_staticLabelFilenameTrack.SetWindowText(TaskDetailsView::LabelFilenameOrTrackFromInfo(taskInfo));

   m_editTextFilenameTrack.SetWindowText(taskInfo.Name());

   CString description = taskInfo.Description();

   description.Replace(_T("\n"), _T("\r\n"));
   description += _T("\r\n\r\n");

   m_editTextTaskDescription.SetWindowText(description);
}

CString TaskDetailsView::TaskTypeFromInfo(const TaskInfo& info)
{
   int resourceID = -1;
   switch (info.Type())
   {
   case TaskInfo::taskEncoding: resourceID = IDS_MAIN_TASKS_TASKTYPE_ENCODE; break;
   case TaskInfo::taskCdExtraction: resourceID = IDS_MAIN_TASKS_TASKTYPE_CDREAD; break;
   case TaskInfo::taskWritePlaylist: resourceID = IDS_MAIN_TASKS_TASKTYPE_PLAYLIST; break;
   case TaskInfo::taskUnknown:
   default:
      ATLASSERT(false);
      break;
   }

   return resourceID == -1 ? CString() : CString(MAKEINTRESOURCE(resourceID));
}

CString TaskDetailsView::LabelFilenameOrTrackFromInfo(const TaskInfo& info)
{
   int resourceID = -1;
   switch (info.Type())
   {
   case TaskInfo::taskEncoding: resourceID = IDS_MAIN_TASKS_FILENAME_OR_TRACK_ENCODE; break;
   case TaskInfo::taskCdExtraction: resourceID = IDS_MAIN_TASKS_FILENAME_OR_TRACK_CDREAD; break;
   case TaskInfo::taskWritePlaylist: resourceID = IDS_MAIN_TASKS_FILENAME_OR_TRACK_PLAYLIST; break;
   case TaskInfo::taskUnknown:
   default:
      ATLASSERT(false);
      break;
   }

   return resourceID == -1 ? CString() : CString(MAKEINTRESOURCE(resourceID));
}
