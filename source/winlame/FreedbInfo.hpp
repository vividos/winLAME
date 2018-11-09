//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file FreedbInfo.hpp
/// \brief FreeDB CD infos
//
#pragma once

/// Infos about a CD, retrieved from FreeDB
class FreedbInfo
{
public:
   /// ctor; parses http response entry
   explicit FreedbInfo(const CString& entry);

   /// returns disc artist
   CString DiscArtist() const { return EntryOrDefault(_T("DARTIST")); }

   /// returns disc title
   CString DiscTitle() const { return EntryOrDefault(_T("DTITLE")); }

   /// returns disc genre
   CString Genre() const { return EntryOrDefault(_T("DGENRE")); }

   /// returns disc year
   CString Year() const { return EntryOrDefault(_T("DYEAR")); }

   /// returns list of track titles
   const std::vector<CString>& TrackTitles() const { return m_allTrackTitles; }

private:
   /// parses http response entry
   void ParseEntry(CString entry);

   /// parses track titles from all entries
   void ParseTrackTitles();

   /// returns entry with given name, or default
   CString EntryOrDefault(LPCTSTR entryName, LPCTSTR defaultValue = _T("")) const;

private:
   /// all CD info entries
   std::map<CString, CString> m_allEntries;

   /// all track titles
   std::vector<CString> m_allTrackTitles;
};
