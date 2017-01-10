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
/// \file CDRipTrackInfo.h
/// \brief CD rip track info
//
#pragma once

/// contains infos about a CD track to rip
struct CDRipTrackInfo
{
   /// ctor
   CDRipTrackInfo()
      :m_numTrackOnDisc(0),
      m_trackLengthInSeconds(0),
      m_isActive(true)
   {
   }

   /// number of track on disc
   unsigned int m_numTrackOnDisc;

   /// track title
   CString m_trackTitle;
   //CString m_trackArtist; // TODO fill

   /// track length, in seconds
   unsigned int m_trackLengthInSeconds;

   /// indicates if track is active (should be ripped)
   bool m_isActive;

   /// ripped filename in temp folder
   CString m_rippedFilename;
};
