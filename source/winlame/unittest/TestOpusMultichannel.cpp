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
/// \file TestOpusMultichannel.cpp
/// \brief Tests decoding and encoding Opus multichannel files

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
   /// tests for decoding and encoding Opus multichannel files
   TEST_CLASS(TestOpusMultichannel), public EncoderTestFixture
   {
   public:
      /// sets up test; called before each test
      TEST_CLASS_INITIALIZE(SetUp)
      {
         EncoderTestFixture::SetUp();
      }

      /// tests decoding Opus multichannel file
      TEST_METHOD(TestDecodeOpusMultichannelFile)
      {
         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.opus")).ToString();
         ExtractFromResource(IDR_SAMPLE_OPUS_MULTICHANNEL, filename);

         // decode file
         Encoder::EncoderImpl encoder;

         Encoder::EncoderSettings encoderSettings;
         encoderSettings.m_inputFilename = filename;
         encoderSettings.m_outputFilename = Path::Combine(folder.FolderName(), _T("output.wav")).ToString();
         encoderSettings.m_outputModuleID = ID_OM_WAVE; // decode to Wave

         encoder.SetEncoderSettings(encoderSettings);

         SettingsManager settingsManager;
         settingsManager.setValue(SndFileFormat, SF_FORMAT_WAV);
         settingsManager.setValue(SndFileSubType, SF_FORMAT_PCM_16);

         encoder.SetSettingsManager(&settingsManager);

         StartEncodeAndWaitForFinish(encoder);

         // output file must exist
         Assert::IsTrue(Path(encoderSettings.m_outputFilename).FileExists(), _T("output file must exist"));

         int inputNumChannels = 0;
         int outputNumChannels = 0;
         int dummy = 0;
         GetAudioFileInfos(filename, inputNumChannels, dummy, dummy, dummy);
         GetAudioFileInfos(encoderSettings.m_outputFilename, outputNumChannels, dummy, dummy, dummy);

         Assert::AreEqual(6, outputNumChannels, _T("output file must have 6 channels"));
         Assert::AreEqual(inputNumChannels, outputNumChannels, _T("output file must have same number of channels as input file"));
      }

      /// tests transcoding Opus multichannel file to Opus, forcing it to use downmix by using a
      /// very low bitrate value
      TEST_METHOD(TestTranscodeOpusMultichannelWithDownmix)
      {
         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.opus")).ToString();
         ExtractFromResource(IDR_SAMPLE_OPUS_MULTICHANNEL, filename);

         // transcode file
         Encoder::EncoderImpl encoder;

         Encoder::EncoderSettings encoderSettings;
         encoderSettings.m_inputFilename = filename;
         encoderSettings.m_outputFilename = Path::Combine(folder.FolderName(), _T("output.opus")).ToString();
         encoderSettings.m_outputModuleID = ID_OM_OPUS;

         encoder.SetEncoderSettings(encoderSettings);

         SettingsManager settingsManager;
         settingsManager.setValue(OpusTargetBitrate, 15 * 6); // use low bitrate of 15 kbps/channel to force downmix
         settingsManager.setValue(OpusComplexity, 10);
         settingsManager.setValue(OpusBitrateMode, 0); // --vbr

         encoder.SetSettingsManager(&settingsManager);

         StartEncodeAndWaitForFinish(encoder);

         // output file must exist
         Assert::IsTrue(Path(encoderSettings.m_outputFilename).FileExists(), _T("output file must exist"));

         int outputNumChannels = 0;
         int dummy = 0;
         GetAudioFileInfos(encoderSettings.m_outputFilename, outputNumChannels, dummy, dummy, dummy);

         Assert::AreEqual(2, outputNumChannels, _T("output file must have been downmixed to 2 channels"));
      }
   };
}
