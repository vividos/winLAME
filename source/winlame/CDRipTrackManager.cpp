/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005 Michael Fink

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
/// \file CDRipTrackManager.cpp
/// \brief contains the cd audio extraction manager

// needed includes
#include "stdafx.h"
#include "CDRipTrackManager.h"
#include <memory>

LPCTSTR g_pszCDRipPrefix = _T("cdrip://");

CDRipTrackManager* CDRipTrackManager::m_pManagerInstance = NULL;

static std::auto_ptr<CDRipTrackManager> g_apAutoFreeTrackManager;


// CDRipTrackManager methods
CDRipTrackManager::~CDRipTrackManager()
{
   // delete all temporary files not removed yet
   unsigned int nMax = m_vecTrackInfo.size();
   for(unsigned int n=0; n<nMax; n++)
   {
      CDRipTrackInfo& cdtrackinfo = m_vecTrackInfo[n];
      if (!cdtrackinfo.m_rippedFilename.IsEmpty())
      {
         DeleteFile(cdtrackinfo.m_rippedFilename);
         cdtrackinfo.m_rippedFilename.Empty();
      }
   }
}

CDRipTrackManager* CDRipTrackManager::getCDRipTrackManager()
{
   if (CDRipTrackManager::m_pManagerInstance == NULL)
   {
      m_pManagerInstance = new CDRipTrackManager;
      g_apAutoFreeTrackManager = std::auto_ptr<CDRipTrackManager>(m_pManagerInstance);
   }

   return m_pManagerInstance;
}
