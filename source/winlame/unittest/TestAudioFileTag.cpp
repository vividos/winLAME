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
/// \file TestAudioFileTag.cpp
/// \brief Tests class AudioFileTag

#include "stdafx.h"
#include "CppUnitTest.h"
#include <ulib/Path.hpp>
#include <ulib/unittest/AutoCleanupFolder.hpp>
#include "resource_unittest.h"
#include <ulib/win32/ResourceData.hpp>
#include "TrackInfo.hpp"
#include "AudioFileTag.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unittest
{
   /// tests for AudioFileTag class
   TEST_CLASS(TestAudioFileTag)
   {
   public:
      /// tests ReadFromFile() method
      TEST_METHOD(TestReadTrackInfo)
      {
         // set up
         HINSTANCE hInstance = g_hDllInstance;
         Win32::ResourceData data(MAKEINTRESOURCE(IDR_SAMPLE_MP3), _T("\"RT_RCDATA\""), hInstance);

         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.mp3")).ToString();
         data.AsFile(filename);

         // run
         Encoder::TrackInfo trackInfo;
         Encoder::AudioFileTag tag(trackInfo);

         Assert::IsTrue(tag.ReadFromFile(filename), _T("reading from file must succeed"));

         // check
         bool isAvail = false;

         CString textValue = trackInfo.TextInfo(Encoder::TrackInfoTitle, isAvail);
         Assert::IsTrue(isAvail && !textValue.IsEmpty(), _T("tag value must have been read"));

         textValue = trackInfo.TextInfo(Encoder::TrackInfoArtist, isAvail);
         Assert::IsTrue(isAvail && !textValue.IsEmpty(), _T("tag value must have been read"));

         textValue = trackInfo.TextInfo(Encoder::TrackInfoDiscArtist, isAvail);
         Assert::IsTrue(isAvail && !textValue.IsEmpty(), _T("tag value must have been read"));

         textValue = trackInfo.TextInfo(Encoder::TrackInfoAlbum, isAvail);
         Assert::IsTrue(isAvail && !textValue.IsEmpty(), _T("tag value must have been read"));

         textValue = trackInfo.TextInfo(Encoder::TrackInfoComment, isAvail);
         Assert::IsTrue(isAvail && !textValue.IsEmpty(), _T("tag value must have been read"));

         textValue = trackInfo.TextInfo(Encoder::TrackInfoGenre, isAvail);
         Assert::IsTrue(isAvail && !textValue.IsEmpty(), _T("tag value must have been read"));

         textValue = trackInfo.TextInfo(Encoder::TrackInfoComposer, isAvail);
         Assert::IsTrue(isAvail && !textValue.IsEmpty(), _T("tag value must have been read"));

         const Encoder::TrackInfo& constTrackInfo = trackInfo;
         int intValue = constTrackInfo.NumberInfo(Encoder::TrackInfoYear, isAvail);
         Assert::IsTrue(isAvail && intValue != 0, _T("tag value must have been read"));

         intValue = constTrackInfo.NumberInfo(Encoder::TrackInfoTrack, isAvail);
         Assert::IsTrue(isAvail && intValue != 0, _T("tag value must have been read"));

         // note: Encoder::TrackInfoDiscNumber is not set in the sample mp3 file
         //intValue = constTrackInfo.NumberInfo(Encoder::TrackInfoDiscNumber, isAvail);
         //Assert::IsTrue(isAvail && intValue != 0, _T("tag value must have been read"));

         std::vector<unsigned char> binaryInfo;
         isAvail = constTrackInfo.BinaryInfo(Encoder::TrackInfoFrontCover, binaryInfo);
         Assert::IsTrue(isAvail && !binaryInfo.empty(), _T("tag value must have been read"));
      }

      /// tests GetTagLength() method
      TEST_METHOD(TestGetTagLength)
      {
         // set up
         HINSTANCE hInstance = g_hDllInstance;
         Win32::ResourceData data(MAKEINTRESOURCE(IDR_SAMPLE_MP3), _T("\"RT_RCDATA\""), hInstance);

         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.mp3")).ToString();
         data.AsFile(filename);

         // run
         Encoder::TrackInfo trackInfo;
         Encoder::AudioFileTag tag(trackInfo);

         tag.ReadFromFile(filename);

         Assert::IsTrue(tag.GetTagLength() > 0, _T("tag length must be > 0"));
      }

      /// tests WriteToFile() method
      TEST_METHOD(TestWriteToFile)
      {
         // set up
         HINSTANCE hInstance = g_hDllInstance;
         Win32::ResourceData data(MAKEINTRESOURCE(IDR_SAMPLE_MP3), _T("\"RT_RCDATA\""), hInstance);

         UnitTest::AutoCleanupFolder folder;

         CString filename = Path::Combine(folder.FolderName(), _T("sample.mp3")).ToString();
         data.AsFile(filename);

         // run
         Encoder::TrackInfo trackInfo;
         Encoder::AudioFileTag tag(trackInfo);

         Assert::IsTrue(tag.ReadFromFile(filename), _T("reading from file must succeed"));
         Assert::IsTrue(tag.WriteToFile(filename), _T("writing to file must succeed"));;
      }
   };
}
