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
/// \file CoverArtArchive.cpp
/// \brief Access to coverart.org
//

#include "stdafx.h"
#include "CoverArtArchive.hpp"
#include "HttpClient.hpp"

CoverArtArchive::CoverArtArchive(const std::string& userAgent)
   :m_userAgent(userAgent)
{
}

CoverArtArchive::~CoverArtArchive()
{
}

/// \param[in] musicBrainzDiscId is a SHA-1 hash of the disc TOC, e.g. like
/// h1RuRzDGd48cHYnfOVN6e6wGr3o-
/// \param[in] onlyLoadFirst indicates if only the first entry should be loaded
std::vector<CoverArtResult> CoverArtArchive::Request(const std::string& musicBrainzDiscId, bool onlyLoadFirst)
{
   //http:// 9712d52a-4509-3d4b-a1a2-67c88c643e31
   //http://coverartarchive.org/release/0a7dd30c-ab7e-4a94-9607-00f02f4115be/front-500

   // first request: get release(s) from disc id
   std::string responseText = GetMusicBrainzDiscIdInfo(musicBrainzDiscId);

   // parse response xml
   std::vector<char> responseTextBuffer;
   responseTextBuffer.assign(responseText.begin(), responseText.end());
   responseTextBuffer.push_back(0);

   rapidxml::xml_document<char> responseXml;

   try
   {
      responseXml.parse<0>(responseTextBuffer.data());
   }
   catch (const std::exception& ex)
   {
      UNUSED(ex);
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
      return std::vector<CoverArtResult>();
   }

   // collect all release IDs
   std::vector<std::string> releaseIdList;
   CollectReleaseIds(responseXml, releaseIdList);

   std::vector<CoverArtResult> resultList;

   for (size_t index = 0, maxIndex = releaseIdList.size(); index < maxIndex; index++)
   {
      std::string coverArtReleaseId = releaseIdList[index];

      DownloadCoverArt(coverArtReleaseId, resultList);

      if (onlyLoadFirst)
         break;
   }

   return resultList;
}

std::string CoverArtArchive::GetMusicBrainzDiscIdInfo(const std::string& musicBrainzDiscId)
{
   HttpClient client(m_userAgent);

   std::string urlPath = "/ws/2/discid/" + musicBrainzDiscId;

   auto response = client.Request("musicbrainz.org", urlPath);
   response.response_data.push_back(0);

   return std::string(reinterpret_cast<char*>(response.response_data.data()));
}

void CoverArtArchive::CollectReleaseIds(rapidxml::xml_document<char>& responseXml, std::vector<std::string>& releaseIdList)
{
   try
   {
      rapidxml::xml_node<char>* metadataNode =
         responseXml.first_node("metadata");

      if (metadataNode == nullptr)
         return;

      rapidxml::xml_node<char>* discNode =
         metadataNode->first_node("disc");

      if (discNode == nullptr)
         return;

      rapidxml::xml_node<char>* releaseListNode =
         discNode->first_node("release-list");

      rapidxml::xml_node<char>* releaseNode =
         releaseListNode->first_node("release");

      while (releaseNode != nullptr)
      {
         rapidxml::xml_attribute<char>* attr = releaseNode->first_attribute("id");

         if (attr == nullptr)
            continue;

         std::string attributeValue = attr->value();

         releaseIdList.push_back(attributeValue);

         releaseNode = releaseNode->next_sibling("release");
      }
   }
   catch (const std::exception& ex)
   {
      UNUSED(ex);
      ATLTRACE(_T("xml exception: %hs\n"), ex.what());
   }
}

/// param[in] coverArtReleaseId is a release ID that is used by MusicBrainz
/// and CoverArt to describe a unique artist or release, written as a UUID,
/// e.g. like 9712d52a-4509-3d4b-a1a2-67c88c643e31
void CoverArtArchive::DownloadCoverArt(const std::string& coverArtReleaseId, std::vector<CoverArtResult>& resultList)
{
   std::string urlPath = "/release/" + coverArtReleaseId + "/front-500";

   HttpClient client(m_userAgent);

   auto response = client.Request("coverartarchive.org", urlPath);
   while (response.status_code / 100 == 3)
   {
      // 3xx are redirect status codes, to point to the actual cover art
      for (size_t headerIndex = 0, maxHeaderIndex = response.header_lines.size(); headerIndex < maxHeaderIndex; headerIndex++)
      {
         std::string& headerLine = response.header_lines[headerIndex];
         if (headerLine.find("Location: ") != std::string::npos)
         {
            CString url(headerLine.c_str());
            url.Trim();
            url.Replace(_T("Location: "), _T(""));
            url.Replace(_T("http://"), _T(""));

            int pos = url.Find(_T('/'));
            if (pos != -1)
            {
               std::string host = CStringA(url.Left(pos)).GetString();
               std::string path = CStringA(url.Mid(pos)).GetString();

               response = client.Request(host, path);
            }
         }
      }
   }

   resultList.push_back(CoverArtResult(response.response_data));
}

/// \see http://stackoverflow.com/questions/3514275/load-image-from-memory-buffer-using-atl-cimage
bool CoverArtArchive::ImageFromJpegByteArray(const std::vector<unsigned char>& data, ATL::CImage& image)
{
   HGLOBAL hGlobal = ::GlobalAlloc(GHND, data.size());
   if (hGlobal == nullptr)
      return false;

   LPBYTE lpByte = (LPBYTE)::GlobalLock(hGlobal);
   if (lpByte == nullptr)
   {
      GlobalFree(hGlobal);
      return false;
   }

   CopyMemory(lpByte, data.data(), data.size());
   ::GlobalUnlock(hGlobal);

   IStream* pStream = nullptr;

   HRESULT hr = ::CreateStreamOnHGlobal(hGlobal, FALSE, &pStream);

   bool ret = SUCCEEDED(hr);
   if (ret)
   {
      image.Destroy();
      image.Load(pStream);
      pStream->Release();
   }

   GlobalFree(hGlobal);

   return ret;
}
