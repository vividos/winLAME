//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2021 Michael Fink
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
#include <ulib/DynamicLibrary.hpp>
#include "AudioFileTag.hpp"

using Encoder::FlacInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;
using Encoder::FLAC_context;

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
   const unsigned wide_samples = frame->header.blocksize;
   unsigned wide_sample, sample, channel;

   if (context->abortFlag)
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

   for (sample = context->numSamplesInReservoir * numChannels, wide_sample = 0;
      wide_sample < wide_samples;
      wide_sample++)
   {
      for (channel = 0; channel < numChannels; channel++, sample++)
      {
         context->reservoir[sample] = buffer[channel][wide_sample];
      }
   }

   context->numSamplesInReservoir += wide_samples;

   return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void FLAC_vorbis_comment_split_name_value(
   const FLAC__StreamMetadata_VorbisComment_Entry entry,
   CString& name, CString& value)
{
   CString temp(reinterpret_cast<char*>(entry.entry), entry.length);
   int pos = temp.Find(_T('='));
   name = temp.Left(pos);
   value = temp.Mid(pos + 1);
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

   case FLAC__METADATA_TYPE_VORBIS_COMMENT:
      if (context->trackInfo == nullptr)
         break;

      for (unsigned i = 0; i < metadata->data.vorbis_comment.num_comments; i++)
      {
         CString name, value;

         FLAC_vorbis_comment_split_name_value(
            metadata->data.vorbis_comment.comments[i],
            name, value);

         if (name.CompareNoCase(_T("title")) == 0)
         {
            context->trackInfo->SetTextInfo(Encoder::TrackInfoTitle, value);
         }
         else if (name.CompareNoCase(_T("artist")) == 0)
         {
            context->trackInfo->SetTextInfo(Encoder::TrackInfoArtist, value);
         }
         else if (name.CompareNoCase(_T("album")) == 0)
         {
            context->trackInfo->SetTextInfo(Encoder::TrackInfoAlbum, value);
         }
         else if (name.CompareNoCase(_T("comment")) == 0)
         {
            context->trackInfo->SetTextInfo(Encoder::TrackInfoComment, value);
         }
         else if (name.CompareNoCase(_T("genre")) == 0)
         {
            if (!value.IsEmpty())
               context->trackInfo->SetTextInfo(Encoder::TrackInfoGenre, value);
         }
         else if (name.CompareNoCase(_T("tracknumber")) == 0)
         {
            context->trackInfo->SetNumberInfo(Encoder::TrackInfoTrack, _ttoi(value));
         }
         else if (name.CompareNoCase(_T("year")) == 0)
         {
            context->trackInfo->SetNumberInfo(Encoder::TrackInfoYear, _ttoi(value));
         }
         else if (name.CompareNoCase(_T("date")) == 0)
         {
            // Sanity check: Many FLACs seem to have 'date' actually containing
            // only 4-digit year. If this is the case, use 'date' for 'year'.
            if (value.GetLength() == 4 &&
               value.SpanIncluding(_T("0123456789")).GetLength() == 4)
            {
               context->trackInfo->SetNumberInfo(Encoder::TrackInfoYear, _ttoi(value));
            }
         }
      }
      break;

   default:
      ATLASSERT(false);
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
   DynamicLibrary lib(_T("libFLAC_dynamic.dll"));

   if (lib.IsLoaded())
   {
      return lib.IsFunctionAvail("FLAC__stream_decoder_new");
   }

   return false;
}

CString FlacInputModule::GetDescription() const
{
   if (m_flacContext == nullptr)
      return CString();

   CString desc;
   desc.Format(IDS_FORMAT_INFO_FLAC_INPUT,
      (m_fileLength << 3 / m_flacContext->totalLengthInMs) / 1000,
      m_flacContext->streamInfo.sample_rate,
      m_flacContext->streamInfo.channels,
      m_flacContext->streamInfo.bits_per_sample);

   return desc;
}

void FlacInputModule::GetVersionString(CString& version, int special) const
{
   DynamicLibrary lib(_T("libFLAC_dynamic.dll"));

   if (lib.IsLoaded())
   {
      version = *(lib.GetFunction<const char**>("FLAC__VERSION_STRING"));
   }
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
   ReadTrackMetadata(infilename, trackinfo);

   // find out length of file
   struct _stat statbuf;
   ::_tstat(infilename, &statbuf);
   m_fileLength = statbuf.st_size; // 32 bit max.

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
   bitrateInBps = m_fileLength << 3 / m_flacContext->totalLengthInMs;
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

void FlacInputModule::ReadTrackMetadata(LPCTSTR filename, TrackInfo& trackInfo)
{
   AudioFileTag tag{ trackInfo };
   tag.ReadFromFile(filename);

   // since AudioFileTag (via TagLib library) can't currently read the PICTURE
   // metadata block, read it using FLAC functions
   CStringA ansiFilename{ GetAnsiCompatFilename(filename) };

   // first, try to get front conver
   FLAC__StreamMetadata* picture = nullptr;
   FLAC__bool ret = FLAC__metadata_get_picture(
      ansiFilename, &picture,
      FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER,
      nullptr, nullptr,
      unsigned(-1),
      unsigned(-1),
      unsigned(-1),
      unsigned(-1));

   if (!ret && picture == nullptr)
   {
      // try to get any picture
      ret = FLAC__metadata_get_picture(
         ansiFilename, &picture,
         (FLAC__StreamMetadata_Picture_Type)-1,
         nullptr, nullptr,
         unsigned(-1),
         unsigned(-1),
         unsigned(-1),
         unsigned(-1));
   }

   if (ret && picture != nullptr)
   {
      const std::vector<unsigned char> binaryData(
         picture->data.picture.data,
         picture->data.picture.data + picture->data.picture.data_length);

      trackInfo.SetBinaryInfo(TrackInfoFrontCover, binaryData);

      FLAC__metadata_object_delete(picture);
   }
}
