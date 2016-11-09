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
#include "CDRipTrackManager.h"

CString CDRipTitleFormatManager::FormatTitle(const CString& format,
   const CDRipDiscInfo& discInfo, const CDRipTrackInfo& trackInfo)
{
   CString title = format;

   title.Replace(_T("%album%"), discInfo.m_cszDiscTitle);
   title.Replace(_T("%artist%"), /*discInfo.m_bVariousArtists ? trackInfo.m_cszTrackArtist :*/ discInfo.m_cszDiscArtist);
   title.Replace(_T("%genre%"), discInfo.m_cszGenre);
   title.Replace(_T("%title%"), trackInfo.m_cszTrackTitle);

   CString trackNumber;
   trackNumber.Format(_T("%u"), trackInfo.m_nTrackOnDisc + 1);
   title.Replace(_T("%track%"), trackNumber);

   CString year;
   year.Format(_T("%u"), discInfo.m_nYear);
   title.Replace(_T("%year%"), year);

   title.Replace(_T("%albumartist%"), discInfo.m_cszDiscArtist);

   CString lengthInSeconds;
   lengthInSeconds.Format(_T("%u"), trackInfo.m_nTrackLength);
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
