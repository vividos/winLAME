//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2026 Michael Fink
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
/// \file TestEncodeDecodeFlac.cpp
/// \brief Tests encoding to FLAC and decoding back again to wave

#include "stdafx.h"
#include "CppUnitTest.h"
#include "EncoderTestFixture.hpp"
#include <ulib/Path.hpp>
#include <ulib/unittest/AutoCleanupFolder.hpp>
#include "resource_unittest.h"
#include "EncoderImpl.hpp"
#include "ModuleManager.hpp"
#include "ModuleManagerImpl.hpp"
#include <sndfile.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unittest
{
   /// tests for encoding and decoding FLAC format
   TEST_CLASS(TestEncodeDecodeFlac), public EncoderTestFixture
   {
   public:
      /// sets up test; called before each test
      TEST_CLASS_INITIALIZE(SetUp)
      {
         EncoderTestFixture::SetUp();
      }

      /// tests encoding wave to FLAC to wave
      TEST_METHOD(TestEncodeDecode)
      {
         UnitTest::AutoCleanupFolder folder;

         CString originalFilename = Path::Combine(folder.FolderName(), _T("sample.wav"));
         ExtractFromResource(IDR_SAMPLE_WAV, originalFilename);

         // encode file
         CString encodedFilename;
         {
            Encoder::EncoderImpl encoder;

            Encoder::EncoderSettings encoderSettings;
            encoderSettings.m_inputFilename = originalFilename;
            encoderSettings.m_outputFilename = Path::Combine(folder.FolderName(), _T("encoded.flac"));
            encoderSettings.m_outputModuleID = ID_OM_WAVE; // encode to FLAC using libsndfile

            encoder.SetEncoderSettings(encoderSettings);

            SettingsManager settingsManager;
            settingsManager.setValue(SndFileFormat, SF_FORMAT_FLAC);
            //settingsManager.setValue(SndFileSubType, SF_FORMAT_PCM_S8);
            settingsManager.setValue(SndFileSubType, SF_FORMAT_PCM_16);
            //settingsManager.setValue(SndFileSubType, SF_FORMAT_PCM_24);
            encoder.SetSettingsManager(&settingsManager);

            StartEncodeAndWaitForFinish(encoder);

            // output file must exist
            Assert::IsTrue(Path::FileExists(encoderSettings.m_outputFilename), _T("output file must exist"));

            encodedFilename = encoderSettings.m_outputFilename;
         }

         // decode back to wave
         CString decodedFilename;
         {
            Encoder::EncoderImpl decoder;

            Encoder::EncoderSettings decoderSettings;
            decoderSettings.m_inputFilename = encodedFilename;
            decoderSettings.m_outputFilename = Path::Combine(folder.FolderName(), _T("decoded.wav"));
            decoderSettings.m_outputModuleID = ID_OM_WAVE; // encode to WAVE using libsndfile

            decoder.SetEncoderSettings(decoderSettings);

            SettingsManager settingsManagerDecoder;
            settingsManagerDecoder.setValue(SndFileFormat, SF_FORMAT_WAV);
            settingsManagerDecoder.setValue(SndFileSubType, SF_FORMAT_PCM_16);
            decoder.SetSettingsManager(&settingsManagerDecoder);

            StartEncodeAndWaitForFinish(decoder);

            // output file must exist
            Assert::IsTrue(Path::FileExists(decoderSettings.m_outputFilename), _T("output file must exist"));

            decodedFilename = decoderSettings.m_outputFilename;
         }

         // file contents of originalFilename and decodedFilename must match
      }
   };
}
