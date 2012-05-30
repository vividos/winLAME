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

   $Id: OutputPage.h,v 1.19 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file OutputPage.h

   \brief contains the output page

*/
/*! \ingroup userinterface */
/*! @{ */

// prevent multiple including
#ifndef __wloutputpage_h_
#define __wloutputpage_h_

// needed includes
#include "resource.h"
#include "PageBase.h"
#include "wlCommonStuff.h"


//! output settings page

class OutputPage:
   public PageBase,
   public CDialogResize<OutputPage>
{
public:
   //! ctor
   OutputPage()
   {
      IDD = IDD_DLG_OUTPUT;
      captionID = IDS_DLG_CAP_OUTPUT;
      descID = IDS_DLG_DESC_OUTPUT;
      helpID = IDS_HTML_OUTPUT;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(OutputPage)
   DLGRESIZE_CONTROL(IDC_OUT_BEVEL1, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_OUT_COMBO_OUTMODULE, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_OUT_CHECK_WARN, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_OUT_BEVEL2, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_OUT_OUTPATH, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_OUT_SELECTPATH, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_OUT_COMBO_FINISHED_ACTION, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_OUT_PLAYLISTNAME, DLSZ_SIZE_X)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(OutputPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
   COMMAND_HANDLER(IDC_OUT_SELECTPATH, BN_CLICKED, OnButtonSelectOutputPath)
   COMMAND_HANDLER(IDC_OUT_CHECK_FINISHED_ACTION, BN_CLICKED, OnCheckFinishedAction)
   COMMAND_HANDLER(IDC_OUT_CREATEPLAYLIST, BN_CLICKED, OnCheckCreatePlaylist)
   COMMAND_HANDLER(IDC_OUT_USE_INDIR, BN_CLICKED, OnCheckUseInputFolder)
   COMMAND_HANDLER(IDC_OUT_OUTPATH ,CBN_SELENDOK, OnOutPathSelEndOk)
   CHAIN_MSG_MAP(CDialogResize<OutputPage>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   //! inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   //! called when the page is about to be destroyed
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   //! called when the user clicks on the button to select the output path
   LRESULT OnButtonSelectOutputPath(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      wlUISettings &settings = pui->getUISettings();
      CString path = settings.outputdir;

      // lets user select a path
      if (wlBrowseForFolder(m_hWnd, path))
      {
         // move history entries
         settings.outputhistory.insert(settings.outputhistory.begin(),
            settings.outputdir);
         settings.outputdir = path;

         // update combobox
         RefreshHistory();
      }

      return 0;
   }

   //! called when the state of the "finished action" check changes
   LRESULT OnCheckFinishedAction(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      int check = SendDlgItemMessage(IDC_OUT_CHECK_FINISHED_ACTION, BM_GETCHECK);
      ::EnableWindow(GetDlgItem(IDC_OUT_COMBO_FINISHED_ACTION),
         check==BST_CHECKED ? TRUE: FALSE);

      return 0;
   }

   //! called when the state of the "create playlist" check changes
   LRESULT OnCheckCreatePlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      int check = SendDlgItemMessage(IDC_OUT_CREATEPLAYLIST, BM_GETCHECK);
      ::EnableWindow(GetDlgItem(IDC_OUT_PLAYLISTNAME),
         check==BST_CHECKED ? TRUE: FALSE);

      return 0;
   }

   //! called when check "use input dir as output location" is clicked
   LRESULT OnCheckUseInputFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      BOOL check = SendDlgItemMessage(IDC_OUT_USE_INDIR, BM_GETCHECK) == BST_CHECKED ? FALSE : TRUE;
      ::EnableWindow(GetDlgItem(IDC_OUT_OUTPATH),check);
      ::EnableWindow(GetDlgItem(IDC_OUT_SELECTPATH),check);

      return 0;
   }

   //! called when output path combo box selection ends
   LRESULT OnOutPathSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // on selection, remove selected item from history
      wlUISettings &settings = pui->getUISettings();

      int sel = SendDlgItemMessage(wID,CB_GETCURSEL);
      if (sel>0 && unsigned(sel)<=settings.outputhistory.size())
      {
         settings.outputhistory.insert(settings.outputhistory.begin(), settings.outputdir);
         settings.outputdir = settings.outputhistory[sel];
         settings.outputhistory.erase(settings.outputhistory.begin()+sel);
      }

      RefreshHistory();
      return 0;
   }

   //! refreshes output dir history combobox
   void RefreshHistory();

   // virtual functions from PageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   BevelLine bevel1; //!< bevel line
   BevelLine bevel2; //!< bevel line
   BevelLine bevel3; //!< bevel line

   //! icon list for image buttons
   CImageList ilIcons;
};


//@}

#endif
