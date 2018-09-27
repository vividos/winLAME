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
/// \file TestEncodeMp3ToOggVorbis.cpp
/// \brief Tests encoding an mp3 input file with the Ogg Vorbis encoder

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
#include "TestEncoderErrorHandler.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unittest
{
   /// tests for encoding to Ogg Vorbis format
   TEST_CLASS(TestEncodeMp3ToOggVorbis)
   {
      /// LAME nogap instance manager
      static std::shared_ptr<Encoder::LameNogapInstanceManager> m_spLameNogapInstanceManager;

      /// module manager
      static std::shared_ptr<Encoder::ModuleManager> m_spModuleManager;

      /// encoding error handler
      static TestEncoderErrorHandler m_encodingErrorHandler;

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

      /// tests encoding wave file to Opus
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
         encoderSettings.m_outputFilename = Path::Combine(folder.FolderName(), _T("output.ogg")).ToString();
         encoderSettings.m_outputModuleID = ID_OM_OGGV; // encode to Ogg Vorbis

         encoder.SetEncoderSettings(encoderSettings);

         SettingsManager settingsManager;
         settingsManager.setValue(OggBitrateMode, 0); // base quality mode
         settingsManager.setValue(OggBaseQuality, 400);
         encoder.SetSettingsManager(&settingsManager);

         encoder.SetErrorHandler(&m_encodingErrorHandler);

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
   std::shared_ptr<Encoder::LameNogapInstanceManager> TestEncodeMp3ToOggVorbis::m_spLameNogapInstanceManager;

   /// instance of static module manager
   std::shared_ptr<Encoder::ModuleManager> TestEncodeMp3ToOggVorbis::m_spModuleManager;

   /// instance of static encoding error handler
   TestEncoderErrorHandler TestEncodeMp3ToOggVorbis::m_encodingErrorHandler;
}
