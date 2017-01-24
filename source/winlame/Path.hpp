//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2017 Michael Fink
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
/// \file Path.hpp
/// \brief Path class
//
#pragma once

/// file and folder path class
class Path
{
public:
   // ctors

   /// default ctor
   Path() throw()
   {
   }

   /// ctor that takes a path
   Path(const CString& path)
      :m_path(path)
   {
   }

   // operators

   /// returns CString
   operator const CString&() const throw() { return m_path; }
   /// returns raw string pointer
   operator LPCTSTR() const throw() { return m_path; }

   // getters

   /// converts path to string
   CString ToString() const { return m_path; }

   /// returns filename and extension
   CString FilenameAndExt() const;

   /// returns filename without extension
   CString FilenameOnly() const;

   /// returns folder name, without filename, but ending slash
   CString FolderName() const;

   /// returns short path name (filename in 8.3 format)
   CString ShortPathName() const;

   /// make this path relative to the given root path and returns it
   CString MakeRelativeTo(const CString& rootPath);

   /// returns if stored path is a relative path
   bool IsRelative() const throw();

   /// returns if path represents a file and if it exists
   bool FileExists() const throw();

   /// returns if path represents a folder and if it exists
   bool FolderExists() const throw();

   // methods

   /// canonicalizes path by removing '..', etc.
   bool Canonicalize();

   /// combine path with given second part and return new path
   Path Combine(const CString& part2);

   /// adds a backslash at the end of the path
   static void AddEndingBackslash(CString& path);

   /// combine both path parts and return new path
   static Path Combine(const CString& part1, const CString& part2)
   {
      Path path1(part1);
      return path1.Combine(part2);
   }

   /// returns special folder; see CSIDL_* constants
   static CString SpecialFolder(int csidl);

   /// returns the windows folder
   static CString WindowsFolder();

   // public members

   /// path separator string
   static const TCHAR Separator[2];

   /// path separator character
   static const TCHAR SeparatorCh = _T('\\');

private:
   /// path
   CString m_path;
};
