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
/// \file unittest/stdafx.h
/// \brief include file for include files used for precompiled headers
/// include file for standard system include files,
/// or project specific include files that are used frequently, but
/// are changed infrequently

#pragma once

#include <ulib/config/Win32.hpp>
#include <ulib/config/Atl.hpp>

// undefine macros so that std::min and std::max can be used
#undef min
#undef max

#include "../StdCppLib.hpp"

/// define that is used to mark unused parameters or parameters only used in ATLASSERTs
#ifndef UNUSED
#define UNUSED(x) (void)(x);
#endif

// redefine ATLVERIFY when analyzing using Coverity Scan
#if !defined(_DEBUG) && defined(__COVERITY__)
#undef ATLVERIFY
#define ATLVERIFY(expr) (void)(expr)
#endif

// unit test includes
#include "CppUnitTest.h"

/// this dll's HINSTANCE handle
extern HINSTANCE g_hDllInstance;
