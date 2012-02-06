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

   $Id: LanguageResourceManager.hpp,v 1.1 2009/04/10 09:47:40 vividos Exp $

*/
/*! \file LanguageResourceManager.hpp

   \brief language resource manager

*/

#pragma once

class CLanguageResourceInfo
{
public:
   CLanguageResourceInfo(LPCTSTR pszDllName, UINT uiLanguageId, LPCTSTR pszLanguageName) throw()
      :m_cszDllName(pszDllName),
       m_uiLanguageId(uiLanguageId),
       m_cszLanguageName(pszLanguageName)
   {
   }

   const CString& DllName() const throw() { return m_cszDllName; }
   UINT LanguageId() const throw() { return m_uiLanguageId; }
   const CString& LanguageName() const throw() { return m_cszLanguageName; }

private:
   CString m_cszDllName;
   UINT m_uiLanguageId;
   CString m_cszLanguageName;
};

class CLanguageResourceManager
{
public:
   CLanguageResourceManager(LPCTSTR pszLangDllSearchPattern, UINT uiLangNameId, UINT uiLangNameNativeId) throw();
   ~CLanguageResourceManager() throw();

   const std::vector<CLanguageResourceInfo>& LanguageResourceList() const  throw() { return m_vecLangResourceInfo; }

   bool IsLangResourceAvail(UINT uiLanguageId) const throw();

   void LoadLangResource(UINT uiLanguageId) throw();

private:
   void UnloadLangResource() throw();

   void ScanResourceDlls(LPCTSTR pszLangDllSearchPattern) throw();

   void AddLanguageResourceInfo(LPCTSTR pszLanghModSearchPattern, const CString& cszResourceFilename,
      const CString& cszResourceFilenameWithoutPath, HMODULE hMod) throw();

private:
   std::vector<CLanguageResourceInfo> m_vecLangResourceInfo;
   HMODULE m_hLoadedResourceDll;
   UINT m_uiLangNameId;
   UINT m_uiLangNameNativeId;
};
