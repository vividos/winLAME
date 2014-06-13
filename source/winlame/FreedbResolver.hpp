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
/// \file FreedbResolver.hpp
/// \brief freedb CD info resolver

// include guard
#pragma once

// includes
#include <freedb.hpp>

/// resolves CD infos by using freedb server
class FreedbResolver
{
public:
   /// ctor
   FreedbResolver(const CString& cszServer, const CString& cszUsername);
   /// dtor
   ~FreedbResolver() throw();

   /// returns if internet (for resolving cd infos) is avail
   static bool IsAvail();

   /// looks up CD info by CDDB CD ID
   bool Lookup(LPCSTR pszCddbCdId, CString& cszErrorMessage);

   /// reads CD info for a given result index
   void ReadResultInfo(unsigned int uiResultIndex);

   /// returns all results found (may be multiple)
   const std::vector<Freedb::SearchResult>& Results() const throw() { return m_vecResults; }

   /// returns resolved CD info (after calling ReadResultInfo())
   const Freedb::CDInfo& CDInfo() const throw() { return m_info; }

private:
   /// freedb username
   CString m_cszUsername;

   /// freedb remote instance
   Freedb::Remote m_remoteFreedb;

   /// search results
   std::vector<Freedb::SearchResult> m_vecResults;

   /// CD info
   Freedb::CDInfo m_info;
};
