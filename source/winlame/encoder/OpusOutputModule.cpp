//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2016 Michael Fink
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
/// \file OpusOutputModule.cpp
/// \brief Opus output module
//

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "OpusOutputModule.hpp"

#pragma comment(lib, "libopusfile-0.lib")
#pragma comment(lib, "libopus-0.lib")


// OpusOutputModule methods

OpusOutputModule::OpusOutputModule()
{
   module_id = ID_OM_OPUS;
}

OpusOutputModule::~OpusOutputModule()
{
}

bool OpusOutputModule::isAvailable()
{
   // always available
   return true;
}

void OpusOutputModule::getDescription(CString& desc)
{
}

void OpusOutputModule::getVersionString(CString& version, int special)
{
   ATLASSERT(false);
}

int OpusOutputModule::initOutput(LPCTSTR outfilename,
   SettingsManager& mgr, const TrackInfo& trackinfo,
   SampleContainer& samplecont)
{
   return 0;
}

int OpusOutputModule::encodeSamples(SampleContainer& samples)
{
   int numsamples = 0;

   return numsamples;
}

void OpusOutputModule::doneOutput()
{
}
