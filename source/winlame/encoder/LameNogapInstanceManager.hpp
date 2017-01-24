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
/// \file LameNogapInstanceManager.hpp
/// \brief Manager for LAME nogap encoding instances
//
#pragma once

#include "nlame.h"
#include <map>

namespace Encoder
{
   /// instance manager for nlame instances used for nogap encoding
   class LameNogapInstanceManager
   {
   public:
      /// ctor
      LameNogapInstanceManager();

      /// returns next free nogap instance ID
      int NextNogapInstanceId();

      /// returns if a nogap instance ID is already registered
      bool IsRegistered(int nogapInstanceId) const;

      /// registers an nlame instance using a nogap instance ID
      void RegisterInstance(int nogapInstanceId, nlame_instance_t* instance);

      /// returns saved nlame instance for nogap encoding
      nlame_instance_t* GetInstance(int nogapInstanceId);

      /// unregisters an instance again
      void UnregisterInstance(int nogapInstanceId);

   private:
      /// next nogap instance ID
      static int s_nextNogapInstanceId;

      /// mapping from instance ID to instance
      std::map<int, nlame_instance_t*> m_allInstances;
   };

} // namespace Encoder
