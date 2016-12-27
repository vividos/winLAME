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
/// \file ModuleManagerImpl.hpp
/// \brief contains the module manager implementation class definition
//
#pragma once

#include <map>
#include "ModuleManager.hpp"
#include "ModuleInterface.hpp"

namespace Encoder
{
   /// module manager implementation class
   class ModuleManagerImpl : public ModuleManager
   {
   public:
      /// ctor
      ModuleManagerImpl();
      /// dtor
      ~ModuleManagerImpl();

      // info functions

      /// returns currently available filter string for open file dialog
      virtual void GetFilterString(CString& filterstring) const override;

      /// returns infos about audio file; returns false when not supported
      virtual bool GetAudioFileInfo(LPCTSTR filename,
         int& lengthInSeconds, int& bitrateInBps, int& samplerateInHz, CString& errorMessage) override;

      // input module

      /// returns the number of available input modules
      virtual int GetInputModuleCount() const override
      {
         return m_inputModules.size();
      }

      /// returns the name of the input module
      virtual CString GetInputModuleName(int index) const override
      {
         return m_inputModules[index]->GetModuleName();
      }

      /// returns the input module ID
      virtual int GetInputModuleID(int index) const override
      {
         return m_inputModules[index]->GetModuleID();
      }

      /// returns the input module filter string
      virtual CString GetInputModuleFilterString(int index) const override
      {
         return m_inputModules[index]->GetFilterString();
      }

      /// returns input module instance
      virtual InputModule* GetInputModuleInstance(int index) override
      {
         return m_inputModules[index];
      }

      /// chooses an input module suitable for opening file with given filename;
      /// pointer has to be deleted!
      InputModule* ChooseInputModule(LPCTSTR filename);

      // output module

      /// returns the number of available output modules
      virtual int GetOutputModuleCount() override
      {
         return m_outputModules.size();
      }

      /// returns the name of an output module
      virtual CString GetOutputModuleName(int index) override
      {
         return m_outputModules[index]->GetModuleName();
      }

      /// returns the output module ID
      virtual int GetOutputModuleID(int index) override
      {
         return m_outputModules[index]->GetModuleID();
      }

      /// retrieves a module version string
      virtual void GetModuleVersionString(CString& version, int moduleId, int special = 0) override;

      /// returns output module with given module id; pointer has to be deleted!
      OutputModule* GetOutputModule(int moduleId);

   private:
      /// all available output modules
      std::vector<OutputModule*> m_outputModules;

      /// all available input modules
      std::vector<InputModule*> m_inputModules;

      /// output module ID to index mapping
      std::map<int, int> m_mapOutputModuleIdToModuleIndex;
   };

} //namespace Encoder
