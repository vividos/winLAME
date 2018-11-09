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
/// \file AACSettingsPage.hpp
/// \brief AAC settings page
//
#pragma once

#include "WizardPage.hpp"
#include "resource.h"
#include "BevelLine.hpp"
#include "FixedValueSpinButtonCtrl.hpp"

struct UISettings;

namespace UI
{
   /// \brief AAC settings page
   class AACSettingsPage :
      public WizardPage,
      public CWinDataExchange<AACSettingsPage>,
      public CDialogResize<AACSettingsPage>
   {
   public:
      /// ctor
      explicit AACSettingsPage(WizardPageHost& pageHost)
         :WizardPage(pageHost, IDD_PAGE_AAC_SETTINGS, WizardPage::typeCancelBackNext),
         m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
      {
      }
      /// dtor
      ~AACSettingsPage()
      {
      }

   private:
      friend CDialogResize<AACSettingsPage>;

      BEGIN_DDX_MAP(AACSettingsPage)
         DDX_CONTROL(IDC_AAC_BEVEL1, m_bevel1)
         DDX_CONTROL(IDC_AAC_BEVEL2, m_bevel2)
         DDX_CONTROL(IDC_AAC_SPIN_BITRATE, m_spinBitrate)
         DDX_CONTROL(IDC_AAC_SPIN_BANDWIDTH, m_spinBandwidth)
         DDX_CONTROL_HANDLE(IDC_AAC_SLIDER_QUALITY, m_sliderQuality)
         DDX_CONTROL_HANDLE(IDC_AAC_COMBO_MPEGVER, m_comboMpegVersion)
         DDX_CONTROL_HANDLE(IDC_AAC_COMBO_OBJTYPE, m_comboObjectType)
         DDX_CONTROL_HANDLE(IDC_AAC_CHECK_MIDSIDE, m_checkMidSide)
         DDX_CONTROL_HANDLE(IDC_AAC_CHECK_USETNS, m_checkUseTNS)
         DDX_CONTROL_HANDLE(IDC_AAC_CHECK_USELFE, m_checkUseLFE)
         DDX_CONTROL_HANDLE(IDC_AAC_CHECK_BANDWIDTH, m_checkBandwidth)
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(AACSettingsPage)
         DLGRESIZE_CONTROL(IDC_AAC_BEVEL1, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_AAC_BEVEL2, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_AAC_SLIDER_QUALITY, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_AAC_STATIC_QUALITY, DLSZ_MOVE_X)
         DLGRESIZE_CONTROL(IDC_AAC_STATIC_KBPS, DLSZ_MOVE_X)
         DLGRESIZE_CONTROL(IDC_AAC_COMBO_MPEGVER, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_AAC_COMBO_OBJTYPE, DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(AACSettingsPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
         COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
         COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
         COMMAND_HANDLER(IDC_AAC_CHECK_BANDWIDTH, BN_CLICKED, OnCheckBandwidth)
         MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
         CHAIN_MSG_MAP(CDialogResize<AACSettingsPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

      /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when page is left with Next button
      LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with Cancel button
      LRESULT OnButtonCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with Back button
      LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

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

      /// loads settings data into controls
      void LoadData();

      /// saves settings data from controls
      bool SaveData();

   private:
      // controls

      /// bitrate spin button control
      FixedValueSpinButtonCtrl m_spinBitrate;

      /// bandwidth spin button control
      FixedValueSpinButtonCtrl m_spinBandwidth;

      /// quality slider
      CTrackBarCtrl m_sliderQuality;

      /// MPEG version
      CComboBox m_comboMpegVersion;

      /// object type
      CComboBox m_comboObjectType;

      /// checkbox for mid/side mode
      CButton m_checkMidSide;

      /// checkbox for "use TNS" flag
      CButton m_checkUseTNS;

      /// checkbox for "use LFE" flag
      CButton m_checkUseLFE;

      /// checkbox for bandwidth limiting
      CButton m_checkBandwidth;

      /// bevel line
      BevelLine m_bevel1;

      /// bevel line
      BevelLine m_bevel2;

      // model

      /// settings
      UISettings& m_uiSettings;
   };

} // namespace UI
