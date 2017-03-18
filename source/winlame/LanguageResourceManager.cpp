/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2009 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/// \file LanguageResourceManager.cpp
/// \brief language resource manager

#include "StdAfx.h"
#include "LanguageResourceManager.hpp"

LanguageResourceManager::LanguageResourceManager(LPCTSTR pszLangDllSearchPattern, UINT uiLangNameId, UINT uiLangNameNativeId)
:m_hLoadedResourceDll(NULL),
 m_uiLangNameId(uiLangNameId),
 m_uiLangNameNativeId(uiLangNameNativeId)
{
   // search pattern must contain one asterisk
   ATLASSERT(CString(pszLangDllSearchPattern).Find(_T('*')) != -1);

   // must not contain any path
   ATLASSERT(CString(pszLangDllSearchPattern).Find(_T('\\')) == -1);

   // add english lang resource
   {
      CString cszLanguageName(MAKEINTRESOURCE(m_uiLangNameNativeId));
      LanguageResourceInfo info(_T(""), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), cszLanguageName);
      m_vecLangResourceInfo.push_back(info);
   }

   ScanResourceDlls(pszLangDllSearchPattern);
}

LanguageResourceManager::~LanguageResourceManager() throw()
{
   UnloadLangResource();
}

bool LanguageResourceManager::IsLangResourceAvail(UINT uiLanguageId) const throw()
{
   for (size_t i=0,iMax=m_vecLangResourceInfo.size(); i<iMax; i++)
   {
      if (m_vecLangResourceInfo[i].LanguageId() == uiLanguageId)
         return true;
   }
   return false;
}

void LanguageResourceManager::LoadLangResource(UINT uiLanguageId) throw()
{
   ATLASSERT(IsLangResourceAvail(uiLanguageId));

   for (size_t i=0,iMax=m_vecLangResourceInfo.size(); i<iMax; i++)
   {
      const LanguageResourceInfo& info = m_vecLangResourceInfo[i];

      if (info.LanguageId() == uiLanguageId)
      {
         UnloadLangResource();

         if (info.DllName().IsEmpty())
         {
            // use module instance
            _Module.m_hInstResource = _Module.m_hInst;
         }
         else
         {
            // use loaded instance
            m_hLoadedResourceDll = ::LoadLibrary(info.DllName());
            if (m_hLoadedResourceDll != nullptr)
               _Module.m_hInstResource = m_hLoadedResourceDll;
            else
               _Module.m_hInstResource = _Module.m_hInst;
         }

         break;
      }
   }
}

void LanguageResourceManager::UnloadLangResource() throw()
{
   if (m_hLoadedResourceDll != NULL)
   {
      FreeLibrary(m_hLoadedResourceDll);
      m_hLoadedResourceDll = NULL;
   }
}

void LanguageResourceManager::ScanResourceDlls(LPCTSTR pszLangDllSearchPattern)
{
   // search pattern must contain one asterisk
   ATLASSERT(CString(pszLangDllSearchPattern).Find(_T('*')) != -1);

   // must not contain any path
   ATLASSERT(CString(pszLangDllSearchPattern).Find(_T('\\')) == -1);

   // get application dir
   CString cszApplicationPath;
   ::GetModuleFileName(NULL, cszApplicationPath.GetBuffer(MAX_PATH), MAX_PATH);
   cszApplicationPath.ReleaseBuffer();

   int iPos = cszApplicationPath.ReverseFind(_T('\\'));
   if (iPos == -1)
      cszApplicationPath = _T(".\\");
   else
      cszApplicationPath = cszApplicationPath.Left(iPos+1);

   // find resource hMods
   WIN32_FIND_DATA findData = {0};
   HANDLE hFind = ::FindFirstFile(cszApplicationPath + pszLangDllSearchPattern, &findData);
   if (INVALID_HANDLE_VALUE == hFind)
      return;

   do
   {
      CString cszResourceFilename = cszApplicationPath + findData.cFileName;

      // try to load resource module
      HMODULE hMod = ::LoadLibrary(cszResourceFilename);
      if (hMod != NULL)
      {
         CString cszResourceFilenameWithoutPath(findData.cFileName);
         AddLanguageResourceInfo(pszLangDllSearchPattern, cszResourceFilename, cszResourceFilenameWithoutPath, hMod);

         ::FreeLibrary(hMod);
      }
   }
   while (TRUE == ::FindNextFile(hFind, &findData));

   ::FindClose(hFind);
}

void LanguageResourceManager::AddLanguageResourceInfo(
   LPCTSTR pszLangDllSearchPattern,
   const CString& cszResourceFilename,
   const CString& cszResourceFilenameWithoutPath, HMODULE hMod)
{
   // find out language id
   UINT uiLanguageId = 0;
   {
      CString cszSearchPattern(pszLangDllSearchPattern);
      cszSearchPattern.MakeLower();

      int iPosAsterisk = cszSearchPattern.Find(_T('*'));

      CString cszFilename(cszResourceFilenameWithoutPath.Mid(iPosAsterisk));
      cszFilename.MakeLower();

      int iPosRemaining = cszFilename.Find(cszSearchPattern.Mid(iPosAsterisk+1));
      cszFilename = cszFilename.Left(iPosRemaining);

      uiLanguageId = static_cast<UINT>(_tcstoul(cszFilename, NULL, 16)); // in hex
   }

   // load language name from info
   CString cszLanguageName;
   {
      UINT uiId = m_uiLangNameNativeId;

      LPCTSTR pszName = MAKEINTRESOURCE((uiId >> 4) + 1);
      if (::FindResource(hMod, pszName, RT_STRING) != NULL)
      {
         ::LoadString(hMod, uiId, cszLanguageName.GetBuffer(255), 255);
         cszLanguageName.ReleaseBuffer();
      }
   }

   // add
   LanguageResourceInfo info(cszResourceFilename, uiLanguageId, cszLanguageName);
   m_vecLangResourceInfo.push_back(info);
}
