//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
// Copyright (c) 2004 DeXT
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file SndFileOutputModule.cpp
/// \brief contains the implementation of the wave output module
//
#include "stdafx.h"
#include "resource.h"
#include "SndFileOutputModule.hpp"
#include "SndFileFormats.hpp"
#include "DynamicLibrary.hpp"
#include "App.hpp"

using Encoder::SndFileOutputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

SndFileOutputModule::SndFileOutputModule()
   :m_sndfile(nullptr),
   m_format(SF_FORMAT_WAV),
   m_subType(SF_FORMAT_PCM_16)
{
   m_moduleId = ID_OM_WAVE;
   memset(&m_sfinfo, 0, sizeof(m_sfinfo));
}

bool SndFileOutputModule::IsAvailable() const
{
   DynamicLibrary dll(_T("libsndfile-1.dll"));

   bool avail = dll.IsLoaded();

   // check for old libsndfile.dll (pre-1.x); not supported
   if (dll.IsFunctionAvail("sf_get_lib_version"))
      avail = false;

   return avail;
}

CString SndFileOutputModule::GetDescription() const
{
   CString formatName, outputExtension;
   SndFileFormats::GetFormatInfo(m_format, formatName, outputExtension);

   CString desc;
   desc.Format(IDS_FORMAT_INFO_WAVE_OUTPUT,
      formatName.GetString(),
      m_sfinfo.samplerate,
      m_sfinfo.channels);

   return desc;
}

CString SndFileOutputModule::GetOutputExtension() const
{
   CString formatName, outputExtension;
   SndFileFormats::GetFormatInfo(m_format, formatName, outputExtension);

   return outputExtension;
}

void SndFileOutputModule::PrepareOutput(SettingsManager& mgr)
{
   m_format = mgr.QueryValueInt(SndFileFormat);
   m_subType = mgr.QueryValueInt(SndFileSubType);
}

int SndFileOutputModule::InitOutput(LPCTSTR outfilename,
   SettingsManager& mgr, const TrackInfo& trackInfo,
   SampleContainer& samples)
{
   // setup sndfile info structure
   memset(&m_sfinfo, 0, sizeof(m_sfinfo));

   m_sfinfo.samplerate = samples.GetInputModuleSampleRate();
   m_sfinfo.channels = samples.GetInputModuleChannels();

   m_sfinfo.format = m_format | m_subType;

   // opens the file for writing
#ifdef UNICODE
   m_sndfile = sf_wchar_open(outfilename, SFM_WRITE, &m_sfinfo);
#else
   m_sndfile = sf_open(CStringA(outfilename), SFM_WRITE, &m_sfinfo);
#endif

   if (m_sndfile == nullptr)
   {
      char buffer[512];
      sf_error_str(m_sndfile, buffer, 512);

      m_lastError.LoadString(IDS_ENCODER_OUTPUT_FILE_CREATE_ERROR);
      m_lastError.AppendFormat(_T(" (%hs)"), buffer);

      return -1;
   }

   SetTrackInfo(trackInfo);

   int numOutputBits;
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
      numOutputBits = 32;
      break;
   case SF_FORMAT_PCM_16:
   default:
      numOutputBits = 16;
      break;
   }

   // set up output traits
   samples.SetOutputModuleTraits(numOutputBits, SamplesInterleaved);

   return 0;
}

int SndFileOutputModule::EncodeSamples(SampleContainer& samples)
{
   sf_count_t ret;

   // get samples
   int numSamples = 0;
   void* sampleBuffer = samples.GetSamplesInterleaved(numSamples);

   // write samples
   if (samples.GetOutputModuleBitsPerSample() == 16)
   {
      ret = sf_write_short(m_sndfile, (short*)sampleBuffer, numSamples * m_sfinfo.channels);
   }
   else if ((m_format & SF_FORMAT_SUBMASK) == SF_FORMAT_FLOAT ||
      (m_format & SF_FORMAT_SUBMASK) == SF_FORMAT_DOUBLE)
   {
      int* intSampleBuffer = (int*)sampleBuffer;

      std::vector<float> floatBuffer(numSamples * m_sfinfo.channels);

      for (int i = 0; i < numSamples * m_sfinfo.channels; i++)
         floatBuffer[i] = float(intSampleBuffer[i]) / (1 << 31);

      ret = sf_write_float(m_sndfile, floatBuffer.data(), floatBuffer.size());
   }
   else if (samples.GetOutputModuleBitsPerSample() == 32)
   {
      ret = sf_write_int(m_sndfile, (int*)sampleBuffer, numSamples * m_sfinfo.channels);
   }
   else
      ret = -1;

   if (ret < 0)
   {
      char buffer[512];
      sf_error_str(m_sndfile, buffer, 512);

      m_lastError = CString(buffer);
   }

   return int(ret);
}

void SndFileOutputModule::DoneOutput()
{
   sf_close(m_sndfile);
}

void SndFileOutputModule::SetTrackInfo(const TrackInfo& trackInfo)
{
   // Note: all track info values are converted to ANSI; there is no way to
   // know how the values are stored; at least for RIFF Wave INFO header, it
   // seems that UTF-8 as text would be ignored.
   bool avail;
   CStringA text = trackInfo.TextInfo(TrackInfoTitle, avail);
   if (avail && !text.IsEmpty())
   {
      sf_set_string(m_sndfile, SF_STR_TITLE, text.GetString());
   }

   text = trackInfo.TextInfo(TrackInfoArtist, avail);
   if (avail && !text.IsEmpty())
   {
      sf_set_string(m_sndfile, SF_STR_ARTIST, text.GetString());
   }

   text = trackInfo.TextInfo(TrackInfoAlbum, avail);
   if (avail && !text.IsEmpty())
   {
      sf_set_string(m_sndfile, SF_STR_ALBUM, text.GetString());
   }

   int number = trackInfo.NumberInfo(TrackInfoYear, avail);
   if (avail && number > 0)
   {
      text.Format("%i", number);
      sf_set_string(m_sndfile, SF_STR_DATE, text.GetString());
   }

   text = trackInfo.TextInfo(TrackInfoComment, avail);
   if (avail && !text.IsEmpty())
   {
      sf_set_string(m_sndfile, SF_STR_COMMENT, text.GetString());
   }

   number = trackInfo.NumberInfo(TrackInfoTrack, avail);
   if (avail && number > 0)
   {
      text.Format("%i", number);
      sf_set_string(m_sndfile, SF_STR_TRACKNUMBER, text.GetString());
   }

   text = trackInfo.TextInfo(TrackInfoGenre, avail);
   if (avail && !text.IsEmpty())
   {
      sf_set_string(m_sndfile, SF_STR_GENRE, text.GetString());
   }

   text.Format("winLAME %ls", App::Version().GetString());
   sf_set_string(m_sndfile, SF_STR_SOFTWARE, text.GetString());
}
