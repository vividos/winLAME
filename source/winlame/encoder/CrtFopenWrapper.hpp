/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005-2014 Michael Fink

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
/// \file CrtFopenWrapper.hpp
/// \brief contains wrapper for msvcrt fopen/fclose functions
/// \ingroup encoder
/// @{

// include guard
#pragma once

/// wrapper for CRT fopen/fclose functions in msvcrt.dll
class CrtFopenWrapper
{
public:
   /// ctor
   CrtFopenWrapper()
   {
      HMODULE hMod = ::GetModuleHandle(_T("msvcrt.dll"));
      m_msvcrt_wfopen = (T_fn_msvcrt_wfopen)::GetProcAddress(hMod, "_wfopen");
      m_msvcrt_fclose = (T_fn_msvcrt_fclose)::GetProcAddress(hMod, "fclose");
   }

   /// returns if wrapper functions are available
   bool IsAvail() const throw()
   {
      return m_msvcrt_wfopen != nullptr &&
         m_msvcrt_fclose != nullptr;
   }

   /// calls CRT's _wfopen() function
   FILE* _wfopen(const wchar_t* name, const wchar_t* mode)
   {
      ATLASSERT(m_msvcrt_wfopen != nullptr);

      return m_msvcrt_wfopen(name, mode);
   }

   /// calls CRT's fclose() function
   int fclose(FILE* f)
   {
      ATLASSERT(m_msvcrt_fclose != nullptr);

      return m_msvcrt_fclose(f);
   }

private:
   // msvcrt function pointers

   /// function pointer type for _wfopen
   typedef FILE* (*T_fn_msvcrt_wfopen)(const wchar_t*, const wchar_t*);

   /// function pointer type for fclose
   typedef int(*T_fn_msvcrt_fclose)(FILE*);

   /// function pointer to _wfopen
   T_fn_msvcrt_wfopen m_msvcrt_wfopen;

   /// function pointer to fclose
   T_fn_msvcrt_fclose m_msvcrt_fclose;
};

/// @}
