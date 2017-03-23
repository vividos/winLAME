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
/// \file Path.cpp
/// \brief Path class

#include "StdAfx.h"
#include "Path.hpp"

const TCHAR Path::Separator[2] = _T("\\");

bool Path::Canonicalize()
{
   CString newPath;
   BOOL ret = ::PathCanonicalize(newPath.GetBuffer(MAX_PATH), m_path);
   newPath.ReleaseBuffer();

   if (ret != FALSE)
      m_path = newPath;

   return ret != FALSE;
}

Path Path::Combine(const CString& part2)
{
   CString part1 = m_path;

   AddEndingBackslash(part1);

   return Path(part1 + part2);
}

CString Path::FilenameAndExt() const
{
   int pos = m_path.ReverseFind(Path::SeparatorCh);
   if (pos == -1)
      return m_path;

   return m_path.Mid(pos + 1);
}

CString Path::FilenameOnly() const
{
   int pos = m_path.ReverseFind(Path::SeparatorCh);

   int pos2 = m_path.ReverseFind(_T('.'));
   if (pos2 == -1)
      return m_path.Mid(pos + 1);

   return m_path.Mid(pos + 1, pos2 - pos - 1);
}

CString Path::FolderName() const
{
   int pos = m_path.ReverseFind(Path::SeparatorCh);
   if (pos == -1)
      return m_path;

   return m_path.Left(pos + 1);
}

CString Path::ShortPathName() const
{
   CString shortPathName;
   DWORD ret = ::GetShortPathName(m_path, shortPathName.GetBuffer(MAX_PATH), MAX_PATH);
   shortPathName.ReleaseBuffer();

   return ret == 0 ? m_path : shortPathName;
}

CString Path::MakeRelativeTo(const CString& rootPath)
{
   CString relativePath;

   DWORD pathAttr = ::GetFileAttributes(m_path) & FILE_ATTRIBUTE_DIRECTORY;

   BOOL ret = ::PathRelativePathTo(
      relativePath.GetBuffer(MAX_PATH),
      rootPath, FILE_ATTRIBUTE_DIRECTORY,
      m_path, pathAttr);

   relativePath.ReleaseBuffer();

   return ret == FALSE ? _T("") : relativePath;
}

bool Path::IsRelative() const
{
   BOOL ret = ::PathIsRelative(m_path);
   return ret != FALSE;
}

bool Path::FileExists() const
{
   DWORD ret = ::GetFileAttributes(m_path);
   if (ret == INVALID_FILE_ATTRIBUTES)
      return false; // doesn't exist

   if ((ret & FILE_ATTRIBUTE_DIRECTORY) != 0)
      return false; // no, it's a folder

   return true;
}

bool Path::FolderExists() const
{
   DWORD ret = ::GetFileAttributes(m_path);
   if (ret == INVALID_FILE_ATTRIBUTES)
      return false; // doesn't exist

   if ((ret & FILE_ATTRIBUTE_DIRECTORY) != 0)
      return true; // yes, it's a folder

   return false;
}

void Path::AddEndingBackslash(CString& path)
{
   if (path.Right(1) != Path::Separator)
      path += Path::Separator;
}

CString Path::SpecialFolder(int csidl)
{
   CString specialFolder;

   ::SHGetFolderPath(nullptr, csidl, nullptr, SHGFP_TYPE_CURRENT, specialFolder.GetBuffer(MAX_PATH));
   specialFolder.ReleaseBuffer();

   return specialFolder;
}

CString Path::WindowsFolder()
{
   CString windowsFolder;

   ::GetWindowsDirectory(windowsFolder.GetBuffer(MAX_PATH), MAX_PATH);
   windowsFolder.ReleaseBuffer();

   return windowsFolder;
}

CString Path::TempFolder()
{
   CString tempFolder;

   DWORD ret = ::GetTempPath(MAX_PATH, tempFolder.GetBuffer(MAX_PATH));
   tempFolder.ReleaseBuffer(ret);

   return tempFolder;
}
