/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2005 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: wlLameSimpleSettingsPage.cpp,v 1.5 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlLameSimpleSettingsPage.cpp

   \brief contains implementation of the basic settings page

*/

// needed includes
#include "stdafx.h"
#include "wlLameSimpleSettingsPage.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// arrays and mappings

//! bitrate values
const int wlLameBitrates[] =
{
   8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 192, 224, 256, 320
};

//! quality values
const int wlLameQualities[] =
{
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};


// wlLameSimpleSettingsPage methods

LRESULT wlLameSimpleSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass spin button controls
   m_bitrateSpin.SubclassWindow(GetDlgItem(IDC_LAME_SPIN_BITRATE));
   m_bitrateSpin.SetBuddy(GetDlgItem(IDC_LAME_EDIT_BITRATE));
   m_bitrateSpin.SetFixedValues(wlLameBitrates, sizeof(wlLameBitrates)/sizeof(wlLameBitrates[0]));

   m_qualitySpin.SubclassWindow(GetDlgItem(IDC_LAME_SPIN_QUALITY));
   m_qualitySpin.SetBuddy(GetDlgItem(IDC_LAME_EDIT_QUALITY));
   m_qualitySpin.SetFixedValues(wlLameQualities, sizeof(wlLameQualities)/sizeof(wlLameQualities[0]));

   DDX_Control_Handle(IDC_LAME_COMBO_ENCODING_QUALITY, m_cbEncodingQuality, DDX_LOAD);
   DDX_Control_Handle(IDC_LAME_COMBO_VBR_MODE, m_cbVariableBitrateMode, DDX_LOAD);

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

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT wlLameSimpleSettingsPage::OnRadioEncodeType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   BOOL fEnableBitrate = TRUE;
   BOOL fEnableQuality = FALSE;

   if (wID == IDC_LAME_RADIO_TYPE2)
   {
      fEnableBitrate = FALSE;
      fEnableQuality = TRUE;
   }

   ::EnableWindow(GetDlgItem(IDC_LAME_CHECK_CBR), fEnableBitrate);
   ::EnableWindow(GetDlgItem(IDC_LAME_EDIT_BITRATE), fEnableBitrate);
   ::EnableWindow(GetDlgItem(IDC_LAME_SPIN_BITRATE), fEnableBitrate);

   ::EnableWindow(GetDlgItem(IDC_LAME_EDIT_QUALITY), fEnableQuality);
   ::EnableWindow(GetDlgItem(IDC_LAME_SPIN_QUALITY), fEnableQuality);
   ::EnableWindow(GetDlgItem(IDC_LAME_COMBO_VBR_MODE), fEnableQuality);
   ::EnableWindow(GetDlgItem(IDC_LAME_STATIC_VBR_MODE), fEnableQuality);

   return 0;
}

void wlLameSimpleSettingsPage::OnEnterPage()
{
   // get settings manager
   wlSettingsManager &mgr = pui->getUISettings().settings_manager;

   // encode quality
   int value = mgr.queryValueInt(wlLameSimpleEncodeQuality);
   m_cbEncodingQuality.SetCurSel(value);

   // radio buttons for quality or bitrate
   value = mgr.queryValueInt(wlLameSimpleQualityOrBitrate);
   DDX_Radio(IDC_LAME_RADIO_TYPE1, value, DDX_LOAD);
   BOOL bDummy = true;
   OnRadioEncodeType(0, value == 0 ? IDC_LAME_RADIO_TYPE1 : IDC_LAME_RADIO_TYPE2, NULL, bDummy);

   // Mono check
   SendDlgItemMessage(IDC_LAME_CHECK_MONO, BM_SETCHECK,
      mgr.queryValueInt(wlLameSimpleMono)==0 ? BST_UNCHECKED : BST_CHECKED);

   // bitrate
   value = mgr.queryValueInt(wlLameSimpleBitrate);
   SetDlgItemInt(IDC_LAME_EDIT_BITRATE, value, FALSE);

   // quality
   value = mgr.queryValueInt(wlLameSimpleQuality);
   SetDlgItemInt(IDC_LAME_EDIT_QUALITY, value, FALSE);

   // CBR check
   SendDlgItemMessage(IDC_LAME_CHECK_CBR, BM_SETCHECK,
      mgr.queryValueInt(wlLameSimpleCBR)==0 ? BST_UNCHECKED : BST_CHECKED);

   // VBR mode
   value = mgr.queryValueInt(wlLameSimpleVBRMode);
   m_cbVariableBitrateMode.SetCurSel(value);

   // "nogap" check
   SendDlgItemMessage(IDC_LAME_CHECK_NOGAP, BM_SETCHECK,
      mgr.queryValueInt(wlLameOptNoGap)==0 ? BST_UNCHECKED : BST_CHECKED);

   // "prepend RIFF WAVE Header" check
   SendDlgItemMessage(IDC_LAME_CHECK_WRITE_WAVEMP3, BM_SETCHECK,
      mgr.queryValueInt(wlLameWriteWaveHeader)==0 ? BST_UNCHECKED : BST_CHECKED);
}

bool wlLameSimpleSettingsPage::OnLeavePage()
{
   // get settings manager
   wlSettingsManager &mgr = pui->getUISettings().settings_manager;

   // encode quality
   mgr.setValue(wlLameSimpleEncodeQuality, m_cbEncodingQuality.GetCurSel());

   // radio buttons for quality or bitrate
   int value = 0;
   DDX_Radio(IDC_LAME_RADIO_TYPE1, value, DDX_SAVE);
   mgr.setValue(wlLameSimpleQualityOrBitrate, value);

   // Mono check
   value = SendDlgItemMessage(IDC_LAME_CHECK_MONO,BM_GETCHECK)==BST_CHECKED ? 1 : 0;
   mgr.setValue(wlLameSimpleMono,value);

   // bitrate
   mgr.setValue(wlLameSimpleBitrate,(int)GetDlgItemInt(IDC_LAME_EDIT_BITRATE,NULL,FALSE));

   // quality
   mgr.setValue(wlLameSimpleQuality,(int)GetDlgItemInt(IDC_LAME_EDIT_QUALITY,NULL,FALSE));

   // CBR check
   value = SendDlgItemMessage(IDC_LAME_CHECK_CBR,BM_GETCHECK)==BST_CHECKED ? 1 : 0;
   mgr.setValue(wlLameSimpleCBR,value);

   // VBR mode
   mgr.setValue(wlLameSimpleVBRMode, m_cbVariableBitrateMode.GetCurSel());

   // "nogap" check
   value = SendDlgItemMessage(IDC_LAME_CHECK_NOGAP,BM_GETCHECK)==BST_CHECKED ? 1 : 0;
   mgr.setValue(wlLameOptNoGap,value);

   // "prepend RIFF WAVE Header" check
   value = SendDlgItemMessage(IDC_LAME_CHECK_WRITE_WAVEMP3,BM_GETCHECK)==BST_CHECKED ? 1 : 0;
   mgr.setValue(wlLameWriteWaveHeader,value);

   // switch on simple mode
   mgr.setValue(wlLameSimpleMode,1);

   return true;
}
