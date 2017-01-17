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
/// \file Win32.hpp
/// \brief Win32 includes

#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define VC_EXTRALEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NORPC
#define NOPROXYSTUB
#define NOTAPE
#define NOCRYPT
#define NOIMAGE

// Win32 includes
#pragma warning(push)
#pragma warning(disable: 4091) // 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
#include <shlobj.h>
#include <shellapi.h>
#undef ExtractIcon
#pragma warning(pop)
