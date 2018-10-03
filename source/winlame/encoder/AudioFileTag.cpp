//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2018 Michael Fink
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
/// \file AudioFileTag.cpp
/// \brief represents an audio file tag
/// \details supports reading and writing TrackInfo as tag from and to files
//
#include "stdafx.h"
#include "AudioFileTag.hpp"
#include "TrackInfo.hpp"
#include "ID3/File.h"
#include <id3tag.h>

using Encoder::AudioFileTag;
using Encoder::TrackInfo;

bool AudioFileTag::ReadFromFile(const CString& filename)
{
   ID3::File file(filename, true);

   if (!file.HasID3v2Tag())
      return false;

   // get primary tag
   ID3::Tag tag = file.GetTag();

   return ReadTrackInfoFromTag(tag);
}

bool AudioFileTag::ReadTrackInfoFromTag(ID3::Tag& tag)
{
   // retrieve field values
   CString textValue;
   if (tag.IsFrameAvail(ID3::FrameId::Title))
   {
      textValue = tag.FindFrame(ID3::FrameId::Title).GetString(1);
      m_trackInfo.TextInfo(TrackInfoTitle, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Artist))
   {
      textValue = tag.FindFrame(ID3::FrameId::Artist).GetString(1);
      m_trackInfo.TextInfo(TrackInfoArtist, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::AlbumArtist))
   {
      textValue = tag.FindFrame(ID3::FrameId::AlbumArtist).GetString(1);
      m_trackInfo.TextInfo(TrackInfoDiscArtist, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Composer))
   {
      textValue = tag.FindFrame(ID3::FrameId::Composer).GetString(1);
      m_trackInfo.TextInfo(TrackInfoComposer, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Comment))
   {
      // COMM field is layout differently: 0: ID3_FIELD_TYPE_TEXTENCODING, 1: ID3_FIELD_TYPE_LANGUAGE, 2: ID3_FIELD_TYPE_STRING, 3: ID3_FIELD_TYPE_STRINGFULL
      textValue = tag.FindFrame(ID3::FrameId::Comment).GetString(3);
      m_trackInfo.TextInfo(TrackInfoComment, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::AlbumTitle))
   {
      textValue = tag.FindFrame(ID3::FrameId::AlbumTitle).GetString(1);
      m_trackInfo.TextInfo(TrackInfoAlbum, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::RecordingTime))
   {
      textValue = tag.FindFrame(ID3::FrameId::RecordingTime).GetString(1);
      m_trackInfo.NumberInfo(TrackInfoYear, _ttoi(textValue));
   }

   if (tag.IsFrameAvail(ID3::FrameId::TrackNumber))
   {
      textValue = tag.FindFrame(ID3::FrameId::TrackNumber).GetString(1);
      m_trackInfo.NumberInfo(TrackInfoTrack, _ttoi(textValue));
   }

   if (tag.IsFrameAvail(ID3::FrameId::Genre))
   {
      textValue = tag.FindFrame(ID3::FrameId::Genre).GetString(1);
      if (!textValue.IsEmpty())
         m_trackInfo.TextInfo(TrackInfoGenre, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::AttachedPicture))
   {
      const std::vector<unsigned char> binaryData =
         tag.FindFrame(ID3::FrameId::AttachedPicture).GetBinaryData(4);

      if (!binaryData.empty())
         m_trackInfo.BinaryInfo(TrackInfoFrontCover, binaryData);
   }

   return true;
}

unsigned int AudioFileTag::GetTagLength() const
{
   ID3::Tag tag;

   // set the same flags that mp3tag would use in an ID3v2 tag
   tag.SetOption(ID3::Tag::foUnsynchronisation, 0);
   tag.SetOption(ID3::Tag::foCompression, 0);
   tag.SetOption(ID3::Tag::foCRC, 0);
   tag.SetOption(ID3::Tag::foID3v1, 0);

   StoreTrackInfoInTag(tag);

   return tag.ID3v2TagLength();
}

bool AudioFileTag::WriteToFile(const CString& filename) const
{
   ID3::File file(filename, false); // read-write

   ATLASSERT(file.IsValid());
   if (!file.IsValid())
      return false;

   // get primary tag
   ID3::Tag tag = file.GetTag();

   // set the same flags that mp3tag would use in an ID3v2 tag
   tag.SetOption(ID3::Tag::foUnsynchronisation, 0);
   tag.SetOption(ID3::Tag::foCompression, 0);
   tag.SetOption(ID3::Tag::foCRC, 0);
   tag.SetOption(ID3::Tag::foID3v1, 0);

   StoreTrackInfoInTag(tag);

   return file.Update();
}

void AudioFileTag::StoreTrackInfoInTag(ID3::Tag& tag) const
{
   const TrackInfo& trackInfo = m_trackInfo;

   // add all frames
   bool isAvail = false;
   CString textValue = trackInfo.TextInfo(TrackInfoTitle, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Title);

      ID3::Frame frame(ID3::FrameId::Title);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoArtist, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Artist);

      ID3::Frame frame(ID3::FrameId::Artist);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoDiscArtist, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::AlbumArtist);

      ID3::Frame frame(ID3::FrameId::AlbumArtist);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoComposer, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Composer);

      ID3::Frame frame(ID3::FrameId::Composer);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoComment, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Comment);

      ID3::Frame frame(ID3::FrameId::Comment);
      // COMM field is layout differently: 0: ID3_FIELD_TYPE_TEXTENCODING, 1: ID3_FIELD_TYPE_LANGUAGE, 2: ID3_FIELD_TYPE_STRING, 3: ID3_FIELD_TYPE_STRINGFULL
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, _T("eng")); // it's what Mp3tag writes
      frame.SetString(3, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoAlbum, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::AlbumTitle);

      ID3::Frame frame(ID3::FrameId::AlbumTitle);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   // numeric

   int intValue = trackInfo.NumberInfo(TrackInfoYear, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);

      tag.RemoveFrame(ID3::FrameId::RecordingTime);

      ID3::Frame frame(ID3::FrameId::RecordingTime);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   intValue = trackInfo.NumberInfo(TrackInfoTrack, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);

      tag.RemoveFrame(ID3::FrameId::TrackNumber);

      ID3::Frame frame(ID3::FrameId::TrackNumber);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoGenre, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Genre);

      ID3::Frame frame(ID3::FrameId::Genre);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   // binary

   std::vector<unsigned char> binaryInfo;

   isAvail = trackInfo.BinaryInfo(TrackInfoFrontCover, binaryInfo);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::AttachedPicture);

      ID3::Frame frame(ID3::FrameId::AttachedPicture);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, "image/jpeg");
      frame.SetInt8(2, 0x03); // 0x03 = Front Cover
      frame.SetBinaryData(4, binaryInfo);
      tag.AttachFrame(frame);
   }
}
