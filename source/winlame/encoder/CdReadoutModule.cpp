//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2005-2017 Michael Fink
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
/// \file CDReadoutModule.cpp
/// \brief contains the implementation of the Bass CD Readout module
//
#include "stdafx.h"
#include "resource.h"
#include "CDReadoutModule.hpp"
#include "CDRipTrackManager.hpp"

using Encoder::CDReadoutModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

CDReadoutModule::CDReadoutModule()
   :m_trackIndex(0)
{
   m_moduleId = ID_IM_CDRIP;
}

Encoder::InputModule* CDReadoutModule::CloneModule()
{
   return new CDReadoutModule;
}

CString CDReadoutModule::GetDescription() const
{
   CString text;
   text.Format(IDS_CDEXTRACT_DESC_US,
      m_trackIndex,
      SndFileInputModule::GetDescription().GetString());
   return text;
}

CString CDReadoutModule::GetFilterString() const
{
   return CString(); // no filter
}

void CDReadoutModule::ResolveRealFilename(CString& filename)
{
   CDRipTrackManager* ripTrackManager = CDRipTrackManager::getCDRipTrackManager();

   unsigned int trackIndex = static_cast<unsigned int>(
      _tcstoul(static_cast<LPCTSTR>(filename) + _tcslen(g_pszCDRipPrefix), NULL, 10));
   ATLASSERT(trackIndex < ripTrackManager->GetMaxTrackInfo());

   filename = ripTrackManager->GetTrackInfo(trackIndex).m_rippedFilename;
}

int CDReadoutModule::InitInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackInfo, SampleContainer& samples)
{
   CString ripFilename(infilename);
   ATLASSERT(0 == ripFilename.Find(g_pszCDRipPrefix)); // must start with prefix

   CDRipTrackManager* ripTrackManager = CDRipTrackManager::getCDRipTrackManager();

   m_trackIndex = static_cast<unsigned int>(_tcstoul(ripFilename.Mid(_tcslen(g_pszCDRipPrefix)), NULL, 10));
   ATLASSERT(m_trackIndex < ripTrackManager->GetMaxTrackInfo());

   CDRipTrackInfo& cdTrackInfo = ripTrackManager->GetTrackInfo(m_trackIndex);

   int nRet = SndFileInputModule::InitInput(cdTrackInfo.m_rippedFilename, mgr, trackInfo, samples);

   CDRipDiscInfo& discInfo = ripTrackManager->GetDiscInfo();

   // add track info
   trackInfo.TextInfo(TrackInfoTitle, cdTrackInfo.m_trackTitle);

   CString value = discInfo.m_discArtist;
   if (discInfo.m_variousArtists)
      value.LoadString(IDS_CDRIP_ARTIST_VARIOUS);

   trackInfo.TextInfo(TrackInfoArtist, value);

   trackInfo.TextInfo(TrackInfoAlbum, discInfo.m_discTitle);

   trackInfo.TextInfo(TrackInfoDiscArtist, discInfo.m_discArtist);

   if (discInfo.m_year != 0)
      trackInfo.NumberInfo(TrackInfoYear, discInfo.m_year);

   trackInfo.NumberInfo(TrackInfoTrack, cdTrackInfo.m_numTrackOnDisc + 1);

   if (!discInfo.m_genre.IsEmpty())
      trackInfo.TextInfo(TrackInfoGenre, discInfo.m_genre);

   if (!ripTrackManager->GetFrontCoverArtImage().empty())
   {
      trackInfo.BinaryInfo(TrackInfoFrontCover, ripTrackManager->GetFrontCoverArtImage());
   }

   return nRet;
}

void CDReadoutModule::DoneInput(bool isTrackCompleted)
{
   SndFileInputModule::DoneInput();

   // delete temporary file
   if (isTrackCompleted)
   {
      CDRipTrackManager* ripTrackManager = CDRipTrackManager::getCDRipTrackManager();
      CDRipTrackInfo& cdTrackInfo = ripTrackManager->GetTrackInfo(m_trackIndex);

      DeleteFile(cdTrackInfo.m_rippedFilename);
      cdTrackInfo.m_rippedFilename.Empty();
      cdTrackInfo.m_isActive = false; // switch inactive
   }
}
