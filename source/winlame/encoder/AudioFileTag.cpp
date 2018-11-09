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
#pragma warning(push)
#pragma warning(disable: 4251) // class 'T' needs to have dll-interface to be used by clients of class 'C'
#include <taglib/fileref.h>
#include <taglib/toolkit/tpropertymap.h>
#include <taglib/mpeg/id3v2/id3v2tag.h>
#include <taglib/mpeg/mpegfile.h>
#include <taglib/mpeg/id3v2/frames/attachedpictureframe.h>
#pragma warning(pop)

#ifdef _DEBUG
#pragma comment(lib, "taglib_debug.lib")
#else
#pragma comment(lib, "taglib.lib")
#endif

using Encoder::AudioFileTag;
using Encoder::TrackInfo;

bool AudioFileTag::ReadFromFile(const CString& filename, AudioFileType audioFileType)
{
   std::shared_ptr<TagLib::File> spFile = OpenFile(filename, audioFileType);

   if (spFile == nullptr)
      return false;

   TagLib::Tag* tag = spFile->tag();
   if (tag == nullptr)
      return false;

   TagLib::ID3v2::Tag* id3v2tag = FindId3v2Tag(spFile);

   return ReadTrackInfoFromTag(tag, id3v2tag);
}

std::shared_ptr<TagLib::File> AudioFileTag::OpenFile(const CString& filename, AudioFileType audioFileType)
{
   std::shared_ptr<TagLib::File> spFile;

   if (audioFileType == AudioFileType::FromExtension)
   {
      spFile.reset(TagLib::FileRef::create(
         TagLib::FileName(filename),
         true,
         TagLib::AudioProperties::ReadStyle::Accurate));
   }
   else if (audioFileType == AudioFileType::MPEG)
   {
      spFile = std::make_shared<TagLib::MPEG::File>(
         TagLib::FileName(filename),
         true,
         TagLib::AudioProperties::ReadStyle::Accurate);
   }

   return spFile;
}

TagLib::ID3v2::Tag* AudioFileTag::FindId3v2Tag(std::shared_ptr<TagLib::File> spFile)
{
   TagLib::ID3v2::Tag* id3v2tag = dynamic_cast<TagLib::ID3v2::Tag*>(spFile->tag());
   if (id3v2tag != nullptr)
      return id3v2tag;

   std::shared_ptr<TagLib::MPEG::File> mpegFile = std::dynamic_pointer_cast<TagLib::MPEG::File>(spFile);
   if (mpegFile != nullptr)
   {
      return mpegFile->ID3v2Tag(true); // create when it's not there yet
   }

   return nullptr;
}

bool AudioFileTag::ReadTrackInfoFromTag(TagLib::Tag* tag, TagLib::ID3v2::Tag* id3v2tag)
{
   if (id3v2tag != nullptr)
   {
      for (auto frame : id3v2tag->frameList())
      {
         auto frameId = frame->frameID();
         ATLTRACE(_T("Frame: %c%c%c%c: %hs\n"),
            frameId[0],
            frameId[1],
            frameId[2],
            frameId[3],
            frame->toString().toCString());
      }
   }

   // retrieve field values
   CString textValue;
   if (tag->title() != TagLib::String::null)
   {
      textValue = tag->title().toCWString();
      m_trackInfo.TextInfo(TrackInfoTitle, textValue);
   }

   if (tag->artist() != TagLib::String::null)
   {
      textValue = tag->artist().toCWString();
      m_trackInfo.TextInfo(TrackInfoArtist, textValue);
   }

   if (tag->comment() != TagLib::String::null)
   {
      textValue = tag->comment().toCWString();
      m_trackInfo.TextInfo(TrackInfoComment, textValue);
   }

   if (tag->album() != TagLib::String::null)
   {
      textValue = tag->album().toCWString();
      m_trackInfo.TextInfo(TrackInfoAlbum, textValue);
   }

   if (tag->year() != 0)
   {
      m_trackInfo.NumberInfo(TrackInfoYear, static_cast<int>(tag->year()));
   }

   if (tag->track() != 0)
   {
      m_trackInfo.NumberInfo(TrackInfoTrack, static_cast<int>(tag->track()));
   }

   if (tag->genre() != TagLib::String::null)
   {
      textValue = tag->genre().toCWString();
      if (!textValue.IsEmpty())
         m_trackInfo.TextInfo(TrackInfoGenre, textValue);
   }

   if (id3v2tag != nullptr)
   {
      TagLib::PropertyMap propertyMap = id3v2tag->properties();

      if (propertyMap.contains(TagLib::String("ALBUMARTIST")))
      {
         textValue = propertyMap[TagLib::String("ALBUMARTIST")].toString().toCWString();
         m_trackInfo.TextInfo(TrackInfoDiscArtist, textValue);
      }

      if (propertyMap.contains(TagLib::String("COMPOSER")))
      {
         textValue = propertyMap[TagLib::String("COMPOSER")].toString().toCWString();
         m_trackInfo.TextInfo(TrackInfoComposer, textValue);
      }

      for (auto frame : id3v2tag->frameList())
      {
         auto pictureFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame);
         if (pictureFrame != nullptr &&
            pictureFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
         {
            const auto pictureData = pictureFrame->picture();
            const unsigned char* data =
               reinterpret_cast<const unsigned char*>(pictureData.data());

            const std::vector<unsigned char> binaryData(
               data,
               data + pictureFrame->picture().size());

            if (!binaryData.empty())
               m_trackInfo.BinaryInfo(TrackInfoFrontCover, binaryData);
         }
      }
   }

   return true;
}

unsigned int AudioFileTag::GetTagLength() const
{
   // create an in-memory ID3v2 tag, fill and render it
   TagLib::ID3v2::Tag tag;

   StoreTrackInfoInTag(&tag, &tag);

   unsigned int size = tag.render().size();
   return size;
}

bool AudioFileTag::WriteToFile(const CString& filename, AudioFileType audioFileType) const
{
   std::shared_ptr<TagLib::File> spFile = OpenFile(filename, audioFileType);

   if (spFile == nullptr)
      return false;

   TagLib::Tag* tag = spFile->tag();
   if (tag == nullptr)
      return false;

   TagLib::ID3v2::Tag* id3v2tag = FindId3v2Tag(spFile);

   StoreTrackInfoInTag(tag, id3v2tag);

   return spFile->save();
}

void AudioFileTag::StoreTrackInfoInTag(TagLib::Tag* tag, TagLib::ID3v2::Tag* id3v2tag) const
{
   // add all frames
   bool isAvail = false;
   CString textValue = m_trackInfo.TextInfo(TrackInfoTitle, isAvail);
   if (isAvail)
      tag->setTitle(TagLib::String(textValue));

   const TrackInfo& trackInfo = m_trackInfo;

   textValue = trackInfo.TextInfo(TrackInfoArtist, isAvail);
   if (isAvail)
   {
      tag->setArtist(TagLib::String(textValue));
   }

   textValue = trackInfo.TextInfo(TrackInfoDiscArtist, isAvail);
   if (isAvail)
   {
      if (id3v2tag != nullptr)
      {
         TagLib::PropertyMap propertyMap = id3v2tag->properties();

         propertyMap.replace(TagLib::String("ALBUMARTIST"), TagLib::StringList(TagLib::String(textValue)));

         id3v2tag->setProperties(propertyMap);
      }
   }

   textValue = trackInfo.TextInfo(TrackInfoComposer, isAvail);
   if (isAvail)
   {
      if (id3v2tag != nullptr)
      {
         TagLib::PropertyMap propertyMap = id3v2tag->properties();

         propertyMap.replace(TagLib::String("COMPOSER"), TagLib::StringList(TagLib::String(textValue)));

         id3v2tag->setProperties(propertyMap);
      }
   }

   textValue = trackInfo.TextInfo(TrackInfoComment, isAvail);
   if (isAvail)
   {
      tag->setComment(TagLib::String(textValue));
   }

   textValue = trackInfo.TextInfo(TrackInfoAlbum, isAvail);
   if (isAvail)
   {
      tag->setAlbum(TagLib::String(textValue));
   }

   textValue = trackInfo.TextInfo(TrackInfoGenre, isAvail);
   if (isAvail)
   {
      tag->setGenre(TagLib::String(textValue));
   }

   // numeric

   int intValue = trackInfo.NumberInfo(TrackInfoYear, isAvail);
   if (isAvail)
   {
      tag->setYear(static_cast<unsigned int>(intValue));
   }

   intValue = trackInfo.NumberInfo(TrackInfoTrack, isAvail);
   if (isAvail)
   {
      tag->setTrack(static_cast<unsigned int>(intValue));
   }

   // binary

   std::vector<unsigned char> binaryInfo;

   isAvail = trackInfo.BinaryInfo(TrackInfoFrontCover, binaryInfo);
   if (isAvail)
   {
      if (id3v2tag != nullptr)
      {
         for (auto frame : id3v2tag->frameList())
         {
            auto pictureFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame);
            if (pictureFrame != nullptr &&
               (pictureFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover ||
                  pictureFrame->type() == TagLib::ID3v2::AttachedPictureFrame::Other))
            {
               id3v2tag->removeFrame(pictureFrame);
               break;
            }
         }

         auto pictureFrame = new TagLib::ID3v2::AttachedPictureFrame;

         pictureFrame->setPicture(TagLib::ByteVector(reinterpret_cast<const char*>(binaryInfo.data()), binaryInfo.size()));
         pictureFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
         pictureFrame->setMimeType(TagLib::String("image/jpeg"));

         id3v2tag->addFrame(pictureFrame);
      }
   }
}
