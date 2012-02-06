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

   $Id: wlId3v1Tag.cpp,v 1.2 2009/10/25 20:08:06 vividos Exp $

*/
/*! \file wlId3v1Tag.cpp

   \brief id3v1 tag parsing and creating

*/

// needed includes
#include "stdafx.h"
#include "wlId3v1Tag.h"
#include "wlTrackInfo.h"
#include <cstdio>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void wlId3v1Tag::toTrackInfo(wlTrackInfo& ti) const
{
   // insert all id3 tag infos into properties

   // title
   USES_CONVERSION;
   CString prop = CString(A2CT(this->title),30);
   if (this->title[0] != 0)
      ti.TextInfo(wlTrackInfoTitle, prop);

   // artist
   prop = CString(A2CT(this->artist),30);
   if (this->artist[0] != 0)
      ti.TextInfo(wlTrackInfoArtist, prop);

   // album
   prop = CString(A2CT(this->album),30);
   if (this->album[0] != 0)
      ti.TextInfo(wlTrackInfoAlbum, prop);

   // year
   prop = CString(A2CT(this->year),4);
   int iProp = (int)_tcstoul(prop, NULL, 10);
   if (iProp != 0)
      ti.NumberInfo(wlTrackInfoYear, iProp);

   // comment
   prop = CString(A2CT(this->comment),29);
   if (this->comment[0] != 0)
      ti.TextInfo(wlTrackInfoComment, prop);

   // track number
   if (this->track!=0 && this->track != this->comment[28])
      ti.NumberInfo(wlTrackInfoTrack, (int)this->track);

   // genre
   if (this->genre != 0xff)
   {
      CString cszGenre = wlTrackInfo::GenreIDToText(this->genre);
      ti.TextInfo(wlTrackInfoGenre, cszGenre);      
   }
}

void wlId3v1Tag::fromTrackInfo(const wlTrackInfo& ti)
{
   // update id3 tag entries
   memcpy(this->tag, "TAG", 3);

   bool bAvail;
   CString cszValue = ti.TextInfo(wlTrackInfoTitle, bAvail);
   if (bAvail)
      _snprintf(this->title, sizeof(this->title)/sizeof(*this->title), "%.30ls", cszValue.Left(30));

   cszValue = ti.TextInfo(wlTrackInfoArtist, bAvail);
   if (bAvail)
      _snprintf(this->artist, sizeof(this->artist)/sizeof(*this->artist), "%.30ls", cszValue.Left(30));

   cszValue = ti.TextInfo(wlTrackInfoAlbum, bAvail);
   if (bAvail)
      _snprintf(this->album, sizeof(this->album)/sizeof(*this->album), "%.30ls", cszValue.Left(30));

   int iValue = ti.NumberInfo(wlTrackInfoYear, bAvail);
   if (bAvail)
      _snprintf(this->year, sizeof(this->year)/sizeof(*this->year), "%.4u", (unsigned)iValue);

   cszValue = ti.TextInfo(wlTrackInfoComment, bAvail);
   if (bAvail)
      _snprintf(this->comment, sizeof(this->comment)/sizeof(*this->comment), "%.29ls", cszValue.Left(29));

   iValue = ti.NumberInfo(wlTrackInfoTrack, bAvail);
   if (bAvail)
      this->track = (unsigned char)iValue;

   cszValue = ti.TextInfo(wlTrackInfoGenre, bAvail);
   if (bAvail)
      this->genre = static_cast<unsigned char>(wlTrackInfo::TextToGenreID(cszValue));
}
