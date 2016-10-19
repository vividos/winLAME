/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2004 DeXT
   Copyright (c) 2009 Michael Fink

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
/// \file BassWmaOutputModule.cpp
/// \brief contains the implementation of the basswma output module

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "BassWmaOutputModule.h"

// linker options
// already in BassInputModule.cpp
//#if _MSC_VER < 1400
//#pragma comment(linker, "/delayload:bass.dll")
//#pragma comment(linker, "/delayload:basswma.dll")
//#endif

#ifdef _DEBUG
static int dprintf(TCHAR* fmt, ...)
{
   TCHAR printString[1024];
   va_list argp;
   va_start(argp, fmt);
   _vsntprintf(printString, 1024, fmt, argp);
   va_end(argp);
   OutputDebugString(printString);
   return _tcslen(printString);
}
#else
#define dprintf
#endif

// BassWmaOutputModule methods

BassWmaOutputModule::BassWmaOutputModule()
{
   module_id = ID_OM_BASSWMA;
}

bool BassWmaOutputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("bass.dll"));
   bool avail = dll != NULL;
   if (avail) ::FreeLibrary(dll);

   HMODULE wmadll = ::LoadLibrary(_T("basswma.dll"));
   bool basswma = wmadll != NULL;
   if (basswma) ::FreeLibrary(wmadll);

   return (avail && basswma);
}

void BassWmaOutputModule::getDescription(CString& desc)
{
   // format string
   desc.Format(IDS_FORMAT_INFO_BASS_WMA_OUTPUT,
      brmode ? _T("Quality ") : _T(""),
      brmode ? bitrate : bitrate/1000,
      brmode ? _T("%") : _T(" kbps"),
      brmode ? _T("VBR") : _T("CBR"),
      samplerate,
      channels);
}

void BassWmaOutputModule::prepareOutput(SettingsManager &mgr)
{
   brmode = mgr.queryValueInt(WmaBitrateMode);
   switch (brmode)
   {
   case 1: // VBR
      bitrate = mgr.queryValueInt(WmaQuality);
      break;
   case 0: // CBR
   default:
      bitrate = mgr.queryValueInt(WmaBitrate) * 1000;
      break;
   }
}

int BassWmaOutputModule::initOutput(LPCTSTR outfilename,
   SettingsManager &mgr, const TrackInfo& trackinfo,
   SampleContainer &samplecont)
{
   samplerate = samplecont.getInputModuleSampleRate();
   channels = samplecont.getInputModuleChannels();

   USES_CONVERSION;

   /* check that BASS 2.0 was loaded */
   if ((HIWORD(BASS_GetVersion()) != BASSVERSION))
   {
      lasterror.LoadString(IDS_BASS_DLL_ERROR_VERSION_MISMATCH);
      return -1;
   }

   /* not playing anything, so don't need an update thread */
   BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD,0);

   /* setup output - "no sound" device, 44100hz, stereo, 16 bits */
   if (!BASS_Init(0,samplerate,0,0,NULL))
   {
      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);
      return -1;
   }

   // find nearest valid bitrate
   BOOL valid_rate = FALSE;
   DWORD nearest_rate = 0;
   const DWORD* avail_rates=BASS_WMA_EncodeGetRates(samplerate,channels,brmode == 1 ? BASS_WMA_ENCODE_RATES_VBR : 0);
   while (avail_rates && *avail_rates) {
      if (static_cast<DWORD>(bitrate) == *avail_rates) {
         valid_rate = TRUE;
         break;
      }
      if (static_cast<DWORD>(bitrate) < *avail_rates || nearest_rate == 0) nearest_rate = *avail_rates;
      avail_rates++;
   }
   if (!valid_rate) bitrate = nearest_rate;

   // opens the file for writing
   handle=BASS_WMA_EncodeOpenFile(samplerate, channels, 0, bitrate, T2CA(outfilename));
   if(handle == NULL)
   {
      CString cszError;
      int bass_error = BASS_ErrorGetCode();
      switch(bass_error)
      {
      case BASS_ERROR_ILLPARAM:
         cszError = _T("BASS_ERROR_ILLPARAM");
         break;
      case BASS_ERROR_NOTAVAIL:
         cszError = _T("BASS_ERROR_NOTAVAIL");
         break;
      case BASS_ERROR_CREATE:
         cszError = _T("BASS_ERROR_CREATE");
         break;
      case BASS_ERROR_UNKNOWN:
         cszError = _T("BASS_ERROR_UNKNOWN");
         break;
      default:
         cszError.Format(_T("BASS_ErrorCode: %u"), bass_error);
      }

      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);
      lasterror += _T(" (");
      lasterror += cszError;
      lasterror += _T(")");
      return -1;
   }

   // add track properties
   {
      CString prop;
      bool bAvail;

      prop = trackinfo.TextInfo(TrackInfoTitle, bAvail);
      if (bAvail && !prop.IsEmpty())
      {
         BASS_WMA_EncodeSetTag(handle, "Title", T2CA(prop), BASS_WMA_TAG_ANSI);
      }

      prop = trackinfo.TextInfo(TrackInfoArtist, bAvail);
      if (bAvail && !prop.IsEmpty())
      {
         BASS_WMA_EncodeSetTag(handle, "Author", T2CA(prop), BASS_WMA_TAG_ANSI);
      }

      prop = trackinfo.TextInfo(TrackInfoAlbum, bAvail);
      if (bAvail && !prop.IsEmpty())
      {
         BASS_WMA_EncodeSetTag(handle, "WM/AlbumTitle", T2CA(prop), BASS_WMA_TAG_ANSI);
      }

      int iProp = trackinfo.NumberInfo(TrackInfoYear, bAvail);
      if (bAvail && iProp != -1)
      {
         prop.Format(_T("%u"), iProp);
         BASS_WMA_EncodeSetTag(handle, "WM/Year", T2CA(prop), BASS_WMA_TAG_ANSI);
      }

      prop = trackinfo.TextInfo(TrackInfoComment, bAvail);
      if (bAvail && !prop.IsEmpty())
      {
         BASS_WMA_EncodeSetTag(handle, "Description", T2CA(prop), BASS_WMA_TAG_ANSI);
      }

      iProp = trackinfo.NumberInfo(TrackInfoTrack, bAvail);
      if (bAvail && iProp != -1)
      {
         prop.Format(_T("%u"), iProp);
         BASS_WMA_EncodeSetTag(handle, "WM/TrackNumber", T2CA(prop), BASS_WMA_TAG_ANSI);
      }

      prop = trackinfo.TextInfo(TrackInfoGenre, bAvail);
      if (bAvail && !prop.IsEmpty())
      {
         BASS_WMA_EncodeSetTag(handle, "WM/Genre", T2CA(prop), BASS_WMA_TAG_ANSI);
      }

      BASS_WMA_EncodeSetTag(handle,"WM/ToolName","winLAME", BASS_WMA_TAG_ANSI);
      BASS_WMA_EncodeSetTag(handle,0,0, BASS_WMA_TAG_ANSI);
   }

   // set up output traits
   samplecont.setOutputModuleTraits(16, SamplesInterleaved);

   return 0;
}

int BassWmaOutputModule::encodeSamples(SampleContainer &samples)
{
   // get samples
   int numsamples=0;
   void *sbuf = samples.getSamplesInterleaved(numsamples);

   // write samples
   int ret=numsamples*sizeof(short)*channels; // bytes
   if (!BASS_WMA_EncodeWrite(handle,sbuf,ret))
   {
      lasterror.Format(IDS_ENCODER_ERROR_WRITE_OUTPUT);
      return -1;
   }

   return ret;
}

void BassWmaOutputModule::doneOutput()
{
   // closes the file
   BASS_WMA_EncodeClose(handle);
   BASS_Free();
}
