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

   $Id: wlOggVorbisInputModule.cpp,v 1.22 2009/11/02 20:30:52 vividos Exp $

*/
/*! \file wlOggVorbisInputModule.cpp

   \brief contains the implementation of the ogg vorbis input module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "wlOggVorbisInputModule.h"
#include "vorbis/vorbisfile.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



// constants

// channel remap stuff
const int MAX_CHANNELS = 6; ///< make this higher to support files with more channels

/// channel remapping map
const int chmap[MAX_CHANNELS][MAX_CHANNELS] = {
   { 0, },               // mono
   { 0, 1, },            // l, r
   { 0, 2, 1, },         // l, c, r -> l, r, c
   { 0, 1, 2, 3, },      // l, r, bl, br
   { 0, 2, 1, 3, 4, },   // l, c, r, bl, br -> l, r, c, bl, br
   { 0, 2, 1, 5, 3, 4 }  // l, c, r, bl, br, lfe -> l, r, c, lfe, bl, br
};

//! ogg vorbis input buffer size
const int ogg_inbufsize = 512*MAX_CHANNELS; // must be a multiple of max channels

// wlOggVorbisInputModule methods

wlOggVorbisInputModule::wlOggVorbisInputModule()
{
   module_id = ID_IM_OGGV;
   dll = NULL;
   msvcrt_wfopen = NULL;
   msvcrt_fclose = NULL;
   ov_info_func = NULL;
   ov_open_func = NULL;
   ov_clear_func = NULL;
   ov_pcm_total_func = NULL;
   ov_read_func = NULL;
   free_handle = true;
}

wlOggVorbisInputModule::~wlOggVorbisInputModule()
{
   if (dll!=NULL && free_handle)
      ::FreeLibrary(dll);
}

wlInputModule *wlOggVorbisInputModule::cloneModule()
{
   return new wlOggVorbisInputModule;
}

bool wlOggVorbisInputModule::isAvailable()
{
   bool avail=false;

   // test dlls for functions
   for(int i=0; i<2 && !avail; i++)
   {
      if (i==0) dll = ::LoadLibrary(_T("libvorbis.dll"));
      if (i==1) dll = ::LoadLibrary(_T("vorbisfile.dll"));

      if (dll != NULL)
      {
         // retrieve function pointer
         ov_info_func = (vorbis_info *(*)(OggVorbis_File *vf,int link))::GetProcAddress(dll,"ov_info");
         ov_open_func = (int (*)(FILE *f,OggVorbis_File *vf,char *initial,long ibytes))::GetProcAddress(dll,"ov_open");
         ov_clear_func = (int *(*)(OggVorbis_File *vf))::GetProcAddress(dll,"ov_clear");
         ov_pcm_total_func = (ogg_int64_t (*)(OggVorbis_File *vf,int i))::GetProcAddress(dll,"ov_pcm_total");
         ov_read_func = (long (*)(OggVorbis_File *vf,char *buffer,int length,
            int bigendianp,int word,int sgned,int *bitstream))::GetProcAddress(dll,"ov_read");
      }

      // check availability
      avail = dll != NULL &&
         ov_info_func != NULL &&
         ov_open_func != NULL &&
         ov_clear_func != NULL &&
         ov_pcm_total_func != NULL &&
         ov_read_func != NULL;

      if (dll != NULL && !avail)
      {
         ::FreeLibrary(dll);
         dll = NULL;
      }
   }

   return avail;
}

void wlOggVorbisInputModule::getDescription(CString& desc)
{
   vorbis_info* vi=ov_info_func(&vf,-1);

   if (vi->bitrate_upper != -1 && vi->bitrate_lower != -1)
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_VBR,
         vi->bitrate_lower/1000, vi->bitrate_upper/1000,
         channels, samplerate);
   }
   else if (vi->bitrate_nominal==-1)
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_FREE, channels, samplerate);
   }
   else
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_NOMINAL,
         vi->bitrate_nominal/1000, channels, samplerate);
   }
}

CString wlOggVorbisInputModule::getFilterString()
{
   CString cszFilter;
   cszFilter.LoadString(IDS_FILTER_OGG_VORBIS_INPUT);
   return cszFilter;
}

int wlOggVorbisInputModule::initInput(LPCTSTR infilename,
   wlSettingsManager &mgr, wlTrackInfo &trackinfo,
   wlSampleContainer &samplecont)
{
   isAvailable();

   HMODULE hMod = ::GetModuleHandle(_T("msvcrt.dll"));
   msvcrt_wfopen = (FILE* (*)(const wchar_t*, const wchar_t*))::GetProcAddress(hMod,"_wfopen");

   // open input file
   USES_CONVERSION;
   if (msvcrt_wfopen)
      infile = msvcrt_wfopen(infilename, _T("rb"));
   else
      infile = _wfopen(infilename, _T("rb"));

   if (infile==NULL)
   {
      lasterror.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   // open ogg vorbis file
   if (ov_open_func(infile, &vf, NULL, 0) < 0)
   {
      lasterror.Format(IDS_ENCODER_INVALID_FILE_FORMAT);
      return -2;
   }

   // retrieve file infos
   numsamples = 0;
   maxsamples = ov_pcm_total_func(&vf,-1);

   vorbis_info *vi=ov_info_func(&vf,-1);
   channels = vi->channels;
   samplerate = vi->rate;

   // set up input traits
   samplecont.setInputModuleTraits(sizeof(short)*8,wlSamplesInterleaved,samplerate,channels);

   return 0;
}

void wlOggVorbisInputModule::getInfo(int &channels, int &bitrate, int &length, int &samplerate)
{
   vorbis_info *vi=ov_info_func(&vf,-1);

   channels = vi->channels;
   bitrate = vi->bitrate_nominal;
   length = int(maxsamples / vi->rate);
   samplerate = vi->rate;
}

int wlOggVorbisInputModule::decodeSamples(wlSampleContainer &samples)
{
   short buffer[ogg_inbufsize];
   int bitstream;

   // temporary sample buffer
   short *outbuffer;
   short tmpbuf[ogg_inbufsize];

   // read in samples
   int ret = ov_read_func(&vf,reinterpret_cast<char*>(buffer),
      ogg_inbufsize * sizeof(short), 0, sizeof(short), 1, &bitstream);

   if (ret<0)
   {
      lasterror.LoadString(IDS_ENCODER_INTERNAL_DECODE_ERROR);
      return ret;
   }

   ret /= channels*sizeof(short);

   // channel remap
   if (channels > 2 && ret > 0)
   {
      outbuffer = tmpbuf;
      for (int i = 0; i < ret; i++)
         for (int j = 0; j < std::min(channels, MAX_CHANNELS); j++)
            outbuffer[i*channels+j] = buffer[i*channels+(chmap[channels-1][j])];

      // copy the remaining channels
      if (channels > MAX_CHANNELS)
      {
         for (int i = 0; i < ret; i++)
            for (int j = MAX_CHANNELS; j < channels; j++)
               outbuffer[i*channels+j] = buffer[i*channels+j];
      }
   }
   else
      outbuffer = buffer;

   // put samples into container
   if (ret>0)
      samples.putSamplesInterleaved(outbuffer,ret);

   // count samples
   numsamples += ret;

   return ret;
}

void wlOggVorbisInputModule::doneInput()
{
   HMODULE hMod = ::GetModuleHandle(_T("msvcrt.dll"));
   msvcrt_fclose = (int (*)(FILE*))::GetProcAddress(hMod,"fclose");

   // close infile
   if (msvcrt_fclose)
      msvcrt_fclose(infile);
   else
      fclose(infile);

   // cleanup
   ov_clear_func(&vf);
}
