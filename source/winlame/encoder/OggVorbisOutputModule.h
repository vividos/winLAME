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

*/
/// \file OggVorbisOutputModule.h
/// \brief contains the ogg vorbis output module definition
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include <iosfwd>
#include "vorbis/codec.h"


/// ogg vorbis output module

class OggVorbisOutputModule: public OutputModule
{
public:
   // ctor
   OggVorbisOutputModule();
   // dtor
   ~OggVorbisOutputModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("Ogg Vorbis Encoder"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns version string
   virtual void getVersionString(CString& version, int special=0);

   // returns the extension the output module produces
   virtual LPCTSTR getOutputExtension(){ return _T("ogg"); }

   // initializes the output module
   virtual int initOutput(LPCTSTR outfilename, SettingsManager &mgr,
      const TrackInfo& trackinfo, SampleContainer &samplecont);

   // encodes samples from the sample container
   virtual int encodeSamples(SampleContainer &samples);

   // cleans up the output module
   virtual void doneOutput();

protected:
   /// output file stream
   std::ofstream ostr;

   /// last error occured
   CString lasterror;

   /// chosen bitrate mode
   int brmode;

   /// base quality (when brmode==0)
   float base_quality;

   /// end of stream marker
   bool eos;

   /// take physical pages, weld into a logical stream of packets
   ogg_stream_state os;

   /// one ogg bitstream page. vorbis packets are inside
   ogg_page         og;
   
   /// one raw packet of data for decode
   ogg_packet       op;

   /// struct that stores all the static vorbis bitstream settings
   vorbis_info      vi;

   /// struct that stores all the bitstream user comments
   vorbis_comment   vc;

   /// central working state for the packet->PCM decoder
   vorbis_dsp_state vd;

   /// local working space for packet->PCM decode
   vorbis_block     vb;
};


/// @}
