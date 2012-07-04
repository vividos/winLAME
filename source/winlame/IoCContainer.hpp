//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2012 Michael Fink
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
/// \file IoCContainer.hpp
/// \brief Inversion of Control container

// include guard
#pragma once

// includes
#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <boost/ref.hpp>

/// inversion of control container
class IoCContainer: boost::noncopyable
{
public:
   /// returns current instance of IoC container
   static IoCContainer& Current()
   {
      static IoCContainer current;
      return current;
   }

   /// registers ref for class
   template <typename TClass>
   void Register(boost::reference_wrapper<TClass> ref)
   {
      std::string name = typeid(TClass).raw_name();
      m_mapAllInstances.insert(std::make_pair(name, ref));
   }

   /// resolves class to object
   template <typename TInterface>
   TInterface& Resolve()
   {
      std::string name = typeid(TInterface).raw_name();

      T_mapAllInstances::iterator iter = m_mapAllInstances.find(name);

      if (iter == m_mapAllInstances.end())
         throw std::runtime_error(std::string("class not registered: ") + typeid(TInterface).name());

      boost::reference_wrapper<TInterface> ref =
         boost::any_cast<boost::reference_wrapper<TInterface> >(iter->second);

      return ref.get();
   }

private:
   /// instance map type
   typedef std::map<std::string, boost::any> T_mapAllInstances;

   /// instance map
   T_mapAllInstances m_mapAllInstances;
};
