//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file LameNogapInstanceManager.cpp
/// \brief Manager for LAME nogap encoding instances
//
#include "stdafx.h"
#include "LameNogapInstanceManager.hpp"

using Encoder::LameNogapInstanceManager;

int LameNogapInstanceManager::s_nextNogapInstanceId = 0;

LameNogapInstanceManager::LameNogapInstanceManager()
{
}

int LameNogapInstanceManager::NextNogapInstanceId()
{
   return s_nextNogapInstanceId++;
}

bool LameNogapInstanceManager::IsRegistered(int nogapInstanceId) const
{
   auto iter = m_allInstances.find(nogapInstanceId);
   return iter != m_allInstances.end();
}

void LameNogapInstanceManager::RegisterInstance(int nogapInstanceId, nlame_instance_t* instance)
{
   ATLASSERT(!IsRegistered(nogapInstanceId)); // must not be registered yet

   m_allInstances[nogapInstanceId] = instance;
}

nlame_instance_t* LameNogapInstanceManager::GetInstance(int nogapInstanceId)
{
   auto iter = m_allInstances.find(nogapInstanceId);
   if (iter == m_allInstances.end())
      return nullptr;

   return iter->second;
}

void LameNogapInstanceManager::UnregisterInstance(int nogapInstanceId)
{
   ATLASSERT(IsRegistered(nogapInstanceId)); // must have been registered

   m_allInstances.erase(nogapInstanceId);
}
