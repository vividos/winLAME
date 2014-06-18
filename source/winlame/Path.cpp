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
/// \file Path.cpp
/// \brief Path class

#include "StdAfx.h"
#include "Path.hpp"

const TCHAR Path::Separator[2] = _T("\\");

bool Path::Canonicalize()
{
   CString cszNewPath;
   BOOL bRet = ::PathCanonicalize(cszNewPath.GetBuffer(MAX_PATH), m_cszPath);
   cszNewPath.ReleaseBuffer();
   if (bRet != FALSE)
      m_cszPath = cszNewPath;

   return bRet != FALSE;
}

Path Path::Combine(const CString& cszPart2)
{
   CString cszPart1 = m_cszPath;

   AddEndingBackslash(cszPart1);

   return Path(cszPart1 + cszPart2);
}

CString Path::FilenameAndExt() const
{
   int iPos = m_cszPath.ReverseFind(Path::SeparatorCh);
   if (iPos == -1)
      return m_cszPath;

   return m_cszPath.Mid(iPos+1);
}

CString Path::FilenameOnly() const
{
   int iPos = m_cszPath.ReverseFind(Path::SeparatorCh);

   int iPos2 = m_cszPath.ReverseFind(_T('.'));
   if (iPos2 == -1)
      return m_cszPath.Mid(iPos+1);

   return m_cszPath.Mid(iPos+1, iPos2-iPos-1);
}

bool Path::FileExists() const throw()
{
   DWORD dwRet = ::GetFileAttributes(m_cszPath);
   if (dwRet == INVALID_FILE_ATTRIBUTES)
      return false; // doesn't exist

   if ((dwRet & FILE_ATTRIBUTE_DIRECTORY) != 0)
      return false; // no, it's a folder

   return true;
}

bool Path::FolderExists() const throw()
{
   DWORD dwRet = ::GetFileAttributes(m_cszPath);
   if (dwRet == INVALID_FILE_ATTRIBUTES)
      return false; // doesn't exist

   if ((dwRet & FILE_ATTRIBUTE_DIRECTORY) != 0)
      return true; // yes, it's a folder

   return false;
}

void Path::AddEndingBackslash(CString& cszPath)
{
   if (cszPath.Right(1) != Path::Separator)
      cszPath += Path::Separator;
}
