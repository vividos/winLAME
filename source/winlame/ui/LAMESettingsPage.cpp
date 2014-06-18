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
/// \file LAMESettingsPage.cpp
/// \brief LAME settings page

// includes
#include "StdAfx.h"
#include "LAMESettingsPage.hpp"
#include "WizardPageHost.h"
#include "IoCContainer.hpp"
#include "UISettings.h"
#include "OutputSettingsPage.hpp"
#include "PresetSelectionPage.hpp"
#include "FinishPage.hpp"

using namespace UI;

/// bitrate values
static const int LameBitrates[] =
{
   8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 192, 224, 256, 320
};

/// quality values
static const int LameQualities[] =
{
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};


LAMESettingsPage::LAMESettingsPage(WizardPageHost& pageHost) throw()
:WizardPage(pageHost, IDD_PAGE_LAME_SETTINGS, WizardPage::typeCancelBackNext),
m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
m_iRadioType(0)
{
}

LRESULT LAMESettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   m_bitrateSpin.SetBuddy(GetDlgItem(IDC_LAME_EDIT_BITRATE));
   m_bitrateSpin.SetFixedValues(LameBitrates, sizeof(LameBitrates) / sizeof(LameBitrates[0]));

   m_qualitySpin.SetBuddy(GetDlgItem(IDC_LAME_EDIT_QUALITY));
   m_qualitySpin.SetFixedValues(LameQualities, sizeof(LameQualities) / sizeof(LameQualities[0]));

   CString cszText(MAKEINTRESOURCE(IDS_LAME_QUALITY_FAST));
   m_cbEncodingQuality.AddString(cszText);
   cszText.LoadString(IDS_LAME_QUALITY_STANDARD);
   m_cbEncodingQuality.AddString(cszText);
   cszText.LoadString(IDS_LAME_QUALITY_HIGH);
   m_cbEncodingQuality.AddString(cszText);

   cszText.LoadString(IDS_LAME_BITRATE_MODE_STANDARD);
   m_cbVariableBitrateMode.AddString(cszText);
   cszText.LoadString(IDS_LAME_BITRATE_MODE_FAST);
   m_cbVariableBitrateMode.AddString(cszText);

   LoadData();

   return 1;
}

LRESULT LAMESettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new FinishPage(m_pageHost)));

   return 0;
}

LRESULT LAMESettingsPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   return 0;
}

LRESULT LAMESettingsPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

   if (m_uiSettings.preset_avail && presetManager.getPresetCount() > 0)
      m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   else
      m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

LRESULT LAMESettingsPage::OnRadioEncodeType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   BOOL bEnableBitrate = TRUE;
   BOOL bEnableQuality = FALSE;

   if (wID == IDC_LAME_RADIO_TYPE2)
   {
      bEnableBitrate = FALSE;
      bEnableQuality = TRUE;
   }

   m_checkCBR.EnableWindow(bEnableBitrate);
   GetDlgItem(IDC_LAME_EDIT_BITRATE).EnableWindow(bEnableBitrate);
   GetDlgItem(IDC_LAME_SPIN_BITRATE).EnableWindow(bEnableBitrate);

   GetDlgItem(IDC_LAME_EDIT_QUALITY).EnableWindow(bEnableQuality);
   GetDlgItem(IDC_LAME_SPIN_QUALITY).EnableWindow(bEnableQuality);
   GetDlgItem(IDC_LAME_COMBO_VBR_MODE).EnableWindow(bEnableQuality);
   GetDlgItem(IDC_LAME_STATIC_VBR_MODE).EnableWindow(bEnableQuality);

   return 0;
}

void LAMESettingsPage::LoadData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // encode quality
   int value = mgr.queryValueInt(LameSimpleEncodeQuality);
   m_cbEncodingQuality.SetCurSel(value);

   // radio buttons for quality or bitrate
   m_iRadioType = mgr.queryValueInt(LameSimpleQualityOrBitrate);
   DDX_Radio(IDC_LAME_RADIO_TYPE1, m_iRadioType, DDX_LOAD);
   BOOL bDummy = true;
   OnRadioEncodeType(0, m_iRadioType == 0 ? IDC_LAME_RADIO_TYPE1 : IDC_LAME_RADIO_TYPE2, NULL, bDummy);

   // mono check
   m_checkMono.SetCheck(mgr.queryValueInt(LameSimpleMono) == 0 ? BST_UNCHECKED : BST_CHECKED);

   // bitrate
   value = mgr.queryValueInt(LameSimpleBitrate);
   SetDlgItemInt(IDC_LAME_EDIT_BITRATE, value, FALSE);

   // quality
   value = mgr.queryValueInt(LameSimpleQuality);
   SetDlgItemInt(IDC_LAME_EDIT_QUALITY, value, FALSE);

   // CBR check
   m_checkCBR.SetCheck(mgr.queryValueInt(LameSimpleCBR) == 0 ? BST_UNCHECKED : BST_CHECKED);

   // VBR mode
   value = mgr.queryValueInt(LameSimpleVBRMode);
   m_cbVariableBitrateMode.SetCurSel(value);

   // "nogap" check
   m_checkNogap.SetCheck(mgr.queryValueInt(LameOptNoGap) == 0 ? BST_UNCHECKED : BST_CHECKED);

   // "prepend RIFF WAVE Header" check
   m_checkWaveMp3.SetCheck(mgr.queryValueInt(LameWriteWaveHeader) == 0 ? BST_UNCHECKED : BST_CHECKED);
}

void LAMESettingsPage::SaveData()
{
   DoDataExchange(DDX_SAVE);

   SettingsManager& mgr = m_uiSettings.settings_manager;

   // encode quality
   mgr.setValue(LameSimpleEncodeQuality, m_cbEncodingQuality.GetCurSel());

   // radio buttons for quality or bitrate
   int value = 0;
   DDX_Radio(IDC_LAME_RADIO_TYPE1, value, DDX_SAVE);
   mgr.setValue(LameSimpleQualityOrBitrate, value);

   // mono check
   value = m_checkMono.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(LameSimpleMono, value);

   // bitrate
   mgr.setValue(LameSimpleBitrate, (int)GetDlgItemInt(IDC_LAME_EDIT_BITRATE, NULL, FALSE));

   // quality
   mgr.setValue(LameSimpleQuality, (int)GetDlgItemInt(IDC_LAME_EDIT_QUALITY, NULL, FALSE));

   // CBR check
   value = m_checkCBR.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(LameSimpleCBR, value);

   // VBR mode
   mgr.setValue(LameSimpleVBRMode, m_cbVariableBitrateMode.GetCurSel());

   // "nogap" check
   value = m_checkNogap.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(LameOptNoGap, value);

   // "prepend RIFF WAVE Header" check
   value = m_checkWaveMp3.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(LameWriteWaveHeader, value);
}
