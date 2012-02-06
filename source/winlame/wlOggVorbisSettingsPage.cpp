/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2006 Michael Fink

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

   $Id: wlOggVorbisSettingsPage.cpp,v 1.19 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlOggVorbisSettingsPage.cpp

   \brief contains implementation of the ogg vorbis settings page

*/

// needed includes
#include "stdafx.h"
#include "wlOggVorbisSettingsPage.h"
#include <cmath>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// arrays

//! possible bitrates for the fixed value spin button ctrl
static int wlOggVorbisBitrates[] = 
{
   64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500
};


// global functions

//! calculates approximate bitrate for given quality value
/*! this function is Copyright (c) 2002 John Edwards
    see source code for OggDropXPd.
    input: quality value, range 0.f ... 10.f
    output: estimated bitrate in kbps
*/
float wlOggVorbisCalculateBitrate(float quality)
{
   float bitrate;

   if (quality < 4.10)
   {
      bitrate = quality*16 + 64;
   }
   else if (quality < 8.10)
   {
      bitrate = quality*32;
   }
   else if (quality < 9.10)
   {
      bitrate = quality*32 + (quality-8.f)*32;
   }
   else
   {
      bitrate = quality*32 + (quality-8.f)*32 + (quality-9.f)*116;
   }

   return bitrate;
}


// wlOggVorbisSettingsPage methods

LRESULT wlOggVorbisSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_OGGV_BEVEL1));
   bevel2.SubclassWindow(GetDlgItem(IDC_OGGV_BEVEL2));

   // set up range of slider control
   SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY, TBM_SETRANGEMIN, FALSE, -100);
   SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY, TBM_SETRANGEMAX, FALSE, 1000);
   SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY, TBM_SETTICFREQ, 100);

   // subclass spin button controls
   bitrateMinSpin.SubclassWindow(GetDlgItem(IDC_OGGV_SPIN_MIN_BITRATE));
   bitrateMinSpin.SetBuddy(GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE));
   bitrateMinSpin.SetFixedValues(wlOggVorbisBitrates, sizeof(wlOggVorbisBitrates)/sizeof(wlOggVorbisBitrates[0]));

   bitrateNominalSpin.SubclassWindow(GetDlgItem(IDC_OGGV_SPIN_NOM_BITRATE));
   bitrateNominalSpin.SetBuddy(GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE));
   bitrateNominalSpin.SetFixedValues(wlOggVorbisBitrates, sizeof(wlOggVorbisBitrates)/sizeof(wlOggVorbisBitrates[0]));

   bitrateMaxSpin.SubclassWindow(GetDlgItem(IDC_OGGV_SPIN_MAX_BITRATE));
   bitrateMaxSpin.SetBuddy(GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE));
   bitrateMaxSpin.SetFixedValues(wlOggVorbisBitrates, sizeof(wlOggVorbisBitrates)/sizeof(wlOggVorbisBitrates[0]));

   // set up range of slider control
   CUpDownCtrl ud;
   ud.Attach(GetDlgItem(IDC_OGGV_SPIN_QUICK_QUALITY));
   ud.SetRange(0, 2);
   ud.SetPos(1);
   ud.Detach();

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT wlOggVorbisSettingsPage::OnChangeEditNominalBitrate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // get value of radio button
   int value = pui->getUISettings().settings_manager.queryValueInt(wlOggBitrateMode);
   DDX_Radio(IDC_OGGV_RADIO_BRMODE1,value,DDX_SAVE);

   if (value==3)
   {
      // for constant bitrate, we "mirror" the nominal bitrate value to the
      // min and max field, too
      int nombr = GetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE,NULL,FALSE);
      SetDlgItemInt(IDC_OGGV_EDIT_MIN_BITRATE,nombr,FALSE);
      SetDlgItemInt(IDC_OGGV_EDIT_MAX_BITRATE,nombr,FALSE);
   }
   return 0;
}

void wlOggVorbisSettingsPage::UpdateBitrateMode(int pos,bool init)
{
   // get settings manager
   wlSettingsManager &mgr = pui->getUISettings().settings_manager;

   int minbr,nombr,maxbr;
   minbr = GetDlgItemInt(IDC_OGGV_EDIT_MIN_BITRATE,NULL,FALSE);
   nombr = GetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE,NULL,FALSE);
   maxbr = GetDlgItemInt(IDC_OGGV_EDIT_MAX_BITRATE,NULL,FALSE);

   // store old values
   if (!init)
   switch(mgr.queryValueInt(wlOggBitrateMode))
   {
   case 1: // variable bitrate
      mgr.setValue(wlOggVarMinBitrate,minbr);
      mgr.setValue(wlOggVarMaxBitrate,maxbr);
      break;
   case 2: // average bitrate
      mgr.setValue(wlOggVarNominalBitrate,nombr);
      break;
   case 3: // constant bitrate
      mgr.setValue(wlOggVarNominalBitrate,nombr);
      break;
   }

   // enable or disable controls
   BOOL enableQuality = pos==0 ? TRUE : FALSE;
   BOOL enableMin = FALSE;
   BOOL enableNom = FALSE;
   BOOL enableMax = FALSE;

   if (pos != -1)
      mgr.setValue(wlOggBitrateMode,pos);

   switch(pos)
   {
   case 0: // quality settings
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE),_T(""));
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE),_T(""));
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE),_T(""));
      break;

   case 1: // variable bitrate
      SetDlgItemInt(IDC_OGGV_EDIT_MIN_BITRATE,mgr.queryValueInt(wlOggVarMinBitrate),FALSE);
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE),_T(""));
      SetDlgItemInt(IDC_OGGV_EDIT_MAX_BITRATE,mgr.queryValueInt(wlOggVarMaxBitrate),FALSE);

      enableMin = TRUE;
      enableMax = TRUE;
      break;

   case 2: // average bitrate
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE),_T(""));
      SetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE,mgr.queryValueInt(wlOggVarNominalBitrate),FALSE);
      ::SetWindowText(GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE),_T(""));

      enableNom = TRUE;
      break;

   case 3: // constant bitrate
      SetDlgItemInt(IDC_OGGV_EDIT_NOM_BITRATE,mgr.queryValueInt(wlOggVarNominalBitrate),FALSE);

      enableNom = TRUE;
      break;
   }

   // quality settings controls
   ::EnableWindow(GetDlgItem(IDC_OGGV_SLIDER_QUALITY),enableQuality);
   ::EnableWindow(GetDlgItem(IDC_OGGV_STATIC_QUALITY),enableQuality);
   ::EnableWindow(GetDlgItem(IDC_OGGV_STATIC_BITRATE),enableQuality);
   ::EnableWindow(GetDlgItem(IDC_OGGV_STATIC1),enableQuality);
   ::EnableWindow(GetDlgItem(IDC_OGGV_STATIC2),enableQuality);

   // min bitrate controls
   ::EnableWindow(GetDlgItem(IDC_OGGV_STATIC3),enableMin);
   ::EnableWindow(GetDlgItem(IDC_OGGV_EDIT_MIN_BITRATE),enableMin);
   ::EnableWindow(GetDlgItem(IDC_OGGV_SPIN_MIN_BITRATE),enableMin);

   // nominal bitrate controls
   ::EnableWindow(GetDlgItem(IDC_OGGV_STATIC4),enableNom);
   ::EnableWindow(GetDlgItem(IDC_OGGV_EDIT_NOM_BITRATE),enableNom);
   ::EnableWindow(GetDlgItem(IDC_OGGV_SPIN_NOM_BITRATE),enableNom);

   // max bitrate controls
   ::EnableWindow(GetDlgItem(IDC_OGGV_STATIC5),enableMax);
   ::EnableWindow(GetDlgItem(IDC_OGGV_EDIT_MAX_BITRATE),enableMax);
   ::EnableWindow(GetDlgItem(IDC_OGGV_SPIN_MAX_BITRATE),enableMax);
}

void wlOggVorbisSettingsPage::UpdateQuality()
{
   // update slider quality text
   int pos = SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY,TBM_GETPOS);

   CString text;
   text.Format(IDS_OGGV_QUALITY,pos < 0 ? "-" : "",
      unsigned(fabs(pos/100.0)), unsigned(fabs(fmod(pos,100.0))));
   SetDlgItemText(IDC_OGGV_STATIC_QUALITY,text);

   float brate = wlOggVorbisCalculateBitrate(pos/100.f);

   text.Format(IDS_OGGV_BITRATE,int(brate),int(brate*10.f)%10);
   SetDlgItemText(IDC_OGGV_STATIC_BITRATE,text);
}

void wlOggVorbisSettingsPage::OnQuickQualitySpin(WORD wCount, WORD wType)
{
   if (wType == SB_THUMBPOSITION)
   {
      int iDelta = 10;
      int pos = SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY,TBM_GETPOS);
      int iRest = int(fmod(pos, double(iDelta)));

      // we recognize "up" as changing updown control to 2, and
      // "down" as changing to 0.
      bool bUp = (wCount == 2);
      if (bUp)
         pos += iDelta - iRest;
      else
         pos -= iRest == 0 ? iDelta : iRest;

      SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY,TBM_SETPOS,TRUE,(LONG)pos);

      UpdateQuality();
   }

   // reset position to 1
   CUpDownCtrl ud(GetDlgItem(IDC_OGGV_SPIN_QUICK_QUALITY));
   ud.SetPos(1);
   ud.Detach();
}

void wlOggVorbisSettingsPage::OnEnterPage()
{
   // get settings manager
   wlSettingsManager &mgr = pui->getUISettings().settings_manager;

   // bitrate mode radio buttons
   int value = mgr.queryValueInt(wlOggBitrateMode);
   DDX_Radio(IDC_OGGV_RADIO_BRMODE1,value,DDX_LOAD);
   UpdateBitrateMode(value,true);

   // base quality slider
   value = mgr.queryValueInt(wlOggBaseQuality);
   SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY,TBM_SETPOS,TRUE,(LONG)value);
   UpdateQuality();
}

bool wlOggVorbisSettingsPage::OnLeavePage()
{
   // get settings manager
   wlSettingsManager &mgr = pui->getUISettings().settings_manager;

   int value=0;
   DDX_Radio(IDC_OGGV_RADIO_BRMODE1,value,DDX_SAVE);
   mgr.setValue(wlOggBitrateMode,value);

   // base quality slider
   value = SendDlgItemMessage(IDC_OGGV_SLIDER_QUALITY,TBM_GETPOS);
   mgr.setValue(wlOggBaseQuality,value);

   // update bitrate values
   UpdateBitrateMode(-1);

   return true;
}
