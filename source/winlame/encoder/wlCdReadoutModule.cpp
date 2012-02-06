/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005-2007 Michael Fink

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

   $Id: wlCdReadoutModule.cpp,v 1.11 2009/11/02 20:30:52 vividos Exp $

*/
/*! \file wlCdReadoutModule.cpp

   \brief contains the implementation of the Bass CD Readout module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "wlCDReadoutModule.h"
#include "wlCDRipTrackManager.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// wlCDReadoutModule methods

wlCDReadoutModule::wlCDReadoutModule()
{
   module_id = ID_IM_CDRIP;
}

wlInputModule *wlCDReadoutModule::cloneModule()
{
   return new wlCDReadoutModule;
}

void wlCDReadoutModule::getDescription(CString& desc)
{
   wlSndFileInputModule::getDescription(desc); // TODO add track data
}

CString wlCDReadoutModule::getFilterString()
{
   return _T(""); // no filter
}

void wlCDReadoutModule::resolveRealFilename(CString& filename)
{
   wlCDRipTrackManager* pManager = wlCDRipTrackManager::getCDRipTrackManager();

   unsigned int nTrackIndex = static_cast<unsigned int>(
      _tcstoul(static_cast<LPCTSTR>(filename) + _tcslen(g_pszCDRipPrefix), NULL, 10));
   ATLASSERT(nTrackIndex < pManager->GetMaxTrackInfo());

   filename = pManager->GetTrackInfo(m_nTrackIndex).m_cszRippedFilename;
}

int wlCDReadoutModule::initInput(LPCTSTR infilename, wlSettingsManager &mgr,
   wlTrackInfo &trackinfo, wlSampleContainer &samplecont)
{
   CString cszCDRipFilename(infilename);
   ATLASSERT(0 == cszCDRipFilename.Find(g_pszCDRipPrefix));

   wlCDRipTrackManager* pManager = wlCDRipTrackManager::getCDRipTrackManager();

   m_nTrackIndex = static_cast<unsigned int>(_tcstoul(cszCDRipFilename.Mid(_tcslen(g_pszCDRipPrefix)), NULL, 10));
   ATLASSERT(m_nTrackIndex < pManager->GetMaxTrackInfo());

   wlCDRipTrackInfo& cdtrackinfo = pManager->GetTrackInfo(m_nTrackIndex);

   int nRet = wlSndFileInputModule::initInput(cdtrackinfo.m_cszRippedFilename, mgr, trackinfo, samplecont);

   wlCDRipDiscInfo& discinfo = pManager->GetDiscInfo();

   // add track info
   trackinfo.TextInfo(wlTrackInfoTitle, cdtrackinfo.m_cszTrackTitle);

   CString value = discinfo.m_cszDiscArtist;
   if (discinfo.m_bVariousArtists)
      value.LoadString(IDS_CDRIP_ARTIST_VARIOUS);
   
   trackinfo.TextInfo(wlTrackInfoArtist, value);

   trackinfo.TextInfo(wlTrackInfoAlbum, discinfo.m_cszDiscTitle);

   CString cszFormat;

   // year
   if (discinfo.m_nYear != 0)
      trackinfo.NumberInfo(wlTrackInfoYear, discinfo.m_nYear);

   // track number
   trackinfo.NumberInfo(wlTrackInfoTrack, cdtrackinfo.m_nTrackOnDisc+1);

   // genre
   if (!discinfo.m_cszGenre.IsEmpty())
      trackinfo.TextInfo(wlTrackInfoGenre, discinfo.m_cszGenre);

   return nRet;
}

void wlCDReadoutModule::doneInput(bool fCompletedTrack)
{
   wlSndFileInputModule::doneInput();

   // delete temporary file
   if (fCompletedTrack)
   {
      wlCDRipTrackManager* pManager = wlCDRipTrackManager::getCDRipTrackManager();
      wlCDRipTrackInfo& cdtrackinfo = pManager->GetTrackInfo(m_nTrackIndex);

      DeleteFile(cdtrackinfo.m_cszRippedFilename);
      cdtrackinfo.m_cszRippedFilename.Empty();
      cdtrackinfo.m_bActive = false; // switch inactive
   }
}
