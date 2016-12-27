//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2007 Michael Fink
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
/// \file ModuleInterface.hpp
/// \brief contains the interface definition for input and output modules
//
#pragma once

#include <string>
#include "SettingsManager.h"
#include "TrackInfo.hpp"
#include "SampleContainer.hpp"
#include "InputModule.hpp"
#include "OutputModule.hpp"

namespace Encoder
{

   // input and output module ids

#define ID_OM_LAME                      1
#define ID_OM_OGGV                      2
#define ID_OM_WAVE                      3
#define ID_OM_AAC                       4
#define ID_IM_SNDFILE                   5
#define ID_IM_MAD                       6
#define ID_IM_OGGV                      7
#define ID_IM_AAC                       8
#define ID_IM_FLAC                      10
#define ID_IM_BASS                      11
#define ID_OM_BASSWMA                   12
#define ID_IM_CDRIP                     13
#define ID_IM_SPEEX                     14
#define ID_IM_OPUS                      15
#define ID_OM_OPUS                      16

   /// returns a filename compatible for ansi APIs such as fopen()
   CString GetAnsiCompatFilename(LPCTSTR pszFilename);

} // namespace Encoder
