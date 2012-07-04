/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2012 Michael Fink

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
/// \file AboutDlg.h
/// \brief about dialog
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "resource.h"

/// about dialog
class AboutDlg:
   public CAxDialogImpl<AboutDlg>,
   public CDialogResize<AboutDlg>
{
public:
   /// ctor
   AboutDlg()
   {
   }

   /// sets filename of presets.xml file
   void SetPresetsXmlFilename(const CString& cszFilename){ m_cszPresetsXmlFilename = cszFilename; }

private:
   /// dialog id
   enum { IDD = IDD_ABOUTDLG };

   friend CAxDialogImpl<AboutDlg>;
   friend CDialogResize<AboutDlg>;

   // resize map
   BEGIN_DLGRESIZE_MAP(AboutDlg)
      DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_ABOUT_BROWSER, DLSZ_SIZE_X | DLSZ_SIZE_Y)
   END_DLGRESIZE_MAP()

   // message map
   BEGIN_MSG_MAP(AboutDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnExit)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnExit)
      CHAIN_MSG_MAP(CDialogResize<AboutDlg>)
   END_MSG_MAP()

   /// inits about dialog
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called on exiting the about dialog
   LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// retrieves the about box html text
   CString GetAboutHtmlText();

private:
   /// ActiveX host window
   CAxWindow m_browser;

   /// filename of presets.xml file
   CString m_cszPresetsXmlFilename;

   /// window icon
   HICON m_hDlgIcon;
};

/// @}
