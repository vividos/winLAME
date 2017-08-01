//
// ulib - a collection of useful classes
// Copyright (C) 2006,2007,2008 Michael Fink
//
/// \file AutoCleanupFolder.hpp auto-deleting folder
//
#pragma once

namespace UnitTest
{

/// \brief auto-deleting folder
/// This class automatically deletes the content of a temporary folder
class AutoCleanupFolder
{
public:
   /// ctor; creates folder
   AutoCleanupFolder() throw();
   /// dtor; cleans up folder
   ~AutoCleanupFolder() throw();

   /// returns folder name; always ends with a slash or backslash
   const CString& FolderName() const throw() { return m_cszFolderName; }

private:
   /// folder name
   CString m_cszFolderName;
};

} // namespace UnitTest
