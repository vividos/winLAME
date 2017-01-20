/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2016 Michael Fink

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
/// \file OpusSettingsPage.hpp
/// \brief contains the settings page for the Opus encoder
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "resource.h"
#include "PageBase.hpp"
#include "FixedValueSpinButtonCtrl.hpp"

// classes

/// settings page for Opus settings
class OpusSettingsPage:
   public PageBase,
   public CDialogResize<OpusSettingsPage>
{
public:
   /// ctor
   OpusSettingsPage()
   {
      IDD = IDD_DLG_OPUS;
      captionID = IDS_DLG_CAP_OPUS;
      descID = IDS_DLG_DESC_OPUS;
      helpID = IDS_HTML_OPUS;
   }

   BEGIN_DDX_MAP(OpusSettingsPage)
      DDX_CONTROL(IDC_OPUS_SPIN_BITRATE, m_spinBitrate)
      DDX_CONTROL_HANDLE(IDC_OPUS_SLIDER_COMPLEXITY, m_sliderComplexity)
   END_DDX_MAP()

   // resize map
   BEGIN_DLGRESIZE_MAP(OpusSettingsPage)
   END_DLGRESIZE_MAP()

   // message map
   BEGIN_MSG_MAP(OpusSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
      CHAIN_MSG_MAP(CDialogResize<OpusSettingsPage>)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()
   // Handler prototypes:
   //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when slider is moved
   LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // check if the vbr quality slider was moved
      if ((HWND)lParam == GetDlgItem(IDC_OPUS_SLIDER_COMPLEXITY))
         UpdateComplexity();
      return 0;
   }

   /// updates complexity value
   void UpdateComplexity();

   // virtual functions from PageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   /// bitrate spin button control
   UI::FixedValueSpinButtonCtrl m_spinBitrate;

   /// complexity slider
   CTrackBarCtrl m_sliderComplexity;
};


/// @}
