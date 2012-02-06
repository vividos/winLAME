/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2004 Michael Fink

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

   $Id: wlWaveOutputSettingsPage.h,v 1.8 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlWaveOutputSettingsPage.h

   \brief contains the settings page for the wave output module

*/
/*! \ingroup userinterface */
/*! @{ */

// prevent multiple including
#ifndef __wlwaveoutputsettingspage_h_
#define __wlwaveoutputsettingspage_h_

// needed includes
#include "resource.h"
#include "wlPageBase.h"
#include "wlCommonStuff.h"


//! wave output settings page class

class wlWaveOutputSettingsPage:
   public wlPageBase,
   public CDialogResize<wlWaveOutputSettingsPage>
{
public:
   //! ctor
   wlWaveOutputSettingsPage()
   {
      IDD = IDD_DLG_WAVE;
      captionID = IDS_DLG_CAP_WAVE;
      descID = IDS_DLG_DESC_WAVE;
      helpID = IDS_HTML_WAVE;
   }

   // resize map
BEGIN_DLGRESIZE_MAP(wlWaveOutputSettingsPage)
   DLGRESIZE_CONTROL(IDC_WAVE_COMBO_FILEFMT, DLSZ_SIZE_X)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(wlWaveOutputSettingsPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   CHAIN_MSG_MAP(CDialogResize<wlWaveOutputSettingsPage>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   //! inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   // virtual functions from wlPageBase

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

protected:
   //! bevel line
   wlBevelLine bevel1;
};


//@}

#endif
