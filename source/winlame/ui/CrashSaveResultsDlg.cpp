//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2020 Michael Fink
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
#include "stdafx.h"
#include "resource.h"
#include "CrashSaveResultsDlg.hpp"
#include "App.hpp"

using UI::CrashSaveResultsDlg;

CrashSaveResultsDlg::CrashSaveResultsDlg(const std::vector<CString>& resultFilenamesList)
   :m_resultFilenamesList(resultFilenamesList)
{
}

LRESULT CrashSaveResultsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   // center the dialog on the screen
   CenterWindow();

   // set icons
   SetIcon(App::AppIcon(false), TRUE);
   SetIcon(App::AppIcon(true), FALSE);

   // set up results list
   m_listResults.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
   m_listResults.InsertColumn(0, _T("Title"), LVCFMT_LEFT, 200);
   m_listResults.InsertColumn(1, _T("Filename"), LVCFMT_LEFT, 100);
   m_listResults.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

   // add results
   for (size_t index = 0, maxIndex = m_resultFilenamesList.size(); index < maxIndex; index++)
   {
      const CString& entry = m_resultFilenamesList[index];

      int itemIndex = m_listResults.InsertItem(0, Path(entry).FilenameAndExt());
      m_listResults.SetItemText(itemIndex, 1, entry);
      m_listResults.SetItemData(itemIndex, index);
   }

   DlgResize_Init(true);

   return TRUE;
}

LRESULT CrashSaveResultsDlg::OnExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (wID == IDOK)
   {
      if (!SaveResultFiles())
         return 0; // don't close dialog
   }

   EndDialog(wID);
   return 0;
}

LRESULT CrashSaveResultsDlg::OnDoubleClickedResultsList(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
   // double-clicked on an item
   LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pnmh;

   if (lpnmlv->iItem != -1)
   {
      size_t resultIndex = m_listResults.GetItemData(lpnmlv->iItem);

      const CString& entry = m_resultFilenamesList[resultIndex];

      OpenFolder(entry);
   }

   return 0;
}

/// \see http://stackoverflow.com/questions/3010305/programmatically-selecting-file-in-explorer
void CrashSaveResultsDlg::OpenFolder(const CString& filename)
{
   ITEMIDLIST* pidl = ILCreateFromPath(filename);
   if (pidl != nullptr)
   {
      SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
      ILFree(pidl);

      return;
   }

   // workaround for systems where ILCreateFromPath() fails; opens a new explorer
   // window each time it is called: remove ending slash
   CString folderName = filename.Left(filename.ReverseFind(_T('\\')) + 1);

   CString args;
   args.Format(_T("/select, \"%s\""), filename.GetString());
   ::ShellExecute(m_hWnd, _T("open"), _T("explorer.exe"), args, folderName, SW_SHOWNORMAL);
}

bool CrashSaveResultsDlg::SaveResultFiles()
{
   // ask for folder to save
   CString folderName;
   {
      CFolderDialog dlg(m_hWnd, _T("Select folder to store files..."),
         BIF_RETURNONLYFSDIRS | BIF_USENEWUI);
      if (IDOK != dlg.DoModal())
         return false; // return without closing dialog

      folderName = dlg.GetFolderPath();
      Path::AddEndingBackslash(folderName);
   }

   // copy all files to folder
   {
      for (size_t index = 0, maxIndex = m_resultFilenamesList.size(); index < maxIndex; index++)
      {
         const CString& entry = m_resultFilenamesList[index];

         // get filename part
         CString filenameAndExt = Path(entry).FilenameAndExt();

         ::MoveFile(entry, folderName + filenameAndExt);
      }
   }

   return true;
}
