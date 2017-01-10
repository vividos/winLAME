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
/// \file PresetManagerImpl.cpp
/// \brief contains the preset manager implementation
/// \ingroup preset

// needed includes
#include "stdafx.h"
#include "PresetManagerImpl.h"
#include "PropertyDlg.h"
#include "UTF8.hpp"
#include <fstream>

std::tstring convertFromUtf8(const std::string& text)
{
   return std::tstring(UTF8ToString(text.c_str()).GetString());
}

// PresetManagerInterface methods

PresetManagerImpl::PresetManagerImpl()
   :m_currentFacilityNode(nullptr),
   m_viewedPresetIndex(0)
{
}

bool PresetManagerImpl::loadPreset(LPCTSTR filename)
{
   std::ifstream file(filename);
   if (!file.is_open())
      return false;

   std::stringstream buffer;
   buffer << file.rdbuf();

   std::string text = buffer.str();
   m_presetsXmlFileContents.assign(text.begin(), text.end());
   m_presetsXmlFileContents.push_back(0);

   file.close();

   // load xml document
   try
   {
      m_presetsDocument.parse<0>(m_presetsXmlFileContents.data());
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
      return false;
   }

   return true;
}

void PresetManagerImpl::setFacility(LPCTSTR facilityName)
{
   try
   {
      m_currentFacilityName = CStringA(facilityName).GetString();

      rapidxml::xml_node<char>* presetsNode =
         m_presetsDocument.first_node("presets");

      if (presetsNode == nullptr)
         return; // should not happen - no preset root node

      rapidxml::xml_node<char>* facilityNode = presetsNode->first_node("facility");
      while (facilityNode != nullptr)
      {
         rapidxml::xml_attribute<char>* attr = facilityNode->first_attribute("name");

         if (attr == nullptr)
            continue; // should not happen - facility node without name

         std::string attributeName = attr->value();
         if (attributeName == m_currentFacilityName)
         {
            m_currentFacilityNode = facilityNode;

            m_currentPresetsList.clear();

            for (rapidxml::xml_node<char>* presetNode = m_currentFacilityNode->first_node("preset");
               presetNode != nullptr;
               presetNode = presetNode->next_sibling("preset"))
            {
               m_currentPresetsList.push_back(presetNode);
            }

            break;
         }

         facilityNode = facilityNode->next_sibling("facility");
      }
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }
}

size_t PresetManagerImpl::getPresetCount()
{
   return m_currentPresetsList.size();
}

std::tstring PresetManagerImpl::getPresetName(size_t index)
{
   ATLASSERT(index < m_currentPresetsList.size());

   std::tstring ret(_T("- name not found -"));

   try
   {
      rapidxml::xml_node<char>* presetNode = m_currentPresetsList[index];
      std::string name = presetNode->first_attribute("name")->value();

      ret = convertFromUtf8(name);
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }

   return ret;
}

std::tstring PresetManagerImpl::getPresetDescription(size_t index)
{
   ATLASSERT(index < m_currentPresetsList.size());

   std::tstring ret(_T("- no description available -"));
   try
   {
      rapidxml::xml_node<char>* presetNode = m_currentPresetsList[index];
      std::string desc = presetNode->first_node("comment")->value();

      ret = convertFromUtf8(desc);
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }

   // trim description text
   bool first = true;
   std::tstring::size_type pos, len = ret.length();
   for (pos = 0; pos < len; pos++)
   {
      // trim leading whitespaces
      int dist = 0;
      while (pos < len && (ret.at(pos) == ' ' || ret.at(pos) == '\t' ||
         (first && ret.at(pos) == '\n')))
         dist++, pos++;

      first = false;

      if (dist != 0)
      {
         pos -= dist;
         ret.erase(pos, dist);
         len -= dist;
      }

      if (pos >= len) break;

      // search for \n
      while (pos < len && ret.at(pos) != '\n')
         pos++;

      if (pos >= len) break;

      // search back for spaces
      dist = 0;
      while (pos < len && (ret.at(pos - dist - 1) == ' ' || ret.at(pos) == '\t'))
         dist++;

      if (pos >= len) break;

      if (dist > 0)
      {
         pos -= dist;
         ret.erase(pos, dist);
         len -= dist;
      }

      // insert \r before \n
      ret.insert((len++, pos++), 1, '\r');
   }

   // trim last \r\n
   if (ret.size() > 0 && 0 == _tcscmp(ret.c_str() + (ret.size() - 2), _T("\r\n")))
      ret.erase(ret.size() - 2);

   return ret;
}

void PresetManagerImpl::setSettings(size_t index, SettingsManager& settingsManager)
{
   ATLASSERT(index < m_currentPresetsList.size());

   try
   {
      rapidxml::xml_node<char>* presetNode = m_currentPresetsList[index];
      if (presetNode == nullptr)
         return;

      // for all "value" nodes
      for (rapidxml::xml_node<char>* presetValueNode = presetNode->first_node("value");
         presetValueNode != nullptr;
         presetValueNode = presetValueNode->next_sibling("value"))
      {
         // set them in the settings manager
         std::string valueName = presetValueNode->first_attribute("name")->value();

         int valueID = mgr_variables.lookupID(CString(valueName.c_str()));
         if (valueID == -1)
         {
            ATLASSERT(false);
            continue; // not found - wrong ID
         }

         // get CDATA of fist child node: variable's value
         std::string value = presetValueNode->first_node()->value();

         // set name ID and value
         settingsManager.setValue(static_cast<unsigned short>(valueID), atoi(value.c_str()));
      }
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }
}

void PresetManagerImpl::setDefaultSettings(SettingsManager& settingsManager)
{
   for (int i = VarFirst; i < VarLast; i++)
   {
      int def = mgr_variables.lookupDefaultValue(i);
      settingsManager.setValue(static_cast<unsigned short>(i), def);
   }
}

void PresetManagerImpl::showPropertyDialog(size_t index)
{
   ATLASSERT(index < m_currentPresetsList.size());
   m_viewedPresetIndex = index;

   // show edit dialog
   PropertyDlg dlg;
   dlg.SetPropertyManagerInterface(this);
   dlg.DoModal();
}

// interface implementation for the PropertyManagerInterface

int PresetManagerImpl::GetGroupCount()
{
   return 1;
}

std::tstring PresetManagerImpl::GetGroupName(int group)
{
   // look up description for facility
   std::tstring ret;
   int id = mgr_facility.lookupID(CString(m_currentFacilityName.c_str()));
   if (id != -1)
      ret = mgr_facility.lookupDescription(id);
   return ret;
}

int PresetManagerImpl::GetItemCount(int group)
{
   if (m_currentPresetsList.empty())
      return 0;

   int presetValueCount = 0;

   try
   {
      rapidxml::xml_node<char>* presetNode = m_currentPresetsList[m_viewedPresetIndex];

      if (presetNode == nullptr)
         return 0;

      for (rapidxml::xml_node<char>* presetValueNode = presetNode->first_node("value");
         presetValueNode != nullptr;
         presetValueNode = presetValueNode->next_sibling("value"))
      {
         presetValueCount++;
      }
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }

   return presetValueCount;
}

std::tstring PresetManagerImpl::GetItemName(int group, int index)
{
   std::tstring ret(_T("- no item name -"));

   try
   {
      rapidxml::xml_node<char>* presetValueNode = getPresetValueNode(group, index);
      if (presetValueNode == nullptr)
         return ret; // not found

      // get name of node
      std::string valueName = presetValueNode->first_attribute("name")->value();
      int id = mgr_variables.lookupID(CString(valueName.c_str()));
      if (id != -1)
         ret = mgr_variables.lookupDescription(id);
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }

   return ret;
}

std::tstring PresetManagerImpl::GetItemValue(int group, int index)
{
   std::tstring ret(_T("- no item value -"));

   try
   {
      // search node
      rapidxml::xml_node<char>* presetValueNode = getPresetValueNode(group, index);
      if (presetValueNode == nullptr)
         return ret; // not found

      // get CDATA of fist child node
      std::string value = presetValueNode->first_node()->value();

      ret = CString(value.c_str());
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }

   return ret;
}

void PresetManagerImpl::SetItemValue(int group, int index, std::tstring val)
{
   // editing not supported
}

rapidxml::xml_node<char>* PresetManagerImpl::getPresetValueNode(int group, int index)
{
   if (m_currentFacilityNode == nullptr)
      return nullptr;

   if (m_currentPresetsList.empty())
      return nullptr;

   try
   {
      rapidxml::xml_node<char>* presetNode = m_currentPresetsList[m_viewedPresetIndex];

      if (presetNode == nullptr)
         return nullptr;

      int presetValueIndex = 0;

      for (rapidxml::xml_node<char>* presetValueNode = presetNode->first_node("value");
         presetValueNode != nullptr;
         presetValueNode = presetValueNode->next_sibling("value"))
      {
         if (presetValueIndex == index)
            return presetValueNode;

         presetValueIndex++;
      }
   }
   catch (const std::exception& ex)
   {
      ex;
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }

   return nullptr;
}
