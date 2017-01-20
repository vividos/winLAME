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
/// \file CDReadSettingsPage.cpp
/// \brief CD read settings page
//
#include "StdAfx.h"
#include "CDReadSettingsPage.hpp"
#include "UISettings.hpp"
#include "LanguageResourceManager.hpp"
#include "LangCountryMapper.hpp"
#include "CDRipTitleFormatManager.hpp"
#include "CommonStuff.hpp"
#include "App.hpp"
#include <vector>

using namespace UI;

LRESULT CDReadSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   // fill freedb server combobox
   {
      CString cszRandomServer;
      cszRandomServer.LoadString(IDS_CDRIP_RANDOM_FREEDB_SERVER);

      m_cbFreedbServer.AddString(_T("freedb.freedb.org (") + cszRandomServer + _T(")"));
      m_cbFreedbServer.SelectString(-1, m_settings.freedb_server);
   }

   // set button icons
   {
      m_ilIcons.Create(MAKEINTRESOURCE(IDB_BITMAP_BTNICONS),16,0,RGB(192,192,192));

      m_btnSelectPath.ModifyStyle(0, BS_ICON);
      m_btnSelectPath.SetIcon(m_ilIcons.ExtractIcon(0));
   }

   return 1;
}

LRESULT CDReadSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (!DoDataExchange(DDX_SAVE))
      return 1;
   else
   {
      // cut off after first spaces
      int iPos = m_settings.freedb_server.Find(_T(' '));
      if (iPos > 0)
         m_settings.freedb_server = m_settings.freedb_server.Left(iPos);
   }

   m_settings.StoreSettings();

   return 0;
}

LRESULT CDReadSettingsPage::OnButtonSelectPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_SAVE, IDC_CDRIP_OPT_EDIT_TEMP_FOLDER);

   CString cszPathname(m_settings.cdrip_temp_folder);
   if (BrowseForFolder(m_hWnd, cszPathname))
   {
      m_settings.cdrip_temp_folder = cszPathname;
      DoDataExchange(DDX_LOAD, IDC_CDRIP_OPT_EDIT_TEMP_FOLDER);
   }
   return 0;
}

LRESULT CDReadSettingsPage::OnButtonVariousTrackTags(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   CButton button(GetDlgItem(wID));
   CEdit edit(GetDlgItem(IDC_CDRIP_OPT_EDIT_FORMAT_VARIOUS_TRACK));

   ShowTagsContextMenu(button, edit);

   return 0;
}
LRESULT CDReadSettingsPage::OnButtonAlbumTrackTags(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   CButton button(GetDlgItem(wID));
   CEdit edit(GetDlgItem(IDC_CDRIP_OPT_EDIT_FORMAT_ALBUM_TRACK));

   ShowTagsContextMenu(button, edit);

   return 0;
}

void CDReadSettingsPage::ShowTagsContextMenu(CButton& button, CEdit& edit)
{
   std::vector<CString> allTags =
      CDRipTitleFormatManager::GetAllTags();

   CMenu contextMenu;
   BOOL retCreate = contextMenu.CreatePopupMenu();
   ATLASSERT(retCreate == TRUE); retCreate;

   //std::for_each(allTags.begin(), allTags.end(), [&](const CString& singleTag) {
   for (size_t tagIndex = 0; tagIndex < allTags.size(); tagIndex++)
   {
      contextMenu.AppendMenu(MF_STRING, tagIndex + 1, allTags[tagIndex]);
   }

   CRect rectButton;
   button.GetWindowRect(rectButton);
   CPoint pos(rectButton.right + 1, rectButton.top);

   // track popup menu
   int ret = contextMenu.TrackPopupMenu(
      TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
      pos.x, pos.y,
      m_hWnd,
      nullptr);

   if (ret == 0)
      return;

   size_t selectedTagIndex = ret - 1;
   CString tagToInsert = _T("%") + allTags[selectedTagIndex] + _T("%");

   int startChar = 0, endChar = 0;
   edit.GetSel(startChar, endChar);

   if (startChar != endChar)
      edit.ReplaceSel(tagToInsert, true);
   else
      edit.InsertText(startChar, tagToInsert, false, true);
}
