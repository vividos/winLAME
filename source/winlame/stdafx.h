/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2009 Michael Fink

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
/*! \file stdafx.h

   \brief include file for include files used for precompiled headers

   include file for standard system include files,
   or project specific include files that are used frequently, but
   are changed infrequently

*/

#pragma once

#define WINVER         0x0502 
#define _WIN32_WINNT   0x0502 ///< Windows Server 2003 with SP1, Windows XP with SP2
#define _WIN32_IE      0x0600 ///< Internet Explorer 6.0

#pragma warning(disable: 4100) // unreferenced formal parameter

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

// define this to prevent the following warning in atlapp.hpp:
// warning C4996: '_vswprintf': swprintf has been changed to conform with the ISO C standard, adding an extra character count parameter. To use traditional Microsoft swprintf, set _CRT_NON_CONFORMING_SWPRINTFS.
#if _MSC_VER >= 1400
#  define _CRT_NON_CONFORMING_SWPRINTFS
#endif

// ATL includes
#include <atlbase.h>
#if _ATL_VER >= 0x0700
   #include <atlcoll.h>
   #include <atlstr.h>
   #include <atltypes.h>
   #define _WTL_NO_CSTRING
   #define _WTL_NO_WTYPES
#else
   #define _WTL_USE_CSTRING
#endif

#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlcom.h>
#include <atlhost.h>

// WTL includes
#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlddx.h>
#include <atlframe.h>


// Standard C++ Library includes
#include <string>
#include <vector>


#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

#define DEBUG_NEW new(THIS_FILE, __LINE__)

inline void* __cdecl operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
   return _malloc_dbg(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}


// delete macros so that std::min and std::max can be used
#undef min
#undef max

// input and output modules

#define ID_OM_LAME                      1
#define ID_OM_OGGV                      2
#define ID_OM_WAVE                      3
#define ID_OM_AAC                       4
#define ID_IM_SNDFILE                   5
#define ID_IM_MAD                       6
#define ID_IM_OGGV                      7
#define ID_IM_AAC                       8
#define ID_IM_WINAMP                    9
#define ID_IM_FLAC                      10
#define ID_IM_BASS                      11
#define ID_OM_BASSWMA                   12
#define ID_IM_CDRIP                     13


// timer id's

#define IDT_CDRIP_CHECK                 66
#define IDT_ENC_UPDATEINFO              67
