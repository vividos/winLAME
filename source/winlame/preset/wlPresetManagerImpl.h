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

   $Id: wlPresetManagerImpl.h,v 1.19 2004/01/14 21:32:20 vividos Exp $

*/
/*! \file wlPresetManagerImpl.h

   \brief contains the preset manager implementation definition

*/
/*! \ingroup preset */
/*! @{ */

// prevent multiple including
#ifndef __wlpresetmanagerimpl_h_
#define __wlpresetmanagerimpl_h_

// needed includes
#include "../wlPresetManagerInterface.h"
#include "wlPropertyListBox.h"
#include "../wlVariableManager.h"
#include <string>
#include "cppxml/cppxml.hpp"


//! preset manager implementation

class wlPresetManagerImpl:
   public wlPresetManagerInterface,
   public wlPropertyManagerInterface
{
public:
   //! ctor
   wlPresetManagerImpl();

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
   virtual void setSettings(int index, wlSettingsManager &settings_mgr);

   //! sets the default settings for all variables
   virtual void setDefaultSettings(wlSettingsManager &settings_mgr);

   //! shows the edit settings dialog for a specific preset
   virtual void editSettingsDialog(int index);


   // interface implementation for the wlPropertyManagerInterface

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
   wlVarMgrFacilities mgr_facility;

   //! var manager for variables
   wlVarMgrVariables mgr_variables;
};


//@}

#endif
