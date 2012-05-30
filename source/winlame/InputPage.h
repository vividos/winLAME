/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: InputPage.h,v 1.29 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file InputPage.h

   \brief contains the input page and a list ctrl to show the files to encode

*/
/*! \ingroup userinterface */
/*! @{ */

// prevent multiple including
#ifndef __wlinputpage_h_
#define __wlinputpage_h_

// needed includes
#include "resource.h"
#include "PageBase.h"
#include "InputListCtrl.h"


//! input modules config selection dialog

class InputConfigDlg: public CDialogImpl<InputConfigDlg>
{
public:
   //! ctor
   InputConfigDlg(){}

   //! sets module indices to use
   void Init(ModuleManager* mgr, std::vector<int> indices);

   //! dialog id
   enum { IDD = IDD_INMODULE_CFG };

   // message map
BEGIN_MSG_MAP(InputPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   COMMAND_HANDLER(IDOK, BN_CLICKED, OnExit)
   COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnExit)
   COMMAND_HANDLER(IDC_INCFG_LISTBOX, LBN_DBLCLK, OnListBoxDoubleClick)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()

   //! called to init the dialog
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   //! called when double-clicking on a listbox item
   LRESULT OnListBoxDoubleClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   //! called on exiting the dialog
   LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      EndDialog(0);
      return 0;
   }

protected:
   //! module manager to use
   ModuleManager* mgr;

   //! module indices
   std::vector<int> indices;
};


//! input files page

class InputPage:
   public PageBase,
   public CDialogResize<InputPage>
{
public:
   //! ctor
   InputPage()
   {
      IDD = IDD_DLG_INPUT;
      captionID = IDS_DLG_CAP_INPUT;
      descID = IDS_DLG_DESC_INPUT;
      helpID = IDS_HTML_INPUT;

      recursive = false;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(InputPage)
   DLGRESIZE_CONTROL(IDC_INPUT_LIST_INPUTFILES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
   DLGRESIZE_CONTROL(IDC_STATIC_TIMECOUNT, DLSZ_MOVE_X)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(InputPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
   MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
   MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   NOTIFY_HANDLER(IDC_INPUT_LIST_INPUTFILES, LVN_ITEMCHANGED, OnListItemChanged)
   NOTIFY_HANDLER(IDC_INPUT_LIST_INPUTFILES, NM_DBLCLK, OnDoubleClickedList)
   COMMAND_HANDLER(IDC_INPUT_BUTTON_PLAY, BN_CLICKED, OnButtonPlay)
   COMMAND_HANDLER(IDC_INPUT_BUTTON_INFILESEL, BN_CLICKED, OnButtonInputFileSel)
   COMMAND_HANDLER(IDC_INPUT_BUTTON_DELETE, BN_CLICKED, OnButtonDeleteAll)
   COMMAND_HANDLER(IDC_INPUT_BUTTON_CDRIP, BN_CLICKED, OnButtonCDRip)
   COMMAND_HANDLER(IDC_INPUT_BUTTON_CONFIG, BN_CLICKED, OnButtonConfig)
   CHAIN_MSG_MAP(CDialogResize<InputPage>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   //! inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   //! called when page is about to be destroyed
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      ilIcons.Destroy();
      return 0;
   }

   //! called when the button for adding input files is pressed
   LRESULT OnButtonInputFileSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // open file dialog
      OpenFileDialog();
      return 0;
   }

   //! called when the user clicks on the button to delete all selected files
   LRESULT OnButtonDeleteAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   //! called when button "config" was pressed
   LRESULT OnButtonConfig(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   //! called when button "CD Rip" was pressed
   LRESULT OnButtonCDRip(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   //! called when user dropped files on the list ctrl
   LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   //! called for processing key presses
   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   //! called when the selected item in the list ctrl changes
   LRESULT OnListItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
   //! called when user double-clicks on an item in list
   LRESULT OnDoubleClickedList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
   //! called when user presses the play button
   LRESULT OnButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   // virtual functions from PageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   //! plays the file
   void PlayFile(LPCTSTR filename);

   //! inserts a filename with icon in the list ctrl
   void InsertFilename(LPCTSTR fname);

   //! opens up the file dialog
   void OpenFileDialog();

   //! imports .m3u playlist
   void ImportM3uPlaylist(LPCTSTR filename);

   //! imports .pls playlist
   void ImportPlsPlaylist(LPCTSTR filename);

   //! imports cue sheets
   void ImportCueSheet(LPCTSTR filename);

   //! inserts filename with icon
   void InsertFileWithIcon(LPCTSTR pszRealFilename, LPCTSTR pszFilenameForIcon,
      int nSamplefreq, int nBps, int nLength);

   /// deletes selected tracks from cd rip track manager
   void DeleteSelectedFromCDRipTrackManager();

   /// updates time count of audio files
   void UpdateTimeCount();

protected:
   //! icon list for image buttons
   CImageList ilIcons;

   //! indicates if the system image list is already set
   bool setsysimagelist;

   //! list ctrl
   InputListCtrl listctrl;

   //! string with all file filters
   CString filterstring;

   //! indicates if InsertFilename() currently recurses through directory trees
   bool recursive;

   //! id's of all modules that allow configuration
   std::vector<int> config_modules_id;

   //! string with input file errors
   CString input_errors;
};


//@}

#endif
