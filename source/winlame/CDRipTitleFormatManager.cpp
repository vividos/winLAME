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
/// \file CDRipTitleFormatManager.cpp
/// \brief CD rip title format manager
//
#include "stdafx.h"
#include "CDRipTitleFormatManager.hpp"
#include "CDRipTrackManager.hpp"
#include "UISettings.hpp"

CString CDRipTitleFormatManager::FormatTitle(UISettings& settings,
   const CDRipDiscInfo& discInfo, const CDRipTrackInfo& trackInfo)
{
   return FormatTitle(
      discInfo.m_variousArtists ? settings.cdrip_format_various_track : settings.cdrip_format_album_track,
      discInfo, trackInfo);
}

CString CDRipTitleFormatManager::FormatTitle(const CString& format,
   const CDRipDiscInfo& discInfo, const CDRipTrackInfo& trackInfo)
{
   CString title = format;

   title.Replace(_T("%album%"), discInfo.m_discTitle);
   title.Replace(_T("%artist%"), discInfo.m_variousArtists ? trackInfo.m_trackArtist : discInfo.m_discArtist);
   title.Replace(_T("%genre%"), discInfo.m_genre);
   title.Replace(_T("%title%"), trackInfo.m_trackTitle);

   CString trackNumber;
   int numDigits = discInfo.m_numTracks < 10 ? 1 : discInfo.m_numTracks < 100 ? 2 : 3;
   trackNumber.Format(_T("%0*u"), numDigits, trackInfo.m_numTrackOnDisc + 1);
   title.Replace(_T("%track%"), trackNumber);

   CString year;
   year.Format(_T("%u"), discInfo.m_year);
   title.Replace(_T("%year%"), year);

   title.Replace(_T("%albumartist%"), discInfo.m_discArtist);

   CString lengthInSeconds;
   lengthInSeconds.Format(_T("%u"), trackInfo.m_trackLengthInSeconds);
   title.Replace(_T("%length%"), lengthInSeconds);

   //title.Replace(_T("%discnumber%"), discInfo.);

   return title;
}

CString CDRipTitleFormatManager::GetFilenameByTitle(const CString& title)
{
   CString filename = title;

   filename.Replace(_T("\\"), _T(""));
   filename.Replace(_T("/"), _T(""));
   filename.Replace(_T(":"), _T(""));
   filename.Replace(_T("*"), _T(""));
   filename.Replace(_T("?"), _T(""));
   filename.Replace(_T("\""), _T(""));
   filename.Replace(_T("<"), _T(""));
   filename.Replace(_T(">"), _T(""));
   filename.Replace(_T("|"), _T(""));

   return filename;
}

std::vector<CString> CDRipTitleFormatManager::GetAllTags()
{
   std::vector<CString> allTags =
   {
      _T("album"),
      _T("artist"),
      _T("genre"),
      _T("title"),
      _T("track"),
      _T("year"),
      _T("albumartist"),
      _T("length"),
      //_T("discnumber"),
   };

   return allTags;
}
