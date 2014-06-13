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
/// \file InputFilesParser.h
/// \brief Input files parser

// include guard
#pragma once

/// includes
#include <vector>

/// \brief parses input files
/// \details When input is folder name, the parser adds all files recursively.
/// When input is playlists (.m3u, .pls) or cue sheets (.cue), it adds the the referenced files.
/// When input is a normal existing file, it adds it to the file list.
class InputFilesParser
{
public:
   /// ctor
   InputFilesParser() throw()
      :m_bRecursive(false)
   {
   }

   /// returns file list; const version
   const std::vector<CString>& FileList() const { return m_vecFileList; }
   /// returns file list
         std::vector<CString>& FileList()       { return m_vecFileList; }

   /// returns playlist name; when a playlist was added, this is the same name (else it is empty)
   CString PlaylistName() { return m_cszPlaylistName; }

   /// parses list of filenames
   void Parse(const std::vector<CString>& vecFilenames);

   /// inserts a single file
   void Insert(LPCTSTR filename);

private:
   /// inserts filename; no parsing or recursing
   void InsertFilename(LPCTSTR filename);

   /// imports .m3u playlist
   void ImportM3uPlaylist(LPCTSTR filename);

   /// imports .pls playlist
   void ImportPlsPlaylist(LPCTSTR filename);

   /// imports .cue cue sheet
   void ImportCueSheet(LPCTSTR filename);

private:
   /// indicates if Insert() currently operates recursively
   bool m_bRecursive;

   /// file list
   std::vector<CString> m_vecFileList;

   /// possible playlist name
   CString m_cszPlaylistName;
};
