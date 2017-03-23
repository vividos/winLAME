//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2014 Michael Fink
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
#include "StdAfx.h"
#include "InputFilesPage.hpp"
#include "UISettings.hpp"
#include "InputFilesParser.hpp"
#include "DropFilesManager.hpp"
#include "CommonStuff.hpp"
#include "WizardPageHost.hpp"
#include "OutputSettingsPage.hpp"
#include "RedrawLock.hpp"

using namespace UI;

CString InputFilesPage::m_cszFilterString;

InputFilesPage::InputFilesPage(WizardPageHost& pageHost,
   const std::vector<CString>& vecInputFiles)
:WizardPage(pageHost, IDD_PAGE_INPUT_FILES, WizardPage::typeCancelNext),
m_pageWidth(0),
m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
m_bSetSysImageList(false),
m_vecInputFiles(vecInputFiles)
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

   AddFiles(m_vecInputFiles);
   m_vecInputFiles.clear();

   GetDlgItem(IDC_INPUT_BUTTON_PLAY).EnableWindow(false);
   GetDlgItem(IDC_INPUT_BUTTON_DELETE).EnableWindow(false);

   UpdateTimeCount();

   return 1;
}

LRESULT InputFilesPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
   m_audioFileInfoManager.Stop();

   int max = m_inputFilesList.GetItemCount();

   // when no input files are chosen, refuse to leave the page
   if (max == 0)
   {
      AppMessageBox(m_hWnd, IDS_INPUT_NOINFILES, MB_OK | MB_ICONEXCLAMATION);
      return 1; // prevent leaving dialog
   }

   // add encoder job for every file in list
   for (int i = 0; i < max; i++)
   {
      CString filename = m_inputFilesList.GetFileName(i);
      m_uiSettings.encoderjoblist.push_back(Encoder::EncoderJob(filename));
   }

   m_uiSettings.m_bFromInputFilesPage = true;
   m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

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
      int pos = m_inputFilesList.GetNextItem(-1, LVIS_SELECTED);

      // delete all files
      m_inputFilesList.DeleteSelectedListItems();

      // set selection on next item
      m_inputFilesList.SetItemState(pos,
         LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

      UpdateTimeCount();

      m_inputFilesList.Invalidate();
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

   m_inputFilesList.UpdateAudioFileInfo(*entry);

   delete entry;

   UpdateTimeCount();

   return 0;
}

LRESULT InputFilesPage::OnListItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   // called when the selected item in the list changes

   // look if we have to disable the play and delete button
   BOOL enable = (0 != m_inputFilesList.GetSelectedCount());

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
      CString filename = m_inputFilesList.GetFileName(lpnmlv->iItem);

      // play file
      PlayFile(filename);
   }

   return 0;
}

LRESULT InputFilesPage::OnButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // find out which file had the focus
   int index = m_inputFilesList.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

   CString filename = m_inputFilesList.GetFileName(index);
   PlayFile(filename);

   return 0;
}

LRESULT InputFilesPage::OnButtonInputFileSel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   // open files
   if (!InputFilesPage::OpenFileDialog(m_hWnd, m_vecInputFiles))
      return 0;

   AddFiles(m_vecInputFiles);
   m_vecInputFiles.clear();

   return 0;
}

LRESULT InputFilesPage::OnButtonDeleteAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // delete all list items
   m_inputFilesList.DeleteSelectedListItems();

   UpdateTimeCount();

   m_inputFilesList.Invalidate();

   return 0;
}

void InputFilesPage::SetupListCtrl()
{
   // find out width of the list ctrl
   CRect rc;
   m_inputFilesList.GetWindowRect(&rc);
   int width = rc.right - rc.left - 4;

   // load strings
   CString asColumnNames[4];
   asColumnNames[0].LoadString(IDS_INPUT_LIST_COLNAME1);
   asColumnNames[1].LoadString(IDS_INPUT_LIST_COLNAME2);
   asColumnNames[2].LoadString(IDS_INPUT_LIST_COLNAME3);
   asColumnNames[3].LoadString(IDS_INPUT_LIST_COLNAME4);

   // insert list ctrl columns
   double aSizes[4] = { 0.57, 0.17, 0.15, 0.11 };

   LVCOLUMN lvColumn = { LVCF_TEXT | LVCF_WIDTH, 0, 0, NULL, 0, 0 };

   for (int i = 0; i<4; i++)
   {
      lvColumn.cx = int(width * aSizes[i]);
      lvColumn.pszText = const_cast<LPTSTR>(asColumnNames[i].GetString());
      m_inputFilesList.InsertColumn(i, &lvColumn);
   }

   // set extended list ctrl styles
   m_inputFilesList.SetExtendedListViewStyle(
      LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);
}

void InputFilesPage::ResizeListCtrlColumns(int dx)
{
   int column0Width = m_inputFilesList.GetColumnWidth(0);
   column0Width += dx;

   m_inputFilesList.SetColumnWidth(0, column0Width);
}

void InputFilesPage::AddFiles(const std::vector<CString>& vecInputFiles)
{
   InputFilesParser parser;
   parser.Parse(vecInputFiles);

   InsertFilenames(parser.FileList());

   if (!parser.PlaylistName().IsEmpty())
   {
      CString cszName = Path(parser.PlaylistName()).FilenameOnly();
      m_uiSettings.playlist_filename = cszName + _T(".m3u");
   }
}

void InputFilesPage::InsertFilenames(const std::vector<CString>& vecInputFiles)
{
   RedrawLock lock(m_inputFilesList);

   for (size_t i = 0, iMax = vecInputFiles.size(); i < iMax; i++)
      InsertFilenameWithIcon(vecInputFiles[i]);
}

void InputFilesPage::InsertFilenameWithIcon(const CString& cszFilename)
{
   // find out icon image
   SHFILEINFO sfi = {0};
   HIMAGELIST hImageList = (HIMAGELIST)SHGetFileInfo(
      cszFilename, 0, &sfi, sizeof(SHFILEINFO),
      SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

   if (!m_bSetSysImageList)
   {
      // set system wide image list
      m_inputFilesList.SetImageList(hImageList, LVSIL_SMALL);
      m_bSetSysImageList = true;
   }

   m_inputFilesList.InsertFile(cszFilename, sfi.iIcon, -1, -1, -1);

   m_audioFileInfoManager.AsyncGetAudioFileInfo(cszFilename,
      std::bind(&InputFilesPage::OnRetrievedAudioFileInfo, this,
         cszFilename,
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
   {
      m_audioFileInfoManager.Stop();

      // TODO output message
      errorMessage;
   }

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
   unsigned int nTime = m_inputFilesList.GetTotalLength();

   CString cszText;
   cszText.Format(IDS_INPUT_TIME_UU, nTime / 60, nTime % 60);

   SetDlgItemText(IDC_STATIC_TIMECOUNT, cszText);
}

CString InputFilesPage::GetFilterString()
{
   if (!m_cszFilterString.IsEmpty())
      return m_cszFilterString;

   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
   moduleManager.GetFilterString(m_cszFilterString);

   CString cszText;
   cszText.LoadString(IDS_INPUT_FILTER_PLAYLISTS);
   m_cszFilterString += cszText;

   cszText.LoadString(IDS_INPUT_FILTER_CUESHEETS);
   m_cszFilterString += cszText;
   m_cszFilterString.Insert(m_cszFilterString.Find('|') + 1, _T("*.m3u;*.pls;*.cue;"));

   cszText.LoadString(IDS_INPUT_FILTER_ALLFILES);
   m_cszFilterString += cszText + _T("|"); // add extra pipe char for end of filter

   return m_cszFilterString;
}

bool InputFilesPage::OpenFileDialog(HWND hWndParent, std::vector<CString>& vecFilenames)
{
   // get filter string
   CString filterString = GetFilterString();

   // exchange pipe char '|' with 0-char for commdlg
   for (int pos = filterString.GetLength() - 1; pos >= 0; pos--)
      if (filterString.GetAt(pos) == _T('|'))
         filterString.SetAt(pos, 0);

   // load title
   CString cszTitle;
   cszTitle.LoadString(IDS_INPUT_INFILES_SELECT);

   // file dialog setup
   CFileDialog dlg(TRUE, NULL, NULL,
      OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER,
      filterString,
      hWndParent);

   // fill file buffer
   TCHAR szBuffer[MAX_PATH * 1024] = { 0 };
   const UINT uiBufferLenCch = sizeof(szBuffer) / sizeof(*szBuffer);

   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
   CString& lastinputpath = settings.lastinputpath;

   // copy last input path to buffer, as init
   _tcsncpy_s(
      szBuffer, uiBufferLenCch,
      lastinputpath, lastinputpath.GetLength());

   dlg.m_ofn.lpstrFile = szBuffer;
   dlg.m_ofn.nMaxFile = uiBufferLenCch;

   // do file dialog
   if (IDOK != dlg.DoModal())
      return false;

   if (dlg.m_ofn.nFileExtension == 0)
   {
      ParseMultiSelectionFiles(szBuffer, vecFilenames);
   }
   else
   {
      // single file selection
      vecFilenames.push_back(szBuffer);

      // get the used directory
      lastinputpath = szBuffer;
   }

   return true;
}

void InputFilesPage::ParseMultiSelectionFiles(LPCTSTR pszBuffer, std::vector<CString>& vecFilenames)
{
   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
   CString& lastinputpath = settings.lastinputpath;

   // multiple file selection
   LPCTSTR pszStart = pszBuffer;

   lastinputpath = pszStart;
   Path::AddEndingBackslash(lastinputpath);

   // go to the first file
   while (*pszStart++ != 0);

   // while not at end of the list
   while (*pszStart != 0)
   {
      // construct pathname
      CString cszFilename = lastinputpath + pszStart;

      vecFilenames.push_back(cszFilename);

      // go to the next entry
      while (*pszStart++ != 0);
   }

   // set last selected file
   if (!vecFilenames.empty())
      lastinputpath = vecFilenames.back();
}
