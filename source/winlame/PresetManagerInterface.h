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

   $Id: PresetManagerInterface.h,v 1.10 2004/01/14 21:32:20 vividos Exp $

*/
/*! \file PresetManagerInterface.h

   \brief PresetManagerInterface is an interface for the preset management

*/
/*! \defgroup preset Preset Management

   contains all classes and functions that have to do with preset management

*/
/*! \ingroup preset */
/*! @{ */

// prevent multiple including
#ifndef __wlpresetmanagerinterface_h_
#define __wlpresetmanagerinterface_h_

// needed includes
#include <string>
#include "SettingsManager.h"


//! encoder interface
class PresetManagerInterface
{
public:
   //! returns a new preset manager object; use delete operator to delete it
   static PresetManagerInterface *getPresetManager();

   //! loads preset from an xml file
   virtual bool loadPreset(LPCTSTR filename)=0;

   //! merge preset from an xml file
   virtual bool mergePreset(LPCTSTR filename)=0;

   //! save preset
   virtual void savePreset()=0;

   //! sets currently used facility
   virtual void setFacility(LPCTSTR facname)=0;

   //! returns number of presets
   virtual int getPresetCount()=0;

   //! returns name of preset
   virtual std::tstring getPresetName(int index)=0;

   //! returns preset description
   virtual std::tstring getPresetDescription(int index)=0;

   //! loads the specified settings into the settings manager
   virtual void setSettings(int index, SettingsManager &settings_mgr)=0;

   //! sets the default settings for all variables
   virtual void setDefaultSettings(SettingsManager &settings_mgr)=0;

   //! shows the edit settings dialog for a specific preset
   virtual void editSettingsDialog(int index)=0;

   //! dtor
   virtual ~PresetManagerInterface(){}
protected:
   //! ctor
   PresetManagerInterface(){}
};


//@}

#endif
