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
/// \file TestEncodeWaveToOpus.cpp
/// \brief Tests encoding a wave input file with the Opus encoder

#include "stdafx.h"
#include "CppUnitTest.h"
#include "EncoderTestFixture.hpp"
#include <ulib/Path.hpp>
#include <ulib/unittest/AutoCleanupFolder.hpp>
#include "resource_unittest.h"
#include "EncoderImpl.hpp"
#include "ModuleManager.hpp"
#include "ModuleManagerImpl.hpp"
#include "TestEncoderErrorHandler.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unittest
{
   /// tests for encoding to Opus format
   TEST_CLASS(TestEncodeWaveToOpus), public EncoderTestFixture
   {
   public:
      /// sets up test; called before each test
      TEST_CLASS_INITIALIZE(SetUp)
      {
         EncoderTestFixture::SetUp();
      }

      /// tests encoding wave file to Opus
      TEST_METHOD(TestEncode)
      {
         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.wav")).ToString();
         ExtractFromResource(IDR_SAMPLE_WAV, filename);

         // encode file
         Encoder::EncoderImpl encoder;

         Encoder::EncoderSettings encoderSettings;
         encoderSettings.m_inputFilename = filename;
         encoderSettings.m_outputFilename = Path::Combine(folder.FolderName(), _T("output.opus")).ToString();
         encoderSettings.m_outputModuleID = ID_OM_OPUS; // encode to Opus

         encoder.SetEncoderSettings(encoderSettings);

         SettingsManager settingsManager;
         settingsManager.setValue(OpusTargetBitrate, 256); // in kbps
         settingsManager.setValue(OpusComplexity, 10);
         settingsManager.setValue(OpusBitrateMode, 0); // --vbr

         encoder.SetSettingsManager(&settingsManager);

         StartEncodeAndWaitForFinish(encoder);

         // output file must exist
         Assert::IsTrue(Path(encoderSettings.m_outputFilename).FileExists(), _T("output file must exist"));
      }
   };
}
