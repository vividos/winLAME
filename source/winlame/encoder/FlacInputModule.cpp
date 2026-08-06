//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2026 Michael Fink
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
/// \file FlacInputModule.cpp
/// \brief contains the implementation of the Flac input module
//
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "FlacInputModule.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include "FLAC/metadata.h"
#include "AudioFileTag.hpp"

using Encoder::FlacInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;
using Encoder::FLAC_context;

namespace Encoder
{
   /// flac decoding context
   struct FLAC_context
   {
      FLAC__StreamMetadata_StreamInfo streamInfo;  ///< stream info
      FLAC__int32* reservoir = nullptr;            ///< reservoir
      unsigned int numSamplesInReservoir = 0;      ///< number of samples in reservoir
      unsigned int totalLengthInMs = 0;            ///< total length in ms
      bool abortFlag = false;                      ///< abort flag

      /// ctor
      FLAC_context()
      {
         memset(&streamInfo, 0, sizeof(streamInfo));
      }
   };
}

// constants

/// frame size; default = 4608
const unsigned int m_flacFrameSize = 576;

// callbacks

static FLAC__StreamDecoderWriteStatus FLAC_WriteCallback(
   const FLAC__StreamDecoder* decoder,
   const FLAC__Frame* frame,
   const FLAC__int32* const buffer[],
   void* clientData)
{
   FLAC_context* context = (FLAC_context*)clientData;

   const unsigned numChannels = context->streamInfo.channels;
   const unsigned numWideSamples = frame->header.blocksize;
   unsigned wide_sample, sample, channel;

   if (context->abortFlag)
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

   for (sample = context->numSamplesInReservoir * numChannels, wide_sample = 0;
      wide_sample < numWideSamples;
      wide_sample++)
   {
      for (channel = 0; channel < numChannels; channel++, sample++)
      {
         context->reservoir[sample] = buffer[channel][wide_sample];
      }
   }

   context->numSamplesInReservoir += numWideSamples;

   return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void FLAC_MetadataCallback(const FLAC__StreamDecoder* decoder,
   const FLAC__StreamMetadata* metadata,
   void* clientData)
{
   FLAC_context* context = (FLAC_context*)clientData;

   switch (metadata->type)
   {
   case FLAC__METADATA_TYPE_STREAMINFO:
      context->streamInfo = metadata->data.stream_info;
      break;

   default:
      // ignored metadata
      break;
   }
}

static void FLAC_ErrorCallback(const FLAC__StreamDecoder* decoder,
   FLAC__StreamDecoderErrorStatus status,
   void* clientData)
{
   FLAC_context* context = (FLAC_context*)clientData;

   if (status != FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
      context->abortFlag = true;
}

/// pack function to store sample data in byte array
unsigned FLAC__pack_pcm_signed_little_endian(FLAC__byte* data, FLAC__int32* input,
   unsigned wide_samples, unsigned numChannels, unsigned sourceBitsPerSample)
{
   FLAC__byte* const start = data;
   unsigned samples = wide_samples * numChannels;
   const unsigned bytesPerSample = sourceBitsPerSample / 8;

   while (samples--)
   {
      FLAC__int32 sample = *input++;

      switch (sourceBitsPerSample)
      {
      case 8:
         data[0] = static_cast<FLAC__byte>(sample ^ 0x80);
         break;
      case 24:
         data[2] = (FLAC__byte)(sample >> 16);
         data[1] = (FLAC__byte)(sample >> 8);
         data[0] = (FLAC__byte)sample;
         break;
      case 16:
         data[1] = (FLAC__byte)(sample >> 8);
         data[0] = (FLAC__byte)sample;
         break;
      default:
         ATLASSERT(false); // invalid bits per sample
         break;
      }

      data += bytesPerSample;
   }

   return data - start;
}


FlacInputModule::FlacInputModule()
   :m_fileLength(0),
   m_flacDecoder(nullptr),
   m_flacContext(nullptr),
   m_samplePosition(0),
   m_pcmBufferLength(0)
{
   m_moduleId = ID_IM_FLAC;
}

Encoder::InputModule* FlacInputModule::CloneModule()
{
   return new FlacInputModule;
}

bool FlacInputModule::IsAvailable() const
{
   // we don't do delay-loading anymore, so it's always available
   return true;
}

CString FlacInputModule::GetDescription() const
{
   if (m_flacContext == nullptr)
      return CString();

   CString desc;
   desc.Format(IDS_FORMAT_INFO_FLAC_INPUT,
      static_cast<unsigned int>((m_fileLength << 3 / m_flacContext->totalLengthInMs) / 1000),
      m_flacContext->streamInfo.sample_rate,
      m_flacContext->streamInfo.channels,
      m_flacContext->streamInfo.bits_per_sample);

   return desc;
}

void FlacInputModule::GetVersionString(CString& version, int special) const
{
   version = FLAC__VERSION_STRING;
}

CString FlacInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_FLAC_INPUT);
   return filterString;
}

int FlacInputModule::InitInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackinfo, SampleContainer& samplecont)
{
   AudioFileTag tag{ trackinfo };
   tag.ReadFromFile(infilename);

   // find out length of file
   struct _stat64 statbuf;
   ::_tstat64(infilename, &statbuf);
   m_fileLength = statbuf.st_size;

   m_flacContext = new FLAC_context;
   memset((void*)m_flacContext, 0, sizeof(FLAC_context));
   //m_flacContext->trackInfo = &trackinfo;

   m_flacDecoder = FLAC__stream_decoder_new();

   // open stream
   CStringA ansiFilename(GetAnsiCompatFilename(infilename));
   FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_file(m_flacDecoder,
      ansiFilename,
      FLAC_WriteCallback,
      FLAC_MetadataCallback,
      FLAC_ErrorCallback,
      m_flacContext);

   if (!m_flacDecoder || initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
   {
      m_lastError.LoadString(IDS_ENCODER_ERROR_INIT_DECODER);
      return -1;
   }

   if (!FLAC__stream_decoder_process_until_end_of_metadata(m_flacDecoder))
   {
      m_lastError.LoadString(IDS_ENCODER_ERROR_GET_FILE_INFOS);
      return -1;
   }

   m_samplePosition = 0;
   m_flacContext->totalLengthInMs =
      static_cast<unsigned int>(m_flacContext->streamInfo.total_samples * 1000 / m_flacContext->streamInfo.sample_rate);
   m_pcmBufferLength = (m_flacFrameSize * m_flacContext->streamInfo.channels * m_flacContext->streamInfo.bits_per_sample);
   m_flacContext->reservoir = new FLAC__int32[m_flacContext->streamInfo.max_blocksize * m_flacContext->streamInfo.channels * 2];

   m_inputBuffer.resize(m_pcmBufferLength);

   // set up input traits
   samplecont.SetInputModuleTraits(m_flacContext->streamInfo.bits_per_sample, SamplesChannelArray,
      m_flacContext->streamInfo.sample_rate, m_flacContext->streamInfo.channels);

   return 0;
}

void FlacInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   numChannels = m_flacContext->streamInfo.channels;
   bitrateInBps = static_cast<int>(m_fileLength << 3 / m_flacContext->totalLengthInMs);
   lengthInSeconds = m_flacContext->totalLengthInMs / 1000;
   samplerateInHz = m_flacContext->streamInfo.sample_rate;
}

int FlacInputModule::DecodeSamples(SampleContainer& samples)
{
   while (m_flacContext->numSamplesInReservoir < m_flacFrameSize)
   {
      if (FLAC__stream_decoder_get_state(m_flacDecoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
      {
         return 0;
      }
      else if (!FLAC__stream_decoder_process_single(m_flacDecoder))
      {
         return 0;
      }
   }

   unsigned int numSamples = std::min(m_flacContext->numSamplesInReservoir, m_flacFrameSize);

   FLAC__pack_pcm_signed_little_endian(
      (unsigned char*)m_inputBuffer.data(),
      m_flacContext->reservoir,
      numSamples,
      m_flacContext->streamInfo.channels,
      m_flacContext->streamInfo.bits_per_sample);

   unsigned int delta = numSamples * m_flacContext->streamInfo.channels;
   for (unsigned int i = delta; i < m_flacContext->numSamplesInReservoir * m_flacContext->streamInfo.channels; i++)
      m_flacContext->reservoir[i - delta] = m_flacContext->reservoir[i];

   m_flacContext->numSamplesInReservoir -= numSamples;
   m_samplePosition += numSamples;

   // copy the samples to the sample container
   samples.PutSamplesInterleaved(m_inputBuffer.data(), numSamples);

   return numSamples;
}

float FlacInputModule::PercentDone() const
{
   return float(__int64(m_samplePosition))*100.f / __int64(m_flacContext->streamInfo.total_samples);
}

void FlacInputModule::DoneInput()
{
   if (m_flacDecoder)
   {
      FLAC__stream_decoder_finish(m_flacDecoder);
      FLAC__stream_decoder_delete(m_flacDecoder);
   }

   m_flacDecoder = nullptr;

   if (m_flacContext)
   {
      if (m_flacContext->reservoir)
         delete[] m_flacContext->reservoir;

      m_flacContext->reservoir = nullptr;

      delete m_flacContext;
      m_flacContext = nullptr;
   }
}
