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

*/
/*! \file CdReadoutModule.cpp

   \brief contains the implementation of the Bass CD Readout module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "CDReadoutModule.h"
#include "CDRipTrackManager.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CDReadoutModule methods

CDReadoutModule::CDReadoutModule()
{
   module_id = ID_IM_CDRIP;
}

InputModule *CDReadoutModule::cloneModule()
{
   return new CDReadoutModule;
}

void CDReadoutModule::getDescription(CString& desc)
{
   SndFileInputModule::getDescription(desc); // TODO add track data
}

CString CDReadoutModule::getFilterString()
{
   return _T(""); // no filter
}

void CDReadoutModule::resolveRealFilename(CString& filename)
{
   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

   unsigned int nTrackIndex = static_cast<unsigned int>(
      _tcstoul(static_cast<LPCTSTR>(filename) + _tcslen(g_pszCDRipPrefix), NULL, 10));
   ATLASSERT(nTrackIndex < pManager->GetMaxTrackInfo());

   filename = pManager->GetTrackInfo(m_nTrackIndex).m_cszRippedFilename;
}

int CDReadoutModule::initInput(LPCTSTR infilename, SettingsManager &mgr,
   TrackInfo &trackinfo, SampleContainer &samplecont)
{
   CString cszCDRipFilename(infilename);
   ATLASSERT(0 == cszCDRipFilename.Find(g_pszCDRipPrefix));

   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

   m_nTrackIndex = static_cast<unsigned int>(_tcstoul(cszCDRipFilename.Mid(_tcslen(g_pszCDRipPrefix)), NULL, 10));
   ATLASSERT(m_nTrackIndex < pManager->GetMaxTrackInfo());

   CDRipTrackInfo& cdtrackinfo = pManager->GetTrackInfo(m_nTrackIndex);

   int nRet = SndFileInputModule::initInput(cdtrackinfo.m_cszRippedFilename, mgr, trackinfo, samplecont);

   CDRipDiscInfo& discinfo = pManager->GetDiscInfo();

   // add track info
   trackinfo.TextInfo(TrackInfoTitle, cdtrackinfo.m_cszTrackTitle);

   CString value = discinfo.m_cszDiscArtist;
   if (discinfo.m_bVariousArtists)
      value.LoadString(IDS_CDRIP_ARTIST_VARIOUS);
   
   trackinfo.TextInfo(TrackInfoArtist, value);

   trackinfo.TextInfo(TrackInfoAlbum, discinfo.m_cszDiscTitle);

   CString cszFormat;

   // year
   if (discinfo.m_nYear != 0)
      trackinfo.NumberInfo(TrackInfoYear, discinfo.m_nYear);

   // track number
   trackinfo.NumberInfo(TrackInfoTrack, cdtrackinfo.m_nTrackOnDisc+1);

   // genre
   if (!discinfo.m_cszGenre.IsEmpty())
      trackinfo.TextInfo(TrackInfoGenre, discinfo.m_cszGenre);

   return nRet;
}

void CDReadoutModule::doneInput(bool fCompletedTrack)
{
   SndFileInputModule::doneInput();

   // delete temporary file
   if (fCompletedTrack)
   {
      CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();
      CDRipTrackInfo& cdtrackinfo = pManager->GetTrackInfo(m_nTrackIndex);

      DeleteFile(cdtrackinfo.m_cszRippedFilename);
      cdtrackinfo.m_cszRippedFilename.Empty();
      cdtrackinfo.m_bActive = false; // switch inactive
   }
}
