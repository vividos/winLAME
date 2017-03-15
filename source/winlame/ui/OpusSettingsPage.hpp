//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2016 Michael Fink
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
/// \file ui\OpusSettingsPage.hpp
/// \brief Opus encoder settings page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "resource.h"
#include "FixedValueSpinButtonCtrl.hpp"

// forward references
struct UISettings;

namespace UI
{

/// \brief Opus settings page
class OpusSettingsPage:
   public WizardPage,
   public CWinDataExchange<OpusSettingsPage>,
   public CDialogResize<OpusSettingsPage>
{
public:
   /// ctor
   OpusSettingsPage(WizardPageHost& pageHost) throw()
      :WizardPage(pageHost, IDD_PAGE_OPUS_SETTINGS, WizardPage::typeCancelBackNext),
      m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
   {
   }
   /// dtor
   ~OpusSettingsPage() throw()
   {
   }

private:
   friend CDialogResize<OpusSettingsPage>;

   BEGIN_DDX_MAP(OpusSettingsPage)
      DDX_CONTROL(IDC_OPUS_SPIN_BITRATE, m_spinBitrate)
      DDX_CONTROL_HANDLE(IDC_OPUS_SLIDER_COMPLEXITY, m_sliderComplexity)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(OpusSettingsPage)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(OpusSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
      MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
      CHAIN_MSG_MAP(CDialogResize<OpusSettingsPage>)
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

   /// called when slider is moved
   LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // check if the vbr quality slider was moved
      if ((HWND)lParam == GetDlgItem(IDC_OPUS_SLIDER_COMPLEXITY))
         UpdateComplexity();
      return 0;
   }

   /// updates complexity value
   void UpdateComplexity();

   /// loads settings data into controls
   void LoadData();

   /// saves settings data from controls
   bool SaveData();

private:
   // controls

   /// bitrate spin button control
   FixedValueSpinButtonCtrl m_spinBitrate;

   /// complexity slider
   CTrackBarCtrl m_sliderComplexity;

   // model

   /// settings
   UISettings& m_uiSettings;
};

} // namespace UI
