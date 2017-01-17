/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2016 Michael Fink

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

#include "Win32.h"
#include "Atl.h"
#include "Wtl.h"

// undefine macros so that std::min and std::max can be used
#undef min
#undef max

#include "StdCppLib.h"
#include "Boost.hpp"

// winLAME includes
#include "IoCContainer.hpp"
#include "Path.hpp"
#include "ModuleManager.hpp"
#include "encoder\ModuleInterface.hpp"
#include "ui\WizardPage.hpp"
#include "ui\WizardPageHost.hpp"

#pragma warning(disable: 4100) // unreferenced formal parameter
