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
/// \file AacSettingsPage.hpp
/// \brief contains the basic settings page for the AAC encoder
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "resource.h"
#include "PageBase.hpp"
#include "CommonStuff.hpp"


// classes

/// settings page for AAC settings
class AacSettingsPage:
   public PageBase,
   public CDialogResize<AacSettingsPage>
{
public:
   /// ctor
   AacSettingsPage()
   {
      IDD = IDD_DLG_AAC;
      captionID = IDS_DLG_CAP_AAC;
      descID = IDS_DLG_DESC_AAC;
      helpID = IDS_HTML_AAC;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(AacSettingsPage)
   DLGRESIZE_CONTROL(IDC_AAC_BEVEL1, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_AAC_BEVEL2, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_AAC_SLIDER_QUALITY, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_AAC_STATIC_QUALITY, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_AAC_STATIC_KBPS, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_AAC_COMBO_MPEGVER, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_AAC_COMBO_OBJTYPE, DLSZ_SIZE_X)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(AacSettingsPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   COMMAND_HANDLER(IDC_AAC_CHECK_BANDWIDTH, BN_CLICKED, OnCheckBandwidth)
   MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
   CHAIN_MSG_MAP(CDialogResize<AacSettingsPage>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when user clicks on the bandwidth check
   LRESULT OnCheckBandwidth(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      UpdateBandwidth();
      return 0;
   }

   /// called when slider is moved
   LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // check if the vbr quality slider was moved
      if ((HWND)lParam == GetDlgItem(IDC_AAC_SLIDER_QUALITY))
         UpdateQuality();
      return 0;
   }

   /// updates quality value
   void UpdateQuality();

   /// shows bandwidth edit control
   void UpdateBandwidth();

   // virtual functions from PageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   /// bitrate spin button control
   FixedValueSpinButtonCtrl bitrateSpin;

   /// bandwidth spin button control
   FixedValueSpinButtonCtrl bandwidthSpin;

   /// bevel lines
   BevelLine bevel1, bevel2;
};


/// @}
