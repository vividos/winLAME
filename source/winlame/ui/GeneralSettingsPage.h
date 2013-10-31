//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2012 Michael Fink
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
/// \file GeneralSettingsPage.h
/// \brief General settings page

// include guard
#pragma once

// includes
#include "WizardPage.h"
#include "ImageListComboBox.h"
#include "resource.h"

// forward references
struct UISettings;
class LanguageResourceManager;

/// general settings page
class GeneralSettingsPage:
   public WizardPage,
   public CWinDataExchange<GeneralSettingsPage>,
   public CDialogResize<GeneralSettingsPage>
{
public:
   /// ctor
   GeneralSettingsPage(WizardPageHost& pageHost,
      UISettings& settings, LanguageResourceManager& langResourceManager) throw()
      :WizardPage(pageHost, IDD_SETTINGS_GENERAL, WizardPage::typeCancelOk),
       m_settings(settings),
       m_langResourceManager(langResourceManager)
   {
   }
   /// dtor
   ~GeneralSettingsPage() throw()
   {
   }

private:
   friend CDialogResize<GeneralSettingsPage>;

   BEGIN_DDX_MAP(GeneralSettingsPage)
      DDX_CONTROL(IDC_SETTINGS_COMBO_LANGUAGE, m_cbLanguages)
   END_DDX_MAP()

   // resize map
   BEGIN_DLGRESIZE_MAP(GeneralSettingsPage)
      DLGRESIZE_CONTROL(IDC_SETTINGS_COMBO_LANGUAGE, DLSZ_SIZE_X)
   END_DLGRESIZE_MAP()

   // message map
   BEGIN_MSG_MAP(GeneralSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      CHAIN_MSG_MAP(CDialogResize<GeneralSettingsPage>)
   END_MSG_MAP()

   // Handler prototypes:
   // LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   // LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   // LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is left
   LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
   /// settings
   UISettings& m_settings;

   /// lang resource manager
   LanguageResourceManager& m_langResourceManager;

   // controls

   /// languages combobox
   CImageListComboBox m_cbLanguages;

   /// icons for language flags
   CImageList m_ilIcons;
};
