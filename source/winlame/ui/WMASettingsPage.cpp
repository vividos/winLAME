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
/// \file WMASettingsPage.cpp
/// \brief WMA settings page
//
#include "stdafx.h"
#include "WMASettingsPage.hpp"
#include "WizardPageHost.hpp"
#include <ulib/IoCContainer.hpp>
#include "UISettings.hpp"
#include "OutputSettingsPage.hpp"
#include "PresetSelectionPage.hpp"
#include "FinishPage.hpp"

using namespace UI;

/// suggested bitrate values
static int WmaBitrates[] =
{
   32, 48, 64, 80, 96, 128, 160, 192, 256, 320
};

static int WmaVBRQuality[] =
{
   10, 25, 50, 75, 90, 98, 100
};


LRESULT WMASettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   m_spinBitrate.SetBuddy(GetDlgItem(IDC_WMA_EDIT_BITRATE));
   m_spinBitrate.SetFixedValues(WmaBitrates, sizeof(WmaBitrates) / sizeof(WmaBitrates[0]));

   m_spinQuality.SetBuddy(GetDlgItem(IDC_WMA_EDIT_QUALITY));
   m_spinQuality.SetFixedValues(WmaVBRQuality, sizeof(WmaVBRQuality) / sizeof(WmaVBRQuality[0]));

   LoadData();

   return 1;
}

LRESULT WMASettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new FinishPage(m_pageHost)));

   return 0;
}

LRESULT WMASettingsPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   return 0;
}

LRESULT WMASettingsPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

   if (m_uiSettings.preset_avail && presetManager.getPresetCount() > 0)
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   else
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

void WMASettingsPage::LoadData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // set output bitrate
   SetDlgItemInt(IDC_WMA_EDIT_BITRATE, mgr.queryValueInt(WmaBitrate), FALSE);
   SetDlgItemInt(IDC_WMA_EDIT_QUALITY, mgr.queryValueInt(WmaQuality), FALSE);

   // bitrate mode radio buttons
   int value = mgr.queryValueInt(WmaBitrateMode);
   DDX_Radio(IDC_WMA_RADIO_BRMODE1, value, DDX_LOAD);
}

void WMASettingsPage::SaveData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   mgr.setValue(WmaBitrate, (int)GetDlgItemInt(IDC_WMA_EDIT_BITRATE, NULL, FALSE));
   mgr.setValue(WmaQuality, (int)GetDlgItemInt(IDC_WMA_EDIT_QUALITY, NULL, FALSE));

   int value = 0;
   DDX_Radio(IDC_WMA_RADIO_BRMODE1, value, DDX_SAVE);
   mgr.setValue(WmaBitrateMode, value);
}
