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
/// \file InputFilesPage.cpp
/// \brief Input files page
//
#include "stdafx.h"
#include "InputFilesPage.hpp"
#include "UISettings.hpp"
#include "InputFilesParser.hpp"
#include "DropFilesManager.hpp"
#include "WizardPageHost.hpp"
#include "OutputSettingsPage.hpp"
#include "ClassicModeStartPage.hpp"
#include "RedrawLock.hpp"

using namespace UI;

CString InputFilesPage::m_filterString;

InputFilesPage::InputFilesPage(WizardPageHost& pageHost,
   const std::vector<CString>& inputFilesList)
   :WizardPage(pageHost, IDD_PAGE_INPUT_FILES,
      pageHost.IsClassicMode() ? WizardPage::typeCancelBackNext : WizardPage::typeCancelNext),
   m_pageWidth(0),
   m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
   m_setSysImageList(false),
   m_inputFilesList(inputFilesList)
{
}

LRESULT InputFilesPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   // enable resizing
   CRect rectPage;
   GetClientRect(&rectPage);
   m_pageWidth = rectPage.Width();

   DlgResize_Init(false, false);

   // enable drag & drop
   DragAcceptFiles(TRUE);

   SetupListCtrl();

   AddFiles(m_inputFilesList);
   m_inputFilesList.clear();

   GetDlgItem(IDC_INPUT_BUTTON_PLAY).EnableWindow(false);
   GetDlgItem(IDC_INPUT_BUTTON_DELETE).EnableWindow(false);

   UpdateTimeCount();

   return 1;
}

LRESULT InputFilesPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
   m_audioFileInfoManager.Stop();

   int max = m_listViewInputFiles.GetItemCount();

   // when no input files are chosen, refuse to leave the page
   if (max == 0)
   {
      AtlMessageBox(m_hWnd, IDS_INPUT_NOINFILES, IDS_APP_CAPTION, MB_OK | MB_ICONEXCLAMATION);
      return 1; // prevent leaving dialog
   }

   // add encoder job for every file in list
   for (int i = 0; i < max; i++)
   {
      CString filename = m_listViewInputFiles.GetFileName(i);
      m_uiSettings.encoderjoblist.push_back(Encoder::EncoderJob(filename));
   }

   m_uiSettings.m_bFromInputFilesPage = true;
   m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

LRESULT InputFilesPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (m_pageHost.IsClassicMode())
      m_pageHost.SetWizardPage(std::make_shared<ClassicModeStartPage>(m_pageHost));

   return 0;
}

LRESULT InputFilesPage::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DropFilesManager dropMgr((HDROP)wParam);

   AddFiles(dropMgr.Filenames());

   // redraw after drop
   GetParent().Invalidate();

   return 0;
}

LRESULT InputFilesPage::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // delete key from list ctrl?
   if (VK_DELETE == (int)wParam)
   {
      int pos = m_listViewInputFiles.GetNextItem(-1, LVIS_SELECTED);

      // delete all files
      m_listViewInputFiles.DeleteSelectedListItems();

      // set selection on next item
      m_listViewInputFiles.SetItemState(pos,
         LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

      UpdateTimeCount();

      m_listViewInputFiles.Invalidate();
   }

   // insert key? fake button press
   if (VK_INSERT == (int)wParam)
      OnButtonInputFileSel(0, 0, 0, bHandled);

   // return key? play file
   if (VK_RETURN == (int)wParam)
      OnButtonPlay(0, 0, 0, bHandled);

   return 0;
}

LRESULT InputFilesPage::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   bHandled = false;

   // resize list view columns
   if (wParam != SIZE_MINIMIZED)
   {
      int dx = GET_X_LPARAM(lParam) - m_pageWidth;

      ResizeListCtrlColumns(dx);

      m_pageWidth += dx;
   }

   return 0;
}

LRESULT InputFilesPage::OnUpdateAudioInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   AudioFileEntry* entry = reinterpret_cast<AudioFileEntry*>(lParam);

   m_listViewInputFiles.UpdateAudioFileInfo(*entry);

   delete entry;

   UpdateTimeCount();

   return 0;
}

LRESULT InputFilesPage::OnListItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   // called when the selected item in the list changes

   // look if we have to disable the play and delete button
   BOOL enable = (0 != m_listViewInputFiles.GetSelectedCount());

   GetDlgItem(IDC_INPUT_BUTTON_PLAY).EnableWindow(enable);
   GetDlgItem(IDC_INPUT_BUTTON_DELETE).EnableWindow(enable);

   return 0;
}

LRESULT InputFilesPage::OnDoubleClickedList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   // double-clicked on an item
   LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pnmh;

   if (lpnmlv->iItem != -1)
   {
      CString filename = m_listViewInputFiles.GetFileName(lpnmlv->iItem);

      // play file
      PlayFile(filename);
   }

   return 0;
}

LRESULT InputFilesPage::OnButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // find out which file had the focus
   int index = m_listViewInputFiles.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

   CString filename = m_listViewInputFiles.GetFileName(index);
   PlayFile(filename);

   return 0;
}

LRESULT InputFilesPage::OnButtonInputFileSel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   // open files
   if (!InputFilesPage::OpenFileDialog(m_hWnd, m_inputFilesList))
      return 0;

   AddFiles(m_inputFilesList);
   m_inputFilesList.clear();

   return 0;
}

LRESULT InputFilesPage::OnButtonDeleteAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // delete all list items
   m_listViewInputFiles.DeleteSelectedListItems();

   UpdateTimeCount();

   m_listViewInputFiles.Invalidate();

   return 0;
}

void InputFilesPage::SetupListCtrl()
{
   // find out width of the list ctrl
   CRect rc;
   m_listViewInputFiles.GetWindowRect(&rc);
   int width = rc.right - rc.left - 4;

   // load strings
   CString columnNames[4];
   columnNames[0].LoadString(IDS_INPUT_LIST_COLNAME1);
   columnNames[1].LoadString(IDS_INPUT_LIST_COLNAME2);
   columnNames[2].LoadString(IDS_INPUT_LIST_COLNAME3);
   columnNames[3].LoadString(IDS_INPUT_LIST_COLNAME4);

   // insert list ctrl columns
   double columnSizes[4] = { 0.57, 0.17, 0.15, 0.11 };

   for (int i = 0; i < 4; i++)
   {
      m_listViewInputFiles.InsertColumn(
         i,
         columnNames[i],
         LVCFMT_LEFT,
         int(width * columnSizes[i]));
   }

   // set extended list ctrl styles
   m_listViewInputFiles.SetExtendedListViewStyle(
      LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);
}

void InputFilesPage::ResizeListCtrlColumns(int dx)
{
   int column0Width = m_listViewInputFiles.GetColumnWidth(0);
   column0Width += dx;

   m_listViewInputFiles.SetColumnWidth(0, column0Width);
}

void InputFilesPage::AddFiles(const std::vector<CString>& inputFilesList)
{
   InputFilesParser parser;
   parser.Parse(inputFilesList);

   InsertFilenames(parser.FileList());

   if (!parser.PlaylistName().IsEmpty())
   {
      CString name = Path::FilenameOnly(parser.PlaylistName());
      m_uiSettings.playlist_filename = name + _T(".m3u");
   }
}

void InputFilesPage::InsertFilenames(const std::vector<CString>& inputFilesList)
{
   RedrawLock lock(m_listViewInputFiles);

   for (size_t i = 0, iMax = inputFilesList.size(); i < iMax; i++)
      InsertFilenameWithIcon(inputFilesList[i]);
}

void InputFilesPage::InsertFilenameWithIcon(const CString& filename)
{
   // find out icon image
   SHFILEINFO sfi = { 0 };
   HIMAGELIST imageList = (HIMAGELIST)SHGetFileInfo(
      filename, 0, &sfi, sizeof(SHFILEINFO),
      SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

   if (!m_setSysImageList)
   {
      // set system wide image list
      m_listViewInputFiles.SetImageList(imageList, LVSIL_SMALL);
      m_setSysImageList = true;
   }

   m_listViewInputFiles.InsertFile(filename, sfi.iIcon, -1, -1, -1);

   m_audioFileInfoManager.AsyncGetAudioFileInfo(filename,
      std::bind(&InputFilesPage::OnRetrievedAudioFileInfo, this,
         filename,
         std::placeholders::_1,
         std::placeholders::_2,
         std::placeholders::_3,
         std::placeholders::_4,
         std::placeholders::_5));
}

void InputFilesPage::OnRetrievedAudioFileInfo(const CString& filename, bool error, const CString& errorMessage,
   int lengthInSeconds, int bitrateInBps, int sampleFrequencyInHz)
{
   if (error)
      return;

   AudioFileEntry* entry = new AudioFileEntry;
   entry->filename = filename;
   entry->length = lengthInSeconds;
   entry->bitrate = bitrateInBps;
   entry->samplerate = sampleFrequencyInHz;

   PostMessage(WM_UPDATE_AUDIO_INFO, 0, reinterpret_cast<LPARAM>(entry));
}

void InputFilesPage::PlayFile(LPCTSTR filename)
{
   ::ShellExecute(NULL, _T("open"), filename, NULL, NULL, SW_SHOW);
}

void InputFilesPage::UpdateTimeCount()
{
   unsigned int lengthInSeconds = m_listViewInputFiles.GetTotalLength();

   CString text;
   text.Format(IDS_INPUT_TIME_UU, lengthInSeconds / 60, lengthInSeconds % 60);

   SetDlgItemText(IDC_STATIC_TIMECOUNT, text);
}

CString InputFilesPage::GetFilterString()
{
   if (!m_filterString.IsEmpty())
      return m_filterString;

   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
   moduleManager.GetFilterString(m_filterString);

   CString text;
   text.LoadString(IDS_INPUT_FILTER_PLAYLISTS);
   m_filterString += text;

   text.LoadString(IDS_INPUT_FILTER_CUESHEETS);
   m_filterString += text;
   m_filterString.Insert(m_filterString.Find('|') + 1, _T("*.m3u;*.pls;*.cue;"));

   text.LoadString(IDS_INPUT_FILTER_ALLFILES);
   m_filterString += text + _T("|"); // add extra pipe char for end of filter

   return m_filterString;
}

bool InputFilesPage::OpenFileDialog(HWND hwndParent, std::vector<CString>& filenamesList)
{
   // get filter string
   CString filterString = GetFilterString();

   // exchange pipe char '|' with 0-char for commdlg
   for (int pos = filterString.GetLength() - 1; pos >= 0; pos--)
      if (filterString.GetAt(pos) == _T('|'))
         filterString.SetAt(pos, 0);

   // load title
   CString title;
   title.LoadString(IDS_INPUT_INFILES_SELECT);

   // file dialog setup
   CFileDialog dlg(TRUE, NULL, NULL,
      OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER,
      filterString,
      hwndParent);

   // fill file buffer
   TCHAR szBuffer[MAX_PATH * 1024] = { 0 };
   const UINT uiBufferLenCch = sizeof(szBuffer) / sizeof(*szBuffer);

   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
   CString& lastInputPath = settings.lastinputpath;

   // copy last input path to buffer, as init
   _tcsncpy_s(
      szBuffer, uiBufferLenCch,
      lastInputPath, lastInputPath.GetLength());

   dlg.m_ofn.lpstrFile = szBuffer;
   dlg.m_ofn.nMaxFile = uiBufferLenCch;

   // do file dialog
   if (IDOK != dlg.DoModal())
      return false;

   if (dlg.m_ofn.nFileExtension == 0)
   {
      ParseMultiSelectionFiles(szBuffer, filenamesList);
   }
   else
   {
      // single file selection
      filenamesList.push_back(szBuffer);

      // get the used directory
      lastInputPath = szBuffer;
   }

   return true;
}

void InputFilesPage::ParseMultiSelectionFiles(LPCTSTR buffer, std::vector<CString>& filenamesList)
{
   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
   CString& lastInputPath = settings.lastinputpath;

   // multiple file selection
   LPCTSTR start = buffer;

   lastInputPath = start;
   Path::AddEndingBackslash(lastInputPath);

   // go to the first file
   while (*start++ != 0);

   // while not at end of the list
   while (*start != 0)
   {
      // construct pathname
      CString filename = lastInputPath + start;

      filenamesList.push_back(filename);

      // go to the next entry
      while (*start++ != 0);
   }

   // set last selected file
   if (!filenamesList.empty())
      lastInputPath = filenamesList.back();
}
