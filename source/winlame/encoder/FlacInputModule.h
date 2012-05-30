/*
   winLAME - a frontend for the LAME encoding engine
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
/// \file FlacInputModule.h
/// \brief contains the FLAC input module definition
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include "FLAC/file_decoder.h"

/// flac decoding context
typedef struct {
   FLAC__StreamMetadata_StreamInfo streaminfo;  ///< stream info
   FLAC__int32 *reservoir;                      ///< reservoir
   unsigned int samples_in_reservoir;           ///< number of samples in reservoir
   unsigned int totalLenMs;                     ///< total length in ms
   bool abort_flag;                             ///< abort flag
   TrackInfo* trackInfo;                      ///< track info
} FLAC_context;

/// FLAC input module

class FlacInputModule: public InputModule
{
public:
   // ctor
   FlacInputModule();

   /// clones input module
   virtual InputModule *cloneModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("FLAC Audio File Decoder"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns version string
   virtual void getVersionString(CString& version, int special=0);

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
   virtual float percentDone();

   // called when done with decoding
   virtual void doneInput();

protected:
   /// length of input file
   unsigned long filelen;

   /// last error occured
   CString lasterror;

   /// local variables
   FLAC__FileDecoder *pFLACDec;

   /// flac context
   FLAC_context *context;

   /// sample position
   FLAC__uint64 pos_sample;

   /// buffer length
   unsigned int PCMBuffLen;
};


/// @}
