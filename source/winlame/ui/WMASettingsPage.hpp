//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2014 Michael Fink
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
/// \file WMASettingsPage.hpp
/// \brief WMA settings page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "resource.h"
#include "FixedValueSpinButtonCtrl.hpp"
#include "BevelLine.hpp"

// forward references
struct UISettings;

namespace UI
{

/// \brief WMA settings page
class WMASettingsPage:
   public WizardPage,
   public CWinDataExchange<WMASettingsPage>,
   public CDialogResize<WMASettingsPage>
{
public:
   /// ctor
   WMASettingsPage(WizardPageHost& pageHost)
      :WizardPage(pageHost, IDD_PAGE_WMA_SETTINGS, WizardPage::typeCancelBackNext),
      m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
   {
   }
   /// dtor
   ~WMASettingsPage()
   {
   }

private:
   friend CDialogResize<WMASettingsPage>;

   BEGIN_DDX_MAP(WMASettingsPage)
      DDX_CONTROL(IDC_WMA_BEVEL1, m_bevel1)
      DDX_CONTROL(IDC_WMA_SPIN_BITRATE, m_spinBitrate)
      DDX_CONTROL(IDC_WMA_SPIN_QUALITY, m_spinQuality)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(WMASettingsPage)
      DLGRESIZE_CONTROL(IDC_WMA_BEVEL1, DLSZ_SIZE_X)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(WMASettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
      CHAIN_MSG_MAP(CDialogResize<WMASettingsPage>)
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

   /// loads settings data into controls
   void LoadData();

   /// saves settings data from controls
   void SaveData();

private:
   // controls

   /// bitrate spin button control
   FixedValueSpinButtonCtrl m_spinBitrate;

   /// quality spin button control
   FixedValueSpinButtonCtrl m_spinQuality;

   /// bevel line
   BevelLine m_bevel1;

   // model

   /// settings
   UISettings& m_uiSettings;
};

} // namespace UI
