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

   $Id: wlWaveOutputModule.cpp,v 1.29 2011/01/23 17:02:43 vividos Exp $

*/
/*! \file wlWaveOutputModule.cpp

   \brief contains the implementation of the wave output module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "wlWaveOutputModule.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern "C"
SNDFILE*
sf_wchar_open (LPCWSTR wpath, int mode, SF_INFO *sfinfo);


/// output sample formats for libsndfile
extern
const int wlSndFileOutputFormat[] = {
   SF_FORMAT_PCM_16,
   SF_FORMAT_PCM_24,
   SF_FORMAT_PCM_32,
   SF_FORMAT_FLOAT
};

/// output file formats for libsndfile
enum {
   FILE_WAV = 0,
   FILE_AIFF,
   FILE_W64
};

// wlWaveOutputModule methods

wlWaveOutputModule::wlWaveOutputModule()
{
   module_id = ID_OM_WAVE;
   sndfile = NULL;
}

bool wlWaveOutputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("libsndfile-1.dll"));
   bool avail = dll != NULL;

   // check for old libsndfile.dll (pre-1.x)
   if (::GetProcAddress(dll,"sf_get_lib_version")!=NULL)
      avail=false;

   ::FreeLibrary(dll);

   return avail;
}

void wlWaveOutputModule::getDescription(CString& desc)
{
   SF_FORMAT_INFO formatInfo;
   ZeroMemory(&formatInfo, sizeof(formatInfo));
   formatInfo.format = sfinfo.format;

   sf_command(NULL, SFC_GET_FORMAT_INFO, &formatInfo, sizeof(formatInfo));

   // format string
   desc.Format(IDS_FORMAT_INFO_WAVE_OUTPUT,
      formatInfo.name, sfinfo.samplerate, sfinfo.channels);
}

LPCTSTR wlWaveOutputModule::getOutputExtension()
{
   LPCTSTR ext;

   switch (filefmt)
   {
   case FILE_WAV:
      ext = _T("wav");
      break;
   case FILE_AIFF:
      ext = _T("aiff");
      break;
   case FILE_W64:
      ext = _T("w64");
      break;
   }

   if (rawfile) ext = _T("raw");

   return ext;
}

void wlWaveOutputModule::prepareOutput(wlSettingsManager &mgr)
{
   rawfile = mgr.queryValueInt(wlWaveRawAudioFile)!=0;
   wavex = mgr.queryValueInt(wlWaveWriteWavEx)!=0;
   outfmt = mgr.queryValueInt(wlWaveOutputFormat);
   filefmt = mgr.queryValueInt(wlWaveFileFormat);
}

int wlWaveOutputModule::initOutput(LPCTSTR outfilename,
   wlSettingsManager &mgr, const wlTrackInfo& trackinfo,
   wlSampleContainer &samplecont)
{
   // setup sndfile info structure
   memset(&sfinfo,0,sizeof(sfinfo));

   sfinfo.samplerate = samplecont.getInputModuleSampleRate();
   sfinfo.channels = samplecont.getInputModuleChannels();

   // set file format
   if (rawfile)
   {
      sfinfo.format = SF_FORMAT_RAW | SF_ENDIAN_FILE;
   }
   else if (filefmt == FILE_WAV)
   {
      sfinfo.format = wavex ? SF_FORMAT_WAVEX : SF_FORMAT_WAV;
   }
   else if (filefmt == FILE_AIFF)
   {
      sfinfo.format = SF_FORMAT_AIFF;
   }
   else if (filefmt == FILE_W64)
   {
      sfinfo.format = SF_FORMAT_W64;
   }

   // set sample format
   sfinfo.format |= wlSndFileOutputFormat[outfmt];

   // opens the file for writing
#ifdef UNICODE
   sndfile = sf_wchar_open(outfilename,SFM_WRITE,&sfinfo);
#else
   USES_CONVERSION;
   sndfile = sf_open(T2CA(outfilename),SFM_WRITE,&sfinfo);
#endif

   if (sndfile==NULL)
   {
      char buffer[512];
      sf_error_str(sndfile,buffer,512);

      USES_CONVERSION;
      lasterror.LoadString(IDS_ENCODER_OUTPUT_FILE_CREATE_ERROR);
      lasterror += _T(" (");
      lasterror += A2CT(buffer);
      lasterror += _T(")");
      return -1;
   }

   int outbits;
   switch (wlSndFileOutputFormat[outfmt])
   {
   case SF_FORMAT_PCM_24:
   case SF_FORMAT_PCM_32:
   case SF_FORMAT_FLOAT:
   case SF_FORMAT_DOUBLE:
      outbits = 32;
      break;
   case SF_FORMAT_PCM_16:
   default:
      outbits = 16;
      break;
   }

   // set up output traits
   samplecont.setOutputModuleTraits(outbits, wlSamplesInterleaved);

   return 0;
}

int wlWaveOutputModule::encodeSamples(wlSampleContainer &samples)
{
   sf_count_t ret;

   // get samples
   int numsamples=0;
   void *sbuf = samples.getSamplesInterleaved(numsamples);

   // write samples
   if (wlSndFileOutputFormat[outfmt] == SF_FORMAT_PCM_16)
   {
     ret = sf_write_short(sndfile,(short *)sbuf,numsamples * sfinfo.channels);
   }
   else if (wlSndFileOutputFormat[outfmt] == SF_FORMAT_PCM_24 ||
           wlSndFileOutputFormat[outfmt] == SF_FORMAT_PCM_32)
   {
     ret = sf_write_int(sndfile,(int *)sbuf,numsamples * sfinfo.channels);
   }
   else if (wlSndFileOutputFormat[outfmt] == SF_FORMAT_FLOAT)
   {
     int *samplebuf = (int *)sbuf;
     float *floatbuf = new float[numsamples * sfinfo.channels];
     for (int i = 0; i < numsamples * sfinfo.channels; i++)
        floatbuf[i] = float(samplebuf[i]) / (1<<31);
     ret = sf_write_float(sndfile,floatbuf,numsamples * sfinfo.channels);
     delete[] floatbuf;
   }
   else
     ret = -1;

   if (ret<0)
   {
      char buffer[512];
      sf_error_str(sndfile,buffer,512);

      USES_CONVERSION;
      lasterror = A2CT(buffer);
   }

   return int(ret);
}

void wlWaveOutputModule::doneOutput()
{
   // closes the file
   sf_close(sndfile);
}
