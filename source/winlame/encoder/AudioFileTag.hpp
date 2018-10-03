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

namespace ID3
{
   class Tag;
}

namespace Encoder
{
   class TrackInfo;

   /// audio file tag
   class AudioFileTag
   {
   public:
      /// creates tag instance using track info
      AudioFileTag(TrackInfo& trackInfo)
         :m_trackInfo(trackInfo)
      {
      }

      /// reads tag infos from audio file and stores it in TrackInfo
      bool ReadFromFile(const CString& filename);

      /// determines the length of the (ID3v2) tag that would be written from the track infos
      unsigned int GetTagLength() const;

      /// stores TrackInfo data to tag infos in audio file
      bool WriteToFile(const CString& filename) const;

   private:
      /// reads all track infos from tag
      bool ReadTrackInfoFromTag(ID3::Tag& tag);

      /// stores all track infos in given tag
      void StoreTrackInfoInTag(ID3::Tag& tag) const;

   private:
      /// track info to read or store
      TrackInfo& m_trackInfo;
   };

} // namespace Encoder
