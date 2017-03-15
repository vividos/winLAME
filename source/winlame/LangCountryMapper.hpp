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
/// \file LangCountryMapper.hpp
/// \brief mapping from language code to country code
//
#pragma once

#include <map>

/// mapping from language code to country code
class LangCountryMapper
{
public:
   /// ctor
   LangCountryMapper();

   /// maps from language code to country code
   LPCTSTR CountryCodeFromLanguageCode(UINT uiLanguageCode) const;

   /// returns image index from language code
   int IndexFromLanguageCode(UINT uiLanguageCode) const;

public:
   /// map with mapping from country code to index
   std::map<CString, int> m_mapCountryCodeToIndex;
};
