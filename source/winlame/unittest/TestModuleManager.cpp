//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
#include <ulib/Path.hpp>
#include <ulib/unittest/AutoCleanupFolder.hpp>
#include "resource_unittest.h"
#include <ulib/win32/ResourceData.hpp>

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

            Assert::IsTrue(inputModule.GetLastError().IsEmpty(), _T("last error must be empty after creating new module"));
            Assert::IsFalse(inputModule.GetModuleName().IsEmpty(), _T("module name must not be empty"));

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

      /// tests all output modules by accessing and instantiating them
      TEST_METHOD(TestOutputModules)
      {
         Encoder::ModuleManagerImpl moduleManager;

         int countModules = moduleManager.GetOutputModuleCount();

         Assert::IsTrue(countModules > 0, _T("there must be at least one output module"));

         for (int moduleIndex = 0; moduleIndex < countModules; moduleIndex++)
         {
            CString name = moduleManager.GetOutputModuleName(moduleIndex);
            Assert::IsFalse(name.IsEmpty(), _T("output module name name must not be empty"));

            int moduleID = moduleManager.GetOutputModuleID(moduleIndex);

            CString versionText;
            moduleManager.GetModuleVersionString(versionText, moduleID, 0);
            Assert::IsFalse(versionText.IsEmpty(), _T("version text must not be empty"));

            Encoder::OutputModule* pOutputModule = moduleManager.GetOutputModule(moduleID);

            Assert::IsNotNull(pOutputModule, _T("output module ptr must not be null"));

            Encoder::OutputModule& outputModule = *pOutputModule;

            Assert::IsTrue(outputModule.GetLastError().IsEmpty(), _T("last error must be empty after creating new module"));
            Assert::IsFalse(outputModule.GetModuleName().IsEmpty(), _T("module name must not be empty"));

            // getting description must not crash, even when no file was loaded yet
            try
            {
               CString description = outputModule.GetDescription();
            }
            catch (...)
            {
               Assert::Fail(_T("getting description must not throw an exception"));
            }

            CString extension = outputModule.GetOutputExtension();
            Assert::IsFalse(extension.IsEmpty(), _T("output extension string must not be empty"));
         }
      }

      /// Tests getting combined filter string
      TEST_METHOD(TestGetFilterString)
      {
         // run
         Encoder::ModuleManagerImpl moduleManager;

         CString filterstring;
         moduleManager.GetFilterString(filterstring);

         // check
         Assert::IsFalse(filterstring.IsEmpty(), _T("filter string must not be empty"));
      }

      /// Tests getting audio file infos
      TEST_METHOD(GetAudioFileInfo)
      {
         // set up
         HINSTANCE hInstance = g_hDllInstance;
         Win32::ResourceData data(MAKEINTRESOURCE(IDR_SAMPLE_MP3), _T("\"RT_RCDATA\""), hInstance);

         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.mp3")).ToString();
         data.AsFile(filename);

         // run
         Encoder::ModuleManagerImpl moduleManager;

         int lengthInSeconds = 0;
         int bitrateInBps = 0;
         int samplerateInHz = 0;
         CString errorMessage;

         moduleManager.GetAudioFileInfo(filename, lengthInSeconds, bitrateInBps, samplerateInHz, errorMessage);

         // check
         Assert::AreEqual(10, lengthInSeconds, _T("length must be 10 seconds"));
         Assert::AreEqual(128000, bitrateInBps, _T("bitrate must be 128 kbps"));
         Assert::AreEqual(44100, samplerateInHz, _T("sample rate must be 44100 Hz"));
         Assert::IsTrue(errorMessage.IsEmpty(), _T("error message must be empty"));
      }
   };

   /// instance of static LAME NoGap instance manager
   std::shared_ptr<Encoder::LameNogapInstanceManager> TestModuleManager::m_spLameNogapInstanceManager;
}
