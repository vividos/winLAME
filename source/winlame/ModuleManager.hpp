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
/// \file ModuleManager.hpp
/// \brief Module Manager interface
//
#pragma once

namespace Encoder
{
   class InputModule;

   /// module manager interface
   class ModuleManager
   {
   public:
      /// dtor
      virtual ~ModuleManager() {}

      // info functions

      /// returns currently available filter string for open file dialog
      virtual void GetFilterString(CString& filterstring) const = 0;

      /// returns infos about audio file; returns false when not supported
      virtual bool GetAudioFileInfo(LPCTSTR filename,
         int& lengthInSeconds, int& bitrateInBps, int& samplerateInHz, CString& errorMessage) = 0;

      // module functions

      /// returns the number of available input modules
      virtual size_t GetInputModuleCount() const = 0;

      /// returns the name of the input module
      virtual CString GetInputModuleName(size_t index) const = 0;

      /// returns the input module ID
      virtual int GetInputModuleID(size_t index) const = 0;

      /// returns the input module filter string
      virtual CString GetInputModuleFilterString(size_t index) const = 0;

      /// returns the input module filter string
      virtual InputModule* GetInputModuleInstance(size_t index) = 0;

      /// returns the number of available output modules
      virtual size_t GetOutputModuleCount() = 0;

      /// returns the name of the output module
      virtual CString GetOutputModuleName(size_t index) = 0;

      /// returns the output module ID
      virtual int GetOutputModuleID(size_t index) = 0;

      /// retrieves a module version string
      virtual void GetModuleVersionString(CString& version, int m_moduleId, int special = 0) = 0;
   };

} // namespace Encoder
