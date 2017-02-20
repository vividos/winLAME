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
/// \file CrashSaveResultsDlg.cpp
/// \brief Dialog to save crash result files
//
#pragma once

#include <vector>

/// dialog to save crash results
class CrashSaveResultsDlg :
   public CDialogImpl<CrashSaveResultsDlg>,
   public CWinDataExchange<CrashSaveResultsDlg>,
   public CDialogResize<CrashSaveResultsDlg>
{
public:
   /// ctor
   CrashSaveResultsDlg(const std::vector<CString>& resultFilenamesList);

   /// dialog ID
   enum { IDD = IDD_CRASHDUMP_SAVE_RESULTS };

private:
   BEGIN_DDX_MAP(CrashSaveResultsDlg)
      DDX_CONTROL_HANDLE(IDC_CRASH_LIST_RESULTS, m_listResults)
   END_DDX_MAP()

   friend CDialogResize<CrashSaveResultsDlg>;

   BEGIN_DLGRESIZE_MAP(CrashSaveResultsDlg)
      DLGRESIZE_CONTROL(IDC_CRASH_STATIC_TEXT, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_X)
      DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_X)
      DLGRESIZE_CONTROL(IDC_CRASH_LIST_RESULTS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(CrashSaveResultsDlg)
      CHAIN_MSG_MAP(CDialogResize<CrashSaveResultsDlg>)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnExit)
      COMMAND_ID_HANDLER(IDCANCEL, OnExit)
      NOTIFY_HANDLER(IDC_CRASH_LIST_RESULTS, NM_DBLCLK, OnDoubleClickedResultsList)
   END_MSG_MAP()

   /// called when dialog is created
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when the dialog is closed
   LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when results list item was double-clicked
   LRESULT OnDoubleClickedResultsList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
   /// opens folder of given filename and selects file
   void OpenFolder(const CString& filename);

   /// saves result files in selected folder
   bool SaveResultFiles();

private:
   /// list of crash result filenames
   const std::vector<CString>& m_resultFilenamesList;

   /// list view to display results
   CListViewCtrl m_listResults;
};
