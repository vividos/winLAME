/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2008 Michael Fink

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
/// \file SettingsManager.cpp
/// \brief SettingsManagerInterface is an interface for encoder settings management

// needed includes
#include "stdafx.h"
#include "SettingsManager.hpp"

// SettingsManager methods

int SettingsManager::queryValueInt(unsigned short name)
{
   int var=-1;
   SettingsList::iterator iter = settings.find(name);
   if (iter == settings.end())
   {
      // look up default
      var = mgr_vars.lookupDefaultValue(name);
   }
   else
      var = (*iter).second;

   return var;
}

void SettingsManager::setValue(unsigned short name, int val)
{
   SettingsList::iterator iter = settings.find(name);
   if (iter == settings.end())
   {
      settings.insert(std::make_pair(static_cast<unsigned int>(name),val));
   }
   else
   {
      (*iter).second = val;
   }
}
