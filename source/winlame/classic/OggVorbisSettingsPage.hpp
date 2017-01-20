//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2006 Michael Fink
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
/// \file OggVorbisSettingsPage.hpp
/// \brief contains the settings page for the ogg vorbis encoder
//
#pragma once

#include "resource.h"
#include "PageBase.hpp"
#include "CommonStuff.hpp"

/// ogg vorbis settings page class
class OggVorbisSettingsPage:
   public PageBase,
   public CDialogResize<OggVorbisSettingsPage>
{
public:
   /// ctor
   OggVorbisSettingsPage()
   {
      IDD = IDD_DLG_OGGVORBIS;
      captionID = IDS_DLG_CAP_OGGVORBIS;
      descID = IDS_DLG_DESC_OGGVORBIS;
      helpID = IDS_HTML_OGGVORBIS;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(OggVorbisSettingsPage)
   DLGRESIZE_CONTROL(IDC_OGGV_BEVEL1, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_OGGV_SLIDER_QUALITY, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_OGGV_STATIC_QUALITY, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_OGGV_STATIC_BITRATE, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_OGGV_STATIC2, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_OGGV_BEVEL2, DLSZ_SIZE_X)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(OggVorbisSettingsPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog_)
   COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE1, BN_CLICKED, OnSetBitrateMode)
   COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE2, BN_CLICKED, OnSetBitrateMode)
   COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE3, BN_CLICKED, OnSetBitrateMode)
   COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE4, BN_CLICKED, OnSetBitrateMode)
   COMMAND_HANDLER(IDC_OGGV_EDIT_NOM_BITRATE, EN_CHANGE, OnChangeEditNominalBitrate)
   MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
   CHAIN_MSG_MAP(CDialogResize<OggVorbisSettingsPage>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog_(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when the bitrate mode changes
   LRESULT OnSetBitrateMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      // look which radio button was activated
      int pos;
      DDX_Radio(IDC_OGGV_RADIO_BRMODE1,pos,DDX_SAVE);
      UpdateBitrateMode(pos);

      return 0;
   }

   /// called when the text of the "nominal bitrate" edit control changes
   LRESULT OnChangeEditNominalBitrate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when slider is moved
   LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // check if the vbr quality slider was moved
      if ((HWND)lParam == GetDlgItem(IDC_OGGV_SLIDER_QUALITY))
         UpdateQuality();
      else
         if ((HWND)lParam == GetDlgItem(IDC_OGGV_SPIN_QUICK_QUALITY))
            OnQuickQualitySpin(HIWORD(wParam), LOWORD(wParam));
      return 0;
   }

   /// updates controls when bitrate mode changes
   void UpdateBitrateMode(int pos,bool init=false);

   /// updates quality value
   void UpdateQuality();

   /// user clicked on quick quality spin control
   void OnQuickQualitySpin(WORD wCount, WORD wType);

   // virtual functions from PageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   FixedValueSpinButtonCtrl bitrateNominalSpin;  ///< nominal bitrate spin button control
   FixedValueSpinButtonCtrl bitrateMinSpin;      ///< min bitrate spin button control
   FixedValueSpinButtonCtrl bitrateMaxSpin;      ///< max bitrate spin button control
   BevelLine bevel1; ///< bevel line
   BevelLine bevel2; ///< bevel line
};
