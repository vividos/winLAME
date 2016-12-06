/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2016 Michael Fink

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
/// \file WaveOutputSettingsPage.cpp
/// \brief contains implementation of the wave output settings page

// needed includes
#include "stdafx.h"
#include "WaveOutputSettingsPage.h"
#include "encoder/SndFileFormats.hpp"

// WaveOutputSettingsPage methods

LRESULT WaveOutputSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, true);

   UpdateFileFormatList();

   int itemToSelect = m_cbFormat.FindString(0, _T("WAV (Microsoft) (wav)"));
   m_cbFormat.SetCurSel(itemToSelect != -1 ? itemToSelect : 0);

   UpdateSubTypeCombobox();

   return 1;  // let the system set the focus
}

LRESULT WaveOutputSettingsPage::OnFormatSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   UpdateSubTypeCombobox();

   return 0;
}

void WaveOutputSettingsPage::UpdateFileFormatList()
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

void WaveOutputSettingsPage::UpdateSubTypeCombobox()
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

      if (SndFileFormats::IsValidFormatCombo(format, subType))
      {
         CString subTypeName = SndFileFormats::GetSubTypeName(subType);

         ATLTRACE(_T("   SndFile sub type %i: Name: \"%s\"\n"),
            subType, subTypeName.GetString());

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

void WaveOutputSettingsPage::OnEnterPage()
{
   // get settings manager
   SettingsManager& mgr = pui->getUISettings().settings_manager;

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

bool WaveOutputSettingsPage::OnLeavePage()
{
   // get settings manager
   SettingsManager& mgr = pui->getUISettings().settings_manager;

   // format
   int itemIndex = m_cbFormat.GetCurSel();
   int value = static_cast<int>(m_cbFormat.GetItemData(itemIndex));
   mgr.setValue(SndFileFormat, value);

   // sub type
   itemIndex = m_cbSubType.GetCurSel();
   value = static_cast<int>(m_cbSubType.GetItemData(itemIndex));
   mgr.setValue(SndFileSubType, value);

   return true;
}
