/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2005 Michael Fink

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

   $Id: stdafx.cpp,v 1.11 2011/01/21 15:41:54 vividos Exp $

*/
/*! \file stdafx.cpp

   \brief source file that includes just the standard includes

   winlame.pch will be the pre-compiled header
   stdafx.obj will contain the pre-compiled type information

*/

// needed includes
#include "stdafx.h"

#if (_ATL_VER < 0x0700)
#include <atlimpl.cpp>
#endif //(_ATL_VER < 0x0700)

#if (_ATL_VER >= 0x0710)
#pragma comment(lib, "atlthunk.lib")
#endif //(_ATL_VER < 0x0700)

/*

// prevent linking against msvcp80[d].dll when only std::uncaught_exception() is needed
#ifdef _STLPORT_VERSION

#ifdef _DEBUG
#  pragma comment(linker, "/nodefaultlib:msvcprtd.lib")
#  if _MSC_VER >= 1400
#    pragma comment(linker, "/nodefaultlib:msvcp80d.lib")
#  elif _MSC_VER >= 1310
#    pragma comment(linker, "/nodefaultlib:msvcp71d.lib")
#  endif
#else
#  pragma comment(linker, "/nodefaultlib:msvcprt.lib")
#  if _MSC_VER >= 1400
#    pragma comment(linker, "/nodefaultlib:msvcp80.lib")
#  elif _MSC_VER >= 1310
#    pragma comment(linker, "/nodefaultlib:msvcp71.lib")
#  endif
#endif

#pragma warning(push)
#pragma warning(disable: 4273) // 'std::uncaught_exception' : inconsistent dll linkage
#pragma warning(disable: 4049) // locally defined symbol ?uncaught_exception@std@@YA_NXZ (bool __cdecl std::uncaught_exception(void)) imported

#include <exception>

// STLport redefines std, so undef it here
#undef std

namespace std
{

bool __cdecl uncaught_exception()
{
   // report if handling a throw
   return (false);
}

} // namespace std

#pragma warning(pop)

#endif // _STLPORT_VERSION
*/
