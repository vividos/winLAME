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
/// \file LibsndfileSettingsPage.hpp
/// \brief Libsndfile settings page
//
#pragma once

#include "WizardPage.hpp"
#include "resource.h"
#include "CommonStuff.hpp"

struct UISettings;

namespace UI
{
   /// \brief Libsndfile settings page
   class LibsndfileSettingsPage :
      public WizardPage,
      public CWinDataExchange<LibsndfileSettingsPage>,
      public CDialogResize<LibsndfileSettingsPage>
   {
   public:
      /// ctor
      LibsndfileSettingsPage(WizardPageHost& pageHost)
         :WizardPage(pageHost, IDD_PAGE_LIBSNDFILE_SETTINGS, WizardPage::typeCancelBackNext),
         m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
      {
      }
      /// dtor
      ~LibsndfileSettingsPage()
      {
      }

   private:
      friend CDialogResize<LibsndfileSettingsPage>;

      BEGIN_DDX_MAP(LibsndfileSettingsPage)
         DDX_CONTROL(IDC_WAVE_BEVEL1, m_bevel1);
      DDX_CONTROL_HANDLE(IDC_WAVE_COMBO_FORMAT, m_comboFormat);
      DDX_CONTROL_HANDLE(IDC_WAVE_COMBO_SUBTYPE, m_comboSubType);
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(LibsndfileSettingsPage)
         DLGRESIZE_CONTROL(IDC_WAVE_BEVEL1, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_WAVE_COMBO_FORMAT, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_WAVE_COMBO_SUBTYPE, DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(LibsndfileSettingsPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDC_WAVE_COMBO_FORMAT, CBN_SELENDOK, OnFormatSelEndOk)
         COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
         COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
         COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
         CHAIN_MSG_MAP(CDialogResize<LibsndfileSettingsPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

      // Handler prototypes:
      // LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      // LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
      // LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

      /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when selection on format combobox has changed
      LRESULT OnFormatSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with Next button
      LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with Cancel button
      LRESULT OnButtonCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with Back button
      LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// updates file formats combobox list
      void UpdateFileFormatList();

      /// updates sub type combobox based on selected format
      void UpdateSubTypeCombobox();

      /// loads settings data into controls
      void LoadData();

      /// saves settings data from controls
      void SaveData();

   private:
      // controls

      /// bevel line
      BevelLine m_bevel1;

      /// format combobox
      CComboBox m_comboFormat;

      /// file format combobox
      CComboBox m_comboSubType;

      // model

      /// settings
      UISettings& m_uiSettings;
   };

} // namespace UI
