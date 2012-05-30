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
/// \file WmaOutputSettingsPage.h
/// \brief contains the settings page for the Wma output module
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "resource.h"
#include "PageBase.h"
#include "CommonStuff.h"


/// Wma output settings page class

class WmaOutputSettingsPage:
   public PageBase,
   public CDialogResize<WmaOutputSettingsPage>
{
public:
   /// ctor
   WmaOutputSettingsPage()
   {
      IDD = IDD_DLG_WMA;
      captionID = IDS_DLG_CAP_WMA;
      descID = IDS_DLG_DESC_WMA;
      helpID = IDS_HTML_WMA;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(WmaOutputSettingsPage)
   // empty
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(WmaOutputSettingsPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   CHAIN_MSG_MAP(CDialogResize<WmaOutputSettingsPage>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   // virtual functions from PageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   /// bitrate spin button control
   FixedValueSpinButtonCtrl bitrateSpin;
   /// quality spin button control
   FixedValueSpinButtonCtrl qualitySpin;

   /// bevel line
   BevelLine bevel1;
};


/// @}
