//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014 Michael Fink
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
/// \file FreedbResolver.cpp
/// \brief freedb CD info resolver

#include "stdafx.h"
#include "FreedbResolver.hpp"
#include "DynamicLibrary.h"
#include "App.h"

FreedbResolver::FreedbResolver(const CString& cszServer, const CString& cszUsername)
:m_cszUsername(cszUsername),
m_remoteFreedb(std::string(CStringA(cszServer).GetString()))
{
   WSADATA wsaData;
   WORD wVersionRequested = MAKEWORD(2, 2);
   WSAStartup(wVersionRequested, &wsaData);
}

FreedbResolver::~FreedbResolver() throw()
{
   WSACleanup();
}

bool FreedbResolver::IsAvail()
{
   return DynamicLibrary(_T("ws2_32.dll")).IsLoaded();
}

bool FreedbResolver::Lookup(LPCSTR pszCddbCdId, CString& cszErrorMessage)
{
   bool bSuccess = true;

   try
   {
      CString cszWinlameVersion = App::Version();
      cszWinlameVersion.Replace(_T(' '), _T('-'));

      std::string username(CStringA(m_cszUsername).GetString());
      std::string version(CStringA(cszWinlameVersion).GetString());
      m_remoteFreedb.doHandshake(username, "winLAME", version);

      m_vecResults = m_remoteFreedb.query_cddb_raw(pszCddbCdId);

      unsigned int nMax = m_vecResults.size();
      if (nMax == 0)
         cszErrorMessage.LoadString(IDS_CDRIP_NO_CDINFO_AVAIL);
   }
   catch(Freedb::Error& err)
   {
      CString cszText, cszTemp;
      cszText.LoadString(IDS_CDRIP_ERROR_FREEDB);
      cszTemp.Format(_T(" (code=%i text=\"%hs\")"), err.code, err.msg.c_str());
      cszTemp.Replace(_T("\n"), _T(""));
      cszTemp.Replace(_T("\r"), _T(""));
      cszText += cszTemp;

      cszErrorMessage = cszText;
      bSuccess = false;
   }
   catch(const std::string& strError)
   {
      CString cszText;
      cszText.LoadString(IDS_CDRIP_ERROR_FREEDB);
      if (!strError.empty())
      {
         cszText += _T(" (");
         cszText += CString(strError.c_str());
         cszText += _T(")");
      }

      cszErrorMessage = cszText;
      bSuccess = false;
   }
   catch(...)
   {
      cszErrorMessage.LoadString(IDS_CDRIP_ERROR_FREEDB);
      bSuccess = false;
   }

   return bSuccess;
}

void FreedbResolver::ReadResultInfo(unsigned int uiResultIndex)
{
   const Freedb::SearchResult& result = m_vecResults[uiResultIndex];

   m_info = m_remoteFreedb.read(result.category, result.discid);
}
