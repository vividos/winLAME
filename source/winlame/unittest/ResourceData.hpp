//
// ulib - a collection of useful classes
// Copyright (C) 2004,2005,2006,2007,2008,2011 Michael Fink
//
/// \file ResourceData.hpp resource data class
//
#pragma once

// includes
#include <vector>

namespace Win32
{

/// resource data helper
/** This class extracts resource data (that may be stored as RT_RCDATA type)
    and extracts it as string or file on disk.
*/
class ResourceData
{
public:
   /// ctor
   ResourceData(LPCTSTR pszResourceName, LPCTSTR pszResourceType = _T("\"RT_RCDATA\""), HINSTANCE hInstance = NULL);

   /// returns true if the resource is available
   bool IsAvailable() const;

   /// returns resource data as byte array
   bool AsRawData(std::vector<BYTE>& vecRawData);

   /// returns resource data as string
   CString AsString(bool bStoredAsUnicode = false);

   /// saves resource data as file
   bool AsFile(LPCTSTR pszFilename);

private:
   LPVOID GetResource(DWORD& dwSize);

   void CloseResource();

private:
   /// resource name
   LPCTSTR m_pszResourceName;

   /// resource type
   LPCTSTR m_pszResourceType;

   HGLOBAL m_hGlobal;

   DWORD m_dwSize;

   HINSTANCE m_hInstance;
};

} // namespace Win32
