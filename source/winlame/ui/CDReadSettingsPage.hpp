//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file CDReadSettingsPage.hpp
/// \brief CD read settings page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "ImageListComboBox.hpp"
#include "UISettings.hpp"
#include "resource.h"

// forward references
struct UISettings;
class LanguageResourceManager;

namespace UI
{

/// general settings page
class CDReadSettingsPage:
   public WizardPage,
   public CWinDataExchange<CDReadSettingsPage>,
   public CDialogResize<CDReadSettingsPage>
{
public:
   /// ctor
   CDReadSettingsPage(WizardPageHost& pageHost, UISettings& settings)
      :WizardPage(pageHost, IDD_SETTINGS_CDREAD, WizardPage::typeCancelOk),
       m_settings(settings)
   {
   }
   /// dtor
   ~CDReadSettingsPage()
   {
   }

private:
   friend CDialogResize<CDReadSettingsPage>;

   BEGIN_DDX_MAP(CDReadSettingsPage)
      DDX_CONTROL_HANDLE(IDC_CDRIP_OPT_COMBO_FREEDB_SERVER, m_cbFreedbServer);
      DDX_TEXT(IDC_CDRIP_OPT_COMBO_FREEDB_SERVER, m_settings.freedb_server);
      DDX_TEXT(IDC_CDRIP_OPT_EDIT_TEMP_FOLDER, m_settings.cdrip_temp_folder);
      DDX_CONTROL_HANDLE(IDC_CDRIP_OPT_BUTTON_TEMP_SELECTPATH, m_btnSelectPath);
      DDX_CHECK(IDC_CDRIP_OPT_CHECK_CDPLAYER_INI, m_settings.store_disc_infos_cdplayer_ini);
      DDX_TEXT(IDC_CDRIP_OPT_EDIT_FORMAT_VARIOUS_TRACK, m_settings.cdrip_format_various_track);
      DDX_TEXT(IDC_CDRIP_OPT_EDIT_FORMAT_ALBUM_TRACK, m_settings.cdrip_format_album_track);
      DDX_CONTROL_HANDLE(IDC_CDRIP_OPT_BUTTON_VARIOUS_TRACK_TAGS, m_btnVariousTrackTags);
      DDX_CONTROL_HANDLE(IDC_CDRIP_OPT_BUTTON_ALBUM_TRACK_TAGS, m_btnAlbumTrackTags);
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(CDReadSettingsPage)
      DLGRESIZE_CONTROL(IDC_CDRIP_OPT_COMBO_FREEDB_SERVER, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_CDRIP_OPT_EDIT_FORMAT_VARIOUS_TRACK, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_CDRIP_OPT_EDIT_FORMAT_ALBUM_TRACK, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_CDRIP_OPT_BUTTON_VARIOUS_TRACK_TAGS, DLSZ_MOVE_X)
      DLGRESIZE_CONTROL(IDC_CDRIP_OPT_BUTTON_ALBUM_TRACK_TAGS, DLSZ_MOVE_X)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(CDReadSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_ID_HANDLER(IDC_CDRIP_OPT_BUTTON_TEMP_SELECTPATH, OnButtonSelectPath)
      COMMAND_ID_HANDLER(IDC_CDRIP_OPT_BUTTON_VARIOUS_TRACK_TAGS, OnButtonVariousTrackTags)
      COMMAND_ID_HANDLER(IDC_CDRIP_OPT_BUTTON_ALBUM_TRACK_TAGS, OnButtonAlbumTrackTags)
      CHAIN_MSG_MAP(CDialogResize<CDReadSettingsPage>)
   END_MSG_MAP()

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is left
   LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

   /// called when user clicks on select path button
   LRESULT OnButtonSelectPath(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

   /// called when user clicks on various track tags button
   LRESULT OnButtonVariousTrackTags(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when user clicks on album track tags button
   LRESULT OnButtonAlbumTrackTags(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
   /// shows a tags context menu besides the given button, and inserts selected tag into edit control
   void ShowTagsContextMenu(CButton& button, CEdit& edit);

private:
   /// settings
   UISettings& m_settings;

   // controls

   /// freedb server combobox
   CComboBox m_cbFreedbServer;

   /// icons image list
   CImageList m_ilIcons;

   /// button to select path
   CButton m_btnSelectPath;

   /// various track tags select button
   CButton m_btnVariousTrackTags;

   /// albun track tags select button
   CButton m_btnAlbumTrackTags;
};

} // namespace UI
