/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*! \file Id3v1Tag.h

   \brief manages id3 tag reading and writing

*/
/*! \ingroup encoder */
/*! @{ */

// include guard
#pragma once

// needed includes

// forward references
class TrackInfo;

#pragma pack(push, 1)

/// id3 tag structure
struct Id3v1Tag
{
   char tag[3];      ///< tag start, always "TAG"
   char title[30];   ///< title
   char artist[30];  ///< artist
   char album[30];   ///< album name
   char year[4];     ///< year
   char comment[29]; ///< comment
   unsigned char track; ///< track
   unsigned char genre; ///< genre id

   /// ctor
   Id3v1Tag()
   {
      memset(this, 0, sizeof(*this));
   }

   /// ctor; constructs id3 tag from track info
   Id3v1Tag(const TrackInfo& ti)
   {
      memset(this, 0, sizeof(*this));
      fromTrackInfo(ti);
   }

   /// returns pointer to data; length always 128 bytes
   unsigned int* getData()
   {
      ATLASSERT(sizeof(*this) == 128);
      return reinterpret_cast<unsigned int*>(this);
   }

   /// checks if data starts with "TAG"
   bool isValidTag() const
   {
      return strncmp(this->tag, "TAG", 3) == 0;
   }

   /// converts id3 tag to track infos
   void toTrackInfo(TrackInfo& ti) const;

   /// converts track infos to id3 tag
   void fromTrackInfo(const TrackInfo& ti);
};

#pragma pack(pop)

//@}
