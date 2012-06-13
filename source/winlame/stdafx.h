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
/// \file stdafx.h
/// \brief include file for include files used for precompiled headers
/// include file for standard system include files,
/// or project specific include files that are used frequently, but
/// are changed infrequently

#pragma once

#define WINVER         0x0601 
#define _WIN32_WINNT   0x0601
#define _WIN32_IE      0x0700

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

// ATL includes
#include <atlbase.h>
#if _ATL_VER < 0x0700
#error ATL7 or higher must be used to compile
#endif

#include <atlcoll.h>
#include <atlstr.h>
#include <atltypes.h>
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES

#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlcom.h>
#include <atlhost.h>

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// WTL includes
#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlddx.h>
#include <atlframe.h>
#include <atlribbon.h>

// undefine macros so that std::min and std::max can be used
#undef min
#undef max

// Standard C++ Library includes
#include <string>
#include <vector>

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

// Boost includes
// don't link to several boost libraries
#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
