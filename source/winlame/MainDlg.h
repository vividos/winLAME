/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2009 Michael Fink

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

*/
/// \file MainDlg.h
/// \brief contains the main dialog window
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "resource.h"
#include "PageBase.h"
#include "UIinterface.h"
#include "LanguageResourceManager.hpp"


/// main dialog class

class MainDlg:
   public CDialogImpl<MainDlg>,
   public UIinterface,
   public CDialogResize<MainDlg>
{
public:
   /// ctor
   MainDlg(UISettings& settings, LanguageResourceManager& langResourceManager);

   /// dtor
   ~MainDlg();

   // dialog id
   enum { IDD = IDD_MAINDLG };

   /// runs the winLAME dialog
   void RunDialog();

   // resize map
BEGIN_DLGRESIZE_MAP(MainDlg)
   DLGRESIZE_CONTROL(IDC_MDLG_CAPTIONBAR, DLSZ_SIZE_X)
   // m_hWnd
   DLGRESIZE_CONTROL(IDC_MDLG_FRAME, DLSZ_SIZE_X | DLSZ_SIZE_Y)
   DLGRESIZE_CONTROL(IDC_STATIC_WINLAME_LOGO, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_STATIC_MAIN_BEVEL, DLSZ_MOVE_Y | DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_STATIC_MAIN_BEVEL2, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_MDLG_EXIT, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_MDLG_NEXT, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_MDLG_BACK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_MDLG_HELP, DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(MainDlg)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
   COMMAND_ID_HANDLER(IDCANCEL, OnExit)
   COMMAND_HANDLER(IDC_MDLG_EXIT, BN_CLICKED, OnExit)
   COMMAND_HANDLER(IDC_MDLG_BACK, BN_CLICKED, OnButtonBack)
   COMMAND_HANDLER(IDC_MDLG_NEXT, BN_CLICKED, OnButtonNext)
   COMMAND_HANDLER(IDC_MDLG_HELP, BN_CLICKED, OnHelpButton)
   COMMAND_HANDLER(IDC_MDLG_BACK, 1, OnButtonBack) // from accelerator keys
   COMMAND_HANDLER(IDC_MDLG_NEXT, 1, OnButtonNext)
   COMMAND_HANDLER(IDC_INPUT_BUTTON_INFILESEL, 1, OnSendToPage)
   MESSAGE_HANDLER(WM_HELP, OnHelp)
   MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
   MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
   MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   MESSAGE_HANDLER(WM_SIZE, OnSize)
   CHAIN_MSG_MAP(CDialogResize<MainDlg>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// initializes the main dialog
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when the main dialog is about to be destroyed
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // destroy currently active page
      if (currentpage != -1 && unsigned(currentpage) < pages.size())
         pages[currentpage]->DestroyWindow();
      return 0;
   }

   /// called before a windows message is processed
   virtual BOOL PreTranslateMessage(MSG* pMsg)
   {
      // check for escape key pressed or released
      if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
         m_bKeyDownEscape = true;

      if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_ESCAPE)
         m_bKeyDownEscape = false;

      // relays the message to the tooltip ctrl
      if (pMsg->message==WM_MOUSEMOVE)
         tooltips.RelayEvent(pMsg);

      // translate accelerator keys to commands
      if (actable!=NULL && pMsg->message>=WM_KEYFIRST && pMsg->message<=WM_KEYLAST)
         return ::TranslateAccelerator(m_hWnd,actable,pMsg);

      // don't skip that message
      return FALSE;
   }

   /// called on exiting the main dialog
   LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // closing via ESC key or menu entry?
      if (m_bKeyDownEscape && wID == IDCANCEL && wNotifyCode == 0)
      {
         m_bKeyDownEscape = false;
         return 1; // prevent from exiting via ESC key
      }

      // ends the nonmodal dialog
      PostQuitMessage(wID);
      //EndDialog(wID);
      return 0;
   }

   /// draws the caption bar
   LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // draws all sorts of owner drawn items
      UINT idCtl = (UINT) wParam;
      LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

      if (idCtl==IDC_MDLG_CAPTIONBAR)
         DrawCaptionBar(lpdis->hDC,lpdis->rcItem);

      return 0;
   }

   /// called on clicking on the back button
   LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // activate last page
      if (currentpage>0)
         ActivatePage(currentpage-1);
      return 0;
   }

   /// called on clicking on the next button
   LRESULT OnButtonNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // activate next page
      if (unsigned(currentpage+1)<pages.size())
         ActivatePage(currentpage+1);
      return 0;
   }

   /// sends a command message to the page window
   LRESULT OnSendToPage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // passes message to page window
      PageBase *pageWnd = pages[currentpage];
      pageWnd->SendMessage(WM_COMMAND,MAKEWPARAM(wID, BN_CLICKED), (LPARAM)hWndCtl);
      return 0;
   }

   /// called on clicking on the help button
   LRESULT OnHelpButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      if (helpavailable)
      {
         // load the string with the help string ID
         CString helppath;
         helppath.LoadString(pages[currentpage]->helpID);

         // display help topic
//         if (helppath.GetLength()!=0)
//            htmlhelp.DisplayTopic(helppath);
      }
      return 0;
   }

   /// called when pressing F1
   LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // same as pressing the help button
      return OnHelpButton(0,0,0,bHandled);
   }

   /// called for every system command; used for the about box system menu entry
   LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called for processing key presses
   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // ignore escape key
      if (VK_ESCAPE == (int)wParam)
         return 1;
      return 0;
   }

   /// called when sizing dialog
   LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

   // virtual interface methods from UIinterface

   virtual int getCurrentWizardPage()
   {
      return currentpage;
   }

   virtual int getWizardPageCount()
   {
      return pages.size();
   }

   virtual void insertWizardPage(int at, PageBase *page)
   {
      pages.insert(pages.begin()+at,page);
   }

   virtual int getWizardPageID(int at)
   {
      return pages[at]->IDD;
   }

   virtual void deleteWizardPage(int at)
   {
      if (unsigned(at)<pages.size())
      {
         delete pages[at];
         pages.erase(pages.begin()+at);
      }
   }

   virtual void setCurrentPage(int page)
   {
      currentpage = page;
   }

   virtual void lockWizardButtons(bool lock, bool only_lock_next=false)
   {
      if (lock)
      {
         if (!only_lock_next)
            ::EnableWindow(GetDlgItem(IDC_MDLG_BACK),FALSE);
         ::EnableWindow(GetDlgItem(IDC_MDLG_NEXT),FALSE);
      }
      else
      {
         ::EnableWindow(GetDlgItem(IDC_MDLG_BACK),currentpage==0 ? FALSE : TRUE );
         ::EnableWindow(GetDlgItem(IDC_MDLG_NEXT),unsigned(currentpage)==(pages.size()-1) ? FALSE : TRUE );
      }
   }

   virtual UISettings& getUISettings()
   {
      return settings;
   }

   virtual LanguageResourceManager& GetLanguageResourceManager()
   {
      return m_langResourceManager; 
   }

protected:
   /// app window icon
   HICON wndicon;

   /// small app window icon
   HICON wndicon_small;

   /// all pages
   std::vector<PageBase *> pages;

   /// currently activated page
   int currentpage;

   /// tooltips ctrl
   CToolTipCtrl tooltips;

   /// help icon
   CImageList ilHelpIcon;

   /// indicates if help file is available
   bool helpavailable;

   /// ui settings
   UISettings& settings;

   /// app accelerator table
   HACCEL actable;

   /// size of page; for resizing
   SIZE m_sizePage;

   /// indicates if escape key is currently pressed
   bool m_bKeyDownEscape;

   /// language resource manager
   LanguageResourceManager& m_langResourceManager;

protected:
   /// draws the caption bar
   void DrawCaptionBar(HDC &hDC, RECT &rc);

   /// activates a page; returns true if successful
   bool ActivatePage(int page);

   /// retrieves winLAME base dir
   CString GetWinlameDir();

   /// collects files specified at the command line
   void GetCommandLineFiles();
};


/// @}
