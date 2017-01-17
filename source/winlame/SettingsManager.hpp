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
/// \file SettingsManager.hpp
/// \brief SettingsManagerInterface is an interface for encoder settings management

/// \defgroup settings Settings Management
/// contains all classes and functions that have to do with settings management

/// \ingroup settings
/// @{

// include guard
#pragma once

// needed includes
#include <map>
#include "VariableManager.hpp"


/// settings list type
typedef std::map<unsigned int,int> SettingsList;


/// manages settings for encoding
class SettingsManager
{
public:
   /// ctor
   SettingsManager() {}

   /// returns variable
   int QueryValueInt(unsigned short name) { return queryValueInt(name); }

   /// returns variable
   int queryValueInt(unsigned short name);

   /// sets new variable value
   void setValue(unsigned short name, int val);

protected:
   /// map with settings
   SettingsList settings;

   /// variable manager for variables
   VarMgrVariables mgr_vars;
};


/// @}
