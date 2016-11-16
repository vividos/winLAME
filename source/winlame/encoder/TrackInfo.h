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
/// \file TrackInfo.h
/// \brief manages informations and properties of a track to encode
/// \details id3 tags can be passed in and can be generated from the properties
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include <string>
#include <map>


/// track text info enum
typedef enum
{
   TrackInfoTitle=0,
   TrackInfoArtist,
   TrackInfoAlbum,
   TrackInfoComment,
   TrackInfoGenre,
} TrackInfoTextType;

/// track number info enum
typedef enum
{
   TrackInfoYear=0,
   TrackInfoTrack,
} TrackInfoNumberType;


/// track info class
class TrackInfo
{
public:
   /// ctor
   TrackInfo()
   {
   }

   /// resets all infos
   void ResetInfos()
   {
      m_mapTextInfos.clear();
      m_mapNumberInfos.clear();
   }

   /// sets a text info value
   void TextInfo(TrackInfoTextType type, CString value)
   {
      m_mapTextInfos[type] = value;
   }

   /// retrieves a text info value
   CString TextInfo(TrackInfoTextType type, bool& avail) const
   {
      std::map<TrackInfoTextType, CString>::const_iterator iter = m_mapTextInfos.find(type);
      avail = iter != m_mapTextInfos.end();
      return avail ? iter->second : _T("");
   }

   /// retrieves a text info value
   bool TextInfo(TrackInfoTextType type, CString& text) const
   {
      std::map<TrackInfoTextType, CString>::const_iterator iter = m_mapTextInfos.find(type);
      bool avail = iter != m_mapTextInfos.end();
      if (avail)
         text = iter->second;
      else
         text.Empty();

      return avail;
   }

   /// sets a number info value
   void NumberInfo(TrackInfoNumberType type, int value)
   {
      m_mapNumberInfos[type] = value;
   }

   /// retrieves a number info value
   int NumberInfo(TrackInfoNumberType type, bool& avail) const
   {
      std::map<TrackInfoNumberType, int>::const_iterator iter = m_mapNumberInfos.find(type);
      avail = iter != m_mapNumberInfos.end();
      return avail ? iter->second : -1;
   }

   /// returns if track info is empty
   bool IsEmpty() const throw() { return m_mapTextInfos.empty() && m_mapNumberInfos.empty(); }

   /// converts genre ID to text
   static CString GenreIDToText(unsigned int uGenreID);

   /// converts genre text to id
   static unsigned int TextToGenreID(const CString& cszText);

   /// returns genre list array
   static LPCTSTR* GetGenreList();

   /// returns length of genre list array
   static unsigned int GetGenreListLength();

protected:
   /// text infos map
   std::map<TrackInfoTextType, CString> m_mapTextInfos;

   /// number infos map
   std::map<TrackInfoNumberType, int> m_mapNumberInfos;
};


/// @}
