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
/// \file GeneralSettingsPage.cpp
/// \brief General settings page

// includes
#include "StdAfx.h"
#include "GeneralSettingsPage.h"
#include "UISettings.h"
#include "LanguageResourceManager.hpp"
#include "LangCountryMapper.hpp"

LRESULT GeneralSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   // set language combobox icons
   {
      CImageList ilLangIcons;

      ilLangIcons.Create(16, 11, ILC_MASK | ILC_COLOR32, 0, 0);
      CBitmap bmpIcons;
      // load bitmap, but always from main module (bmp not in translation dlls)
      bmpIcons.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP_FLAGS)));
      ilLangIcons.Add(bmpIcons, RGB(255,255,255));

      m_cbLanguages.SetImageList(ilLangIcons);   
   }

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

         if (m_settings.language_id == info.LanguageId())
            iSelectedLangIndex = iItem;
      }
      m_cbLanguages.SetCurSel(iSelectedLangIndex);
   }

   return 1;
}

LRESULT GeneralSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   int iSelectedLangIndex = m_cbLanguages.GetCurSel();
   if (iSelectedLangIndex != -1)
   {
      UINT uiLangId = static_cast<UINT>(m_cbLanguages.GetItemData(iSelectedLangIndex));
      if (m_langResourceManager.IsLangResourceAvail(uiLangId))
      {
         m_settings.language_id = uiLangId;
         m_langResourceManager.LoadLangResource(uiLangId);
      }

      m_settings.StoreSettings();
   }

   return 0;
}
