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
/// \file TestTransportMetadata.cpp
/// \brief Tests transporting metadata like ID3v2 tags from input to output file

#include "stdafx.h"
#include "CppUnitTest.h"
#include "EncoderTestFixture.hpp"
#include <ulib/Path.hpp>
#include <ulib/unittest/AutoCleanupFolder.hpp>
#include "resource_unittest.h"
#include "EncoderImpl.hpp"
#include "ModuleManager.hpp"
#include "ModuleManagerImpl.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unittest
{
   /// tests for transporting metadata
   TEST_CLASS(TestTransportMetadata), public EncoderTestFixture
   {
      std::vector<std::tuple<UINT, LPCTSTR>> inputFilesList =
      {
         std::make_tuple(IDR_SAMPLE_MP3, _T("sample.mp3")),
         std::make_tuple(IDR_SAMPLE_WAV, _T("sample.wav")),
         std::make_tuple(IDR_SAMPLE_OPUS, _T("sample.opus")),
         std::make_tuple(IDR_SAMPLE_OGGV, _T("sample.ogg")),
         //std::make_tuple(IDR_SAMPLE_AAC, _T("sample.aac")), // bugged file
         std::make_tuple(IDR_SAMPLE_WMA, _T("sample.wma")),
         std::make_tuple(IDR_SAMPLE_FLAC, _T("sample.flac")),
         std::make_tuple(IDR_SAMPLE_AIFF, _T("sample.aiff")),
         std::make_tuple(IDR_SAMPLE_SPEEX, _T("sample.spx")),
         std::make_tuple(IDR_SAMPLE_MONKEYS_AUDIO, _T("sample.ape")),
      };

      std::vector<std::tuple<int, LPCTSTR>> outputFilesList =
      {
         std::make_tuple(ID_OM_LAME, _T("output.mp3")),
         std::make_tuple(ID_OM_OGGV, _T("output.ogg")),
         //std::make_tuple(ID_OM_WAVE, _T("output.wav")), // not supported writing tags to .wav
         //std::make_tuple(ID_OM_AAC, _T("output.aac")), // not supported writing tags to .aac
         std::make_tuple(ID_OM_BASSWMA, _T("output.wma")),
         std::make_tuple(ID_OM_OPUS, _T("output.opus")),
      };

   public:
      /// sets up test; called before each test
      TEST_CLASS_INITIALIZE(SetUp)
      {
         EncoderTestFixture::SetUp();
      }

      /// tests transporting metadata
      TEST_METHOD(TestTransport)
      {
         int failedCount = 0;
         for (auto inputInfos : inputFilesList)
         {
            for (auto outputInfos : outputFilesList)
            {
               ATLTRACE(_T("Encoding from %s to %s...\n"),
                  std::get<1>(inputInfos),
                  std::get<1>(outputInfos));

               bool result = EncodeAndTransportMetadata(inputInfos, outputInfos);
               ATLTRACE(_T("Encoding from %s to %s: %s\n"),
                  std::get<1>(inputInfos),
                  std::get<1>(outputInfos),
                  result ? _T("succeeded to transport metadata") : _T("failed to transport all metadata"));

               if (!result)
                  failedCount++;
            }
         }

         Assert::AreEqual(0, failedCount, _T("transporting must not have failed once"));
      }

      /// encodes input file to output file and checks if metadata was transported correctly
      bool EncodeAndTransportMetadata(std::tuple<UINT, LPCTSTR> inputInfos, std::tuple<int, LPCTSTR> outputInfos)
      {
         UnitTest::AutoCleanupFolder folder;

         CString inputFilename = Path::Combine(folder.FolderName(), std::get<1>(inputInfos)).ToString();
         ExtractFromResource(std::get<0>(inputInfos), inputFilename);

         Encoder::TrackInfo inputTrackInfo = GetTrackInfo(inputFilename);

         CString outputFilename = Path::Combine(folder.FolderName(), std::get<1>(outputInfos)).ToString();

         // encode file
         Encode(inputFilename, outputFilename, std::get<0>(outputInfos));

         // output file must exist
         Assert::IsTrue(Path(outputFilename).FileExists(), _T("output file must exist"));

         Encoder::TrackInfo outputTrackInfo = GetTrackInfo(outputFilename);

         return CheckSameTrackInfos(inputTrackInfo, outputTrackInfo);
      }

      /// encodes a file with default settings, using specified output module
      void Encode(const CString& inputFilename, const CString outputFilename, int outputModuleID)
      {
         Encoder::EncoderImpl encoder;

         Encoder::EncoderSettings encoderSettings;
         encoderSettings.m_inputFilename = inputFilename;
         encoderSettings.m_outputFilename = outputFilename;
         encoderSettings.m_outputModuleID = outputModuleID;

         encoder.SetEncoderSettings(encoderSettings);

         // use default settings
         SettingsManager settingsManager;
         encoder.SetSettingsManager(&settingsManager);

         StartEncodeAndWaitForFinish(encoder);
      }

      /// compares metadata of input and output tracks; output track info must contain all infos from input track
      bool CheckSameTrackInfos(const Encoder::TrackInfo& inputTrackInfo, const Encoder::TrackInfo& outputTrackInfo)
      {
         if (inputTrackInfo.IsEmpty())
            return true;

         bool isAvailInput = false;
         bool isAvailOutput = false;

         std::vector<Encoder::TrackInfoNumberType> numberTypesList =
         {
            Encoder::TrackInfoYear,
            Encoder::TrackInfoTrack,
            Encoder::TrackInfoDiscNumber,
         };

         for (Encoder::TrackInfoNumberType numberType : numberTypesList)
         {
            int numberInfo = inputTrackInfo.GetNumberInfo(numberType, isAvailInput);

            if (isAvailInput &&
               (numberInfo != outputTrackInfo.GetNumberInfo(numberType, isAvailOutput) ||
               isAvailInput != isAvailOutput))
            {
               return false;
            }
         }

         std::vector<Encoder::TrackInfoTextType> textTypesList =
         {
            Encoder::TrackInfoTitle,
            Encoder::TrackInfoArtist,
            Encoder::TrackInfoDiscArtist,
            Encoder::TrackInfoAlbum,
            Encoder::TrackInfoComment,
            Encoder::TrackInfoGenre,
            Encoder::TrackInfoComposer,
         };

         for (Encoder::TrackInfoTextType textType : textTypesList)
         {
            CString textInfo = inputTrackInfo.GetTextInfo(textType, isAvailInput);

            if (isAvailInput &&
               (textInfo != outputTrackInfo.GetTextInfo(textType, isAvailOutput) ||
                  isAvailInput != isAvailOutput))
            {
               return false;
            }
         }

         std::vector<unsigned char> inputBinaryInfo, outputBinaryInfo;
         isAvailInput = inputTrackInfo.GetBinaryInfo(Encoder::TrackInfoBinaryType::TrackInfoFrontCover, inputBinaryInfo);
         isAvailOutput = outputTrackInfo.GetBinaryInfo(Encoder::TrackInfoBinaryType::TrackInfoFrontCover, outputBinaryInfo);

         if (isAvailInput &&
            (isAvailInput != isAvailOutput || inputBinaryInfo != outputBinaryInfo))
         {
            return false;
         }

         return true;
      }
   };
}
