/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2014 Michael Fink
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
/// \file WaveOutputModule.cpp
/// \brief contains the implementation of the wave output module

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "WaveOutputModule.h"
#include "SndFileFormats.hpp"

/// output sample formats for libsndfile
extern
const int SndFileOutputFormat[] = {
   SF_FORMAT_PCM_16,
   SF_FORMAT_PCM_24,
   SF_FORMAT_PCM_32,
   SF_FORMAT_FLOAT
};


// WaveOutputModule methods

WaveOutputModule::WaveOutputModule()
{
   module_id = ID_OM_WAVE;
   sndfile = NULL;
}

bool WaveOutputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("libsndfile-1.dll"));
   bool avail = dll != NULL;

   // check for old libsndfile.dll (pre-1.x)
   if (::GetProcAddress(dll,"sf_get_lib_version")!=NULL)
      avail=false;

   ::FreeLibrary(dll);

   return avail;
}

void WaveOutputModule::getDescription(CString& desc)
{
   CString formatName, outputExtension;
   SndFileFormats::GetFormatInfo(m_format, formatName, outputExtension);

   // format string
   desc.Format(IDS_FORMAT_INFO_WAVE_OUTPUT,
      formatName.GetString(), sfinfo.samplerate, sfinfo.channels);
}

LPCTSTR WaveOutputModule::getOutputExtension()
{
   CString formatName, outputExtension;
   SndFileFormats::GetFormatInfo(m_format, formatName, outputExtension);

   static CString s_outputExtension;
   s_outputExtension = outputExtension;

   return s_outputExtension;
}

void WaveOutputModule::prepareOutput(SettingsManager &mgr)
{
   m_format = mgr.queryValueInt(SndFileFormat);
   m_subType = mgr.queryValueInt(SndFileSubType);
}

int WaveOutputModule::initOutput(LPCTSTR outfilename,
   SettingsManager &mgr, const TrackInfo& trackinfo,
   SampleContainer &samplecont)
{
   // setup sndfile info structure
   memset(&sfinfo,0,sizeof(sfinfo));

   sfinfo.samplerate = samplecont.getInputModuleSampleRate();
   sfinfo.channels = samplecont.getInputModuleChannels();

   sfinfo.format = m_format | m_subType;

   // opens the file for writing
#ifdef UNICODE
   sndfile = sf_wchar_open(outfilename,SFM_WRITE,&sfinfo);
#else
   sndfile = sf_open(CStringA(outfilename),SFM_WRITE,&sfinfo);
#endif

   if (sndfile==NULL)
   {
      char buffer[512];
      sf_error_str(sndfile,buffer,512);

      lasterror.LoadString(IDS_ENCODER_OUTPUT_FILE_CREATE_ERROR);
      lasterror += _T(" (");
      lasterror += CString(buffer);
      lasterror += _T(")");
      return -1;
   }

   int outbits;
   switch (m_format & SF_FORMAT_SUBMASK)
   {
   case SF_FORMAT_PCM_24:
   case SF_FORMAT_PCM_32:
   case SF_FORMAT_FLOAT:
   case SF_FORMAT_DOUBLE:
   case SF_FORMAT_DWVW_24:
   case SF_FORMAT_ALAC_20:
   case SF_FORMAT_ALAC_24:
   case SF_FORMAT_ALAC_32:
      outbits = 32;
      break;
   case SF_FORMAT_PCM_16:
   default:
      outbits = 16;
      break;
   }

   // set up output traits
   samplecont.setOutputModuleTraits(outbits, SamplesInterleaved);

   return 0;
}

int WaveOutputModule::encodeSamples(SampleContainer &samples)
{
   sf_count_t ret;

   // get samples
   int numsamples=0;
   void *sbuf = samples.getSamplesInterleaved(numsamples);

   // write samples
   if (samples.getOutputModuleBitsPerSample() == 16)
   {
     ret = sf_write_short(sndfile,(short *)sbuf,numsamples * sfinfo.channels);
   }
   else if ((m_format & SF_FORMAT_SUBMASK) == SF_FORMAT_FLOAT ||
      (m_format & SF_FORMAT_SUBMASK) == SF_FORMAT_DOUBLE)
   {
     int *samplebuf = (int *)sbuf;
     float *floatbuf = new float[numsamples * sfinfo.channels];
     for (int i = 0; i < numsamples * sfinfo.channels; i++)
        floatbuf[i] = float(samplebuf[i]) / (1<<31);
     ret = sf_write_float(sndfile,floatbuf,numsamples * sfinfo.channels);
     delete[] floatbuf;
   }
   else if (samples.getOutputModuleBitsPerSample() == 32)
   {
      ret = sf_write_int(sndfile, (int *)sbuf, numsamples * sfinfo.channels);
   }
   else
     ret = -1;

   if (ret<0)
   {
      char buffer[512];
      sf_error_str(sndfile,buffer,512);

      lasterror = CString(buffer);
   }

   return int(ret);
}

void WaveOutputModule::doneOutput()
{
   // closes the file
   sf_close(sndfile);
}
