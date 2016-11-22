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
/// \file CDRipTrackInfo.hpp
/// \brief CD rip track info
//
#pragma once

struct CDRipTrackInfo
{
   CDRipTrackInfo()
      :m_nTrackOnDisc(0),
      m_nTrackLength(0),
      m_bActive(true)
   {
   }

   unsigned int m_nTrackOnDisc;
   CString m_cszTrackTitle;
   //CString m_cszTrackArtist; // TODO fill

   unsigned int m_nTrackLength;

   bool m_bActive;

   CString m_cszRippedFilename;
};
