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

   $Id: wlCDRipTrackManager.cpp,v 1.3 2005/09/27 18:30:10 vividos Exp $

*/
/*! \file wlCDRipTrackManager.cpp

   \brief contains the cd audio extraction manager

*/

// needed includes
#include "stdafx.h"
#include "wlCDRipTrackManager.h"
#include <memory>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


LPCTSTR g_pszCDRipPrefix = _T("cdrip://");

wlCDRipTrackManager* wlCDRipTrackManager::m_pManagerInstance = NULL;

static std::auto_ptr<wlCDRipTrackManager> g_apAutoFreeTrackManager;


// wlCDRipTrackManager methods
wlCDRipTrackManager::~wlCDRipTrackManager()
{
   // delete all temporary files not removed yet
   unsigned int nMax = m_vecTrackInfo.size();
   for(unsigned int n=0; n<nMax; n++)
   {
      wlCDRipTrackInfo& cdtrackinfo = m_vecTrackInfo[n];
      if (!cdtrackinfo.m_cszRippedFilename.IsEmpty())
      {
         DeleteFile(cdtrackinfo.m_cszRippedFilename);
         cdtrackinfo.m_cszRippedFilename.Empty();
      }
   }
}

wlCDRipTrackManager* wlCDRipTrackManager::getCDRipTrackManager()
{
   if (wlCDRipTrackManager::m_pManagerInstance == NULL)
   {
      m_pManagerInstance = new wlCDRipTrackManager;
      g_apAutoFreeTrackManager = std::auto_ptr<wlCDRipTrackManager>(m_pManagerInstance);
   }

   return m_pManagerInstance;
}
