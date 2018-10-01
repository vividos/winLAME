//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
//
#include "StdAfx.h"
#include "GeneralSettingsPage.hpp"
#include "LanguageResourceManager.hpp"
#include "LangCountryMapper.hpp"
#include <thread>

using UI::GeneralSettingsPage;

/// possible numbers of CPU cores
static int CpuCores[] =
{
   1, 2, 4, 8
};

LRESULT GeneralSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   // set language combobox icons
   {
      CImageList languageIcons;

      languageIcons.Create(16, 11, ILC_MASK | ILC_COLOR32, 0, 0);
      CBitmap bmpIcons;
      // load bitmap, but always from main module (bmp not in translation dlls)
      bmpIcons.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP_FLAGS)));
      languageIcons.Add(bmpIcons, RGB(255, 255, 255));

      m_comboLanguages.SetImageList(languageIcons);
   }

   // fill language combobox
   {
      LangCountryMapper langCountryMapper;

      const std::vector<LanguageResourceInfo>& vecLangResourceList = m_langResourceManager.LanguageResourceList();
      int iSelectedLangIndex = -1;
      for (size_t i = 0, iMax = vecLangResourceList.size(); i < iMax; i++)
      {
         const LanguageResourceInfo& info = vecLangResourceList[i];

         // map language id to image list index
         int iIconIndex = langCountryMapper.IndexFromLanguageCode(info.LanguageId());
         iIconIndex;

         int iItem = m_comboLanguages.AddString(info.LanguageName(), iIconIndex);
         m_comboLanguages.SetItemData(iItem, info.LanguageId());

         if (m_settings.language_id == info.LanguageId())
            iSelectedLangIndex = iItem;
      }

      m_comboLanguages.SetCurSel(iSelectedLangIndex);
   }

   {
      m_spinCpuCores.SetBuddy(m_editCpuCores);
      m_spinCpuCores.SetFixedValues(CpuCores, sizeof(CpuCores) / sizeof(CpuCores[0]));

      m_staticNoteCpuCores.ShowWindow(SW_HIDE);

      CString checkBoxText;
      m_checkBoxAutoTasks.GetWindowText(checkBoxText);

      CString numCoresText;
      numCoresText.Format(_T("%u"), std::thread::hardware_concurrency());

      checkBoxText.Replace(_T("%cores%"), numCoresText);

      m_checkBoxAutoTasks.SetWindowText(checkBoxText);

      if (m_settings.m_taskManagerConfig.m_bAutoTasksPerCpu)
      {
         m_spinCpuCores.EnableWindow(FALSE);
         m_editCpuCores.EnableWindow(FALSE);
      }
   }

   return 1;
}

LRESULT GeneralSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (!DoDataExchange(DDX_SAVE))
      return 1;

   if (m_settings.m_taskManagerConfig.m_uiUseNumTasks < 1)
   {
      m_settings.m_taskManagerConfig.m_uiUseNumTasks = 1;
   }

   if (m_settings.m_taskManagerConfig.m_uiUseNumTasks > 64)
   {
      m_settings.m_taskManagerConfig.m_uiUseNumTasks = 64;
   }

   int selectedLangIndex = m_comboLanguages.GetCurSel();
   if (selectedLangIndex != -1)
   {
      UINT languageId = static_cast<UINT>(m_comboLanguages.GetItemData(selectedLangIndex));
      if (m_langResourceManager.IsLangResourceAvail(languageId))
      {
         m_settings.language_id = languageId;
         m_langResourceManager.LoadLangResource(languageId);
      }
   }

   return 0;
}

LRESULT GeneralSettingsPage::OnCheckAutoTasks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   bool isChecked = m_checkBoxAutoTasks.GetCheck() == BST_CHECKED;

   m_spinCpuCores.EnableWindow(!isChecked);
   m_editCpuCores.EnableWindow(!isChecked);

   m_staticNoteCpuCores.ShowWindow(SW_SHOW);

   return 0;
}

LRESULT GeneralSettingsPage::OnChangeEditCpuCores(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (m_staticNoteCpuCores.IsWindow())
      m_staticNoteCpuCores.ShowWindow(SW_SHOW);

   return 0;
}
