//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2005 Michael Fink
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
/// \file CDRipTrackManager.hpp
/// \brief contains the cd audio extraction manager
//
#pragma once

// needed includes
#include <boost/noncopyable.hpp>
#include "CDRipDiscInfo.hpp"
#include "CDRipTrackInfo.hpp"

extern LPCTSTR g_pszCDRipPrefix;

/// cd audio extraction manager
class CDRipTrackManager: public boost::noncopyable
{
public:
   static CDRipTrackManager* getCDRipTrackManager();
   ~CDRipTrackManager();

   CDRipDiscInfo& GetDiscInfo(){ return m_discInfo; }

   unsigned int GetMaxTrackInfo(){ return m_vecTrackInfo.size(); }

   CDRipTrackInfo& GetTrackInfo(unsigned int nIndex){ ATLASSERT(nIndex < GetMaxTrackInfo()); return m_vecTrackInfo[nIndex]; }

   void RemoveTrackInfo(unsigned int nIndex){ ATLASSERT(nIndex < GetMaxTrackInfo()); m_vecTrackInfo.erase(m_vecTrackInfo.begin() + nIndex); }

   void ResetTrackInfo(){ m_vecTrackInfo.clear(); }

   void AddTrackInfo(CDRipTrackInfo& info){ m_vecTrackInfo.push_back(info); }

protected:
   CDRipDiscInfo m_discInfo;
   std::vector<CDRipTrackInfo> m_vecTrackInfo;

   static CDRipTrackManager* m_pManagerInstance;
};
