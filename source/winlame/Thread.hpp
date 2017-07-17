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
/// \file Thread.hpp
/// \brief Thread class
//
#pragma once

#ifndef MS_VC_EXCEPTION
/// exception code for visual studio functions
#define MS_VC_EXCEPTION 0x406D1388
#endif

#pragma pack(push, 8)
/// thread name info struct
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType;     ///< Must be 0x1000.
   LPCSTR szName;    ///< Pointer to name (in user addr space).
   DWORD dwThreadID; ///< Thread ID (-1=caller thread).
   DWORD dwFlags;    ///< Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)


/// thread helper class
class Thread
{
public:
   /// sets thread name for current or specified thread; unicode version
   static void SetName(LPCWSTR pszThreadName, DWORD dwThreadId = DWORD(-1)) throw()
   {
      Thread::SetName(CStringA(pszThreadName), dwThreadId);
   }

   /// sets thread name for current or specified thread; ansi version
   /// \note from http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
   static void SetName(LPCSTR pszThreadName, DWORD dwThreadId = DWORD(-1)) throw()
   {
      THREADNAME_INFO info;
      info.dwType = 0x1000;
      info.szName = pszThreadName;
      info.dwThreadID = dwThreadId;
      info.dwFlags = 0;

      __try
      {
         RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
      }
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
      }
   }

   /// returns current thread id
   static DWORD CurrentId() throw()
   {
      return ::GetCurrentThreadId();
   }
};
