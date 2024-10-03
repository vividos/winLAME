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
/// \file AACSettingsPage.cpp
/// \brief AAC settings page
//
#include "stdafx.h"
#include "AACSettingsPage.hpp"
#include "WizardPageHost.hpp"
#include <ulib/IoCContainer.hpp>
#include "UISettings.hpp"
#include "OutputSettingsPage.hpp"
#include "PresetSelectionPage.hpp"
#include "FinishPage.hpp"

using namespace UI;

// arrays and mappings

/// possible bitrate values
static int AacBitrates[] =
{
   24, 32, 48, 56, 64, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 576
};

/// possible bandwidth frequency values
static int AacBandwidthValues[] =
{
   8000, 11025, 16000, 18000, 19500, 22050, 24000, 32000, 44100, 48000
};


LRESULT AACSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   // spin button controls
   m_spinBitrate.SetBuddy(GetDlgItem(IDC_AAC_EDIT_BITRATE));
   m_spinBitrate.SetFixedValues(AacBitrates, sizeof(AacBitrates) / sizeof(AacBitrates[0]));

   m_spinBandwidth.SetBuddy(GetDlgItem(IDC_AAC_EDIT_BANDWIDTH));
   m_spinBandwidth.SetFixedValues(AacBandwidthValues, sizeof(AacBandwidthValues) / sizeof(AacBandwidthValues[0]));

   // set up range of slider control
   m_sliderQuality.SetRangeMin(10);
   m_sliderQuality.SetRangeMax(500);
   m_sliderQuality.SetTicFreq(10);

   // combo box with mpeg versions
   m_comboMpegVersion.AddString(_T("MPEG2"));
   m_comboMpegVersion.AddString(_T("MPEG4"));

   // combo box with aac object type
   // Note: libfaac only supports LOW (=1) now, so just add this
   //m_comboObjectType.AddString(_T("Main"));
   m_comboObjectType.AddString(_T("Low Complexity"));
   //m_comboObjectType.AddString(_T("LTP"));

   LoadData();

   return 1;
}

LRESULT AACSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (!SaveData())
      return 1; // prevent leaving page

   m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new FinishPage(m_pageHost)));

   return 0;
}

LRESULT AACSettingsPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   return 0;
}

LRESULT AACSettingsPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

   if (m_uiSettings.preset_avail && presetManager.getPresetCount() > 0)
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   else
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

void AACSettingsPage::UpdateQuality()
{
   // update slider quality text
   int pos = m_sliderQuality.GetPos();

   CString text;
   text.Format(IDS_AAC_QUALITY_U, pos);
   SetDlgItemText(IDC_AAC_STATIC_QUALITY, text);

   int kbps = ((pos * 2 / 5) + 85) / 2;

   text.Format(IDS_AAC_KBPS_PER_CH_U, kbps);
   SetDlgItemText(IDC_AAC_STATIC_KBPS, text);
}

void AACSettingsPage::UpdateBandwidth()
{
   // check if bandwidth button is checked
   CButton checkBandwidth(GetDlgItem(IDC_AAC_CHECK_BANDWIDTH));
   bool enabled = BST_CHECKED != checkBandwidth.GetCheck();

   GetDlgItem(IDC_AAC_EDIT_BANDWIDTH).EnableWindow(enabled);
   m_spinBandwidth.EnableWindow(enabled);
}

void AACSettingsPage::LoadData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // set bitrate and bandwidth
   SetDlgItemInt(IDC_AAC_EDIT_BITRATE, mgr.queryValueInt(AacBitrate), FALSE);
   SetDlgItemInt(IDC_AAC_EDIT_BANDWIDTH, mgr.queryValueInt(AacBandwidth), FALSE);

   // set combo box selections
   m_comboMpegVersion.SetCurSel(mgr.queryValueInt(AacMpegVersion) == 2 ? 0 : 1);

   // Note: libfaac only supports LOW (=1) now, so disregard the selection
   //m_comboObjectType.SetCurSel(mgr.queryValueInt(AacObjectType));
   m_comboObjectType.SetCurSel(0);

   // set checks
   m_checkMidSide.SetCheck(mgr.queryValueInt(AacAllowMS) == 0 ? BST_UNCHECKED : BST_CHECKED);
   m_checkUseTNS.SetCheck(mgr.queryValueInt(AacUseTNS) == 0 ? BST_UNCHECKED : BST_CHECKED);
   m_checkUseLFE.SetCheck(mgr.queryValueInt(AacUseLFEChan) == 0 ? BST_UNCHECKED : BST_CHECKED);
   m_checkBandwidth.SetCheck(mgr.queryValueInt(AacAutoBandwidth) == 0 ? BST_UNCHECKED : BST_CHECKED);

   UpdateBandwidth();

   // bitrate control
   int value = mgr.queryValueInt(AacBRCMethod);
   DDX_Radio(IDC_AAC_RADIO_BRCMODE1, value, DDX_LOAD);

   // quality slider
   value = mgr.queryValueInt(AacQuality);
   m_sliderQuality.SetPos(value);
   UpdateQuality();
}

bool AACSettingsPage::SaveData()
{
   DoDataExchange(DDX_SAVE);

   SettingsManager& mgr = m_uiSettings.settings_manager;

   // get bitrate and bandwidth
   mgr.setValue(AacBitrate, (int)GetDlgItemInt(IDC_AAC_EDIT_BITRATE, NULL, FALSE));
   mgr.setValue(AacBandwidth, (int)GetDlgItemInt(IDC_AAC_EDIT_BANDWIDTH, NULL, FALSE));

   // get combo box selections
   int mpeg = m_comboMpegVersion.GetCurSel();
   mgr.setValue(AacMpegVersion, mpeg == 0 ? 2 : 4);

   // Note: libfaac only supports LOW (=1) now, so disregard the selection
   //int value = m_comboObjectType.GetCurSel();
   mgr.setValue(AacObjectType, 1);

   // currently it is not possible to use LTP together with MPEG2
   if (mpeg == 0 && value == 2)
   {
      // give user a message explaining that fact
      AtlMessageBox(m_hWnd, IDS_AAC_NO_MPEG2_LTP, IDS_APP_CAPTION, MB_OK | MB_ICONEXCLAMATION);
      return false;
   }

   // get checks
   value = m_checkMidSide.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacAllowMS, value);

   value = m_checkUseTNS.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacUseTNS, value);

   value = m_checkUseLFE.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacUseLFEChan, value);

   value = m_checkBandwidth.GetCheck() == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacAutoBandwidth, value);

   // bitrate control
   DDX_Radio(IDC_AAC_RADIO_BRCMODE1, value, DDX_SAVE);
   mgr.setValue(AacBRCMethod, value);

   // quality slider
   value = m_sliderQuality.GetPos();
   mgr.setValue(AacQuality, value);

   return true;
}
