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

   $Id: wlTrackInfo.h,v 1.12 2009/10/26 22:17:57 vividos Exp $

*/
/*! \file wlTrackInfo.h

   \brief manages informations and properties of a track to encode

   id3 tags can be passed in and can be generated from the properties

*/
/*! \ingroup encoder */
/*! @{ */

// prevent multiple including
#ifndef wltrackinfo_h_
#define wltrackinfo_h_

// needed includes
#include <string>
#include <map>


//! track text info enum
typedef enum
{
   wlTrackInfoTitle=0,
   wlTrackInfoArtist,
   wlTrackInfoAlbum,
   wlTrackInfoComment,
   wlTrackInfoGenre,
} wlTrackInfoTextType;

//! track number info enum
typedef enum
{
   wlTrackInfoYear=0,
   wlTrackInfoTrack,
} wlTrackInfoNumberType;


//! track info class
class wlTrackInfo
{
public:
   //! ctor
   wlTrackInfo()
   {
   }

   //! resets all infos
   void ResetInfos()
   {
      m_mapTextInfos.clear();
      m_mapNumberInfos.clear();
   }

   //! sets a text info value
   void TextInfo(wlTrackInfoTextType type, CString value)
   {
      m_mapTextInfos[type] = value;
   }

   //! retrieves a text info value
   CString TextInfo(wlTrackInfoTextType type, bool& avail) const
   {
      std::map<wlTrackInfoTextType, CString>::const_iterator iter = m_mapTextInfos.find(type);
      avail = iter != m_mapTextInfos.end();
      return avail ? iter->second : _T("");
   }

   //! sets a number info value
   void NumberInfo(wlTrackInfoNumberType type, int value)
   {
      m_mapNumberInfos[type] = value;
   }

   //! retrieves a number info value
   int NumberInfo(wlTrackInfoNumberType type, bool& avail) const
   {
      std::map<wlTrackInfoNumberType, int>::const_iterator iter = m_mapNumberInfos.find(type);
      avail = iter != m_mapNumberInfos.end();
      return avail ? iter->second : -1;
   }

   //! returns if track info is empty
   bool IsEmpty() const throw() { return m_mapTextInfos.empty() && m_mapNumberInfos.empty(); }

   //! converts genre ID to text
   static CString GenreIDToText(unsigned int uGenreID);

   //! converts genre text to id
   static unsigned int TextToGenreID(const CString& cszText);

   /// returns genre list array
   static LPCTSTR* GetGenreList();

   /// returns length of genre list array
   static unsigned int GetGenreListLength();

protected:
   //! text infos map
   std::map<wlTrackInfoTextType, CString> m_mapTextInfos;

   //! number infos map
   std::map<wlTrackInfoNumberType, int> m_mapNumberInfos;
};


//@}

#endif
