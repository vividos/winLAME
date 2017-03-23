//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file CoverArtArchive.hpp
/// \brief Access to coverart.org
//
#pragma once

#include <map>
#include "rapidxml/rapidxml.hpp"
// atlimage.h declares throw() on methods that may throw; this would lead to calling
// std::unexpected(), which would quit the application; define throw() as empty macro in order to
// get the different behavior that exceptions are propagated to the caller.
#define throw()
#pragma warning(push)
#pragma warning(disable: 4002) // too many actual parameters for macro 'throw'
#include <atlimage.h>
#undef throw
#pragma warning(pop)

/// contains result infos for cover art request
class CoverArtResult
{
public:
   /// creates a new cover art result object with given data for front cover
   CoverArtResult(const std::vector<unsigned char>& frontCover)
      :m_frontCover(frontCover)
   {
   }

   /// returns jpeg data of front cover
   const std::vector<unsigned char>& FrontCover() const { return m_frontCover; }

private:
   /// jpeg data of front cover
   std::vector<unsigned char> m_frontCover;
};

/// \brief Manages access to http://www.coverartarchive.org/
class CoverArtArchive
{
public:
   /// ctor; specifies user agent
   CoverArtArchive(const std::string& userAgent);
   /// dtor
   ~CoverArtArchive();

   /// sends a request
   std::vector<CoverArtResult> Request(const std::string& musicBrainzDiscId, bool onlyLoadFirst);

   /// converts image from jpeg byte array to ATL CImage object
   static bool ImageFromJpegByteArray(const std::vector<unsigned char>& data, ATL::CImage& image);

private:
   /// gets MusicBrainz disc info as xml response, from given disc id
   std::string GetMusicBrainzDiscIdInfo(const std::string& musicBrainzDiscId);

   /// collects all cover art release ids inside the response xml document
   void CollectReleaseIds(rapidxml::xml_document<char>& responseXml, std::vector<std::string>& releaseIdList);

   /// downloads cover art for given release id and stores result in result list
   void DownloadCoverArt(const std::string& coverArtReleaseId, std::vector<CoverArtResult>& resultList);

private:
   /// user agent to use for requests
   std::string m_userAgent;

   /// cache for cover art results
   std::map<std::string, CoverArtResult> m_resultCache;
};
