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
/// \file PresetSelectionPage.cpp
/// \brief Preset selection
//
#include "StdAfx.h"
#include "PresetSelectionPage.hpp"
#include "WizardPageHost.hpp"
#include "IoCContainer.hpp"
#include "UISettings.h"
#include "OutputSettingsPage.hpp"
#include "FinishPage.hpp"
#include "EncoderInterface.h"

using namespace UI;

LRESULT PresetSelectionPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   LoadData();

   return 1;
}

LRESULT PresetSelectionPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   if (m_uiSettings.m_iLastSelectedPresetIndex == 0)
   {
      ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
      int modid = moduleManager.getOutputModuleID(m_uiSettings.output_module);

      OutputSettingsPage::SetWizardPageByOutputModule(m_pageHost, modid);
   }
   else
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new FinishPage(m_pageHost)));

   return 0;
}

LRESULT PresetSelectionPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   return 0;
}

LRESULT PresetSelectionPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

LRESULT PresetSelectionPage::OnSelItemChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // set new description
   int index = m_lbPresets.GetCurSel();
   if (index != 0)
      SetDlgItemText(IDC_PRE_DESC, m_presetManager.getPresetDescription(index - 1).c_str());
   else
   {
      // set default description
      CString text;
      text.LoadString(IDS_PRE_DESC_DEFAULT);
      SetDlgItemText(IDC_PRE_DESC, text);
   }

   return 0;
}

LRESULT PresetSelectionPage::OnLButtonDblClk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
#ifdef _DEBUG // only in debug mode
   int index = m_lbPresets.GetCurSel();

   // edit doubleclicked item
   if (index != 0)
      m_presetManager.editSettingsDialog(index - 1);
#endif

   return 0;
}

void PresetSelectionPage::LoadData()
{
   // fill listbox
   CString cszText(MAKEINTRESOURCE(IDS_PRESETS_CUSTOM_SETTINGS));
   m_lbPresets.AddString(cszText);

   int max = m_presetManager.getPresetCount();
   for (int i = 0; i<max; i++)
      m_lbPresets.AddString(m_presetManager.getPresetName(i).c_str());

   m_lbPresets.SetCurSel(m_uiSettings.m_iLastSelectedPresetIndex);

   BOOL dummy;
   OnSelItemChanged(0, 0, GetDlgItem(IDC_PRE_LIST_PRESET), dummy);
}

void PresetSelectionPage::SaveData()
{
   // set default values
   m_presetManager.setDefaultSettings(m_uiSettings.settings_manager);

   m_uiSettings.m_iLastSelectedPresetIndex = m_lbPresets.GetCurSel();

   // set selected preset
   if (m_uiSettings.m_iLastSelectedPresetIndex != 0)
      m_presetManager.setSettings(m_uiSettings.m_iLastSelectedPresetIndex - 1, m_uiSettings.settings_manager);
}
