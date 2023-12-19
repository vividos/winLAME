//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2023 Michael Fink
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
/// \file CDRipDiscInfo.hpp
/// \brief CD rip disc info
//
#pragma once

/// contains infos about a CD disc to rip
struct CDRipDiscInfo
{
   /// disc title
   CString m_discTitle;

   /// disc artist
   CString m_discArtist;

   /// indicates if disc has various artists
   bool m_variousArtists = false;

   /// disc year
   unsigned int m_year = 0;

   /// disc drive index
   unsigned int m_discDrive = 0;

   /// genre of CD
   CString m_genre;

   /// FreeDB's CDID
   CString m_CDID;

   /// number of tracks on the CD
   unsigned int m_numTracks = 0;

   /// disc number; starts at 1
   unsigned int m_discNumber = 0;
};
