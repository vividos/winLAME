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
/*! \file OggVorbisOutputModule.h

   \brief contains the ogg vorbis output module definition

*/
/*! \ingroup encoder */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include <iosfwd>
#include "vorbis/codec.h"


//! ogg vorbis output module

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
   //! output file stream
   std::ofstream ostr;

   //! last error occured
   CString lasterror;

   //! chosen bitrate mode
   int brmode;

   //! base quality (when brmode==0)
   float base_quality;

   //! end of stream marker
   bool eos;

   //! take physical pages, weld into a logical stream of packets
   ogg_stream_state os;

   //! one ogg bitstream page. vorbis packets are inside
   ogg_page         og;
   
   //! one raw packet of data for decode
   ogg_packet       op;

   //! struct that stores all the static vorbis bitstream settings
   vorbis_info      vi;

   //! struct that stores all the bitstream user comments
   vorbis_comment   vc;

   //! central working state for the packet->PCM decoder
   vorbis_dsp_state vd;

   //! local working space for packet->PCM decode
   vorbis_block     vb;

   HMODULE vorbisenc;
   HMODULE vorbis;
   HMODULE ogg;

   // vorbisenc.dll function pointer

   int (*vorbis_encode_init_func)(vorbis_info *vi,long channels,long rate,
      long max_bitrate,long nominal_bitrate,long min_bitrate);

   int (*vorbis_encode_init_vbr_func)(vorbis_info *vi,long channels,
      long rate,float base_quality);

   // vorbis.dll function pointer

   void (*vorbis_info_init_func)(vorbis_info *vi);
   void (*vorbis_info_clear_func)(vorbis_info *vi);
   void (*vorbis_dsp_clear_func)(vorbis_dsp_state *v);
   int  (*vorbis_block_init_func)(vorbis_dsp_state *v, vorbis_block *vb);
   int  (*vorbis_block_clear_func)(vorbis_block *vb);

   void (*vorbis_comment_init_func)(vorbis_comment *vc);
   void (*vorbis_comment_add_func)(vorbis_comment *vc, char *comment);
   void (*vorbis_comment_add_tag_func)(vorbis_comment *vc, char *tag, char *contents);
   void (*vorbis_comment_clear_func)(vorbis_comment *vc);

   int (*vorbis_analysis_init_func)(vorbis_dsp_state *v,vorbis_info *vi);
   int (*vorbis_analysis_func)(vorbis_block *vb,ogg_packet *op);
   int (*vorbis_analysis_headerout_func)(vorbis_dsp_state *v,vorbis_comment *vc,
      ogg_packet *op,ogg_packet *op_comm,ogg_packet *op_code);
   float **(*vorbis_analysis_buffer_func)(vorbis_dsp_state *v,int vals);
   int (*vorbis_analysis_wrote_func)(vorbis_dsp_state *v,int vals);
   int (*vorbis_analysis_blockout_func)(vorbis_dsp_state *v,vorbis_block *vb);
   int (*vorbis_synthesis_headerin_func)(vorbis_info *vi,vorbis_comment *vc,ogg_packet *op);
   int (*vorbis_bitrate_addblock_func)(vorbis_block *vb);
   int (*vorbis_bitrate_flushpacket_func)(vorbis_dsp_state *vd,ogg_packet *op);

   // ogg.dll function pointer

   int (*ogg_stream_init_func)(ogg_stream_state *os,int serialno);
   int (*ogg_stream_packetin_func)(ogg_stream_state *os, ogg_packet *op);
   int (*ogg_stream_flush_func)(ogg_stream_state *os, ogg_page *og);
   int (*ogg_stream_pageout_func)(ogg_stream_state *os, ogg_page *og);
   int (*ogg_page_eos_func)(ogg_page *og);
   int (*ogg_stream_clear_func)(ogg_stream_state *os);
};


//@}
