//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file DynamicLibrary.hpp
/// \brief dynamic library loading
//
#pragma once

/// dynamic library loading
class DynamicLibrary
{
public:
   /// ctor; loads module
   DynamicLibrary(LPCTSTR moduleFilename)
      :m_module(LoadLibrary(moduleFilename))
   {
   }

   /// dtor; frees module again
   ~DynamicLibrary()
   {
      FreeLibrary(m_module);
   }

   /// checks if library is loaded
   bool IsLoaded() const { return m_module != nullptr; }

   /// checks if function with given name is available
   bool IsFunctionAvail(LPCSTR functionName)
   {
      return GetProcAddress(m_module, functionName) != nullptr;
   }

   /// returns function with given function name and given function signature
   template <typename Signature>
   Signature GetFunction(LPCSTR functionName)
   {
      return reinterpret_cast<Signature>(GetProcAddress(m_module, functionName));
   }

private:
   /// dynamic library module handle
   HMODULE m_module;
};
