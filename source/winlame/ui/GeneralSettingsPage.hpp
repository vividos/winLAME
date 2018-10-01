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
/// \file GeneralSettingsPage.hpp
/// \brief General settings page
//
#pragma once

#include "WizardPage.hpp"
#include "ImageListComboBox.hpp"
#include "FixedValueSpinButtonCtrl.hpp"
#include "UISettings.hpp"
#include "resource.h"

struct UISettings;
class LanguageResourceManager;

namespace UI
{
   /// general settings page
   class GeneralSettingsPage :
      public WizardPage,
      public CWinDataExchange<GeneralSettingsPage>,
      public CDialogResize<GeneralSettingsPage>
   {
   public:
      /// ctor
      GeneralSettingsPage(WizardPageHost& pageHost,
         UISettings& settings, LanguageResourceManager& langResourceManager)
         :WizardPage(pageHost, IDD_SETTINGS_GENERAL, WizardPage::typeCancelOk),
         m_settings(settings),
         m_langResourceManager(langResourceManager)
      {
      }
      /// dtor
      ~GeneralSettingsPage()
      {
      }

   private:
      friend CDialogResize<GeneralSettingsPage>;

      BEGIN_DDX_MAP(GeneralSettingsPage)
         DDX_CONTROL(IDC_SETTINGS_COMBO_LANGUAGE, m_comboLanguages)
         DDX_INT(IDC_SETTINGS_EDIT_CPU_CORES, m_settings.m_taskManagerConfig.m_uiUseNumTasks)
         DDX_CONTROL_HANDLE(IDC_SETTINGS_EDIT_CPU_CORES, m_editCpuCores)
         DDX_CONTROL(IDC_SETTINGS_SPIN_CPU_CORES, m_spinCpuCores)
         DDX_CONTROL_HANDLE(IDC_SETTINGS_STATIC_NOTE_CPU_CORES, m_staticNoteCpuCores)
         DDX_CHECK(IDC_SETTINGS_CHECK_AUTO_TASKS, m_settings.m_taskManagerConfig.m_bAutoTasksPerCpu)
         DDX_CONTROL_HANDLE(IDC_SETTINGS_CHECK_AUTO_TASKS, m_checkBoxAutoTasks)
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(GeneralSettingsPage)
         DLGRESIZE_CONTROL(IDC_SETTINGS_COMBO_LANGUAGE, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_SETTINGS_CHECK_AUTO_TASKS, DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(GeneralSettingsPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
         COMMAND_HANDLER(IDC_SETTINGS_CHECK_AUTO_TASKS, BN_CLICKED, OnCheckAutoTasks)
         COMMAND_HANDLER(IDC_SETTINGS_EDIT_CPU_CORES, EN_CHANGE, OnChangeEditCpuCores)
         CHAIN_MSG_MAP(CDialogResize<GeneralSettingsPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

      /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when page is left
      LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

      /// called when the "auto tasks" check changes
      LRESULT OnCheckAutoTasks(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when the CPU cores edit field changes
      LRESULT OnChangeEditCpuCores(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   private:
      /// settings
      UISettings& m_settings;

      /// language resource manager
      LanguageResourceManager& m_langResourceManager;

      // controls

      /// languages combobox
      ImageListComboBox m_comboLanguages;

      /// icons for language flags
      CImageList m_imageListIcons;

      /// edit control with number of CPU cores
      CEdit m_editCpuCores;

      /// CPU cores spin button control
      FixedValueSpinButtonCtrl m_spinCpuCores;

      /// static control with a note about the changed CPU cores setting
      CStatic m_staticNoteCpuCores;

      /// checkbox for automatic choosing of number of tasks
      CButton m_checkBoxAutoTasks;
   };

} // namespace UI
