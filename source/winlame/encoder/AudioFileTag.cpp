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
#include <taglib/mpeg/id3v2/frames/textidentificationframe.h>
#include <taglib/flac/flacfile.h>
#include <taglib/ogg/flac/oggflacfile.h>
#include <taglib/ogg/xiphcomment.h>
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

   ReadTrackInfoFromTag(tag);

   TagLib::ID3v2::Tag* id3v2tag = FindId3v2Tag(spFile);
   if (id3v2tag != nullptr)
      ReadTrackInfoFromId3v2Tag(id3v2tag);

   TagLib::Ogg::XiphComment* oggXiphComment = FindOggXiphCommentTag(spFile);
   if (oggXiphComment != nullptr)
      ReadTrackInfoFromXiphCommentTag(oggXiphComment);

   return true;
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

TagLib::Ogg::XiphComment* AudioFileTag::FindOggXiphCommentTag(std::shared_ptr<TagLib::File> spFile)
{
   TagLib::Ogg::XiphComment* xiphComment = dynamic_cast<TagLib::Ogg::XiphComment*>(spFile->tag());
   if (xiphComment != nullptr)
      return xiphComment;

   // try opening ogg flac file
   std::shared_ptr<TagLib::Ogg::FLAC::File> oggFile = std::dynamic_pointer_cast<TagLib::Ogg::FLAC::File>(spFile);
   if (oggFile != nullptr &&
      oggFile->hasXiphComment())
   {
      return oggFile->tag();
   }

   // try opening flac file
   std::shared_ptr<TagLib::FLAC::File> flacFile = std::dynamic_pointer_cast<TagLib::FLAC::File>(spFile);
   if (flacFile != nullptr)
   {
      return flacFile->xiphComment(true); // create when it's not there yet
   }

   return nullptr;
}

void AudioFileTag::ReadTrackInfoFromTag(TagLib::Tag* tag)
{
   for (auto property : tag->properties())
   {
      ATLTRACE(_T("TagLib property: %hs: %hs\n"),
         property.first.toCString(),
         property.second.toString().toCString());
   }

   // retrieve field values
   CString textValue;
   if (!tag->title().isEmpty())
   {
      textValue = tag->title().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoTitle, textValue);
   }

   if (!tag->artist().isEmpty())
   {
      textValue = tag->artist().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoArtist, textValue);
   }

   if (!tag->comment().isEmpty())
   {
      textValue = tag->comment().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoComment, textValue);
   }

   if (!tag->album().isEmpty())
   {
      textValue = tag->album().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoAlbum, textValue);
   }

   if (tag->year() != 0)
   {
      m_trackInfo.SetNumberInfo(TrackInfoYear, static_cast<int>(tag->year()));
   }

   if (tag->track() != 0)
   {
      m_trackInfo.SetNumberInfo(TrackInfoTrack, static_cast<int>(tag->track()));
   }

   if (!tag->genre().isEmpty())
   {
      textValue = tag->genre().toCWString();
      if (!textValue.IsEmpty())
         m_trackInfo.SetTextInfo(TrackInfoGenre, textValue);
   }
}

void AudioFileTag::ReadTrackInfoFromId3v2Tag(TagLib::ID3v2::Tag* id3v2tag)
{
   for (auto frame : id3v2tag->frameList())
   {
      auto frameId = frame->frameID();
      if (frameId.size() != 4)
         continue;

      ATLTRACE(_T("ID3v2 Frame: %c%c%c%c: %hs\n"),
         frameId[0],
         frameId[1],
         frameId[2],
         frameId[3],
         frame->toString().toCString());
   }

   TagLib::PropertyMap propertyMap = id3v2tag->properties();

   CString textValue;
   if (propertyMap.contains(TagLib::String("ALBUMARTIST")))
   {
      textValue = propertyMap[TagLib::String("ALBUMARTIST")].toString().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoDiscArtist, textValue);
   }

   if (propertyMap.contains(TagLib::String("COMPOSER")))
   {
      textValue = propertyMap[TagLib::String("COMPOSER")].toString().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoComposer, textValue);
   }

   for (auto frame : id3v2tag->frameList())
   {
      auto tposTagName = TagLib::ByteVector::fromCString("TPOS");

      auto tposFrameIter = id3v2tag->frameListMap().find(tposTagName);
      if (tposFrameIter != id3v2tag->frameListMap().end())
      {
         // only use first
         auto tposFrameList = tposFrameIter->second;
         if (!tposFrameList.isEmpty())
         {
            int intValue = tposFrameList.front()->toString().toInt();
            m_trackInfo.SetNumberInfo(TrackInfoDiscNumber, intValue);
         }
      }

      // find picture frames
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
            m_trackInfo.SetBinaryInfo(TrackInfoFrontCover, binaryData);
      }
   }
}

void AudioFileTag::ReadTrackInfoFromXiphCommentTag(TagLib::Ogg::XiphComment* xiphComment)
{
   for (auto field : xiphComment->fieldListMap())
   {
      ATLTRACE(_T("Xiph comment field: %hs: %hs\n"),
         field.first.toCString(),
         field.second.toString().toCString());
   }

   TagLib::PropertyMap propertyMap = xiphComment->properties();

   CString textValue;
   if (propertyMap.contains(TagLib::String("ALBUMARTIST")))
   {
      textValue = propertyMap[TagLib::String("ALBUMARTIST")].toString().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoDiscArtist, textValue);
   }

   if (propertyMap.contains(TagLib::String("COMPOSER")))
   {
      textValue = propertyMap[TagLib::String("COMPOSER")].toString().toCWString();
      m_trackInfo.SetTextInfo(TrackInfoComposer, textValue);
   }

   if (propertyMap.contains(TagLib::String("DISCNUMBER")))
   {
      int intValue = propertyMap[TagLib::String("DISCNUMBER")].toString().toInt();
      m_trackInfo.SetNumberInfo(TrackInfoDiscNumber, intValue);
   }

   const TagLib::List<TagLib::FLAC::Picture*>& pictures = xiphComment->pictureList();

   TagLib::List<TagLib::FLAC::Picture*>::ConstIterator iter = pictures.begin();
   if (pictures.size() > 1)
   {
      // find front cover
      while (iter != pictures.end() && (*iter)->type() != TagLib::FLAC::Picture::FrontCover)
         iter++;

      // when none was found, use the first
      if (iter == pictures.end())
         pictures.begin();
   }

   if (iter != pictures.end())
   {
      const TagLib::FLAC::Picture& picture = **iter;

      const std::vector<unsigned char> binaryData(
         picture.data().begin(),
         picture.data().end());

      if (!binaryData.empty())
         m_trackInfo.SetBinaryInfo(TrackInfoFrontCover, binaryData);
   }
}

unsigned int AudioFileTag::GetTagLength() const
{
   // create an in-memory ID3v2 tag, fill and render it
   TagLib::ID3v2::Tag tag;

   StoreTrackInfoInTag(&tag);
   StoreTrackInfoInId3v2Tag(&tag);

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

   StoreTrackInfoInTag(tag);

   TagLib::ID3v2::Tag* id3v2tag = FindId3v2Tag(spFile);
   if (id3v2tag != nullptr)
      StoreTrackInfoInId3v2Tag(id3v2tag);

   TagLib::Ogg::XiphComment* oggXiphComment = FindOggXiphCommentTag(spFile);
   if (oggXiphComment != nullptr)
      StoreTrackInfoInXiphCommentTag(oggXiphComment);

   return spFile->save();
}

void AudioFileTag::StoreTrackInfoInId3v2Tag(TagLib::ID3v2::Tag* id3v2tag) const
{
   bool isAvail = false;

   CString textValue = m_trackInfo.GetTextInfo(TrackInfoDiscArtist, isAvail);
   if (isAvail)
   {
      TagLib::PropertyMap propertyMap = id3v2tag->properties();

      propertyMap.replace(TagLib::String("ALBUMARTIST"), TagLib::StringList(TagLib::String(textValue)));

      id3v2tag->setProperties(propertyMap);
   }

   textValue = m_trackInfo.GetTextInfo(TrackInfoComposer, isAvail);
   if (isAvail)
   {
      TagLib::PropertyMap propertyMap = id3v2tag->properties();

      propertyMap.replace(TagLib::String("COMPOSER"), TagLib::StringList(TagLib::String(textValue)));

      id3v2tag->setProperties(propertyMap);
   }

   int intValue = m_trackInfo.GetNumberInfo(TrackInfoDiscNumber, isAvail);
   if (isAvail)
   {
      auto tposFrame = new TagLib::ID3v2::TextIdentificationFrame(
         TagLib::ByteVector::fromCString("TPOS"),
         TagLib::String::Type::Latin1);
      tposFrame->setText(TagLib::String::number(intValue));
      id3v2tag->addFrame(tposFrame);
   }

   std::vector<unsigned char> binaryInfo;

   isAvail = m_trackInfo.GetBinaryInfo(TrackInfoFrontCover, binaryInfo);
   if (isAvail)
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

void AudioFileTag::StoreTrackInfoInXiphCommentTag(TagLib::Ogg::XiphComment* oggXiphComment) const
{
   bool isAvail = false;
   int intValue = m_trackInfo.GetNumberInfo(TrackInfoDiscNumber, isAvail);
   if (isAvail)
   {
      TagLib::PropertyMap propertyMap = oggXiphComment->properties();

      propertyMap.replace(TagLib::String("DISCNUMBER"), TagLib::StringList(TagLib::String::number(intValue)));

      oggXiphComment->setProperties(propertyMap);
   }
}

void AudioFileTag::StoreTrackInfoInTag(TagLib::Tag* tag) const
{
   // add all frames
   bool isAvail = false;
   CString textValue = m_trackInfo.GetTextInfo(TrackInfoTitle, isAvail);
   if (isAvail)
   {
      tag->setTitle(TagLib::String(textValue));
   }

   const TrackInfo& trackInfo = m_trackInfo;

   textValue = trackInfo.GetTextInfo(TrackInfoArtist, isAvail);
   if (isAvail)
   {
      tag->setArtist(TagLib::String(textValue));
   }

   textValue = trackInfo.GetTextInfo(TrackInfoComment, isAvail);
   if (isAvail)
   {
      tag->setComment(TagLib::String(textValue));
   }

   textValue = trackInfo.GetTextInfo(TrackInfoAlbum, isAvail);
   if (isAvail)
   {
      tag->setAlbum(TagLib::String(textValue));
   }

   textValue = trackInfo.GetTextInfo(TrackInfoGenre, isAvail);
   if (isAvail)
   {
      tag->setGenre(TagLib::String(textValue));
   }

   int intValue = trackInfo.GetNumberInfo(TrackInfoYear, isAvail);
   if (isAvail)
   {
      tag->setYear(static_cast<unsigned int>(intValue));
   }

   intValue = trackInfo.GetNumberInfo(TrackInfoTrack, isAvail);
   if (isAvail)
   {
      tag->setTrack(static_cast<unsigned int>(intValue));
   }
}
