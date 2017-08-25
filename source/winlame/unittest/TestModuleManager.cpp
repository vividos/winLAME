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
/// \file TestModuleManager.cpp
/// \brief Unit tests for the ModuleManager interface and ModuleManagerImpl class

#include "stdafx.h"
#include "CppUnitTest.h"
#include "ModuleInterface.hpp"
#include "ModuleManager.hpp"
#include "ModuleManagerImpl.hpp"
#include <ulib/IoCContainer.hpp>
#include "LameNogapInstanceManager.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unittest
{
   /// tests for ModuleManager class
   TEST_CLASS(TestModuleManager)
   {
      /// LAME nogap instance manager
      static std::shared_ptr<Encoder::LameNogapInstanceManager> m_spLameNogapInstanceManager;

   public:
      /// sets up test; called before each test
      TEST_CLASS_INITIALIZE(SetUp)
      {
         // register objects in IoC container
         IoCContainer& ioc = IoCContainer::Current();

         m_spLameNogapInstanceManager.reset(new Encoder::LameNogapInstanceManager);
         ioc.Register<Encoder::LameNogapInstanceManager>(boost::ref(*m_spLameNogapInstanceManager.get()));
      }

      /// tests default ctor of ModuleManagerImpl
      TEST_METHOD(TestDefaultCtor)
      {
         Encoder::ModuleManagerImpl moduleManager;
      }

      /// tests all input modules by accessing and instantiating them
      TEST_METHOD(TestInputModules)
      {
         Encoder::ModuleManagerImpl moduleManager;

         int countModules = moduleManager.GetInputModuleCount();

         Assert::IsTrue(countModules > 0, _T("there must be at least one input module"));

         for (int moduleIndex = 0; moduleIndex < countModules; moduleIndex++)
         {
            CString name = moduleManager.GetInputModuleName(moduleIndex);
            Assert::IsFalse(name.IsEmpty(), _T("input module name name must not be empty"));

            CString filter;
            if (name != _T("CD Audio Extraction"))
            {
               filter = moduleManager.GetInputModuleFilterString(moduleIndex);
               Assert::IsFalse(filter.IsEmpty(), _T("filter string must not be empty"));
            }

            int moduleID = moduleManager.GetInputModuleID(moduleIndex);

            CString versionText;
            moduleManager.GetModuleVersionString(versionText, moduleID, 0);
            Assert::IsFalse(versionText.IsEmpty(), _T("version text must not be empty"));

            Encoder::InputModule* pInputModule = moduleManager.GetInputModuleInstance(moduleIndex);

            Assert::IsNotNull(pInputModule, _T("input module ptr must not be null"));

            Encoder::InputModule& inputModule = *pInputModule;

            // getting description must not crash, even when no file was loaded yet
            try
            {
               CString description = inputModule.GetDescription();
            }
            catch (...)
            {
               Assert::Fail(_T("getting description must not throw an exception"));
            }

            if (name != _T("CD Audio Extraction"))
            {
               CString filter2 = inputModule.GetFilterString();
               Assert::IsFalse(filter2.IsEmpty(), _T("filter string must not be empty"));
               Assert::AreEqual(filter.GetString(), filter2.GetString(), _T("filter strings must match"));
            }
         }
      }
   };

   /// instance of static LAME NoGap instance manager
   std::shared_ptr<Encoder::LameNogapInstanceManager> TestModuleManager::m_spLameNogapInstanceManager;
}
