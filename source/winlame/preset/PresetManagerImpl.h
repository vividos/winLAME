/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2004 Michael Fink

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
/*! \file PresetManagerImpl.h

   \brief contains the preset manager implementation definition

*/
/*! \ingroup preset */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "../PresetManagerInterface.h"
#include "PropertyListBox.h"
#include "../VariableManager.h"
#include <string>
#include "cppxml/cppxml.hpp"


//! preset manager implementation

class PresetManagerImpl:
   public PresetManagerInterface,
   public PropertyManagerInterface
{
public:
   //! ctor
   PresetManagerImpl();

   //! loads preset from an xml file
   virtual bool loadPreset(LPCTSTR filename);

   //! merge preset from an xml file
   virtual bool mergePreset(LPCTSTR filename);

   //! save preset
   virtual void savePreset();

   //! sets currently used facility
   void setFacility(LPCTSTR facname);

   //! returns number of presets
   virtual int getPresetCount();

   //! returns name of preset
   virtual std::tstring getPresetName(int index);

   //! returns preset description
   virtual std::tstring getPresetDescription(int index);

   //! loads the specified settings into the settings manager
   virtual void setSettings(int index, SettingsManager &settings_mgr);

   //! sets the default settings for all variables
   virtual void setDefaultSettings(SettingsManager &settings_mgr);

   //! shows the edit settings dialog for a specific preset
   virtual void editSettingsDialog(int index);


   // interface implementation for the PropertyManagerInterface

   virtual int GetGroupCount();
   virtual std::tstring GetGroupName(int group);

   virtual int GetItemCount(int group);
   virtual std::tstring GetItemName(int group, int index);

   virtual std::tstring GetItemValue(int group, int index);
   virtual void SetItemValue(int group, int index, std::tstring val);

protected:
   //! retrieves xml node pointer for editing
   cppxml::xmlnode_ptr editLookupNode(int group, int index);

protected:
   //! xml filename
   std::tstring xmlfilename;

   //! preset xml document
   cppxml::xmldocument doc;

   //! list of presets of current facility
   cppxml::xmlnodelist_ptr presets_list;

   //! currently edited/viewed preset
   cppxml::xmlnode_ptr editing_preset;

   //! indicates if preset xml document was changed
   bool xmlchanged;

   //! name of current facility
   std::tstring facility;

   //! var manager for facilities
   VarMgrFacilities mgr_facility;

   //! var manager for variables
   VarMgrVariables mgr_variables;
};


//@}
