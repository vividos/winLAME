/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2009 Michael Fink

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

   $Id: OptionsDlg.h,v 1.3 2009/12/17 19:11:52 vividos Exp $

*/
/*! \file OptionsDlg.h

   \brief options dialog

*/
#pragma once

#include "wlCommonStuff.h"
#include "wlUIinterface.h"
#include "ImageListComboBox.h"

class CLanguageResourceManager;

class COptionsDlg:
   public CDialogImpl<COptionsDlg>,
   public CWinDataExchange<COptionsDlg>
{
public:
   COptionsDlg(wlUISettings& uiSettings, CLanguageResourceManager& langResourceManager)
      :m_uiSettings(uiSettings),
       m_langResourceManager(langResourceManager)
   {
   }

   enum { IDD = IDD_CDRIP_OPTIONS };

// DLGRESIZE_CONTROL(IDC_CDRIP_OPT_COMBO_LANGUAGE, DLSZ_SIZE_X)
// DLGRESIZE_CONTROL(IDC_CDRIP_OPT_BEVEL1, DLSZ_SIZE_X)

   BEGIN_MSG_MAP(COptionsDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnExit)
      COMMAND_ID_HANDLER(IDCANCEL, OnExit)
      COMMAND_ID_HANDLER(IDC_CDRIP_OPT_BUTTON_TEMP_SELECTPATH, OnButtonSelectPath)
      COMMAND_HANDLER(IDC_CDRIP_OPT_EDIT_FREEDB_USERNAME, EN_CHANGE, OnChangeFreedbUsername)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

   BEGIN_DDX_MAP(COptionsDlg)
      DDX_CONTROL(IDC_CDRIP_OPT_COMBO_LANGUAGE, m_cbLanguages)
      DDX_CONTROL(IDC_CDRIP_OPT_BEVEL1, m_bevel1);
      DDX_CONTROL(IDC_CDRIP_OPT_BEVEL2, m_bevel2);
      DDX_CONTROL_HANDLE(IDC_CDRIP_OPT_BUTTON_TEMP_SELECTPATH, m_btnSelectPath);
      DDX_CONTROL_HANDLE(IDC_CDRIP_OPT_COMBO_FREEDB_SERVER, m_cbFreedbServer);
      DDX_CHECK(IDC_CDRIP_OPT_CHECK_CDPLAYER_INI, m_uiSettings.store_disc_infos_cdplayer_ini);
      DDX_TEXT(IDC_CDRIP_OPT_EDIT_TEMP_FOLDER, m_uiSettings.cdrip_temp_folder);
      DDX_TEXT(IDC_CDRIP_OPT_COMBO_FREEDB_SERVER, m_uiSettings.freedb_server);
      DDX_TEXT(IDC_CDRIP_OPT_EDIT_FREEDB_USERNAME, m_uiSettings.freedb_username);
   END_DDX_MAP()

   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnButtonSelectPath(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnChangeFreedbUsername(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
   wlUISettings& m_uiSettings;
   CLanguageResourceManager& m_langResourceManager;

   CImageListComboBox m_cbLanguages;

   wlBevelLine m_bevel1, m_bevel2;

   CButton m_btnSelectPath;

   CComboBox m_cbFreedbServer;

   CImageList m_ilIcons;
};
