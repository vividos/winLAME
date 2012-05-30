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

*/
/// \file WmaOutputSettingsPage.cpp
/// \brief contains implementation of the Wma output settings page

// needed includes
#include "stdafx.h"
#include "WmaOutputSettingsPage.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/// suggested bitrate values
static int WmaBitrates[] =
{
   32,48,64,80,96,128,160,192,256,320
};

static int WmaVBRQuality[] =
{
   10,25,50,75,90,98,100
};

// WmaOutputSettingsPage methods

LRESULT WmaOutputSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_WMA_BEVEL1));

   bitrateSpin.SubclassWindow(GetDlgItem(IDC_WMA_SPIN_BITRATE));
   bitrateSpin.SetBuddy(GetDlgItem(IDC_WMA_EDIT_BITRATE));
   bitrateSpin.SetFixedValues(WmaBitrates, sizeof(WmaBitrates)/sizeof(WmaBitrates[0]));

   qualitySpin.SubclassWindow(GetDlgItem(IDC_WMA_SPIN_QUALITY));
   qualitySpin.SetBuddy(GetDlgItem(IDC_WMA_EDIT_QUALITY));
   qualitySpin.SetFixedValues(WmaVBRQuality, sizeof(WmaVBRQuality)/sizeof(WmaVBRQuality[0]));

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

void WmaOutputSettingsPage::OnEnterPage()
{
   // get settings manager
   SettingsManager &mgr = pui->getUISettings().settings_manager;
   
   // set output bitrate
   SetDlgItemInt(IDC_WMA_EDIT_BITRATE,mgr.queryValueInt(WmaBitrate),FALSE);
   SetDlgItemInt(IDC_WMA_EDIT_QUALITY,mgr.queryValueInt(WmaQuality),FALSE);

   // bitrate mode radio buttons
   int value = mgr.queryValueInt(WmaBitrateMode);
   DDX_Radio(IDC_WMA_RADIO_BRMODE1,value,DDX_LOAD);
}

bool WmaOutputSettingsPage::OnLeavePage()
{
   // get settings manager
   SettingsManager &mgr = pui->getUISettings().settings_manager;

   mgr.setValue(WmaBitrate,(int)GetDlgItemInt(IDC_WMA_EDIT_BITRATE,NULL,FALSE));
   mgr.setValue(WmaQuality,(int)GetDlgItemInt(IDC_WMA_EDIT_QUALITY,NULL,FALSE));

   int value=0;
   DDX_Radio(IDC_WMA_RADIO_BRMODE1,value,DDX_SAVE);
   mgr.setValue(WmaBitrateMode,value);

   return true;
}
