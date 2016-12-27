//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file Id3v1Tag.cpp
/// \brief id3v1 tag parsing and creating
//
#include "stdafx.h"
#include "Id3v1Tag.hpp"
#include "TrackInfo.hpp"
#include <cstdio>

using Encoder::Id3v1Tag;
using Encoder::TrackInfo;

void Id3v1Tag::ToTrackInfo(TrackInfo& trackInfo) const
{
   // insert all id3 tag infos into properties

   // title
   CString textValue = CString(CStringA(this->title), 30);
   if (this->title[0] != 0)
      trackInfo.TextInfo(TrackInfoTitle, textValue);

   // artist
   textValue = CString(CStringA(this->artist), 30);
   if (this->artist[0] != 0)
      trackInfo.TextInfo(TrackInfoArtist, textValue);

   // album
   textValue = CString(CStringA(this->album), 30);
   if (this->album[0] != 0)
      trackInfo.TextInfo(TrackInfoAlbum, textValue);

   // year
   textValue = CString(CStringA(this->year), 4);
   int intValue = (int)_tcstoul(textValue, NULL, 10);
   if (intValue != 0)
      trackInfo.NumberInfo(TrackInfoYear, intValue);

   // comment
   textValue = CString(CStringA(this->comment), 29);
   if (this->comment[0] != 0)
      trackInfo.TextInfo(TrackInfoComment, textValue);

   // track number
   if (this->track != 0 && this->track != this->comment[28])
      trackInfo.NumberInfo(TrackInfoTrack, (int)this->track);

   // genre
   if (this->genre != 0xff)
   {
      CString cszGenre = TrackInfo::GenreIDToText(this->genre);
      trackInfo.TextInfo(TrackInfoGenre, cszGenre);
   }
}

void Id3v1Tag::FromTrackInfo(const TrackInfo& trackInfo)
{
   // update id3 tag entries
   memcpy(this->tag, "TAG", 3);

   bool isAvail;
   CString textValue = trackInfo.TextInfo(TrackInfoTitle, isAvail);
   if (isAvail)
      _snprintf(this->title, sizeof(this->title) / sizeof(*this->title), "%.30ls", textValue.Left(30).GetString());

   textValue = trackInfo.TextInfo(TrackInfoArtist, isAvail);
   if (isAvail)
      _snprintf(this->artist, sizeof(this->artist) / sizeof(*this->artist), "%.30ls", textValue.Left(30).GetString());

   textValue = trackInfo.TextInfo(TrackInfoAlbum, isAvail);
   if (isAvail)
      _snprintf(this->album, sizeof(this->album) / sizeof(*this->album), "%.30ls", textValue.Left(30).GetString());

   int intValue = trackInfo.NumberInfo(TrackInfoYear, isAvail);
   if (isAvail)
      _snprintf(this->year, sizeof(this->year) / sizeof(*this->year), "%.4u", static_cast<unsigned int>(intValue));

   textValue = trackInfo.TextInfo(TrackInfoComment, isAvail);
   if (isAvail)
      _snprintf(this->comment, sizeof(this->comment) / sizeof(*this->comment), "%.29ls", textValue.Left(29).GetString());

   intValue = trackInfo.NumberInfo(TrackInfoTrack, isAvail);
   if (isAvail)
      this->track = (unsigned char)intValue;

   textValue = trackInfo.TextInfo(TrackInfoGenre, isAvail);
   if (isAvail)
      this->genre = static_cast<unsigned char>(TrackInfo::TextToGenreID(textValue));
}
