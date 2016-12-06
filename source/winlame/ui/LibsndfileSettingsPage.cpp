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
/// \file LibsndfileSettingsPage.cpp
/// \brief Libsndfile settings page
//
#include "StdAfx.h"
#include "LibsndfileSettingsPage.hpp"
#include "WizardPageHost.hpp"
#include "IoCContainer.hpp"
#include "UISettings.h"
#include "OutputSettingsPage.hpp"
#include "PresetSelectionPage.hpp"
#include "FinishPage.hpp"
#include "encoder/SndFileFormats.hpp"

using namespace UI;

LRESULT LibsndfileSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   UpdateFileFormatList();

   int itemToSelect = m_cbFormat.FindString(0, _T("WAV (Microsoft) (wav)"));
   m_cbFormat.SetCurSel(itemToSelect != -1 ? itemToSelect : 0);

   UpdateSubTypeCombobox();

   LoadData();

   return 1;
}

LRESULT LibsndfileSettingsPage::OnFormatSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   UpdateSubTypeCombobox();

   return 0;
}

LRESULT LibsndfileSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new FinishPage(m_pageHost)));

   return 0;
}

LRESULT LibsndfileSettingsPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   return 0;
}

LRESULT LibsndfileSettingsPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

   if (m_uiSettings.preset_avail && presetManager.getPresetCount() > 0)
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   else
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

void LibsndfileSettingsPage::UpdateFileFormatList()
{
   std::vector<int> formatsList = SndFileFormats::EnumFormats();

   for (size_t formatIndex = 0, maxFormatIndex = formatsList.size(); formatIndex < maxFormatIndex; formatIndex++)
   {
      int format = formatsList[formatIndex];

      CString formatName, outputExtension;
      SndFileFormats::GetFormatInfo(format, formatName, outputExtension);

      ATLTRACE(_T("SndFile Format %i: Name: \"%s\", Extension: %s\n"),
         format, formatName.GetString(), outputExtension.GetString());

      CString text;
      text.Format(_T("%s (%s)"), formatName.GetString(), outputExtension.GetString());

      int formatItem = m_cbFormat.AddString(text);
      m_cbFormat.SetItemData(formatItem, format);
   }
}

void LibsndfileSettingsPage::UpdateSubTypeCombobox()
{
   int selectedSubType = -1;

   int selectedItem = m_cbSubType.GetCurSel();
   if (selectedItem != CB_ERR)
      selectedSubType = static_cast<int>(m_cbSubType.GetItemData(selectedItem));

   m_cbSubType.ResetContent();

   int formatIndex = m_cbFormat.GetCurSel();
   if (formatIndex == CB_ERR)
      return;

   int format = static_cast<int>(m_cbFormat.GetItemData(formatIndex));

   CString formatName, outputExtension;
   SndFileFormats::GetFormatInfo(format, formatName, outputExtension);

   ATLTRACE(_T("SndFile Format %i: Name: \"%s\", Extension: %s\n"),
      format, formatName.GetString(), outputExtension.GetString());

   std::vector<int> subTypesList = SndFileFormats::EnumSubTypes();

   int itemToSelect = -1;
   for (size_t subTypeIndex = 0, maxSubTypeIndex = subTypesList.size(); subTypeIndex < maxSubTypeIndex; subTypeIndex++)
   {
      int subType = subTypesList[subTypeIndex];

      // we don't know if it will be a stereo or mono file, so check both
      bool stereoValid = SndFileFormats::IsValidFormatCombo(format, subType, 2);
      bool monoValid = SndFileFormats::IsValidFormatCombo(format, subType, 1);

      if (stereoValid || monoValid)
      {
         CString subTypeName = SndFileFormats::GetSubTypeName(subType);

         ATLTRACE(_T("   SndFile sub type %i: Name: \"%s\", %s\n"),
            subType,
            subTypeName.GetString(),
            stereoValid && !monoValid ? _T("Stereo only") :
            !stereoValid && monoValid ? _T("Mono only") : _T("Mono + Stereo"));

         if (stereoValid && !monoValid)
            subTypeName += _T(" (Stereo)");

         if (!stereoValid && monoValid)
            subTypeName += _T(" (Mono)");

         int subTypeItem = m_cbSubType.AddString(subTypeName);
         m_cbSubType.SetItemData(subTypeItem, subType);

         if (selectedSubType != -1 &&
            subType == selectedSubType)
         {
            itemToSelect = subTypeItem;
         }
      }
   }

   if (itemToSelect == -1)
   {
      itemToSelect = m_cbSubType.FindString(0, _T("Signed 16 bit PCM"));
   }

   m_cbSubType.SetCurSel(itemToSelect != -1 ? itemToSelect : 0);
}

void LibsndfileSettingsPage::LoadData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // format
   int format = mgr.queryValueInt(SndFileFormat);

   for (int itemIndex = 0, maxItemIndex = m_cbFormat.GetCount(); itemIndex < maxItemIndex; itemIndex++)
   {
      int itemFormat = static_cast<int>(m_cbFormat.GetItemData(itemIndex));
      if (itemFormat == format)
      {
         m_cbFormat.SetCurSel(itemIndex);
         break;
      }
   }

   // sub type
   int subType = mgr.queryValueInt(SndFileSubType);

   for (int itemIndex = 0, maxItemIndex = m_cbSubType.GetCount(); itemIndex < maxItemIndex; itemIndex++)
   {
      int itemSubType = static_cast<int>(m_cbSubType.GetItemData(itemIndex));
      if (itemSubType == subType)
      {
         m_cbSubType.SetCurSel(itemIndex);
         break;
      }
   }
}

void LibsndfileSettingsPage::SaveData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // format
   int itemIndex = m_cbFormat.GetCurSel();
   int value = static_cast<int>(m_cbFormat.GetItemData(itemIndex));
   mgr.setValue(SndFileFormat, value);

   // sub type
   itemIndex = m_cbSubType.GetCurSel();
   value = static_cast<int>(m_cbSubType.GetItemData(itemIndex));
   mgr.setValue(SndFileSubType, value);
}
