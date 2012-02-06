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

   $Id: wlTrackInfo.cpp,v 1.14 2008/02/26 20:15:26 vividos Exp $

*/
/*! \file wlTrackInfo.cpp

   \brief routines to manage track infos, as well as id3v1 tag parsing and creating routines

*/

// needed includes
#include "stdafx.h"
#include "wlTrackInfo.h"
#include <stdio.h>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//! genre ID to string mapping
const TCHAR* wlId3GenreIDtoString[] =
{
   // 0..19
   _T("Blues"), _T("Classic Rock"), _T("Country"), _T("Dance"), _T("Disco"), _T("Funk"), _T("Grunge"), _T("Hip-Hop"),
   _T("Jazz"), _T("Metal"), _T("New Age"), _T("Oldies"), _T("Other"), _T("Pop"), _T("R&B"), _T("Rap"), _T("Reggae"), _T("Rock"),
   _T("Techno"), _T("Industrial"),
   // 20..39
   _T("Alternative"), _T("Ska"), _T("Death Metal"), _T("Pranks"), _T("Soundtrack"), _T("Euro-Techno"), _T("Ambient"),
   _T("Trip-Hop"), _T("Vocal"), _T("Jazz+Funk"), _T("Fusion"), _T("Trance"), _T("Classical"), _T("Instrumental"),
   _T("Acid"), _T("House"), _T("Game"), _T("Sound Clip"), _T("Gospel"), _T("Noise"),
   // 40..59
   _T("AlternRock"), _T("Bass"), _T("Soul"), _T("Punk"), _T("Space"), _T("Meditative"), _T("Instrumental Pop"),
   _T("Instrumental Rock"), _T("Ethnic"), _T("Gothic"), _T("Darkwave"), _T("Techno-Industrial"),
   _T("Electronic"), _T("Pop-Folk"), _T("Eurodance"), _T("Dream"), _T("Southern Rock"), _T("Comedy"),
   _T("Cult"), _T("Gangsta"),
   // 60..79
   _T("Top 40"), _T("Christian Rap"), _T("Pop/Funk"), _T("Jungle"), _T("Native American"), _T("Cabaret"), _T("New Wave"),
   _T("Psychadelic"), _T("Rave"), _T("Showtunes"), _T("Trailer"), _T("Lo-Fi"), _T("Tribal"), _T("Acid Punk"), _T("Acid Jazz"),
   _T("Polka"), _T("Retro"), _T("Musical"), _T("Rock & Roll"), _T("Hard Rock"),
   // Winamp extensions:
   // 80..99
   _T("Folk"), _T("Folk-Rock"), _T("National Folk"), _T("Swing"), _T("Fast Fusion"), _T("Bebob"), _T("Latin"),
   _T("Revival"), _T("Celtic"), _T("Bluegrass"), _T("Avantgarde"), _T("Gothic Rock"), _T("Progressive Rock"),
   _T("Psychedelic Rock"), _T("Symphonic Rock"), _T("Slow Rock"), _T("Big Band"), _T("Chorus"),
   _T("Easy Listening"), _T("Acoustic"),
   // 100..119
   _T("Humour"), _T("Speech"), _T("Chanson"), _T("Opera"), _T("Chamber Music"), _T("Sonata"), _T("Symphony"),
   _T("Booty Brass"), _T("Primus"), _T("Porn Groove"), _T("Satire"), _T("Slow Jam"), _T("Club"), _T("Tango"),
   _T("Samba"), _T("Folklore"), _T("Ballad"), _T("Power Ballad"), _T("Rhytmic Soul"), _T("Freestyle"),
   // 120..125
   _T("Duet"), _T("Punk Rock"), _T("Drum Solo"), _T("A Capella"), _T("Euro-House"), _T("Dance Hall"),
};

CString wlTrackInfo::GenreIDToText(unsigned int uGenreID)
{
   if (uGenreID == (unsigned int)-1)
      return _T("");

   ATLASSERT(uGenreID < sizeof(wlId3GenreIDtoString)/sizeof(*wlId3GenreIDtoString));
   return wlId3GenreIDtoString[uGenreID];
}

unsigned int wlTrackInfo::TextToGenreID(const CString& cszText)
{
   for (unsigned int i=0; i<sizeof(wlId3GenreIDtoString)/sizeof(*wlId3GenreIDtoString); i++)
      if (wlId3GenreIDtoString[i] == cszText)
         return i;

   return (unsigned int)-1;
}

LPCTSTR* wlTrackInfo::GetGenreList()
{
   return wlId3GenreIDtoString;
}

unsigned int wlTrackInfo::GetGenreListLength()
{
   return sizeof(wlId3GenreIDtoString) / sizeof(*wlId3GenreIDtoString);
}
