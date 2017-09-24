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
/// \file TaskDetailsView.hpp
/// \brief Tasks detail view window
//
#pragma once

class TaskInfo;

namespace UI
{
   /// dialog based view to show task details
   class TaskDetailsView :
      public CDialogImpl<TaskDetailsView>,
      public CDialogResize<TaskDetailsView>,
      public CWinDataExchange<TaskDetailsView>
   {
   public:
      /// ctor
      TaskDetailsView()
      {
      }

      /// dialog ID
      enum { IDD = IDD_VIEW_TASKDETAILS };

      /// resets view so that no task details are shown
      void ResetTaskDetails();

      /// updates task details shown
      void UpdateTaskDetails(const TaskInfo& taskInfo);

   private:
      friend CDialogResize<TaskDetailsView>;

      BEGIN_DDX_MAP(TaskDetailsView)
         DDX_CONTROL_HANDLE(IDC_STATIC_ICON_TASK_TYPE, m_staticIconTaskType)
         DDX_CONTROL_HANDLE(IDC_STATIC_TEXT_TASK_TYPE, m_staticTextTaskType)
         DDX_CONTROL_HANDLE(IDC_STATIC_LABEL_FILENAME_TRACK, m_staticLabelFilenameTrack)
         DDX_CONTROL_HANDLE(IDC_EDIT_TEXT_FILENAME_TRACK, m_editTextFilenameTrack)
         DDX_CONTROL_HANDLE(IDC_EDIT_TEXT_TASK_DESCRIPTION, m_editTextTaskDescription)
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(TaskDetailsView)
         DLGRESIZE_CONTROL(IDC_STATIC_TEXT_TASK_TYPE, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_EDIT_TEXT_FILENAME_TRACK, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_EDIT_TEXT_TASK_DESCRIPTION, DLSZ_SIZE_X | DLSZ_SIZE_Y)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(TaskDetailsView)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         CHAIN_MSG_MAP(CDialogResize<TaskDetailsView>)
         REFLECT_NOTIFICATIONS() // to make sure superclassed controls get notification messages
      END_MSG_MAP()

   private:
      /// called when dialog view is being shown
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// returns a displayable task type name from given task info
      static CString TaskTypeFromInfo(const TaskInfo& info);

      /// returns a displayable label for the filename/track field
      static CString LabelFilenameOrTrackFromInfo(const TaskInfo& info);

   private:
      // UI

      /// font for caption (the task type)
      CFont m_captionFont;

      /// task list images
      CImageList m_taskImages;

      /// icon for task type
      CStatic m_staticIconTaskType;

      /// static control showing task type (Encoding, CD Read, etc.)
      CStatic m_staticTextTaskType;

      /// label showing filename or track
      CStatic m_staticLabelFilenameTrack;

      /// filename or track name being processed
      CEdit m_editTextFilenameTrack;

      /// task description, including possible errors
      CEdit m_editTextTaskDescription;
   };

} // namespace UI
