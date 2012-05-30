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

   $Id: AacOutputModule.cpp,v 1.27 2010/01/08 19:58:41 vividos Exp $

*/
/*! \file AacOutputModule.cpp

   \brief contains the implementation of the AAC output module

*/

// needed includes
#include "stdafx.h"
#include <fstream>
#include "resource.h"
#include "AacOutputModule.h"
#include "neaacdec.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// linker options

#if _MSC_VER < 1400
#pragma comment(linker, "/delayload:libfaac.dll")
#endif

// channel remap stuff

const int MAX_CHANNELS = 6; ///< make this higher to support files with more channels

/// channel remapping map
const int chmap[MAX_CHANNELS][MAX_CHANNELS] = {
   { 0, },               // mono
   { 0, 1, },            // l, r
   { 2, 0, 1, },         // l, r, c -> c, l, r
   { 2, 0, 1, 3, },      // l, r, c, bc -> c, l, r, bc
   { 2, 0, 1, 3, 4, },   // l, r, c, bl, br -> c, l, r, bl, br
   { 2, 0, 1, 4, 5, 3 }  // l, r, c, lfe, bl, br -> c, l, r, bl, br, lfe
};

// AacOutputModule methods

AacOutputModule::AacOutputModule()
{
   module_id = ID_OM_AAC;
}

bool AacOutputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("libfaac.dll"));
   bool avail = dll != NULL;
   if (avail) ::FreeLibrary(dll);

   return avail;
}

void AacOutputModule::getDescription(CString& desc)
{
   // get config
   faacEncConfigurationPtr config;
   config = faacEncGetCurrentConfiguration(handle);

   // format string
   desc.Format(IDS_FORMAT_INFO_AAC_OUTPUT,
      config->mpegVersion == MPEG4 ? 4 : 2,
      (brcmethod == 0) ? _T("Quality ") : _T(""),
      (brcmethod == 0) ? config->quantqual : config->bitRate/1000,
      (brcmethod == 0) ? _T("") : _T(" kbps/channel"),
      channels, config->bandWidth,
      config->allowMidside == 1 ? _T(", Mid/Side") : _T(""),
      config->useTns == 1 ? _T(", Temporal Noise Shaping") : _T(""),
      config->useLfe == 1 ? _T(", LFE channel") : _T(""));
}

int AacOutputModule::initOutput(LPCTSTR outfilename,
   SettingsManager &mgr, const TrackInfo& trackinfo,
   SampleContainer &samplecont)
{
   USES_CONVERSION;

   // open output file
   ostr.open(T2CA(outfilename),std::ios::out|std::ios::binary);
   if (!ostr.is_open())
   {
      lasterror.LoadString(IDS_ENCODER_OUTPUT_FILE_CREATE_ERROR);
      return -1;
   }

   samplerate = samplecont.getInputModuleSampleRate();
   channels = samplecont.getInputModuleChannels();

   // try to get a handle
   handle = faacEncOpen(samplerate,channels,&aac_inbufsize,&aac_outbufsize);

   if (handle==NULL)
   {
      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);
      return -1;
   }

   // alloc memory for input buffer
   sbuffer = new short[aac_inbufsize];
   sbufhigh = 0;

   // alloc memory for output buffer
   aac_outbuf = new unsigned char[aac_outbufsize];

   // get current config
   faacEncConfigurationPtr config;
   config = faacEncGetCurrentConfiguration(handle);

   // set settings
   config->mpegVersion = mgr.queryValueInt(AacMpegVersion)==4 ? MPEG4 : MPEG2;

   int value = mgr.queryValueInt(AacObjectType);
   config->aacObjectType = value==1 ? LOW : value==2 ? LTP : MAIN;

   config->allowMidside = mgr.queryValueInt(AacAllowMS);
   config->useLfe = mgr.queryValueInt(AacUseLFEChan);
   config->useTns = mgr.queryValueInt(AacUseTNS);
   config->shortctl = SHORTCTL_NORMAL;
   config->inputFormat = FAAC_INPUT_16BIT;

   // set bandwidth
   if (mgr.queryValueInt(AacAutoBandwidth))
   {
      config->bandWidth = 0;
   }
   else
   {
      config->bandWidth = mgr.queryValueInt(AacBandwidth);
   }

   // set bitrate/quality
   brcmethod = mgr.queryValueInt(AacBRCMethod);
   if (brcmethod == 0) // Quality
   {
      config->quantqual = mgr.queryValueInt(AacQuality);
      config->bitRate = 0;
   }
   else // Bitrate
   {
      config->quantqual = 0;
      config->bitRate = mgr.queryValueInt(AacBitrate) * 1000 / channels;
   }

   // channel remap
   for (int i = 0; i < std::min(channels, MAX_CHANNELS); i++)
      config->channel_map[i] = chmap[channels-1][i];

   // set new config
   faacEncSetConfiguration(handle,config);

   // set up output traits
   samplecont.setOutputModuleTraits(16,SamplesInterleaved);

   return 0;
}

int AacOutputModule::encodeSamples(SampleContainer &samples)
{
   // get samples
   int numsamples=0;
   short *sbuf = (short*)samples.getSamplesInterleaved(numsamples);

   // as faacEncEncode() always wants 'aac_inbufsize' number of samples, we
   // have to store samples until a whole block of samples can be passed to
   // the function; otherwise faacEncEncode() would pad the buffer with 0's.

   int size = numsamples*channels;

   // check if input sample buffer is full
   if (unsigned(sbufhigh+size) >= aac_inbufsize)
   {
      int pos=0;
      while(unsigned(sbufhigh+size)>=aac_inbufsize)
      {
         memcpy(sbuffer+sbufhigh,sbuf+pos,(aac_inbufsize-sbufhigh)*sizeof(short));

         // encode the samples
         int ret = faacEncEncode(handle, reinterpret_cast<int*>(sbuffer), aac_inbufsize, aac_outbuf, aac_outbufsize);

         if (ret<0) return ret;

         // write the output buffer
         if (ret>0)
            ostr.write(reinterpret_cast<char*>(aac_outbuf),ret);

         // copy / adjust buffer values
         int rest = size - (aac_inbufsize - sbufhigh);

         if (rest>0)
            memcpy(sbuffer,sbuf+(aac_inbufsize - sbufhigh),rest*sizeof(short));

         // adjust values
         sbufhigh=rest;
         size-=size;
         pos+=size;
      }

      return numsamples;
   }
   else
   {
      // not enough samples yet, store them in the buffer, too
      memcpy(sbuffer+sbufhigh,sbuf,size*sizeof(short));
      sbufhigh += size;

      return 0;
   }
}

void AacOutputModule::doneOutput()
{
   int ret=0;

   // encode the last samples in sample buffer
   if (sbufhigh>0)
   {
      ret = faacEncEncode(handle, reinterpret_cast<int*>(sbuffer), sbufhigh, aac_outbuf, aac_outbufsize);

      if (ret>0)
         ostr.write(reinterpret_cast<char*>(aac_outbuf),ret);
   }

   // finish encoding and write the last aac frames
   while((ret = faacEncEncode(handle, NULL, 0, aac_outbuf, aac_outbufsize)) > 0)
      ostr.write(reinterpret_cast<char*>(aac_outbuf),ret);

   // close file
   ostr.close();

   // dealloc buffers
   delete[] aac_outbuf;
   aac_outbuf = NULL;

   delete[] sbuffer;

   // close handle
   faacEncClose(handle);
}
