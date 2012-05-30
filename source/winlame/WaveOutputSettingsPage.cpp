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

   $Id: WaveOutputSettingsPage.cpp,v 1.12 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file WaveOutputSettingsPage.cpp

   \brief contains implementation of the wave output settings page

*/

// needed includes
#include "stdafx.h"
#include "WaveOutputSettingsPage.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// WaveOutputSettingsPage methods

LRESULT WaveOutputSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_WAVE_BEVEL1));

   // fill output format list
   SendDlgItemMessage(IDC_WAVE_COMBO_OUTFMT, CB_ADDSTRING, 0, (LPARAM)_T("16 bit PCM"));
   SendDlgItemMessage(IDC_WAVE_COMBO_OUTFMT, CB_ADDSTRING, 0, (LPARAM)_T("24 bit PCM"));
   SendDlgItemMessage(IDC_WAVE_COMBO_OUTFMT, CB_ADDSTRING, 0, (LPARAM)_T("32 bit PCM"));
   SendDlgItemMessage(IDC_WAVE_COMBO_OUTFMT, CB_ADDSTRING, 0, (LPARAM)_T("32 bit Float"));

   // fill file format list
   SendDlgItemMessage(IDC_WAVE_COMBO_FILEFMT, CB_ADDSTRING, 0, (LPARAM)_T("Microsoft WAV"));
   SendDlgItemMessage(IDC_WAVE_COMBO_FILEFMT, CB_ADDSTRING, 0, (LPARAM)_T("Apple AIFF"));
   SendDlgItemMessage(IDC_WAVE_COMBO_FILEFMT, CB_ADDSTRING, 0, (LPARAM)_T("SoundForge W64"));

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

void WaveOutputSettingsPage::OnEnterPage()
{
   // get settings manager
   SettingsManager &mgr = pui->getUISettings().settings_manager;

   // set raw audio check
   SendDlgItemMessage(IDC_WAVE_CHECK_RAWAUDIO, BM_SETCHECK, 
      mgr.queryValueInt(WaveRawAudioFile)==1? BST_CHECKED : BST_UNCHECKED );

   // set writewavex check
   SendDlgItemMessage(IDC_WAVE_CHECK_WAVEX, BM_SETCHECK, 
      mgr.queryValueInt(WaveWriteWavEx)==1? BST_CHECKED : BST_UNCHECKED );

   // set output format
   SendDlgItemMessage(IDC_WAVE_COMBO_OUTFMT, CB_SETCURSEL, mgr.queryValueInt(WaveOutputFormat));
   SendDlgItemMessage(IDC_WAVE_COMBO_FILEFMT, CB_SETCURSEL, mgr.queryValueInt(WaveFileFormat));
}

bool WaveOutputSettingsPage::OnLeavePage()
{
   // get settings manager
   SettingsManager &mgr = pui->getUISettings().settings_manager;

   // get raw audio check
   mgr.setValue(WaveRawAudioFile,
      BST_CHECKED==SendDlgItemMessage(IDC_WAVE_CHECK_RAWAUDIO, BM_GETCHECK) ? 1 : 0 );

   // get writewavex check
   mgr.setValue(WaveWriteWavEx,
      BST_CHECKED==SendDlgItemMessage(IDC_WAVE_CHECK_WAVEX, BM_GETCHECK) ? 1 : 0 );

   // get output format
   int value = SendDlgItemMessage(IDC_WAVE_COMBO_OUTFMT, CB_GETCURSEL);
   mgr.setValue(WaveOutputFormat,value);
   value = SendDlgItemMessage(IDC_WAVE_COMBO_FILEFMT, CB_GETCURSEL);
   mgr.setValue(WaveFileFormat,value);

   return true;
}
