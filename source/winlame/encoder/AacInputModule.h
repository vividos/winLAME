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
/// \file AacInputModule.h
/// \brief contains the AAC input module definition
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include <iosfwd>
#include "neaacdec.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "aacinfo/aacinfo.h"

#ifdef __cplusplus
}
#endif


// constants

/// size of one AAC frame
const int aac_framesize = 2048;

/// max. numbers of channels the module is able to handle
const int aac_maxchannels = 8;

/// calculated input buffer size
const int aac_inbufsize = aac_framesize*aac_maxchannels;


/// AAC input module

class AacInputModule: public InputModule
{
public:
   // ctor
   AacInputModule();

   /// clones input module
   virtual InputModule *cloneModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("libfaad2 (") _T(FAAD2_VERSION) _T(") AAC Audio File Decoder"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns filter string
   virtual CString getFilterString();

   // initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager &mgr,
      TrackInfo &trackinfo, SampleContainer &samples);

   // returns info about the input file
   virtual void getInfo(int &channels, int &bitrate, int &length, int &samplerate);

   // decodes samples and stores them in the sample container
   virtual int decodeSamples(SampleContainer &samples);

   // returns the number of percent done
   virtual float percentDone()
   {
      return filelen==0 ? 0.0f : float(filepos)*100.f/filelen;
   }

   // called when done with decoding
   virtual void doneInput();

protected:
   /// libfaad handle
   faacDecHandle decoder;

   /// file input buffer
   unsigned char inbuffer[aac_inbufsize];

   /// high watermark for the inbuffer
   int highmark;

   /// length of input file
   unsigned long filelen;

   /// current position
   unsigned long filepos;

   /// aac file info
   faadAACInfo info;

   /// input file stream
   std::ifstream istr;

   /// last error occured
   CString lasterror;
};


/// @}
