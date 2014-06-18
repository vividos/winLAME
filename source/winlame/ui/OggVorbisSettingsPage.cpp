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
/// \file OggVorbisSettingsPage.cpp
/// \brief Ogg Vorbis settings page

// includes
#include "StdAfx.h"
#include "OggVorbisSettingsPage.hpp"
#include "WizardPageHost.h"
#include "IoCContainer.hpp"
#include "UISettings.h"
#include "OutputSettingsPage.hpp"
#include "PresetSelectionPage.hpp"
#include "FinishPage.hpp"

using namespace UI;

// arrays

/// possible bitrates for the fixed value spin button ctrl
static int OggVorbisBitrates[] =
{
   64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500
};


// global functions
extern float OggVorbisCalculateBitrate(float quality);


LRESULT UI::OggVorbisSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   // set up range of slider control
   m_sliderQuality.SetRangeMin(-100);
   m_sliderQuality.SetRangeMax(1000);
   m_sliderQuality.SetTicFreq(100);

   // subclass spin button controls
   m_spinBitrateMin.SetBuddy(GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE));
   m_spinBitrateMin.SetFixedValues(OggVorbisBitrates, sizeof(OggVorbisBitrates) / sizeof(OggVorbisBitrates[0]));

   m_spinBitrateNominal.SetBuddy(GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE));
   m_spinBitrateNominal.SetFixedValues(OggVorbisBitrates, sizeof(OggVorbisBitrates) / sizeof(OggVorbisBitrates[0]));

   m_spinBitrateMax.SetBuddy(GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE));
   m_spinBitrateMax.SetFixedValues(OggVorbisBitrates, sizeof(OggVorbisBitrates) / sizeof(OggVorbisBitrates[0]));

   // set up range of slider control
   m_spinQuickQuality.SetRange(0, 2);
   m_spinQuickQuality.SetPos(1);

   LoadData();

   return 1;
}

LRESULT UI::OggVorbisSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new FinishPage(m_pageHost)));

   return 0;
}

LRESULT UI::OggVorbisSettingsPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   return 0;
}

LRESULT UI::OggVorbisSettingsPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

   if (m_uiSettings.preset_avail && presetManager.getPresetCount() > 0)
      m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   else
      m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

LRESULT UI::OggVorbisSettingsPage::OnSetBitrateMode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   // look which radio button was activated
   int pos;
   DDX_Radio(IDC_OGGV_RADIO_BRMODE1, pos, DDX_SAVE);
   UpdateBitrateMode(pos);

   return 0;
}

LRESULT UI::OggVorbisSettingsPage::OnChangeEditNominalBitrate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // get value of radio button
   int value = m_uiSettings.settings_manager.queryValueInt(OggBitrateMode);
   DDX_Radio(IDC_OGGV_RADIO_BRMODE1, value, DDX_SAVE);

   if (value == 3)
   {
      // for constant bitrate, we "mirror" the nominal bitrate value to the
      // min and max field, too
      int nombr = GetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE, NULL, FALSE);
      SetDlgItemInt(IDC_OGGV_EDIT_MIN_BITRATE, nombr, FALSE);
      SetDlgItemInt(IDC_OGGV_EDIT_MAX_BITRATE, nombr, FALSE);
   }
   return 0;
}

LRESULT UI::OggVorbisSettingsPage::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
   // check if the vbr quality slider was moved
   if ((HWND)lParam == GetDlgItem(IDC_OGGV_SLIDER_QUALITY))
      UpdateQuality();
   else
      if ((HWND)lParam == GetDlgItem(IDC_OGGV_SPIN_QUICK_QUALITY))
         OnQuickQualitySpin(HIWORD(wParam), LOWORD(wParam));

   return 0;
}

void UI::OggVorbisSettingsPage::UpdateBitrateMode(int pos, bool init)
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   int minbr, nombr, maxbr;
   minbr = GetDlgItemInt(IDC_OGGV_EDIT_MIN_BITRATE, NULL, FALSE);
   nombr = GetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE, NULL, FALSE);
   maxbr = GetDlgItemInt(IDC_OGGV_EDIT_MAX_BITRATE, NULL, FALSE);

   // store old values
   if (!init)
      switch (mgr.queryValueInt(OggBitrateMode))
   {
      case 1: // variable bitrate
         mgr.setValue(OggVarMinBitrate, minbr);
         mgr.setValue(OggVarMaxBitrate, maxbr);
         break;
      case 2: // average bitrate
         mgr.setValue(OggVarNominalBitrate, nombr);
         break;
      case 3: // constant bitrate
         mgr.setValue(OggVarNominalBitrate, nombr);
         break;
   }

   // enable or disable controls
   BOOL enableQuality = pos == 0 ? TRUE : FALSE;
   BOOL enableMin = FALSE;
   BOOL enableNom = FALSE;
   BOOL enableMax = FALSE;

   if (pos != -1)
      mgr.setValue(OggBitrateMode, pos);

   switch (pos)
   {
   case 0: // quality settings
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE), _T(""));
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE), _T(""));
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE), _T(""));
      break;

   case 1: // variable bitrate
      SetDlgItemInt(IDC_OGGV_EDIT_MIN_BITRATE, mgr.queryValueInt(OggVarMinBitrate), FALSE);
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE), _T(""));
      SetDlgItemInt(IDC_OGGV_EDIT_MAX_BITRATE, mgr.queryValueInt(OggVarMaxBitrate), FALSE);

      enableMin = TRUE;
      enableMax = TRUE;
      break;

   case 2: // average bitrate
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE), _T(""));
      SetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE, mgr.queryValueInt(OggVarNominalBitrate), FALSE);
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE), _T(""));

      enableNom = TRUE;
      break;

   case 3: // constant bitrate
      SetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE, mgr.queryValueInt(OggVarNominalBitrate), FALSE);

      enableNom = TRUE;
      break;
   }

   // quality settings controls
   m_sliderQuality.EnableWindow(enableQuality);
   GetDlgItem(IDC_OGGV_STATIC_QUALITY).EnableWindow(enableQuality);
   GetDlgItem(IDC_OGGV_STATIC_BITRATE).EnableWindow(enableQuality);
   GetDlgItem(IDC_OGGV_STATIC1).EnableWindow(enableQuality);
   GetDlgItem(IDC_OGGV_STATIC2).EnableWindow(enableQuality);

   // min bitrate controls
   GetDlgItem(IDC_OGGV_STATIC3).EnableWindow(enableMin);
   GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE).EnableWindow(enableMin);
   m_spinBitrateMin.EnableWindow(enableMin);

   // nominal bitrate controls
   GetDlgItem(IDC_OGGV_STATIC4).EnableWindow(enableNom);
   GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE).EnableWindow(enableNom);
   m_spinBitrateNominal.EnableWindow(enableNom);

   // max bitrate controls
   GetDlgItem(IDC_OGGV_STATIC5).EnableWindow(enableMax);
   GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE).EnableWindow(enableMax);
   m_spinBitrateMax.EnableWindow(enableMax);
}

void UI::OggVorbisSettingsPage::UpdateQuality()
{
   // update slider quality text
   int pos = m_sliderQuality.GetPos();

   CString text;
   text.Format(IDS_OGGV_QUALITY, pos < 0 ? "-" : "",
      unsigned(fabs(pos / 100.0)), unsigned(fabs(fmod(pos, 100.0))));
   SetDlgItemText(IDC_OGGV_STATIC_QUALITY, text);

   float brate = OggVorbisCalculateBitrate(pos / 100.f);

   text.Format(IDS_OGGV_BITRATE, int(brate), int(brate*10.f) % 10);
   SetDlgItemText(IDC_OGGV_STATIC_BITRATE, text);
}

void UI::OggVorbisSettingsPage::OnQuickQualitySpin(WORD wCount, WORD wType)
{
   if (wType == SB_THUMBPOSITION)
   {
      int iDelta = 10;
      int pos = m_sliderQuality.GetPos();
      int iRest = int(fmod(pos, double(iDelta)));

      // we recognize "up" as changing updown control to 2, and
      // "down" as changing to 0.
      bool bUp = (wCount == 2);
      if (bUp)
         pos += iDelta - iRest;
      else
         pos -= iRest == 0 ? iDelta : iRest;

      m_sliderQuality.SetPos(pos);

      UpdateQuality();
   }

   // reset position to 1
   m_spinQuickQuality.SetPos(1);
}

void UI::OggVorbisSettingsPage::LoadData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // bitrate mode radio buttons
   int value = mgr.queryValueInt(OggBitrateMode);
   DDX_Radio(IDC_OGGV_RADIO_BRMODE1, value, DDX_LOAD);
   UpdateBitrateMode(value, true);

   // base quality slider
   value = mgr.queryValueInt(OggBaseQuality);
   m_sliderQuality.SetPos(value);
   UpdateQuality();
}

void UI::OggVorbisSettingsPage::SaveData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   int value = 0;
   DDX_Radio(IDC_OGGV_RADIO_BRMODE1, value, DDX_SAVE);
   mgr.setValue(OggBitrateMode, value);

   // base quality slider
   value = m_sliderQuality.GetPos();
   mgr.setValue(OggBaseQuality, value);

   // update bitrate values
   UpdateBitrateMode(-1);
}
