/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink
   Copyright (c) 2004 DeXT

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
/// \file AacOutputModule.h
/// \brief contains the AAC output module definition
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include <iosfwd>
#include "faac.h"


/// AAC output module

class AacOutputModule: public OutputModule
{
public:
   // ctor
   AacOutputModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("AAC Audio File Encoder (libfaac)"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   /// returns version string
   virtual void getVersionString(CString& version, int special = 0);

   // returns the extension the output module produces
   virtual LPCTSTR getOutputExtension(){ return _T("aac"); }

   // initializes the output module
   virtual int initOutput(LPCTSTR outfilename, SettingsManager &mgr,
      const TrackInfo& trackinfo, SampleContainer &samplecont);

   // encodes samples from the sample container
   virtual int encodeSamples(SampleContainer &samples);

   // cleans up the output module
   virtual void doneOutput();

protected:
   /// encoder handle
   faacEncHandle handle;

   /// size of input buffer in bytes
   unsigned long aac_inbufsize;

   /// size of output buffer in bytes
   unsigned long aac_outbufsize;

   /// output buffer
   unsigned char *aac_outbuf;

   /// sample buffer
   short *sbuffer;

   /// sample buffer high watermark
   int sbufhigh;

   /// bitrate control method
   int brcmethod;

   /// output file stream
   std::ofstream ostr;

   /// last error occured
   CString lasterror;
};


/// @}
