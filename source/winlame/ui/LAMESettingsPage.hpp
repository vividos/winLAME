//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file LAMESettingsPage.hpp
/// \brief LAME settings page
//
#pragma once

#include "WizardPage.hpp"
#include "resource.h"
#include "BevelLine.hpp"
#include "FixedValueSpinButtonCtrl.hpp"

struct UISettings;

namespace UI
{
   /// \brief LAME settings page
   class LAMESettingsPage :
      public WizardPage,
      public CWinDataExchange<LAMESettingsPage>,
      public CDialogResize<LAMESettingsPage>
   {
   public:
      /// ctor
      LAMESettingsPage(WizardPageHost& pageHost);
      /// dtor
      ~LAMESettingsPage()
      {
      }

   private:
      friend CDialogResize<LAMESettingsPage>;

      BEGIN_DDX_MAP(LAMESettingsPage)
         DDX_CONTROL(IDC_LAME_BEVEL1, m_bevel1)
         DDX_CONTROL(IDC_LAME_BEVEL2, m_bevel2)
         DDX_CONTROL(IDC_LAME_BEVEL3, m_bevel3)
         DDX_CONTROL(IDC_LAME_SPIN_BITRATE, m_bitrateSpin)
         DDX_CONTROL(IDC_LAME_SPIN_QUALITY, m_qualitySpin)
         DDX_CONTROL_HANDLE(IDC_LAME_COMBO_ENCODING_QUALITY, m_comboEncodingQuality)
         DDX_CONTROL_HANDLE(IDC_LAME_COMBO_VBR_MODE, m_comboVariableBitrateMode)
         DDX_CONTROL_HANDLE(IDC_LAME_CHECK_MONO, m_checkMono)
         DDX_CONTROL_HANDLE(IDC_LAME_CHECK_CBR, m_checkCBR)
         DDX_CONTROL_HANDLE(IDC_LAME_CHECK_NOGAP, m_checkNogap)
         DDX_CONTROL_HANDLE(IDC_LAME_CHECK_WRITE_WAVEMP3, m_checkWaveMp3)
         DDX_RADIO(IDC_LAME_RADIO_TYPE1, m_radioType);
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(LAMESettingsPage)
         DLGRESIZE_CONTROL(IDC_LAME_BEVEL1, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_LAME_BEVEL2, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_LAME_BEVEL3, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_LAME_COMBO_ENCODING_QUALITY, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_LAME_CHECK_MONO, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_LAME_COMBO_VBR_MODE, DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(LAMESettingsPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
         COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
         COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
         COMMAND_HANDLER(IDC_LAME_RADIO_TYPE1, BN_CLICKED, OnRadioEncodeType)
         COMMAND_HANDLER(IDC_LAME_RADIO_TYPE2, BN_CLICKED, OnRadioEncodeType)
         CHAIN_MSG_MAP(CDialogResize<LAMESettingsPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

      // Handler prototypes:
      // LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      // LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
      // LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

      /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when page is left with Next button
      LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with Cancel button
      LRESULT OnButtonCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with Back button
      LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when radio button for encode type changes
      LRESULT OnRadioEncodeType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// loads settings data into controls
      void LoadData();

      /// saves settings data from controls
      void SaveData();

   private:
      // controls

      /// bitrate spin button control
      FixedValueSpinButtonCtrl m_bitrateSpin;

      /// quality spin button control
      FixedValueSpinButtonCtrl m_qualitySpin;

      /// quality combobox
      CComboBox m_comboEncodingQuality;

      /// VBR combobox
      CComboBox m_comboVariableBitrateMode;

      /// Mono checkbox
      CButton m_checkMono;

      /// CBR checkbox
      CButton m_checkCBR;

      /// Nogap checkbox
      CButton m_checkNogap;

      /// Wave MP3 checkbox
      CButton m_checkWaveMp3;

      BevelLine m_bevel1; ///< bevel line
      BevelLine m_bevel2; ///< bevel line
      BevelLine m_bevel3; ///< bevel line

      // model

      /// settings
      UISettings& m_uiSettings;

      /// output type radio button value
      int m_radioType;
   };

} // namespace UI
