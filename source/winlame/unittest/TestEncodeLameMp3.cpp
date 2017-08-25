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
/// \file TestEncodeLameMp3.cpp
/// \brief Tests encoding an input file with the LAME mp3 encoder

#include "stdafx.h"
#include "CppUnitTest.h"
#include <ulib/IoCContainer.hpp>
#include "LameNogapInstanceManager.hpp"
#include <ulib/Path.hpp>
#include <ulib/unittest/AutoCleanupFolder.hpp>
#include "resource_unittest.h"
#include <ulib/win32/ResourceData.hpp>
#include "EncoderImpl.hpp"
#include "ModuleManager.hpp"
#include "ModuleManagerImpl.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unittest
{
   /// tests for encoding mp3 files
   TEST_CLASS(TestEncodeLameMp3)
   {
      /// LAME nogap instance manager
      static std::shared_ptr<Encoder::LameNogapInstanceManager> m_spLameNogapInstanceManager;

      /// module manager
      static std::shared_ptr<Encoder::ModuleManager> m_spModuleManager;

   public:
      /// sets up test; called before each test
      TEST_CLASS_INITIALIZE(SetUp)
      {
         // register objects in IoC container
         IoCContainer& ioc = IoCContainer::Current();

         m_spLameNogapInstanceManager.reset(new Encoder::LameNogapInstanceManager);
         ioc.Register<Encoder::LameNogapInstanceManager>(boost::ref(*m_spLameNogapInstanceManager.get()));

         m_spModuleManager.reset(new Encoder::ModuleManagerImpl);
         ioc.Register<Encoder::ModuleManager>(boost::ref(*m_spModuleManager.get()));
      }

      /// tests default ctor of ModuleManagerImpl
      TEST_METHOD(TestEncode)
      {
         HINSTANCE hInstance = g_hDllInstance;
         Win32::ResourceData data(MAKEINTRESOURCE(IDR_SAMPLE_MP3), _T("\"RT_RCDATA\""), hInstance);

         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.mp3")).ToString();
         data.AsFile(filename);

         // encode file
         Encoder::EncoderImpl encoder;

         Encoder::EncoderSettings encoderSettings;
         encoderSettings.m_inputFilename = filename;
         encoderSettings.m_outputFilename = Path::Combine(folder.FolderName(), _T("output.mp3")).ToString();
         encoderSettings.m_outputModuleID = ID_OM_LAME; // encode to LAME mp3

         encoder.SetEncoderSettings(encoderSettings);

         SettingsManager settingsManager;
         settingsManager.setValue(LameSimpleQualityOrBitrate, 0);
         settingsManager.setValue(LameSimpleEncodeQuality, 1);
         settingsManager.setValue(LameSimpleQuality, 4);

         encoder.SetSettingsManager(&settingsManager);

         //encoder.SetErrorHandler(); // TODO

         encoder.StartEncode();

         // wait for encoding to finish
         bool isRunning = false;
         do
         {
            Encoder::EncoderState state = encoder.GetEncoderState();
            isRunning = state.m_running;

         } while (isRunning);

         // output file must exist
         Assert::IsTrue(Path(encoderSettings.m_outputFilename).FileExists(), _T("output file must exist"));
      }
   };

   /// instance of static LAME NoGap instance manager
   std::shared_ptr<Encoder::LameNogapInstanceManager> TestEncodeLameMp3::m_spLameNogapInstanceManager;

   /// instance of static module manager
   std::shared_ptr<Encoder::ModuleManager> TestEncodeLameMp3::m_spModuleManager;
}
