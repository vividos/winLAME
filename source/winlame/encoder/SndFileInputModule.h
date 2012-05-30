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
/*! \file SndFileInputModule.h

   \brief contains the libsndfile input module definition

*/
/*! \ingroup encoder */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include "sndfile.h"


//! libsndfile input module

class SndFileInputModule: public InputModule
{
public:
   // ctor
   SndFileInputModule();
   virtual ~SndFileInputModule(){}

   //! clones input module
   virtual InputModule *cloneModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("libsndfile Audio File Decoder"); }

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
   virtual float percentDone()
   {
      return sfinfo.frames != 0 ? float(samplecount)*100.f/float(sfinfo.frames) : 0.f;
   }

   // called when done with decoding
   virtual void doneInput();

protected:
   //! searches for id3 tag chunk in the wave file
   bool waveGetId3(LPCTSTR wavfile, TrackInfo &trackinfo);

protected:
   //! file handle
   SNDFILE *sndfile;

   //! soundfile info
   SF_INFO sfinfo;

   //! counts the samples already decoded
   int samplecount;

   //! last error occured
   CString lasterror;

   //! sample buffer
   void *buffer;

   //! output bits
   int outbits;

#ifdef SNDFILE_1
   //! filter string
   CString filterstring;
#endif
};


//@}
