//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2005 Michael Fink
// Copyright (c) 2004 DeXT
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
/// \file AacSettingsPageClassic.cpp
/// \brief contains implementation of the AAC settings page
//
#include "stdafx.h"
#include "AacSettingsPageClassic.hpp"

using ClassicUI::AacSettingsPage;

// arrays and mappings

/// possible bitrate values
static int AacBitrates[] =
{
   //   24000,32000,48000,56000,64000,96000,112000,128000,160000,192000
      24,32,48,56,64,96,112,128,160,192,224,256,320,384,448,576
};

/// possible bandwidth frequency values
int AacBandwidthValues[] =
{
   8000, 11025, 16000, 18000, 19500, 22050, 24000, 32000, 44100, 48000
};


// AacSettingsPage methods

LRESULT AacSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_AAC_BEVEL1));
   bevel2.SubclassWindow(GetDlgItem(IDC_AAC_BEVEL2));

   // subclass spin button controls
   bitrateSpin.SubclassWindow(GetDlgItem(IDC_AAC_SPIN_BITRATE));
   bitrateSpin.SetBuddy(GetDlgItem(IDC_AAC_EDIT_BITRATE));
   bitrateSpin.SetFixedValues(AacBitrates, sizeof(AacBitrates) / sizeof(AacBitrates[0]));

   bandwidthSpin.SubclassWindow(GetDlgItem(IDC_AAC_SPIN_BANDWIDTH));
   bandwidthSpin.SetBuddy(GetDlgItem(IDC_AAC_EDIT_BANDWIDTH));
   bandwidthSpin.SetFixedValues(AacBandwidthValues, sizeof(AacBandwidthValues) / sizeof(AacBandwidthValues[0]));

   // set up range of slider control
   SendDlgItemMessage(IDC_AAC_SLIDER_QUALITY, TBM_SETRANGEMIN, FALSE, 50);
   SendDlgItemMessage(IDC_AAC_SLIDER_QUALITY, TBM_SETRANGEMAX, FALSE, 250);
   SendDlgItemMessage(IDC_AAC_SLIDER_QUALITY, TBM_SETTICFREQ, 10);

   // combo box with mpeg versions
   SendDlgItemMessage(IDC_AAC_COMBO_MPEGVER, CB_ADDSTRING, 0, (LPARAM)_T("MPEG2"));
   SendDlgItemMessage(IDC_AAC_COMBO_MPEGVER, CB_ADDSTRING, 0, (LPARAM)_T("MPEG4"));

   // combo box with aac object type
   SendDlgItemMessage(IDC_AAC_COMBO_OBJTYPE, CB_ADDSTRING, 0, (LPARAM)_T("Main"));
   SendDlgItemMessage(IDC_AAC_COMBO_OBJTYPE, CB_ADDSTRING, 0, (LPARAM)_T("Low Complexity"));
   SendDlgItemMessage(IDC_AAC_COMBO_OBJTYPE, CB_ADDSTRING, 0, (LPARAM)_T("LTP"));

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

void AacSettingsPage::UpdateQuality()
{
   // update slider quality text
   int pos = SendDlgItemMessage(IDC_AAC_SLIDER_QUALITY, TBM_GETPOS);

   CString text;
   text.Format(IDS_AAC_QUALITY_U, pos);
   SetDlgItemText(IDC_AAC_STATIC_QUALITY, text);

   int kbps = ((pos * 2 / 5) + 85) / 2;

   text.Format(IDS_AAC_KBPS_PER_CH_U, kbps);
   SetDlgItemText(IDC_AAC_STATIC_KBPS, text);
}

void AacSettingsPage::UpdateBandwidth()
{
   // check if bandwidth button is checked
   if (BST_CHECKED == SendDlgItemMessage(IDC_AAC_CHECK_BANDWIDTH, BM_GETCHECK))
   {
      ::EnableWindow(GetDlgItem(IDC_AAC_EDIT_BANDWIDTH), FALSE);
      ::EnableWindow(GetDlgItem(IDC_AAC_SPIN_BANDWIDTH), FALSE);
   }
   else
   {
      ::EnableWindow(GetDlgItem(IDC_AAC_EDIT_BANDWIDTH), TRUE);
      ::EnableWindow(GetDlgItem(IDC_AAC_SPIN_BANDWIDTH), TRUE);
   }
}

void AacSettingsPage::OnEnterPage()
{
   // get settings manager
   SettingsManager& mgr = pui->getUISettings().settings_manager;

   // set bitrate and bandwidth
   SetDlgItemInt(IDC_AAC_EDIT_BITRATE, mgr.queryValueInt(AacBitrate), FALSE);
   SetDlgItemInt(IDC_AAC_EDIT_BANDWIDTH, mgr.queryValueInt(AacBandwidth), FALSE);

   // set combo box selections
   SendDlgItemMessage(IDC_AAC_COMBO_MPEGVER, CB_SETCURSEL,
      mgr.queryValueInt(AacMpegVersion) == 2 ? 0 : 1);

   SendDlgItemMessage(IDC_AAC_COMBO_OBJTYPE, CB_SETCURSEL, mgr.queryValueInt(AacObjectType));

   // set checks
   SendDlgItemMessage(IDC_AAC_CHECK_MIDSIDE, BM_SETCHECK,
      mgr.queryValueInt(AacAllowMS) == 0 ? BST_UNCHECKED : BST_CHECKED);

   SendDlgItemMessage(IDC_AAC_CHECK_USETNS, BM_SETCHECK,
      mgr.queryValueInt(AacUseTNS) == 0 ? BST_UNCHECKED : BST_CHECKED);

   SendDlgItemMessage(IDC_AAC_CHECK_USELFE, BM_SETCHECK,
      mgr.queryValueInt(AacUseLFEChan) == 0 ? BST_UNCHECKED : BST_CHECKED);

   SendDlgItemMessage(IDC_AAC_CHECK_BANDWIDTH, BM_SETCHECK,
      mgr.queryValueInt(AacAutoBandwidth) == 0 ? BST_UNCHECKED : BST_CHECKED);
   UpdateBandwidth();

   // bitrate control
   int value = mgr.queryValueInt(AacBRCMethod);
   DDX_Radio(IDC_AAC_RADIO_BRCMODE1, value, DDX_LOAD);

   // quality slider
   value = mgr.queryValueInt(AacQuality);
   SendDlgItemMessage(IDC_AAC_SLIDER_QUALITY, TBM_SETPOS, TRUE, (LONG)value);
   UpdateQuality();
}

bool AacSettingsPage::OnLeavePage()
{
   // get settings manager
   SettingsManager& mgr = pui->getUISettings().settings_manager;

   // get bitrate and bandwidth
   mgr.setValue(AacBitrate, (int)GetDlgItemInt(IDC_AAC_EDIT_BITRATE, NULL, FALSE));
   mgr.setValue(AacBandwidth, (int)GetDlgItemInt(IDC_AAC_EDIT_BANDWIDTH, NULL, FALSE));

   // get combo box selections
   int mpeg = SendDlgItemMessage(IDC_AAC_COMBO_MPEGVER, CB_GETCURSEL);
   mgr.setValue(AacMpegVersion, mpeg == 0 ? 2 : 4);

   int value = SendDlgItemMessage(IDC_AAC_COMBO_OBJTYPE, CB_GETCURSEL);
   mgr.setValue(AacObjectType, value);

   // currently it is not possible to use LTP together with MPEG2
   if (mpeg == 0 && value == 2)
   {
      // give user a message explaining that fact
      AppMessageBox(m_hWnd, IDS_AAC_NO_MPEG2_LTP, MB_OK | MB_ICONEXCLAMATION);
      return false;
   }

   // get checks
   value = SendDlgItemMessage(IDC_AAC_CHECK_MIDSIDE, BM_GETCHECK) == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacAllowMS, value);

   value = SendDlgItemMessage(IDC_AAC_CHECK_USETNS, BM_GETCHECK) == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacUseTNS, value);

   value = SendDlgItemMessage(IDC_AAC_CHECK_USELFE, BM_GETCHECK) == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacUseLFEChan, value);

   value = SendDlgItemMessage(IDC_AAC_CHECK_BANDWIDTH, BM_GETCHECK) == BST_CHECKED ? 1 : 0;
   mgr.setValue(AacAutoBandwidth, value);

   // bitrate control
   DDX_Radio(IDC_AAC_RADIO_BRCMODE1, value, DDX_SAVE);
   mgr.setValue(AacBRCMethod, value);

   // quality slider
   value = SendDlgItemMessage(IDC_AAC_SLIDER_QUALITY, TBM_GETPOS);
   mgr.setValue(AacQuality, value);

   return true;
}
