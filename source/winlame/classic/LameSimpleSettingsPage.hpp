//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2005 Michael Fink
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
/// \file LameSimpleSettingsPage.hpp
/// \brief contains the basic settings page for the LAME encoder
//
#pragma once

#include "resource.h"
#include "PageBase.hpp"
#include "CommonStuff.hpp"

namespace ClassicUI
{
   /// simple settings page for LAME
   class LameSimpleSettingsPage :
      public PageBase,
      public CDialogResize<LameSimpleSettingsPage>
   {
   public:
      /// ctor
      LameSimpleSettingsPage()
      {
         IDD = IDD_DLG_LAME_SIMPLE;
         captionID = IDS_DLG_CAP_LAME_SIMPLE;
         descID = IDS_DLG_DESC_LAME_SIMPLE;
         helpID = IDS_HTML_LAME_SIMPLE;
      }

      // resize map
      BEGIN_DLGRESIZE_MAP(LameSimpleSettingsPage)
         DLGRESIZE_CONTROL(IDC_LAME_BEVEL1, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_LAME_COMBO_ENCODING_QUALITY, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_LAME_CHECK_MONO, DLSZ_MOVE_X)
         DLGRESIZE_CONTROL(IDC_LAME_COMBO_VBR_MODE, DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      // message map
      BEGIN_MSG_MAP(LameSimpleSettingsPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
         COMMAND_HANDLER(IDC_LAME_RADIO_TYPE1, BN_CLICKED, OnRadioEncodeType)
         COMMAND_HANDLER(IDC_LAME_RADIO_TYPE2, BN_CLICKED, OnRadioEncodeType)
         CHAIN_MSG_MAP(CDialogResize<LameSimpleSettingsPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()
      // Handler prototypes:
      //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
      //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

         /// inits the page
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

      // virtual functions from PageBase

      // called on entering the page
      virtual void OnEnterPage();

      // called on leaving the page
      virtual bool OnLeavePage();

   protected:
      /// bitrate spin button control
      FixedValueSpinButtonCtrl m_bitrateSpin;
      /// quality spin button control
      FixedValueSpinButtonCtrl m_qualitySpin;

      /// quality combobox
      CComboBox m_cbEncodingQuality;
      /// VBR combobox
      CComboBox m_cbVariableBitrateMode;

      BevelLine bevel1; ///< bevel line
      BevelLine bevel2; ///< bevel line
   };

} // namespace ClassicUI
