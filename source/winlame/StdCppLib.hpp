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
/// \file StdCppLib.hpp
/// \brief Standard C++ Library includes

#pragma once

// switch off some /analyze warnings
#pragma warning(push)
#pragma warning(disable: 28252 28253)

#include <string>
#include <vector>
#include <memory>

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#pragma warning(pop)
