/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2004 Michael Fink

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
/*! \file AboutDlg.h

   \brief contains the about dialog window class


*/
/*! \ingroup userinterface */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "resource.h"
#include "EncoderInterface.h"


#ifndef OBM_SIZE
#define OBM_SIZE 32766
#endif


//! about dialog

class AboutDlg: public CAxDialogImpl<AboutDlg>
{
public:
   //! ctor
   AboutDlg(){ min_cx = min_cy = last_cx = last_cy = -1; }

   //! dialog id
   enum { IDD = IDD_ABOUTDLG };

   //! sets module manager to use
   void SetModuleManager(ModuleManager *mgr){ module_manager = mgr; };

   //! sets filename of presets.xml file
   void SetPresetsXmlFilename(const CString& cszFilename){ m_cszPresetsXmlFilename = cszFilename; }

   // message map
BEGIN_MSG_MAP(AboutDlg)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_SIZE, OnSize)
   MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
   COMMAND_HANDLER(IDOK, BN_CLICKED, OnExit)
   COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnExit)
END_MSG_MAP()

   //! inits about dialog
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      CenterWindow(GetParent());

      // create child control
      RECT rc;
      ::GetWindowRect(GetDlgItem(IDC_ABOUT_FRAME),&rc);
      ScreenToClient(&rc);
      browser.Create(m_hWnd,rc,_T(""),WS_CHILD|WS_HSCROLL,0);
      browser.ShowWindow(SW_SHOW);

      // host webbrowser control
      CComBSTR htmlstring;
      GetHtmlString(htmlstring);
      browser.CreateControl(htmlstring);

      // get standard window width
      GetWindowRect(&rc);
      min_cx = rc.right-rc.left;
      min_cy = rc.bottom-rc.top;

      // set window icon
      wndicon = (HICON)::LoadImage(_Module.GetResourceInstance(),
         MAKEINTRESOURCE(IDI_ICON_WINLAME),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
      SetIcon(wndicon,FALSE);

      // create resizing control
      sizebmp = LoadBitmap(NULL,MAKEINTRESOURCE(OBM_SIZE));

      sizehandle.Create(m_hWnd, NULL, NULL,
         WS_CHILD | WS_VISIBLE | SS_BITMAP);

      sizehandle.SendMessage(STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)sizebmp);

      // move to lower right position
      RECT rcIcon, rcClient;
      GetClientRect(&rcClient);
      sizehandle.GetClientRect(&rcIcon);

      rcClient.top = rcClient.bottom - (rcIcon.bottom-rcIcon.top);
      rcClient.left = rcClient.right - (rcIcon.right-rcIcon.left);

      sizehandle.SetWindowPos(HWND_TOP,
         rcClient.right - (rcIcon.right-rcIcon.left),
         rcClient.bottom - (rcIcon.bottom-rcIcon.top),
         0, 0, SWP_NOSIZE);

      return 1;
   }

   //! called before resizing, to determinate min window size
   LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      MINMAXINFO *minmax = (MINMAXINFO *)lParam;
      minmax->ptMinTrackSize.x = min_cx;
      minmax->ptMinTrackSize.y = min_cy;

      return 0;
   }

   //! called on resizing window
   LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // calculate width/height change
      RECT rc;
      int cx = LOWORD(lParam), cy = HIWORD(lParam);
      if (last_cx == -1 ||last_cy == -1){ last_cx = cx; last_cy = cy; }
      int dx = cx - last_cx, dy = cy - last_cy;
      last_cx = cx; last_cy = cy;

      // move browser window
      browser.GetWindowRect(&rc);
      rc.right += dx;
      rc.bottom += dy;
      ScreenToClient(&rc);
      browser.MoveWindow(&rc,TRUE);

      // move ok button
      CWindow button(GetDlgItem(IDOK));
      button.GetWindowRect(&rc);
      rc.left += dx/2;
      rc.right += dx/2;
      rc.top += dy;
      rc.bottom += dy;
      ScreenToClient(&rc);
      button.MoveWindow(&rc,TRUE);

      // move resizing icon
      sizehandle.GetWindowRect(&rc);
      rc.left += dx;
      rc.right += dx;
      rc.top += dy;
      rc.bottom += dy;
      ScreenToClient(&rc);
      sizehandle.MoveWindow(&rc,TRUE);

      return 0;
   }

   //! called on exiting the about dialog
   LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // exits dialog
      browser.DestroyWindow();
      EndDialog(0);

      // delete resources
      ::DestroyIcon(wndicon);
      ::DeleteObject(sizebmp);

      return 0;
   }

protected:
   //! retrieves the html document from the resource
   void GetHtmlString(CComBSTR &string);

   //! ActiveX host window
   CAxWindow browser;

   //! resize handle ctrl
   CStatic sizehandle;

   //! module manager to use
   ModuleManager *module_manager;

   //! filename of presets.xml file
   CString m_cszPresetsXmlFilename;

   //! window icon
   HICON wndicon;

   //! size icon bitmap
   HBITMAP sizebmp;

   //! min x window size
   int min_cx;

   //! min y window size
   int min_cy;

   //! last x window size
   int last_cx;

   //! last y window size
   int last_cy;
};


//@}
