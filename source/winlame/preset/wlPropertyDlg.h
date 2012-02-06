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

   $Id: wlPropertyDlg.h,v 1.8 2009/04/10 22:05:25 vividos Exp $

*/
/*! \file wlPropertyDlg.h

   \brief contains the settings property dialog

*/
/*! \ingroup preset */
/*! @{ */

// prevent multiple including
#ifndef __wlpropertydlg_h_
#define __wlpropertydlg_h_

// needed includes
#include "../resource.h"
#include "wlPropertyListBox.h"


//! property dialog

class wlPropertyDlg: public CDialogImpl<wlPropertyDlg>
{
public:
   //! ctor
   wlPropertyDlg(){}

   //! sets the property manager interface for the list box
   void SetPropertyManagerInterface(wlPropertyManagerInterface *mgr)
   {
      listbox.SetPropertyManager(mgr);
   }

   //! dialog id
   enum { IDD = IDD_DLG_PRESET_PROPERTY };

   // message map
BEGIN_MSG_MAP(wlAboutDlg)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   COMMAND_HANDLER(IDOK, BN_CLICKED, OnExit)
   COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnExit)
   MESSAGE_HANDLER(WM_SIZE, OnSize)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()

   //! initializes dialog class
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      CenterWindow(GetParent());

      // subclass listbox
      listbox.SubclassWindow(GetDlgItem(IDC_PROP_LISTBOX));
      listbox.ModifyStyle(0,LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY);
      listbox.InitListBox();

      return 1;
   }

   //! called on exiting dialog
   LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // exits dialog
      EndDialog(0);
      return 0;
   }

   //! called when resizing dialog
   LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      int cx = LOWORD(lParam);
      int cy = HIWORD(lParam);

      RECT rc;
      listbox.GetWindowRect(&rc);
      ScreenToClient(&rc);

      listbox.MoveWindow(rc.left,rc.top,cx,cy);
      listbox.Invalidate();
      return 0;
   }

protected:
   //! the property listbox control
   wlPropertyListBox listbox;
};


//@}

#endif
