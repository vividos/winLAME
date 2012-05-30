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

   $Id: AacInputModule.cpp,v 1.28 2009/12/17 18:49:51 vividos Exp $

*/
/*! \file AacInputModule.cpp

   \brief contains the implementation of the AAC input module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "AacInputModule.h"
#include <sys/types.h>
#include <sys/stat.h>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// linker options

#if _MSC_VER < 1400
#pragma comment(linker, "/delayload:libfaad2.dll")
#endif

// channel remap stuff

const int MAX_CHANNELS = 6; ///< make this higher to support files with more channels

/// channel remapping map
const int chmap[MAX_CHANNELS][MAX_CHANNELS] = {
   { 0, },               // mono
   { 0, 1, },            // l, r
   { 1, 2, 0, },         // c, l, r -> l, r, c
   { 1, 2, 0, 3, },      // c, l, r, bc -> l, r, c, bc
   { 1, 2, 0, 3, 4, },   // c, l, r, bl, br -> l, r, c, bl, br
   { 1, 2, 0, 5, 3, 4 }  // c, l, r, bl, br, lfe -> l, r, c, lfe, bl, br
};

// AacInputModule methods

AacInputModule::AacInputModule()
{
   module_id = ID_IM_AAC;
}

InputModule *AacInputModule::cloneModule()
{
   return new AacInputModule;
}

bool AacInputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("libfaad2.dll"));
   bool avail = dll != NULL;
   if (avail) ::FreeLibrary(dll);

   return avail;
}

void AacInputModule::getDescription(CString& desc)
{
   // determine object type
   LPCTSTR object = _T("???");
   switch (info.object_type)
   {
   case 0:
      object = _T("MAIN");
      break;
   case 1:
      object = _T("LC");
      break;
   case 2:
      object = _T("SSR");
      break;
   case 3:
      object = _T("LTP");
      break;
   case 4:
      object = _T("HE AAC");
      break;
   case 16:
      object = _T("ER_LC");
      break;
   case 18:
      object = _T("ER_LTP");
      break;
   case 22:
      object = _T("LD");
      break;
   case 26:
      object = _T("DRM_ER_LC");
      break;
   }

   // determine header type
   LPCTSTR header = _T("???");
   switch (info.headertype)
   {
   case 0:
      header = _T("RAW");
      break;
   case 1:
      header = _T("ADIF");
      break;
   case 2:
      header = _T("ADTS");
      break;
   }

   // format info string
   desc.Format(IDS_FORMAT_INFO_AAC_INFO,
      info.version, object, info.bitrate/1000, info.sampling_rate,
      info.channels, header);

}

CString AacInputModule::getFilterString()
{
   CString cszFilter;
   cszFilter.LoadString(IDS_FILTER_AAC_INPUT);
   return cszFilter;
}

int AacInputModule::initInput(LPCTSTR infilename, SettingsManager &mgr,
   TrackInfo &trackinfo, SampleContainer &samplecont)
{
   USES_CONVERSION;

   // open infile
   istr.open(T2CA(wlGetAnsiCompatFilename(infilename)),std::ios::in|std::ios::binary);
   if (!istr.is_open())
   {
      lasterror.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   // find out AAC file infos
   {
      unsigned long *seek_table = NULL;
      int seek_table_len = 0;

      get_AAC_format(infilename, &info, &seek_table, &seek_table_len, 1);
      free(seek_table);
   }

   // find out length of aac file
   struct _stat statbuf;
   ::_tstat(infilename, &statbuf);
   filelen = statbuf.st_size; // 32 bit max.
   filepos = 0;

   // search for begin of aac stream, skipping id3v2 tags; modifies filepos
   // ...

   // retrieve id3 tag
   // ...

   // seek to begin
   istr.seekg(filepos,std::ios::beg);

   // grab decoder instance
   decoder = NeAACDecOpen();

   // set 32 bit output format
   {
      NeAACDecConfigurationPtr config;
      config = NeAACDecGetCurrentConfiguration(decoder);
      config->outputFormat = FAAD_FMT_16BIT;  // 32 bit sounds bad for some reason
      NeAACDecSetConfiguration(decoder,config);
   }

   // read first frame(s) and get infos about the aac file
   istr.read(reinterpret_cast<char*>(inbuffer),aac_inbufsize);

   unsigned long dummy;
   unsigned char dummy2;
   int result = NeAACDecInit(decoder, inbuffer, sizeof(inbuffer), &dummy, &dummy2);

   if (result<0)
   {
      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_DECODER);
      return -2;
   }

   highmark = 0;

   // seek to the next start
   filepos += result;
   istr.seekg(filepos,std::ios::beg);

   // get right file info (for HE AAC files)
   NeAACDecFrameInfo frameinfo;
   NeAACDecDecode(decoder, &frameinfo, inbuffer, sizeof(inbuffer));
   if (frameinfo.error > 0)
   {
      USES_CONVERSION;
      lasterror = A2CT(NeAACDecGetErrorMessage(frameinfo.error));
      return -frameinfo.error;
   }
   else
   {
      info.sampling_rate = frameinfo.samplerate;
      info.channels = frameinfo.channels;
      info.object_type = frameinfo.object_type - 1;
   }

   // set up input traits
   samplecont.setInputModuleTraits(16,SamplesInterleaved,
      info.sampling_rate,info.channels);

   return 0;
}

void AacInputModule::getInfo(int &channels, int &bitrate, int &length, int &samplerate)
{
   channels = info.channels;
   bitrate = info.bitrate;
   length = (filelen << 3) / info.bitrate;
   samplerate = info.sampling_rate;
}

int AacInputModule::decodeSamples(SampleContainer &samples)
{
   // frame decoding info
   NeAACDecFrameInfo frameinfo;

   // temporary sample buffer
   short *outbuffer;
   short tmpbuf[2048*aac_maxchannels];

   // fill input buffer
   if (highmark<aac_inbufsize)
   {
      istr.read(reinterpret_cast<char*>(inbuffer+highmark),aac_inbufsize-highmark);
      int read = static_cast<int>(istr.gcount());

      if (read==0 && highmark==0)
         return 0;

      highmark += read;
      filepos += read;
   }

   // decode buffer
   short *sample_buffer = (short *)NeAACDecDecode(decoder, &frameinfo, inbuffer, highmark); //sizeof(inbuffer));

   highmark -= frameinfo.bytesconsumed;
   memmove(inbuffer, inbuffer+frameinfo.bytesconsumed,
      aac_inbufsize-frameinfo.bytesconsumed);

   // check for return codes
   if (frameinfo.error>0)
   {
      lasterror = NeAACDecGetErrorMessage(frameinfo.error);
      return -frameinfo.error;
   }

   int num_samples = frameinfo.samples / frameinfo.channels;

   // remap channels
   if (frameinfo.channels > 2)
   {
      outbuffer = tmpbuf;
      for (int i = 0; i < num_samples; i++)
         for (int j = 0; j < std::min(static_cast<int>(frameinfo.channels), MAX_CHANNELS); j++)
            outbuffer[i*frameinfo.channels+j] = sample_buffer[i*frameinfo.channels+(chmap[frameinfo.channels-1][j])];

      // copy the remaining channels
      if (frameinfo.channels > MAX_CHANNELS)
      {
         for (int i = 0; i < num_samples; i++)
            for (int j = MAX_CHANNELS; j < frameinfo.channels; j++)
               outbuffer[i*frameinfo.channels+j] = sample_buffer[i*frameinfo.channels+j];
      }
   }
   else
      outbuffer = sample_buffer;

   // copy the samples to the sample container
   samples.putSamplesInterleaved(outbuffer, num_samples);

   return num_samples;
}

void AacInputModule::doneInput()
{
   // close decoder and file
   NeAACDecClose(decoder);
   istr.close();
}
