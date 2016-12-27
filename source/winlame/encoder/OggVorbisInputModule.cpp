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
/// \file OggVorbisInputModule.cpp
/// \brief contains the implementation of the ogg vorbis input module
//
#include "stdafx.h"
#include "OggVorbisInputModule.hpp"
#include "resource.h"
#include <fstream>
#include "vorbis/vorbisfile.h"

using Encoder::OggVorbisInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

// constants

// channel remap stuff

const int MAX_CHANNELS = 6; ///< make this higher to support files with more channels

/// channel remapping map
const int chmap[MAX_CHANNELS][MAX_CHANNELS] =
{
   { 0, },               // mono
   { 0, 1, },            // l, r
   { 0, 2, 1, },         // l, c, r -> l, r, c
   { 0, 1, 2, 3, },      // l, r, bl, br
   { 0, 2, 1, 3, 4, },   // l, c, r, bl, br -> l, r, c, bl, br
   { 0, 2, 1, 5, 3, 4 }  // l, c, r, bl, br, lfe -> l, r, c, lfe, bl, br
};

/// ogg vorbis input buffer size
const int c_oggInputBufferSize = 512 * MAX_CHANNELS; // must be a multiple of max channels

static size_t ReadDataSource(void* buffer, size_t size, size_t count, void* dataSource)
{
   return fread(buffer, size, count, reinterpret_cast<FILE*>(dataSource));
}

static int SeekDataSource(void* dataSource, ogg_int64_t offset, int whence)
{
   return _fseeki64(reinterpret_cast<FILE*>(dataSource), offset, whence);
}

static int CloseDataSource(void* dataSource)
{
   return fclose(reinterpret_cast<FILE*>(dataSource));
}

static long FilePosDataSource(void* dataSource)
{
   return ftell(reinterpret_cast<FILE*>(dataSource));
}

/// ogg vorbis reading callbacks
ov_callbacks c_callbacks =
{
   &::ReadDataSource,
   &::SeekDataSource,
   &::CloseDataSource,
   &::FilePosDataSource
};

OggVorbisInputModule::OggVorbisInputModule()
{
   m_moduleId = ID_IM_OGGV;
}

OggVorbisInputModule::~OggVorbisInputModule() throw()
{
}

Encoder::InputModule* OggVorbisInputModule::CloneModule()
{
   return new OggVorbisInputModule;
}

bool OggVorbisInputModule::IsAvailable() const
{
   // we don't do delay-loading anymore, so it's available always
   return true;
}

void OggVorbisInputModule::GetDescription(CString& desc) const
{
   const vorbis_info* vi = ov_info(const_cast<OggVorbis_File*>(&m_vf), -1);

   if (vi->bitrate_upper != -1 && vi->bitrate_lower != -1)
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_VBR,
         vi->bitrate_lower / 1000,
         vi->bitrate_upper / 1000,
         m_channels,
         m_samplerate);
   }
   else if (vi->bitrate_nominal == -1)
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_FREE,
         m_channels,
         m_samplerate);
   }
   else
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_NOMINAL,
         vi->bitrate_nominal / 1000,
         m_channels,
         m_samplerate);
   }
}

CString OggVorbisInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_OGG_VORBIS_INPUT);
   return filterString;
}

int OggVorbisInputModule::InitInput(LPCTSTR m_inputFilename,
   SettingsManager& mgr, TrackInfo& trackinfo,
   SampleContainer& samplecont)
{
   IsAvailable();

   m_inputFile = _wfopen(m_inputFilename, _T("rb"));

   if (m_inputFile == NULL)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   // open ogg vorbis file
   if (ov_open_callbacks(m_inputFile, &m_vf, NULL, 0, c_callbacks) < 0)
   {
      m_lastError.Format(IDS_ENCODER_INVALID_FILE_FORMAT);
      return -2;
   }

   // retrieve file infos
   m_numCurrentSamples = 0;
   m_numMaxSamples = ov_pcm_total(&m_vf, -1);

   vorbis_info *vi = ov_info(&m_vf, -1);
   m_channels = vi->channels;
   m_samplerate = vi->rate;

   // set up input traits
   samplecont.SetInputModuleTraits(sizeof(short) * 8, SamplesInterleaved, m_samplerate, m_channels);

   return 0;
}

void OggVorbisInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   vorbis_info *vi = ov_info(const_cast<OggVorbis_File*>(&m_vf), -1);

   numChannels = vi->channels;
   bitrateInBps = vi->bitrate_nominal;
   lengthInSeconds = int(m_numMaxSamples / vi->rate);
   samplerateInHz = vi->rate;
}

int OggVorbisInputModule::DecodeSamples(SampleContainer& samples)
{
   short buffer[c_oggInputBufferSize];
   int bitstream;

   // temporary sample buffer
   short* outputBuffer;
   short tempBuffer[c_oggInputBufferSize];

   // read in samples
   int ret = ov_read(&m_vf,
      reinterpret_cast<char*>(buffer),
      c_oggInputBufferSize * sizeof(short),
      0,
      sizeof(short),
      1,
      &bitstream);

   if (ret < 0)
   {
      m_lastError.LoadString(IDS_ENCODER_INTERNAL_DECODE_ERROR);
      return ret;
   }

   ret /= m_channels * sizeof(short);

   // channel remap
   if (m_channels > 2 && ret > 0)
   {
      outputBuffer = tempBuffer;
      for (int i = 0; i < ret; i++)
         for (int j = 0; j < std::min(m_channels, MAX_CHANNELS); j++)
            outputBuffer[i*m_channels + j] = buffer[i*m_channels + (chmap[m_channels - 1][j])];

      // copy the remaining channels
      if (m_channels > MAX_CHANNELS)
      {
         for (int i = 0; i < ret; i++)
            for (int j = MAX_CHANNELS; j < m_channels; j++)
               outputBuffer[i*m_channels + j] = buffer[i*m_channels + j];
      }
   }
   else
      outputBuffer = buffer;

   if (ret > 0)
      samples.PutSamplesInterleaved(outputBuffer, ret);

   m_numCurrentSamples += ret;

   return ret;
}

void OggVorbisInputModule::DoneInput()
{
   ov_clear(&m_vf);

   fclose(m_inputFile);
}
