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
/// \file PresetManagerImpl.h
/// \brief contains the preset manager implementation definition
/// \ingroup preset
/// @{

// include guard
#pragma once

// needed includes
#include "../PresetManagerInterface.hpp"
#include "PropertyListBox.h"
#include "../VariableManager.hpp"
#include <string>
#include "rapidxml/rapidxml.hpp"

/// preset manager implementation
class PresetManagerImpl:
   public PresetManagerInterface,
   public PropertyManagerInterface
{
public:
   /// ctor
   PresetManagerImpl();

   /// loads preset from an xml file
   virtual bool loadPreset(LPCTSTR filename);

   /// sets currently used facility
   void setFacility(LPCTSTR facname);

   /// returns number of presets
   virtual size_t getPresetCount();

   /// returns name of preset
   virtual std::tstring getPresetName(size_t index);

   /// returns preset description
   virtual std::tstring getPresetDescription(size_t index);

   /// loads the specified settings into the settings manager
   virtual void setSettings(size_t index, SettingsManager& settingsManager);

   /// sets the default settings for all variables
   virtual void setDefaultSettings(SettingsManager& settingsManager);

   /// shows the edit settings dialog for a specific preset
   virtual void showPropertyDialog(size_t index);


   // interface implementation for the PropertyManagerInterface

   virtual int GetGroupCount();
   virtual std::tstring GetGroupName(int group);

   virtual int GetItemCount(int group);
   virtual std::tstring GetItemName(int group, int index);

   virtual std::tstring GetItemValue(int group, int index);
   virtual void SetItemValue(int group, int index, std::tstring val);

private:
   /// retrieves xml node pointer for preset value
   rapidxml::xml_node<char>* getPresetValueNode(int group, int index);

private:
   /// contents of the presets.xml file; must live as long as m_presetsDocument
   std::vector<char> m_presetsXmlFileContents;

   /// preset xml document
   rapidxml::xml_document<char> m_presetsDocument;

   /// name of current facility
   std::string m_currentFacilityName;

   /// node of currently set facility, or null if none was found
   rapidxml::xml_node<char>* m_currentFacilityNode;

   /// list of presets of current facility
   std::vector<rapidxml::xml_node<char>*> m_currentPresetsList;

   /// currently viewed preset index
   size_t m_viewedPresetIndex;

   /// var manager for facilities
   VarMgrFacilities mgr_facility;

   /// var manager for variables
   VarMgrVariables mgr_variables;
};


/// @}
