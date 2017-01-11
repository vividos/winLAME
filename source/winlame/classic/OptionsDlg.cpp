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

*/
/// \file OptionsDlg.cpp
/// \brief options dialog

#include "stdafx.h"
#include "resource.h"
#include "OptionsDlg.h"
#include "LanguageResourceManager.hpp"
#include "LangCountryMapper.hpp"

LRESULT OptionsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   // set icons
   HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME), 
      IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
   SetIcon(hIcon, TRUE);
   HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME), 
      IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
   SetIcon(hIconSmall, FALSE);

   // set language combobox icons
   CImageList ilLangIcons;

   ilLangIcons.Create(16, 11, ILC_MASK | ILC_COLOR32, 0, 0);
   CBitmap bmpIcons;
   // load bitmap, but always from main module (bmp not in translation dlls)
   bmpIcons.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP_FLAGS)));
   ilLangIcons.Add(bmpIcons, RGB(255,255,255));

   m_cbLanguages.SetImageList(ilLangIcons);

   // fill language combobox
   {
      LangCountryMapper langCountryMapper;

      const std::vector<LanguageResourceInfo>& vecLangResourceList = m_langResourceManager.LanguageResourceList();
      int iSelectedLangIndex = -1;
      for (size_t i=0,iMax=vecLangResourceList.size(); i<iMax; i++)
      {
         const LanguageResourceInfo& info = vecLangResourceList[i];

         // map language id to image list index
         int iIconIndex = langCountryMapper.IndexFromLanguageCode(info.LanguageId());
         iIconIndex;

         int iItem = m_cbLanguages.AddString(info.LanguageName(), iIconIndex);
         m_cbLanguages.SetItemData(iItem, info.LanguageId());

         if (m_uiSettings.language_id == info.LanguageId())
            iSelectedLangIndex = iItem;
      }
      m_cbLanguages.SetCurSel(iSelectedLangIndex);
   }

//   m_cbLanguages.Invalidate();
//   m_cbLanguages.SetItemHeight(-1, 15);


   // set icons
   m_ilIcons.Create(MAKEINTRESOURCE(IDB_BITMAP_BTNICONS),16,0,RGB(192,192,192));

   m_btnSelectPath.ModifyStyle(0, BS_ICON);
   m_btnSelectPath.SetIcon(m_ilIcons.ExtractIcon(0));

   // fill freedb server combobox
   CString cszRandomServer;
   cszRandomServer.LoadString(IDS_CDRIP_RANDOM_FREEDB_SERVER);

   m_cbFreedbServer.AddString(_T("freedb.freedb.org (") + cszRandomServer + _T(")"));
   m_cbFreedbServer.SelectString(-1, m_uiSettings.freedb_server);

   return 1;
}

LRESULT OptionsDlg::OnExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (wID == IDOK)
   {
      int iSelectedLangIndex = m_cbLanguages.GetCurSel();
      if (iSelectedLangIndex != -1)
      {
         UINT uiLangId = static_cast<UINT>(m_cbLanguages.GetItemData(iSelectedLangIndex));
         if (m_langResourceManager.IsLangResourceAvail(uiLangId))
         {
            m_uiSettings.language_id = uiLangId;
            m_langResourceManager.LoadLangResource(uiLangId);
         }
      }

      if (!DoDataExchange(DDX_SAVE))
         return 0;
      else
      {
         // cut off after first spaces
         int iPos = m_uiSettings.freedb_server.Find(_T(' '));
         if (iPos > 0)
            m_uiSettings.freedb_server = m_uiSettings.freedb_server.Left(iPos);
      }
   }

   EndDialog(wID);
   return 0;
}

LRESULT OptionsDlg::OnButtonSelectPath(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_SAVE, IDC_CDRIP_OPT_EDIT_TEMP_FOLDER);

   CString cszPathname(m_uiSettings.cdrip_temp_folder);
   if (BrowseForFolder(m_hWnd, cszPathname))
   {
      m_uiSettings.cdrip_temp_folder = cszPathname;
      DoDataExchange(DDX_LOAD, IDC_CDRIP_OPT_EDIT_TEMP_FOLDER);
   }
   return 0;
}
