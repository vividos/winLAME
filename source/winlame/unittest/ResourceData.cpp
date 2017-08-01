//
// ulib - a collection of useful classes
// Copyright (C) 2004,2005,2006,2007,2008 Michael Fink
//
/// \file ResourceData.cpp resource data class
//

// includes
#include "stdafx.h"
#include "ResourceData.hpp"

using Win32::ResourceData;

//
// ResourceData
//

ResourceData::ResourceData(LPCTSTR pszResourceName, LPCTSTR pszResourceType, HINSTANCE hInstance)
:m_pszResourceName(pszResourceName),
 m_pszResourceType(pszResourceType),
 m_hGlobal(NULL),
 m_dwSize(0),
 m_hInstance(hInstance)
{
}

bool ResourceData::IsAvailable() const
{
   HRSRC hResource = FindResource(m_hInstance, m_pszResourceName, m_pszResourceType);
   return hResource != NULL;
}

bool ResourceData::AsRawData(std::vector<BYTE>& vecRawData)
{
   DWORD dwSize = 0;
   LPVOID pData = GetResource(dwSize);
   if (pData == NULL)
      return false;

   vecRawData.resize(dwSize);

   memcpy(&vecRawData[0], pData, dwSize);

   CloseResource();
   return true;
}


CString ResourceData::AsString(bool bStoredAsUnicode)
{
   DWORD dwSize = 0;
   LPVOID pData = GetResource(dwSize);
   if (pData == NULL)
      return _T("");

   // convert to string
   CString cszText;

   USES_CONVERSION;
   if (bStoredAsUnicode)
      cszText = CString(W2CT(static_cast<LPCWSTR>(pData)), dwSize/sizeof(WCHAR));
   else
      cszText = CString(A2CT(static_cast<LPCSTR>(pData)), dwSize/sizeof(CHAR));

   CloseResource();

   return cszText;
}

bool ResourceData::AsFile(LPCTSTR pszFilename)
{
   DWORD dwSize = 0;
   LPVOID pData = GetResource(dwSize);
   if (pData == NULL)
      return false;

   // save as file
   {
      HANDLE hFile = CreateFile(pszFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile == NULL)
         return false;

      DWORD dwBytesWritten = 0;
      WriteFile(hFile, pData, dwSize, &dwBytesWritten, NULL);
      ATLASSERT(dwSize == dwBytesWritten);

      CloseHandle(hFile);
   }

   CloseResource();
   return true;
}

LPVOID ResourceData::GetResource(DWORD& dwSize)
{
   HRSRC hResource = FindResource(m_hInstance, m_pszResourceName, m_pszResourceType);
   if (hResource == NULL)
      return NULL;

   dwSize = SizeofResource(m_hInstance, hResource);
   if (dwSize == 0)
      return NULL;

   m_hGlobal = LoadResource(m_hInstance, hResource);
   if (m_hGlobal == NULL)
      return NULL;

   LPVOID pData = LockResource(m_hGlobal);
   return pData;
}

void ResourceData::CloseResource()
{
   if (m_hGlobal != NULL)
   {
      // normally not needed on win32, but for completeness
      // on wince these two doesn't exists
#ifndef _WIN32_WCE
      UnlockResource(m_hGlobal);
      FreeResource(m_hGlobal);
#endif
   }
}
