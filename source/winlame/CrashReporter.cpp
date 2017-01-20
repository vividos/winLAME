//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file CrashReporter.cpp
/// \brief Application crash reporting
//
#include "StdAfx.h"
#include "CrashReporter.hpp"
#include "Path.hpp"
#include <strsafe.h>
#include <ctime>
#include <memory>

#pragma warning(disable: 4091) // 'typedef ': ignored on left of '' when no variable is declared
#include <dbghelp.h>
#pragma warning(default: 4091)

#pragma comment(lib, "dbghelp.lib")

/// base path for writing minidump file
static TCHAR g_minidumpBasePath[MAX_PATH] = { 0 };

/// writes minidump file
bool WriteMinidump(HANDLE fileHandle, _EXCEPTION_POINTERS* exceptionInfo)
{
   __try
   {
      MINIDUMP_EXCEPTION_INFORMATION miniDumpExceptionInfo = { 0 };
      miniDumpExceptionInfo.ThreadId = GetCurrentThreadId();
      miniDumpExceptionInfo.ExceptionPointers = exceptionInfo;
      miniDumpExceptionInfo.ClientPointers = false;

      BOOL bRet = MiniDumpWriteDump(
         GetCurrentProcess(),
         GetCurrentProcessId(),
         fileHandle,
         MiniDumpNormal,
         &miniDumpExceptionInfo,
         nullptr,
         nullptr);

      return bRet != FALSE;
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      return false;
   }
}

/// creates minidump filename in given buffer
void GetMinidumpFilename(LPTSTR minidumpFilename, UINT numMaxChars)
{
   HRESULT hr = StringCchCopy(
      minidumpFilename,
      numMaxChars,
      g_minidumpBasePath);
   ATLASSERT(S_OK == hr); hr;

   size_t lenBasePath = _tcslen(minidumpFilename);
   TCHAR* start = minidumpFilename + lenBasePath;
   unsigned int numRemaining = numMaxChars - lenBasePath;

   // add date
   time_t nowt = ::time(&nowt);

   struct tm now = { 0 };
   localtime_s(&now, &nowt);

   _sntprintf_s(start, numRemaining, numRemaining,
      _T("%04u-%02u-%02u %02u.%02u.%02u.mdmp"),
      now.tm_year + 1900,
      now.tm_mon + 1,
      now.tm_mday,
      now.tm_hour,
      now.tm_min,
      now.tm_sec);
}

/// exception filter function to write minidump file
LONG WINAPI ExceptionFilterWriteMinidump(_EXCEPTION_POINTERS* exceptionInfo)
{
   OutputDebugString(_T("!!! ExceptionFilterWriteMinidump() called!\n"));

   // construct filename
   TCHAR minidumpFilename[MAX_PATH];
   GetMinidumpFilename(minidumpFilename, sizeof(minidumpFilename) / sizeof(*minidumpFilename));

   // write minidump file
   HANDLE fileHandle = CreateFile(
      minidumpFilename,
      GENERIC_WRITE,
      0,
      nullptr,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);

   if (INVALID_HANDLE_VALUE == fileHandle)
      return EXCEPTION_CONTINUE_SEARCH;

   std::shared_ptr<void> file(fileHandle, &::CloseHandle);

   OutputDebugString(_T("!!! Writing minidump to file: "));
   OutputDebugString(minidumpFilename);
   OutputDebugString(_T("\n"));

   WriteMinidump(fileHandle, exceptionInfo);

   CloseHandle(fileHandle);

   // as last resort, try to log error
   ATLTRACE(CString(_T("wrote minidump file: ")) + minidumpFilename);

   return EXCEPTION_CONTINUE_SEARCH;
}

/// handler function for std::terminate
void OnStdTerminate()
{
   OutputDebugString(_T("!!! OnStdTerminate() called!\n"));

   // construct filename
   TCHAR minidumpFilename[MAX_PATH];
   GetMinidumpFilename(minidumpFilename, sizeof(minidumpFilename) / sizeof(*minidumpFilename));

   // cause an exception, so that we can write a minidump
   EXCEPTION_POINTERS* exceptionInfo = nullptr;
   __try
   {
      throw 42;
   }
   __except ((exceptionInfo = GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
   {
      if (exceptionInfo != nullptr)
         ExceptionFilterWriteMinidump(exceptionInfo);
   }
}

void CrashReporter::Init(const CString& appName, const CString& basePath)
{
   // set minidump base path
   CString path = basePath;
   Path::AddEndingBackslash(path);

   ::CreateDirectory(path, nullptr);

   path += appName + _T("-");

   HRESULT hr = StringCchCopy(
      g_minidumpBasePath,
      sizeof(g_minidumpBasePath) / sizeof(*g_minidumpBasePath),
      path);
   ATLASSERT(S_OK == hr); hr;

   // set exception filter
   SetUnhandledExceptionFilter(&ExceptionFilterWriteMinidump);

   // catch all std::terminate() calls
   std::set_terminate(&OnStdTerminate);
}
