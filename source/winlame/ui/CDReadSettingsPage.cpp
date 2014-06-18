//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2012 Michael Fink
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
#include "UISettings.h"
#include "LanguageResourceManager.hpp"
#include "LangCountryMapper.hpp"
#include "CommonStuff.h"
#include "App.h"

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
      m_cbFreedbServer.AddString(_T("at.freedb.org (Vienna, Austria)"));
      m_cbFreedbServer.AddString(_T("au.freedb.org (Sydney, Australia)"));
      m_cbFreedbServer.AddString(_T("ca.freedb.org (Winnipeg, Manitoba, Canada)"));
      m_cbFreedbServer.AddString(_T("ca2.freedb.org (Montreal, Quebec, Canada)"));
      m_cbFreedbServer.AddString(_T("de.freedb.org (Berlin, Germany)"));
      m_cbFreedbServer.AddString(_T("es.freedb.org (Madrid, Spain)"));
      m_cbFreedbServer.AddString(_T("fi.freedb.org (Tampere, Finland)"));
      m_cbFreedbServer.AddString(_T("ru.freedb.org (Saint-Petersburg, Russia)"));
      m_cbFreedbServer.AddString(_T("uk.freedb.org (London, UK)"));
      m_cbFreedbServer.AddString(_T("us.freedb.org (San Jose, California, USA)"));
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

LRESULT CDReadSettingsPage::OnChangeFreedbUsername(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_SAVE, IDC_CDRIP_OPT_EDIT_FREEDB_USERNAME);
   if (-1 != m_settings.freedb_username.Find(_T(' ')))
   {
      App::MessageBox(m_hWnd, IDS_CDRIP_FREEDB_USERNAME_NO_SPACES, MB_OK | MB_ICONEXCLAMATION);
      m_settings.freedb_username.Replace(_T(" "), _T(""));
      DoDataExchange(DDX_LOAD, IDC_CDRIP_OPT_EDIT_FREEDB_USERNAME);
   }
   return 0;
}
