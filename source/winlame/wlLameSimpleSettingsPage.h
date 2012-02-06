/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005 Michael Fink

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

   $Id: wlLameSimpleSettingsPage.h,v 1.3 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlLameSimpleSettingsPage.h

   \brief contains the basic settings page for the LAME encoder

*/
/*! \ingroup userinterface */
/*! @{ */

// prevent multiple including
#ifndef wlLameSimpleSettingsPage_h_
#define wlLameSimpleSettingsPage_h_

// needed includes
#include "resource.h"
#include "wlPageBase.h"
#include "wlCommonStuff.h"


// classes
/*
IDC_LAME_COMBO_ENCODING_QUALITY
IDC_LAME_CHECK_MONO
IDC_LAME_CHECK_CBR
IDC_LAME_RADIO_TYPE1
IDC_LAME_RADIO_TYPE2
IDC_LAME_EDIT_BITRATE
IDC_LAME_SPIN_BITRATE
IDC_LAME_EDIT_QUALITY
IDC_LAME_SPIN_QUALITY
IDC_LAME_COMBO_VBR_MODE
IDC_LAME_STATIC_VBR_MODE
*/
//! simple settings page for LAME
class wlLameSimpleSettingsPage:
   public wlPageBase,
   public CDialogResize<wlLameSimpleSettingsPage>
{
public:
   //! ctor
   wlLameSimpleSettingsPage()
   {
      IDD = IDD_DLG_LAME_SIMPLE;
      captionID = IDS_DLG_CAP_LAME_SIMPLE;
      descID = IDS_DLG_DESC_LAME_SIMPLE;
      helpID = IDS_HTML_LAME_SIMPLE;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(wlLameSimpleSettingsPage)
   DLGRESIZE_CONTROL(IDC_LAME_BEVEL1, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_LAME_COMBO_ENCODING_QUALITY, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_LAME_CHECK_MONO, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_LAME_COMBO_VBR_MODE, DLSZ_SIZE_X)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(wlLameSimpleSettingsPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
   COMMAND_HANDLER(IDC_LAME_RADIO_TYPE1, BN_CLICKED, OnRadioEncodeType)
   COMMAND_HANDLER(IDC_LAME_RADIO_TYPE2, BN_CLICKED, OnRadioEncodeType)
   CHAIN_MSG_MAP(CDialogResize<wlLameSimpleSettingsPage>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   //! inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when the dialog is about to be destroyed
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      m_cbEncodingQuality.m_hWnd = NULL;
      m_cbVariableBitrateMode.m_hWnd = NULL;
      return 0;
   }

   /// called when radio button for encode type changes
   LRESULT OnRadioEncodeType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   // virtual functions from wlPageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   //! bitrate spin button control
   wlFixedValueSpinButtonCtrl m_bitrateSpin;
   //! quality spin button control
   wlFixedValueSpinButtonCtrl m_qualitySpin;

   //! quality combobox
   CComboBox m_cbEncodingQuality;
   //! VBR combobox
   CComboBox m_cbVariableBitrateMode;

   wlBevelLine bevel1; ///< bevel line
   wlBevelLine bevel2; ///< bevel line
};


//@}

#endif
