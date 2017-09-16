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
/// \file unittest/stdafx.cpp
/// \brief source file that includes just the standard includes
/// winlame.pch will be the pre-compiled header
/// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// include atlribbon.h here, to pick up the version that supports Visual Studio 2017
// this block can be removed once WTL supports this compiler (WTL version > 9.1).
#define _WTL_NO_CSTRING ///< don't use WTL CString
#define _WTL_NO_WTYPES ///< don't use WTL types, such as CSize, CRect, etc.
#include <atlapp.h>
#include "../atlribbon.h"

#include <ulib/config/Wtl.hpp>
#include "App.hpp"
#include "CDRipTrackManager.hpp"

// some functions missing from the encoder.lib static library

LPCTSTR g_pszCDRipPrefix = _T("cdrip://");

CString App::Version()
{
   return _T("winLAME UnitTest");
}

CDRipTrackManager* CDRipTrackManager::getCDRipTrackManager()
{
   throw std::runtime_error("unittest");
}
