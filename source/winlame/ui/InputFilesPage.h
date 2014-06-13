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
/// \file InputFilesPage.h
/// \brief Input files page

// include guard
#pragma once

// includes
#include "WizardPage.h"
#include "InputListCtrl.h"
#include "resource.h"

// forward references
struct UISettings;

/// \brief input files page
/// \details shows all files opened/dropped, checks them for audio infos and errors
/// and displays them; processing is done in separate thread.
class InputFilesPage:
   public WizardPage,
   public CWinDataExchange<InputFilesPage>,
   public CDialogResize<InputFilesPage>
{
public:
   /// ctor
   InputFilesPage(WizardPageHost& pageHost,
      const std::vector<CString>& vecInputFiles) throw();
   /// dtor
   ~InputFilesPage() throw()
   {
   }

   /// opens file dialog and returns all files selected
   static bool OpenFileDialog(HWND hWndParent, std::vector<CString>& vecFilenames);

private:
   friend CDialogResize<InputFilesPage>;

   BEGIN_DDX_MAP(InputFilesPage)
      DDX_CONTROL(IDC_INPUT_LIST_INPUTFILES, m_inputFilesList)
   END_DDX_MAP()

   // resize map
   BEGIN_DLGRESIZE_MAP(InputFilesPage)
      DLGRESIZE_CONTROL(IDC_INPUT_LIST_INPUTFILES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
      DLGRESIZE_CONTROL(IDC_STATIC_TIMECOUNT, DLSZ_MOVE_X)
   END_DLGRESIZE_MAP()

   // message map
   BEGIN_MSG_MAP(InputFilesPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
      MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
      NOTIFY_HANDLER(IDC_INPUT_LIST_INPUTFILES, LVN_ITEMCHANGED, OnListItemChanged)
      NOTIFY_HANDLER(IDC_INPUT_LIST_INPUTFILES, NM_DBLCLK, OnDoubleClickedList)
      COMMAND_HANDLER(IDC_INPUT_BUTTON_PLAY, BN_CLICKED, OnButtonPlay)
      COMMAND_HANDLER(IDC_INPUT_BUTTON_INFILESEL, BN_CLICKED, OnButtonInputFileSel)
      COMMAND_HANDLER(IDC_INPUT_BUTTON_DELETE, BN_CLICKED, OnButtonDeleteAll)
      CHAIN_MSG_MAP(CDialogResize<InputFilesPage>)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

   // Handler prototypes:
   // LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   // LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   // LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is left
   LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

   /// called when user dropped files on the list ctrl
   LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when the button for adding input files is pressed
   LRESULT OnButtonInputFileSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   /// called when the user clicks on the button to delete all selected files
   LRESULT OnButtonDeleteAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   /// called when button "CD Rip" was pressed
   LRESULT OnButtonCDRip(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   /// called for processing key presses
   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   /// called when the selected item in the list ctrl changes
   LRESULT OnListItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
   /// called when user double-clicks on an item in list
   LRESULT OnDoubleClickedList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
   /// called when user presses the play button
   LRESULT OnButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
   /// sets up tracks list control
   void SetupListCtrl();

   /// insert new file names into list
   void InsertFilenames(const std::vector<CString>& vecInputFiles);

   /// inserts single filename with icon into list
   void InsertFilenameWithIcon(const CString& cszFilename);

   /// plays file using assigned application
   void PlayFile(LPCTSTR filename);

   /// parse buffer from multi selection from open file dialog
   static void ParseMultiSelectionFiles(LPCTSTR pszBuffer, std::vector<CString>& vecFilenames);

   /// returns filter string
   static CString GetFilterString();

private:
   // controls

   /// list of filenames
   InputListCtrl m_inputFilesList;

   // model

   /// settings
   UISettings& m_uiSettings;

   /// list of passed filenames; cleared after inserting into list
   std::vector<CString> m_vecInputFiles;

   /// indicates if system image list was already set on list control
   bool m_bSetSysImageList;

   /// filter string
   static CString m_cszFilterString;
};
