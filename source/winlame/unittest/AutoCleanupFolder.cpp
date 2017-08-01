//
// ulib - a collection of useful classes
// Copyright (C) 2006,2007,2008,2012 Michael Fink
//
/// \file AutoCleanupFolder.cpp auto-deleting folder
//

// includes
#include "stdafx.h"
#include "AutoCleanupFolder.hpp"
#include <deque>

using UnitTest::AutoCleanupFolder;

/// Creates a temporary folder in the "unittest" subfolder of the
/// TEMP folder of the current user. Under Windows CE the \\Temp folder is
/// used.
AutoCleanupFolder::AutoCleanupFolder() throw()
{
   // search folder name
   CString cszBaseName;
#ifdef _WIN32_WCE
   cszBaseName = _T("\\Temp\\unittest");
#else
   ::GetEnvironmentVariable(_T("TEMP"), cszBaseName.GetBuffer(MAX_PATH), MAX_PATH);
   cszBaseName.ReleaseBuffer();
   cszBaseName += _T("\\unittest");
#endif

   ::CreateDirectory(cszBaseName, NULL);
   cszBaseName += _T("\\");

   unsigned int uiCounter = 0;

   do
   {
      m_cszFolderName.Format(_T("%stest-%06x"), cszBaseName.GetString(), uiCounter++);

   } while(INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(m_cszFolderName));

   ::CreateDirectory(m_cszFolderName, NULL);

   // ensure slash at end
   m_cszFolderName += _T("\\");
}

/// Cleans up folder contents, and all subfolders. Also tries to remove parent
/// folder; this is done to eventually clean up the created base folder; this
/// might fail, though, since there could be more unit test folders.
AutoCleanupFolder::~AutoCleanupFolder() throw()
{
   WIN32_FIND_DATA findData;

   std::deque<CString> dequeFolders;
   dequeFolders.push_back(m_cszFolderName);

   while (!dequeFolders.empty())
   {
      CString cszFolder = dequeFolders[0];

      HANDLE hFind = ::FindFirstFile(cszFolder + _T("*.*"), &findData);
      if (hFind != INVALID_HANDLE_VALUE)
      do
      {
         CString cszFilename = findData.cFileName;
         if (cszFilename == _T(".") || cszFilename == _T(".."))
            continue;

         // check if folder
         if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
         {
            // folder, so add to folder to process
            dequeFolders.push_back(cszFolder + cszFilename + _T("\\"));
         }
         else
         {
            // no, plain file; delete it
            ::DeleteFile(cszFolder + cszFilename);
         }

      } while (FALSE != ::FindNextFile(hFind, &findData));

      ::FindClose(hFind);

      cszFolder.TrimRight(_T('\\'));
      RemoveDirectory(cszFolder);

      dequeFolders.pop_front();
   }

   // try to remove parent dir, in case we're the last folder
   m_cszFolderName.TrimRight(_T('\\'));
   int iPos = m_cszFolderName.ReverseFind(_T('\\'));
   if (iPos > 1)
      RemoveDirectory(m_cszFolderName.Left(iPos));
}
