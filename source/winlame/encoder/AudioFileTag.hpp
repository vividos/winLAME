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
/// \file AudioFileTag.hpp
/// \brief represents an audio file tag
/// \details supports reading and writing TrackInfo as tag from and to files
//
#pragma once

/// \cond false
namespace TagLib
{
   class Tag;
   class File;
   template <class T> class List;
   namespace ID3v2
   {
      class Tag;
   }
   namespace Ogg
   {
      class XiphComment;
   }
   namespace FLAC
   {
      class File;
      class Picture;
   }
}
/// \endcond

namespace Encoder
{
   class TrackInfo;

   /// audio file tag reading/writing
   /// \see https://help.mp3tag.de/main_tags.html
   class AudioFileTag
   {
   public:
      /// specifies the audio file type to use when reading or writing to a file
      enum AudioFileType
      {
         FromExtension = 0,   ///< guess audio file type from extension
         MPEG = 1,            ///< treat audio file as MPEG Layer 1/2/3 file
      };

      /// creates tag instance using track info
      explicit AudioFileTag(TrackInfo& trackInfo)
         :m_trackInfo(trackInfo)
      {
      }

      /// reads tag infos from audio file and stores it in TrackInfo
      bool ReadFromFile(const CString& filename, AudioFileType audioFileType = AudioFileType::FromExtension);

      /// determines the length of the (ID3v2) tag that would be written from the track infos
      unsigned int GetTagLength() const;

      /// stores TrackInfo data to tag infos in audio file
      bool WriteToFile(const CString& filename, AudioFileType audioFileType = AudioFileType::FromExtension) const;

   private:
      /// opens taglib file
      static std::shared_ptr<TagLib::File> OpenFile(const CString& filename, AudioFileType audioFileType);

      /// finds ID3v2 tag in given file, if available
      static TagLib::ID3v2::Tag* FindId3v2Tag(std::shared_ptr<TagLib::File> spFile);

      /// finds Ogg comment tag in given file, if available
      static TagLib::Ogg::XiphComment* AudioFileTag::FindOggXiphCommentTag(std::shared_ptr<TagLib::File> spFile);

      /// reads all track infos from tag
      void ReadTrackInfoFromTag(TagLib::Tag* tag);

      /// reads extra track info from ID3v2 tag
      void ReadTrackInfoFromId3v2Tag(TagLib::ID3v2::Tag* id3v2tag);

      /// reads extra track info from Xiph Comment tag
      void ReadTrackInfoFromXiphCommentTag(TagLib::Ogg::XiphComment* xiphComment);

      /// reads extra track info from FLAC file
      void ReadTrackInfoFromFlacFile(TagLib::FLAC::File* flacFile);

      /// reads front cover picture from given picture list
      void ReadFrontCoverPictureFromPictureList(const TagLib::List<TagLib::FLAC::Picture*>& pictures);

      /// stores all track infos in given tag
      void StoreTrackInfoInTag(TagLib::Tag* tag) const;

      /// stores extra track infos in given ID3v2 tag
      void StoreTrackInfoInId3v2Tag(TagLib::ID3v2::Tag* id3v2tag) const;

      /// stores extra track infos in given Xiph Comment tag
      void StoreTrackInfoInXiphCommentTag(TagLib::Ogg::XiphComment* oggXiphComment) const;

   private:
      /// track info to read or store
      TrackInfo& m_trackInfo;
   };

} // namespace Encoder
