/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink

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

   $Id: wlMadMpegInputModule.h,v 1.17 2009/04/10 22:04:55 vividos Exp $

*/
/*! \file wlMadMpegInputModule.h

   \brief contains the MAD mpeg input module definition

*/
/*! \ingroup encoder */
/*! @{ */

// prevent multiple including
#ifndef __wlmadmpeginputmodule_h_
#define __wlmadmpeginputmodule_h_

// needed includes
#include "wlModuleInterface.h"
#include <iosfwd>
#include "mad.h"
//#include "dither/dither.h"


// constants

//! input buffer size
const int mad_inbufsize = 4096;


//! mad mpeg input module

class wlMadMpegInputModule: public wlInputModule
{
public:
   // ctor
   wlMadMpegInputModule();

   //! clones input module
   virtual wlInputModule *cloneModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("MAD MPEG Audio File Decoder"); }

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
   virtual int initInput(LPCTSTR infilename, wlSettingsManager &mgr,
      wlTrackInfo &trackinfo, wlSampleContainer &samples);

   // returns info about the input file
   virtual void getInfo(int &channels, int &bitrate, int &length, int &samplerate);

   // decodes samples and stores them in the sample container
   virtual int decodeSamples(wlSampleContainer &samples);

   // returns the number of percent done
   virtual float percentDone()
   {
      return maxsamples==0 ? 0.0f : float(numsamples*100)/maxsamples;
   }

   // called when done with decoding
   virtual void doneInput();

private:
   //! checks for Xing / VBR info tag and adjusts bitrate
   bool checkVbrInfoTag(unsigned char *buffer);

   //! retrieves id3v2 tag infos
   static bool GetId3v2TagInfos(const CString& cszFilename, wlTrackInfo& trackinfo);

protected:
   // mad structs

   //! first decoded frame header
   struct mad_header init_header;

   //! stream struct
   struct mad_stream stream;
   //! frame struct
   struct mad_frame frame;
   //! struct for synthesizing output samples
   struct mad_synth synth;
/*
   //! dither structs
   struct audio_dither left_dither, right_dither;
   struct audio_stats stats;
*/

   //! end of MPEG stream
   bool eos;

   //! input file stream
   std::ifstream istr;

   //! file input buffer
   unsigned char inbuffer[mad_inbufsize];

   //! number of samples decoded
   __int64 numsamples;

   //! number of samples in the mpeg file
   __int64 maxsamples;

   //! last error occured
   CString lasterror;
};


//@}

#endif
