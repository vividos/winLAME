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
/// \file OggVorbisOutputModule.cpp
/// \brief contains the implementation of the ogg vorbis output module

// needed includes
#include "stdafx.h"
#include <fstream>
#include "resource.h"
#include "OggVorbisOutputModule.h"
#include "vorbis/vorbisenc.h"
#include <ctime>
#include <cmath>

// link to libvorbis.dll
#pragma comment(lib, "libvorbis.lib")


// channel remap stuff
const int MAX_CHANNELS = 6; ///< make this higher to support files with more channels

/// channel remapping map
const int chmap[MAX_CHANNELS][MAX_CHANNELS] = {
   { 0, },               // mono
   { 0, 1, },            // l, r
   { 0, 2, 1, },         // l, r, c -> l, c, r
   { 0, 1, 2, 3, },      // l, r, bl, br
   { 0, 2, 1, 3, 4, },   // l, r, c, bl, br -> l, c, r, bl, br
   { 0, 2, 1, 4, 5, 3 }  // l, r, c, lfe, bl, br -> l, c, r, bl, br, lfe
};

// OggVorbisOutputModule methods

OggVorbisOutputModule::OggVorbisOutputModule()
{
   module_id = ID_OM_OGGV;
}

OggVorbisOutputModule::~OggVorbisOutputModule()
{
}

bool OggVorbisOutputModule::isAvailable()
{
   // we don't do delay-loading anymore, so it's available always
   return true;
}

void OggVorbisOutputModule::getDescription(CString& desc)
{
   switch (brmode)
   {
   case 0: // quality mode
      {
         CString cszQuality;
         cszQuality.Format(_T("%s%u.%02u"),
            base_quality < 0.0 ? _T("-") : _T(""),
            static_cast<unsigned int>(fabs(base_quality*10.0)),
            static_cast<unsigned int>((fabs(base_quality*10.0) - static_cast<unsigned int>(fabs(base_quality*10.0)))*100.0));

         desc.Format(IDS_FORMAT_INFO_OGGV_OUTPUT_QUALITY,
            cszQuality, m_vi.rate, m_vi.channels);
      }
      break;
   case 1: // variable bitrate
      desc.Format(IDS_FORMAT_INFO_OGGV_OUTPUT_VBR,
         m_vi.bitrate_lower/1000, m_vi.bitrate_upper/1000, m_vi.rate, m_vi.channels);
      break;
   case 2: // average bitrate
      desc.Format(IDS_FORMAT_INFO_OGGV_OUTPUT_ABR,
         m_vi.bitrate_nominal/1000, m_vi.rate, m_vi.channels);
      break;
   case 3: // constant bitrate
      desc.Format(IDS_FORMAT_INFO_OGGV_OUTPUT_CBR,
         m_vi.bitrate_nominal/1000, m_vi.rate, m_vi.channels);
      break;
   }
}

void OggVorbisOutputModule::getVersionString(CString& version, int special)
{
   // retrieve ogg vorbis build version
   if (isAvailable())
   {
      // to get build version, create first 3 ogg packets,
      // take the second and read it back in; the vc.vendor
      // then is filled with build version
      vorbis_info vi;
      vorbis_info_init(&vi);
      vorbis_encode_init(&vi,2,44100, -1, 192000, -1);

      vorbis_block vb;
      vorbis_dsp_state vd;
      vorbis_analysis_init(&vd,&vi);
      vorbis_block_init(&vd,&vb);

      vorbis_comment vc;
      vorbis_comment_init(&vc);

      ogg_packet header;
      ogg_packet header_comm;
      ogg_packet header_code;

      vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);

      vorbis_synthesis_headerin(&vi,&vc,&header_comm);

      USES_CONVERSION;
      version = _T("(");
      version += A2CT(vc.vendor);
      version += _T(")");

      vorbis_block_clear(&vb);
      vorbis_dsp_clear(&vd);
      vorbis_comment_clear(&vc);
      vorbis_info_clear(&vi);
   }
}

/// converts string to UTF-8 encoding
// note: ogg vorbis tag values are stored as UTF-8, so convert here
void UnicodeToUTF8(const CString& prop, std::vector<char>& vecBuffer)
{
#if defined(UNICODE) || defined(_UNICODE)
   // calculate the bytes necessary
   unsigned int uiLen = ::WideCharToMultiByte(CP_UTF8, 0, prop, -1, NULL, 0, NULL, NULL);

   vecBuffer.resize(uiLen);

   // convert
   ::WideCharToMultiByte(CP_UTF8, 0, prop, -1, &vecBuffer[0], uiLen, NULL, NULL);
#else
#error non-unicode variant not implemented!
#endif
}

int OggVorbisOutputModule::initOutput(LPCTSTR outfilename,
   SettingsManager &mgr, const TrackInfo& trackinfo,
   SampleContainer &samplecont)
{
   isAvailable();

   base_quality = 1.0;
   eos = false;

   m_channels = samplecont.getInputModuleChannels();
   m_samplerate = samplecont.getInputModuleSampleRate();


   // open output file
   USES_CONVERSION;
   ostr.open(T2CA(outfilename),std::ios::out|std::ios::binary);
   if (!ostr.is_open())
   {
      lasterror.LoadString(IDS_ENCODER_OUTPUT_FILE_CREATE_ERROR);
      return -1;
   }


   /********** Encode setup ************/

   /* choose an encoding mode */
   vorbis_info_init(&m_vi);

   int ret = 0;

   switch(brmode = mgr.queryValueInt(OggBitrateMode))
   {
   case 0: // base quality mode
      base_quality = mgr.queryValueInt(OggBaseQuality)/1000.f;
      ret = vorbis_encode_init_vbr(&m_vi, m_channels, m_samplerate,base_quality);
      break;

   case 1: // variable bitrate mode
      {
         int min_bitrate = mgr.queryValueInt(OggVarMinBitrate);
         int max_bitrate = mgr.queryValueInt(OggVarMaxBitrate);
         ret = vorbis_encode_init(&m_vi, m_channels, m_samplerate,
            max_bitrate*1000, -1, min_bitrate*1000);
      }
      break;

   case 2: // average bitrate mode
      {
         int average_bitrate = mgr.queryValueInt(OggVarNominalBitrate);
         ret = vorbis_encode_init(&m_vi, m_channels, m_samplerate,
            -1, average_bitrate*1000, -1);
      }
      break;

   case 3: // constant bitrate mode
      {
         int const_bitrate = mgr.queryValueInt(OggVarNominalBitrate);
         ret = vorbis_encode_init(&m_vi, m_channels, m_samplerate,
            const_bitrate*1000, const_bitrate*1000, const_bitrate*1000);
      }
      break;
   }

   if (ret < 0)
   {
      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);
      lasterror += _T(" (");

      switch(ret)
      {
      case OV_EIMPL:
         lasterror += _T("Unimplemented mode; unable to comply with quality level request.");
         break;

      case OV_EINVAL:
         lasterror += _T("Invalid setup request, e.g., out of range argument.");
         break;

      case OV_EFAULT:
         lasterror += _T("Internal logic fault; indicates a bug or heap/stack corruption.");
         break;
      }

      lasterror += _T(")");

      vorbis_info_clear(&m_vi);

      return ret;
   }

   /* add a comment */
   vorbis_comment_init(&m_vc);

   // add track properties
   {
      USES_CONVERSION;
      CString prop;
      std::vector<char> buffer;

      bool avail = false;
      prop = trackinfo.TextInfo(TrackInfoTitle, avail);
      if (avail && !prop.IsEmpty())
      {
         UnicodeToUTF8(prop, buffer);
         vorbis_comment_add_tag(&m_vc, "TITLE", &buffer[0]);
      }

      prop = trackinfo.TextInfo(TrackInfoArtist, avail);
      if (avail && !prop.IsEmpty())
      {
         UnicodeToUTF8(prop, buffer);
         vorbis_comment_add_tag(&m_vc, "ARTIST", &buffer[0]);
      }

      prop = trackinfo.TextInfo(TrackInfoAlbum, avail);
      if (avail && !prop.IsEmpty())
      {
         UnicodeToUTF8(prop, buffer);
         vorbis_comment_add_tag(&m_vc, "ALBUM", &buffer[0]);
      }

      int iYear = trackinfo.NumberInfo(TrackInfoYear, avail);
      if (avail && iYear > 0)
      {
         prop.Format(_T("%i"), iYear);
         UnicodeToUTF8(prop, buffer);
         vorbis_comment_add_tag(&m_vc, "DATE", &buffer[0]);
      }

      prop = trackinfo.TextInfo(TrackInfoComment, avail);
      if (avail && !prop.IsEmpty())
      {
         UnicodeToUTF8(prop, buffer);
         vorbis_comment_add_tag(&m_vc, "COMMENT", &buffer[0]); // should be DESCRIPTION?
      }

      int iTrack = trackinfo.NumberInfo(TrackInfoTrack, avail);
      if (avail && iTrack >= 0)
      {
         prop.Format(_T("%i"), iTrack);
         UnicodeToUTF8(prop, buffer);
         vorbis_comment_add_tag(&m_vc, "TRACKNUMBER", &buffer[0]);
      }

      CString cszGenre = trackinfo.TextInfo(TrackInfoGenre, avail);
      if (avail)
      {
         if (!cszGenre.IsEmpty())
         {
            UnicodeToUTF8(cszGenre, buffer);
            vorbis_comment_add_tag(&m_vc, "GENRE", &buffer[0]);
         }
      }
   }

   /* set up the analysis state and auxiliary encoding storage */
   vorbis_analysis_init(&m_vd,&m_vi);
   vorbis_block_init(&m_vd,&m_vb);

   /* set up our packet->stream encoder */
   /* pick a random serial number; that way we can more likely build
      chained streams just by concatenation */
   srand(static_cast<unsigned int>(time(NULL)));
   ogg_stream_init(&m_os,rand());


   /* Vorbis streams begin with three headers; the initial header (with
      most of the codec setup parameters) which is mandated by the Ogg
      bitstream spec.  The second header holds any comment fields.  The
      third header holds the bitstream codebook.  We merely need to
      make the headers, then pass them to libvorbis one at a time;
      libvorbis handles the additional Ogg bitstream constraints */

   {
      ogg_packet header;
      ogg_packet header_comm;
      ogg_packet header_code;

      vorbis_analysis_headerout(&m_vd,&m_vc,&header,&header_comm,&header_code);
      /* automatically placed in its own page */
      ogg_stream_packetin(&m_os,&header);

      ogg_stream_packetin(&m_os,&header_comm);
      ogg_stream_packetin(&m_os,&header_code);

      /* We don't have to write out here, but doing so makes streaming 
         much easier, so we do, flushing ALL pages. This ensures the actual
         audio data will start on a new page */
      while(!eos)
      {
         int result=ogg_stream_flush(&m_os,&m_og);
         if(result==0)
            break;

         ostr.write(reinterpret_cast<char*>(m_og.header), m_og.header_len);
         ostr.write(reinterpret_cast<char*>(m_og.body), m_og.body_len);
      }
   }

   // set up output traits
   samplecont.setOutputModuleTraits(16,SamplesChannelArray, m_samplerate, m_channels);

   return 0;
}

int OggVorbisOutputModule::encodeSamples(SampleContainer &samples)
{
   // end of stream?
   if (eos) return 0;

   // get samples
   int numsamples=0;
   short **buffer = (short**)samples.getSamplesArray(numsamples);

   if (numsamples!=0)
   {
      float **sbuffer=vorbis_analysis_buffer(&m_vd,numsamples);

      // copy samples to analysis buffer
     if (m_channels > 2)
     {
        // channel remap
         for(int ch=0; ch<std::min(m_channels,MAX_CHANNELS); ch++)
            for(int i=0; i<numsamples; i++)
               sbuffer[ch][i] = float(buffer[chmap[m_channels-1][ch]][i]) / 32768.f;
        if (m_channels > MAX_CHANNELS)
        {
            for(int ch=MAX_CHANNELS; ch<m_channels; ch++)
               for(int i=0; i<numsamples; i++)
                  sbuffer[ch][i] = float(buffer[ch][i]) / 32768.f;
        }
     }
     else
     {
         for(int ch=0; ch<m_channels; ch++)
            for(int i=0; i<numsamples; i++)
               sbuffer[ch][i] = float(buffer[ch][i]) / 32768.f;
     }
   }

   /* tell the library how much we actually submitted */
   vorbis_analysis_wrote(&m_vd,numsamples);

   /* vorbis does some data preanalysis, then divvies up blocks for
      more involved (potentially parallel) processing.  Get a single
      block for encoding now */

   while(vorbis_analysis_blockout(&m_vd,&m_vb)==1)
   {
      /* analysis, assume we want to use bitrate management */
      vorbis_analysis(&m_vb,&m_op);
      vorbis_bitrate_addblock(&m_vb);

      while(vorbis_bitrate_flushpacket(&m_vd,&m_op))
      {

      /* weld the packet into the bitstream */
      ogg_stream_packetin(&m_os,&m_op);

      /* write out pages (if any) */
      while(!eos)
      {
         int result=ogg_stream_pageout(&m_os,&m_og);
         if (result==0)
            break;

         ostr.write(reinterpret_cast<char*>(m_og.header), m_og.header_len);
         ostr.write(reinterpret_cast<char*>(m_og.body), m_og.body_len);

         /* this could be set above, but for illustrative purposes, I do
            it here (to show that vorbis does know where the stream ends) */

         if(ogg_page_eos(&m_og)) eos=true;
      }

      }
   }

   return numsamples;
}

void OggVorbisOutputModule::doneOutput()
{
   if (!lasterror.IsEmpty())
      return;

   vorbis_analysis_wrote(&m_vd,0);

   // put out the last ogg block

   // this is a very bad hack, don't do this at home!
   SampleContainer samples;
   samples.putSamplesInterleaved(NULL,0);
   encodeSamples(samples);

   /* clean up and exit.  vorbis_info_clear() must be called last */

   ogg_stream_clear(&m_os);
   vorbis_block_clear(&m_vb);
   vorbis_dsp_clear(&m_vd);
   vorbis_comment_clear(&m_vc);
   vorbis_info_clear(&m_vi);

   /* ogg_page and ogg_packet structs always point to storage in
      libvorbis.  They're never freed or manipulated directly */

   // close file
   ostr.close();
}
