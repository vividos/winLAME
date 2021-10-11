//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2021 Michael Fink
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
/// \file InputFilesParser.cpp
/// \brief Input files parser

#include "stdafx.h"
#include "InputFilesParser.hpp"
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

void InputFilesParser::Parse(const std::vector<CString>& vecFilenames)
{
   std::for_each(vecFilenames.begin(), vecFilenames.end(), [&](const CString& cszFilename)
   {
      Insert(cszFilename);
   });
}

void InputFilesParser::Insert(LPCTSTR filename)
{
   // check if given filename is a directory
   DWORD dwAttr = ::GetFileAttributes(filename);
   if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
   {
      CString cszPathStr(filename);
      cszPathStr += _T("\\*.*");

      _tfinddata_t fdata;
      long ffhnd;

      m_bRecursive = true;

      if (-1 != (ffhnd = _tfindfirst(cszPathStr, &fdata)))
         do
         {
         if (fdata.name[0] != '.')
         {
            CString fname(filename);
            fname += _T('\\');
            fname += fdata.name;

            Insert(fname);
         }
         } while (0 == _tfindnext(ffhnd, &fdata));

      _findclose(ffhnd);

      m_bRecursive = false;

      return;
   }

   InsertFilename(filename);
}

void InputFilesParser::InsertFilename(LPCTSTR filename)
{
   bool bFoundPlaylist = false;

   // check if the file is a playlist
   LPCTSTR pos = _tcsrchr(filename, '.');
   if (pos == NULL)
   {
      // just a filename
      m_vecFileList.push_back(filename);
   }
   else
   if (0 == _tcsicmp(pos + 1, _T("m3u")))
   {
      // import m3u playlist
      ImportM3uPlaylist(filename);
      bFoundPlaylist = true;
   }
   else
   if (0 == _tcsicmp(pos + 1, _T("pls")))
   {
      // import pls playlist
      ImportPlsPlaylist(filename);
      bFoundPlaylist = true;
   }
   else
   if (0 == _tcsicmp(pos + 1, _T("cue")))
   {
      // import cue sheet
      ImportCueSheet(filename);
      bFoundPlaylist = true;
   }
   else
   {
      // just a filename
      m_vecFileList.push_back(filename);
   }

   if (bFoundPlaylist)
   {
      // produce playlist filename
      CString plname(filename, pos - filename + 1);
      plname += _T("m3u");

      plname.TrimRight(_T('\\'));

      m_cszPlaylistName = plname;
   }
}

void InputFilesParser::ImportM3uPlaylist(LPCTSTR filename)
{
   // open playlist
   std::ifstream plist(CStringA(filename), std::ios::in);
   if (!plist.is_open()) return;

   // find out pathname (for relative file refs)
   std::tstring path(filename);
   std::tstring::size_type pos = path.find_last_of('\\');
   if (pos != std::tstring::npos)
      path.erase(pos);

   // read in all lines
   std::tstring line;

   while (!plist.eof())
   {
#ifdef UNICODE
      std::string line2;
      std::getline(plist, line2);
      line = CString(line2.c_str());
#else
      std::getline(plist, line);
#endif
      if (line.empty()) continue;
      if (line.at(0) == '#') continue;

      // line contains a file name

      // check if path is relative
      if (_tcschr(line.c_str(), ':') == NULL && _tcsncmp(line.c_str(), _T("\\\\"), 2) != 0)
      {
         if (line.size() > 0 && line.at(0) != '\\')
         {
            // relative to playlist file
            line.insert(0, _T("\\"));
            line.insert(0, path);
         }
         else
         {
            // relative to drive
            line.insert(0, path.c_str(), 2);
         }
      }

      // insert filename
      InsertFilename(line.c_str());
   }

   plist.close();
}

void InputFilesParser::ImportPlsPlaylist(LPCTSTR filename)
{
   // open playlist
   std::ifstream plist(CStringA(filename), std::ios::in);
   if (!plist.is_open()) return;

   // find out pathname (for relative file refs)
   std::tstring path(filename);
   std::tstring::size_type pos = path.find_last_of('\\');
   if (pos != std::tstring::npos)
      path.erase(pos);

   // read in all lines
   std::tstring line;

   while (!plist.eof())
   {
#ifdef UNICODE
      std::string line2;
      std::getline(plist, line2);
      line = CString(line2.c_str());
#else
      std::getline(plist, line);
#endif
      if (line.empty()) continue;
      if (_tcsncmp(line.c_str(), _T("File"), 4) != 0) continue;

      // get file name after equal char
      std::tstring::size_type pos2 = line.find_first_of('=');
      if (pos2 == std::tstring::npos) continue;
      line.erase(0, pos2 + 1);

      // check if path is relative
      if (_tcschr(line.c_str(), ':') == NULL && _tcsncmp(line.c_str(), _T("\\\\"), 2) != 0)
      {
         if (line.size()>0 && line.at(0) != '\\')
         {
            // relative to playlist file
            line.insert(0, _T("\\"));
            line.insert(0, path);
         }
         else
         {
            // relative to drive
            line.insert(0, path.c_str(), 2);
         }
      }

      // insert filename
      InsertFilename(line.c_str());
   }

   plist.close();
}

void InputFilesParser::ImportCueSheet(LPCTSTR filename)
{
   // open cue sheet
   std::ifstream sheet(CStringA(filename), std::ios::in);
   if (!sheet.is_open()) return;

   // find out pathname (for relative file refs)
   std::tstring path(filename);
   std::tstring::size_type pos = path.find_last_of('\\');
   if (pos != std::tstring::npos)
      path.erase(pos);

   // read in all lines
   std::tstring line;

   while (!sheet.eof())
   {
#ifdef UNICODE
      std::string line2;
      std::getline(sheet, line2);
      line = CString(line2.c_str());
#else
      std::getline(sheet, line);
#endif
      if (line.empty()) continue;

      // trim
      while (line.size() != 0 && line.at(0) == ' ') line.erase(0, 1);
      if (line.empty()) continue;

      // file entry?
      if (line.compare(0, 4, _T("FILE")) == 0)
      {
         // trim
         line.erase(0, 5);
         while (line.size() != 0 && line.at(0) == ' ') line.erase(0, 1);
         if (line.empty()) continue;

         // determine endchar
         char endchar = ' ';
         if (line.at(0) == '\"') endchar = '\"';

         // search endchar
         std::tstring::size_type pos2 = line.find_first_of(endchar, 1);
         if (pos2 == std::tstring::npos) continue;

         // cut out filename
         std::tstring fname;
         fname.assign(line.c_str() + (endchar == ' ' ? 0 : 1),
            pos2 - (endchar == ' ' ? 0 : 1));

         // insert filename
         InsertFilename(fname.c_str());
      }
   }

   sheet.close();
}
