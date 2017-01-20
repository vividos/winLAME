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
/// \file OpusSettingsPage_old.cpp
/// \brief contains implementation of the Opus settings page
//
#include "stdafx.h"
#include "OpusSettingsPage.hpp"
#include "CommonStuff.hpp"

using ClassicUI::OpusSettingsPage;

// arrays and mappings

/// possible bitrate values
static int OpusBitrates[] =
{
   6, 8, 16, 32, 48, 64, 96, 128, 144, 160, 176, 192, 208, 224, 240, 256
};

// OpusSettingsPage methods

LRESULT OpusSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, true);

   // spin button controls
   m_spinBitrate.SetBuddy(GetDlgItem(IDC_OPUS_EDIT_BITRATE));
   m_spinBitrate.SetFixedValues(OpusBitrates, sizeof(OpusBitrates) / sizeof(OpusBitrates[0]));

   // set up range of slider control
   m_sliderComplexity.SetRangeMin(0);
   m_sliderComplexity.SetRangeMax(10);
   m_sliderComplexity.SetTicFreq(1);

   return 1;  // let the system set the focus
}

void OpusSettingsPage::UpdateComplexity()
{
   // update slider quality text
   int pos = m_sliderComplexity.GetPos();

   CString text;
   text.Format(_T("%i"), pos);
   SetDlgItemText(IDC_OPUS_STATIC_COMPLEXITY, text);
}

void OpusSettingsPage::OnEnterPage()
{
   // get settings manager
   SettingsManager& mgr = pui->getUISettings().settings_manager;

   // set target bitrate
   int value = mgr.queryValueInt(OpusTargetBitrate);
   if (value < 6 || value > 256)
      value = 256;

   SetDlgItemInt(IDC_OPUS_EDIT_BITRATE, value, FALSE);

   // complexity slider
   value = mgr.queryValueInt(OpusComplexity);
   if (value < 0 || value > 10)
      value = 10;

   m_sliderComplexity.SetPos(value);

   UpdateComplexity();

   // bitrate mode
   value = mgr.queryValueInt(OpusBitrateMode);
   if (value < 0 || value > 2)
      value = 0;

   DDX_Radio(IDC_OPUS_RADIO_BRCMODE1, value, DDX_LOAD);
}

bool OpusSettingsPage::OnLeavePage()
{
   // get settings manager
   SettingsManager& mgr = pui->getUISettings().settings_manager;

   DoDataExchange(DDX_SAVE);

   // get bitrate
   int value = (int)GetDlgItemInt(IDC_OPUS_EDIT_BITRATE, NULL, FALSE);
   if (value < 6 || value > 256)
   {
      AppMessageBox(m_hWnd, IDS_OPUS_INVALID_BITRATE, MB_OK | MB_ICONEXCLAMATION);
      return false;
   }

   mgr.setValue(OpusTargetBitrate, value);

   // complexity slider
   value = m_sliderComplexity.GetPos();
   mgr.setValue(OpusComplexity, value);

   // bitrate mode
   value = 0;
   DDX_Radio(IDC_OPUS_RADIO_BRCMODE1, value, DDX_SAVE);
   mgr.setValue(OpusBitrateMode, value);

   return true;
}
