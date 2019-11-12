//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2016-2018 Michael Fink
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
/// \file OpusOutputModule.cpp
/// \brief Opus output module
//
#include "stdafx.h"
#include "resource.h"
#include "OpusOutputModule.hpp"
#include <ulib/UTF8.hpp>
#include "App.hpp"
#include <wincrypt.h>
#include <ogg/ogg.h>

using Encoder::OpusEncData;

#pragma comment(lib, "opus.lib")
#pragma comment(lib, "opusenc.lib")
#pragma comment(lib, "libogg.lib")

bool GetRandomNumber(int& randomNumber)
{
   HCRYPTPROV provider = 0;
   if (!CryptAcquireContext(&provider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
      return false;

   randomNumber = 0;
   BOOL ret = CryptGenRandom(provider, sizeof(randomNumber), reinterpret_cast<BYTE*>(&randomNumber));

   CryptReleaseContext(provider, 0);

   return ret != FALSE;
}

OpusEncData::OpusEncData()
{
   enc = nullptr;
   total_bytes = 0;
   bytes_written = 0;
   nb_encoded = 0;
   pages_out = 0;
   packets_out = 0;
   peak_bytes = 0;
   min_bytes = 256 * 1275 * 6;
   last_length = 0;
   nb_streams = 1;
   nb_coupled = 0;
}

OpusEncData::~OpusEncData()
{
   Close();
}

bool OpusEncData::Create(opus_int32 inputSampleRate, int channels, CString& lastError)
{
   OpusEncCallbacks callbacks = { write_callback, close_callback };
   int ret;

   enc = ope_encoder_create_callbacks(
      &callbacks,
      this,
      m_comments.get(),
      inputSampleRate,
      channels,
      channels > 8 ? 255 : channels > 2,
      &ret);

   if (enc == nullptr)
   {
      lastError.Format(
         _T("Error: failed to create encoder: %hs"),
         ope_strerror(ret));
      return false;
   }

   ret = ope_encoder_ctl(enc, OPE_SET_PACKET_CALLBACK(packet_callback, this));
   if (ret != 0)
   {
      lastError.Format(
         _T("Error: failed to set packet callback: %hs"),
         ope_strerror(ret));
      return false;
   }

   int serialno = 0;
   if (!GetRandomNumber(serialno))
      return false;

   ret = ope_encoder_ctl(enc, OPE_SET_SERIALNO(serialno));
   if (ret != 0)
   {
      lastError.Format(
         _T("Error: failed to set serial number: %hs"),
         ope_strerror(ret));
      return false;
   }

   return true;
}

void OpusEncData::Close()
{
   if (enc != nullptr)
   {
      ope_encoder_destroy(enc);
      enc = nullptr;
   }

   m_comments.reset();
}

int OpusEncData::write_callback(void* user_data, const unsigned char* ptr, opus_int32 len)
{
   OpusEncData* data = (OpusEncData*)user_data;
   data->bytes_written += len;
   data->pages_out++;
   return fwrite(ptr, 1, len, data->m_outputFile.get()) != (size_t)len;
}

int OpusEncData::close_callback(void* user_data)
{
   OpusEncData* data = (OpusEncData*)user_data;

   int ret = fclose(data->m_outputFile.get()) != 0;
   data->m_outputFile.reset();
   return ret;
}

void OpusEncData::packet_callback(void* user_data, const unsigned char* packet_ptr, opus_int32 packet_len, opus_uint32 flags)
{
   (void)flags;

   OpusEncData* data = (OpusEncData*)user_data;
   int nb_samples = opus_packet_get_nb_samples(packet_ptr, packet_len, 48000);
   if (nb_samples <= 0)
      return;  /* ignore header packets */
   data->total_bytes += packet_len;
   data->peak_bytes = std::max(packet_len, data->peak_bytes);
   data->min_bytes = std::min(packet_len, data->min_bytes);
   data->nb_encoded += nb_samples;
   data->packets_out++;
   data->last_length = packet_len;
}

//#pragma warning (disable: 4706) // assignment within conditional expression

//#define MAX_FRAME_BYTES 61295

using Encoder::OpusOutputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

OpusOutputModule::OpusOutputModule()
   :m_bitrateInBps(-1),
   m_complexity(10),
   m_opusBitrateMode(0),
   m_inputSampleRate(48000),
   m_inputSampleSize(16),
   m_codingRate(48000),
   m_downmix(0),
   m_frameSize(960),
   m_numSamplesPerFrame(0),
   m_32bitMode(false),
   m_outputStreamAtEnd(false)
{
   m_moduleId = ID_OM_OPUS;
}

OpusOutputModule::~OpusOutputModule()
{
}

bool OpusOutputModule::IsAvailable() const
{
   // always available
   return true;
}

CString OpusOutputModule::GetDescription() const
{
   LPCTSTR bitrateMode = _T("???");

   switch (m_opusBitrateMode)
   {
   case 0: bitrateMode = _T("VBR"); break;
   case 1: bitrateMode = _T("CVBR"); break;
   case 2: bitrateMode = _T("Hard-CBR"); break;
   default:
      ATLASSERT(false);
      break;
   }

   CString desc;
   desc.Format(IDS_FORMAT_INFO_OPUS_OUTPUT,
      m_channels,
      m_inputSampleRate,
      m_32bitMode ? 32 : 16,
      bitrateMode,
      m_bitrateInBps / 1000,
      m_complexity);

   if (m_downmix != 0)
      desc.AppendFormat(IDS_FORMAT_INFO_OPUS_OUTPUT_DOWNMIX, m_downmix);

   return desc;
}

void OpusOutputModule::GetVersionString(CString& version, int special) const
{
   version = opus_get_version_string();
}

int OpusOutputModule::InitOutput(LPCTSTR outfilename,
   SettingsManager& mgr, const TrackInfo& trackInfo,
   SampleContainer& samples)
{
   m_inputSampleRate = samples.GetInputModuleSampleRate();
   m_channels = samples.GetInputModuleChannels();
   m_inputSampleSize = samples.GetInputModuleBitsPerSample();

   m_32bitMode = samples.GetInputModuleBitsPerSample() == 32;

   // set options from UI
   m_bitrateInBps = mgr.QueryValueInt(OpusTargetBitrate) * 1000;
   m_complexity = mgr.QueryValueInt(OpusComplexity);
   m_opusBitrateMode = mgr.QueryValueInt(OpusBitrateMode);

   if (!StoreTrackInfos(trackInfo))
      return -1;

   if (!OpenOutputFile(outfilename))
      return -1;

   if (!InitEncoder())
      return -1;

   if (!SetEncoderOptions())
      return -1;

   m_samplerate = m_codingRate;

   // set up output traits
   samples.SetOutputModuleTraits(m_32bitMode ? 32 : 16, SamplesInterleaved, m_samplerate, m_channels);

   return 0;
}

int OpusOutputModule::EncodeSamples(SampleContainer& samples)
{
   int numSamples = RefillInputSampleBuffer(samples);

   if (!EncodeInputBufferUntilEmpty())
      return -1; // error occured

   return numSamples;
}

void OpusOutputModule::DoneOutput()
{
   EncodeRemainingInputBuffer();

   m_encoder.Close();
}

bool OpusOutputModule::StoreTrackInfos(const TrackInfo& trackinfo)
{
   OggOpusComments* comments = ope_comments_create();
   m_encoder.m_comments.reset(comments, ope_comments_destroy);

   // Vendor string should just be the encoder library,
   // the ENCODER comment specifies the tool used.
   char ENCODER_string[1024];
   snprintf(ENCODER_string, sizeof(ENCODER_string), "winLAME %ls", App::Version().GetString());
   ope_comments_add(comments, "ENCODER", ENCODER_string);

   CString encoderOptions;
   if (m_bitrateInBps != -1)
      encoderOptions.AppendFormat(_T("--bitrate %i "), m_bitrateInBps / 1000);

   encoderOptions.AppendFormat(_T("--%s "),
      m_opusBitrateMode == 0 ? _T("vbr") :
      m_opusBitrateMode == 1 ? _T("cvbr") :
      _T("hard-cbr"));

   encoderOptions.AppendFormat(_T("--comp %i"), m_complexity);

   ope_comments_add(comments, "ENCODER_OPTIONS", CStringA(encoderOptions).GetString());

   std::vector<char> utf8Buffer;

   bool avail = false;
   CString text = trackinfo.GetTextInfo(TrackInfoArtist, avail);
   if (avail)
   {
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "artist", utf8Buffer.data());
   }

   text = trackinfo.GetTextInfo(TrackInfoTitle, avail);
   if (avail)
   {
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "title", utf8Buffer.data());
   }

   text = trackinfo.GetTextInfo(TrackInfoAlbum, avail);
   if (avail)
   {
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "album", utf8Buffer.data());
   }

   text = trackinfo.GetTextInfo(TrackInfoDiscArtist, avail);
   if (avail)
   {
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "albumartist", utf8Buffer.data());
   }

   text = trackinfo.GetTextInfo(TrackInfoComposer, avail);
   if (avail)
   {
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "composer", utf8Buffer.data());
   }

   text = trackinfo.GetTextInfo(TrackInfoComment, avail);
   if (avail)
   {
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "comment", utf8Buffer.data());
   }

   int year = trackinfo.GetNumberInfo(TrackInfoYear, avail);
   if (avail && year != -1)
   {
      text.Format(_T("%i"), year);
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "date", utf8Buffer.data());
   }

   int trackNumber = trackinfo.GetNumberInfo(TrackInfoTrack, avail);
   if (avail && trackNumber > 0)
   {
      text.Format(_T("%i"), trackNumber);
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "tracknumber", utf8Buffer.data());
   }

   int discNumber = trackinfo.GetNumberInfo(TrackInfoDiscNumber, avail);
   if (avail && discNumber > 0)
   {
      text.Format(_T("%i"), discNumber);
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "discnumber", utf8Buffer.data());
   }

   text = trackinfo.GetTextInfo(TrackInfoGenre, avail);
   if (avail)
   {
      StringToUTF8(text, utf8Buffer);
      ope_comments_add(comments, "genre", utf8Buffer.data());
   }

   std::vector<unsigned char> binaryInfo;
   avail = trackinfo.GetBinaryInfo(TrackInfoFrontCover, binaryInfo);
   if (avail && !binaryInfo.empty())
   {
      ope_comments_add_picture_from_memory(comments, reinterpret_cast<const char*>(binaryInfo.data()), binaryInfo.size(), -1, nullptr);
   }

   return true;
}

bool OpusOutputModule::InitEncoder()
{
   if (m_inputSampleRate < 100 || m_inputSampleRate > 768000)
   {
      // Crazy rates excluded to avoid excessive memory usage for padding/resampling.
      m_lastError.Format(_T("Error parsing input file: unsupported  sample rate: %ld Hz"), m_inputSampleRate);
      return false;
   }

   if (m_channels > 255 || m_channels < 1)
   {
      m_lastError.Format(
         _T("Error parsing input file: unsupported channel count: %d\nChannel count must be in the range 1 to 255."),
         m_channels);
      return false;
   }

   if (m_downmix == 0 && m_channels > 2 && m_bitrateInBps > 0 && m_bitrateInBps < (16000 * m_channels))
   {
      ATLTRACE("Notice: Surround bitrate less than 16kbit/s per channel, downmixing.\n");
      m_downmix = m_channels > 8 ? 1 : 2;
   }

   if (m_downmix > 0 && m_downmix < m_channels)
   {
      if (!SetupDownmix(static_cast<size_t>(m_channels), static_cast<size_t>(m_downmix)))
         return false;
   }
   else
      m_downmix = 0;

   m_frameSize = 960; // 20 ms frames

   // Initialize Opus encoder
   int outputChannels = m_downmix != 0 ? m_downmix : m_channels;
   if (!m_encoder.Create(m_inputSampleRate, outputChannels, m_lastError))
      return false;

   m_numSamplesPerFrame = m_frameSize * m_channels;

   m_inputFloatBuffer.resize(m_numSamplesPerFrame);

   if (m_downmix != 0)
      m_downmixFloatBuffer.resize(m_numSamplesPerFrame);

   return true;
}

bool OpusOutputModule::SetEncoderOptions()
{
   int with_hard_cbr = 0;
   int with_cvbr = 0;

   switch (m_opusBitrateMode)
   {
   case 0: // --vbr
      with_cvbr = 0;
      with_hard_cbr = 0;
      break;

   case 1: // --cvbr
      with_cvbr = 1;
      with_hard_cbr = 0;
      break;

   case 2: // --hard-cbr
      with_hard_cbr = 1;
      with_cvbr = 0;
      break;

   default:
      ATLASSERT(false);
      break;
   }

   if (m_bitrateInBps < 0)
   {
      // Lower default rate for sampling rates [8000-44100) by a factor of (rate+16k)/(64k)
      m_bitrateInBps = ((64000 * m_encoder.nb_streams + 32000 * m_encoder.nb_coupled) *
         (std::min(48, std::max(8, ((m_inputSampleRate < 44100 ? m_inputSampleRate : 48000) + 1000) / 1000)) + 16) + 32) >> 6;
   }

   if (m_bitrateInBps > (1024000 * m_channels) || m_bitrateInBps < 500) {
      m_lastError.Format(_T("Error: Bitrate %d bits/sec is insane.\nDid you mistake bits for kilobits?\n"), m_bitrateInBps);
      m_lastError.Append(_T("--bitrate values from 6-256 kbit/sec per channel are meaningful.\n"));
      return false;
   }

   m_bitrateInBps = std::min(m_channels * 256000, m_bitrateInBps);

   int ret = ope_encoder_ctl(m_encoder.enc, OPUS_SET_BITRATE(m_bitrateInBps));
   if (ret != OPUS_OK)
   {
      m_lastError.Format(_T("Error OPUS_SET_BITRATE %d returned: %hs\n"), m_bitrateInBps, ope_strerror(ret));
      return false;
   }

   ret = ope_encoder_ctl(m_encoder.enc, OPUS_SET_VBR(!with_hard_cbr));
   if (ret != OPUS_OK)
   {
      m_lastError.Format(_T("Error OPUS_SET_VBR %d returned: %hs\n"), !with_hard_cbr, ope_strerror(ret));
      return false;
   }

   if (!with_hard_cbr)
   {
      ret = ope_encoder_ctl(m_encoder.enc, OPUS_SET_VBR_CONSTRAINT(with_cvbr));
      if (ret != OPUS_OK)
      {
         fprintf(stderr, "Error OPUS_SET_VBR_CONSTRAINT %d returned: %hs\n", with_cvbr, ope_strerror(ret));
         return false;
      }
   }

   // note: OPUS_SET_SIGNAL is kept at OPUS_AUTO

   ret = ope_encoder_ctl(m_encoder.enc, OPUS_SET_COMPLEXITY(m_complexity));
   if (ret != OPUS_OK)
   {
      m_lastError.Format(_T("Error OPUS_SET_COMPLEXITY %d returned: %hs\n"), m_complexity, ope_strerror(ret));
      return false;
   }

   int expect_loss = 0; // never loss of packets
   ret = ope_encoder_ctl(m_encoder.enc, OPUS_SET_PACKET_LOSS_PERC(expect_loss));
   if (ret != OPUS_OK)
   {
      m_lastError.Format(_T("Error OPUS_SET_PACKET_LOSS_PERC %d returned: %hs\n"), expect_loss, ope_strerror(ret));
      return false;
   }

   int lsb_depth = std::max(8, std::min(24, static_cast<int>(m_inputSampleSize)));
   ret = ope_encoder_ctl(m_encoder.enc, OPUS_SET_LSB_DEPTH(lsb_depth));
   if (ret != OPUS_OK)
   {
      m_lastError.Format(_T("Warning OPUS_SET_LSB_DEPTH %d returned: %hs\n"), lsb_depth, ope_strerror(ret));
      return false; // note: original opusenc doesn't have this exit point
   }

   return true;
}

bool OpusOutputModule::OpenOutputFile(LPCTSTR outputFilename)
{
   FILE* fout = _tfopen(outputFilename, _T("wb"));

   if (fout != nullptr)
   {
      m_encoder.m_outputFile.reset(fout, ::fclose);
      return true;
   }

   return false;
}

long OpusOutputModule::ReadFloatSamples16(float* buffer, int samples)
{
   int numSamples = std::min(m_inputInt16Buffer.size(), size_t(samples * m_channels));

   //ATLTRACE(_T("ReadFloatSamples16: Requesting %i samples, returning %i samples\n"), samples, numSamples / m_channels);

   const int16_t scaleFactor = std::numeric_limits<int16_t>::max();

   for (int i = 0; i < numSamples; i++)
   {
      buffer[i] = float(m_inputInt16Buffer[i]) / scaleFactor;
   }

   // remove samples from input buffer
   m_inputInt16Buffer.erase(m_inputInt16Buffer.begin(), m_inputInt16Buffer.begin() + numSamples);

   return numSamples / m_channels;
}

long OpusOutputModule::ReadFloatSamples32(float* buffer, int samples)
{
   int numSamples = std::min(m_inputInt32Buffer.size(), size_t(samples * m_channels));

   //ATLTRACE(_T("ReadFloatSamples32: Requesting %i samples, returning %i samples\n"), samples, numSamples / m_channels);

   const int32_t scaleFactor = std::numeric_limits<int32_t>::max();

   for (int i = 0; i < numSamples; i++)
   {
      buffer[i] = float(m_inputInt32Buffer[i]) / scaleFactor;
   }

   // remove samples from input buffer
   m_inputInt32Buffer.erase(m_inputInt32Buffer.begin(), m_inputInt32Buffer.begin() + numSamples);

   return numSamples / m_channels;
}

int OpusOutputModule::RefillInputSampleBuffer(SampleContainer& samples)
{
   // get samples
   int numSamples = 0;

   if (m_32bitMode)
   {
      static_assert(sizeof(int) == sizeof(opus_int32), "short and opus_int16 must have the same size");

      // numSamples is in "samples per channel", so input buffer contains numSamples*m_channels samples
      opus_int32* inputBuffer = (opus_int32*)samples.GetSamplesInterleaved(numSamples);

      size_t startIndex = m_inputInt32Buffer.size();

      m_inputInt32Buffer.resize(startIndex + numSamples * m_channels);

      std::copy_n(inputBuffer, numSamples * m_channels, m_inputInt32Buffer.begin() + startIndex);
   }
   else
   {
      static_assert(sizeof(short) == sizeof(opus_int16), "short and opus_int16 must have the same size");

      opus_int16* inputBuffer = (opus_int16*)samples.GetSamplesInterleaved(numSamples);

      size_t startIndex = m_inputInt16Buffer.size();

      m_inputInt16Buffer.resize(startIndex + numSamples * m_channels);

      std::copy_n(inputBuffer, numSamples * m_channels, m_inputInt16Buffer.begin() + startIndex);
   }

   return numSamples;
}

bool OpusOutputModule::EncodeInputBufferUntilEmpty()
{
   // as long as the input buffer has samples for one frame, encode it
   bool inputBufferSufficientSamples =
      (m_32bitMode ? m_inputInt32Buffer.size() : m_inputInt16Buffer.size()) >= size_t(m_numSamplesPerFrame);

   while (inputBufferSufficientSamples)
   {
      if (!EncodeInputBufferFrame())
         return false; // error occured

      inputBufferSufficientSamples =
         (m_32bitMode ? m_inputInt32Buffer.size() : m_inputInt16Buffer.size()) >= size_t(m_numSamplesPerFrame);
   }

   return true;
}

void OpusOutputModule::EncodeRemainingInputBuffer()
{
   size_t inputBufferSize = m_32bitMode ? m_inputInt32Buffer.size() : m_inputInt16Buffer.size();

   if (inputBufferSize > 0)
   {
      EncodeInputBufferUntilEmpty();

      inputBufferSize = m_32bitMode ? m_inputInt32Buffer.size() : m_inputInt16Buffer.size();

      if (inputBufferSize > 0)
      {
         while (!m_outputStreamAtEnd)
         {
            if (!EncodeInputBufferFrame())
               return; // there was an error
         }
      }

      int ret = ope_encoder_drain(m_encoder.enc);
      if (ret != OPE_OK)
      {
         m_lastError.Format(
            _T("Error: Encoding aborted: %hs"),
            ope_strerror(ret));
      }
   }
}

bool OpusOutputModule::EncodeInputBufferFrame()
{
   // read samples
   opus_int32 nb_samples = -1; // number of samples, per channel

   if (m_32bitMode)
      nb_samples = ReadFloatSamples32(m_inputFloatBuffer.data(), m_frameSize);
   else
      nb_samples = ReadFloatSamples16(m_inputFloatBuffer.data(), m_frameSize);

   if (nb_samples < m_frameSize)
   {
      m_outputStreamAtEnd = true;
   }

   if (!m_downmixMatrix.empty())
      DownmixSamples(nb_samples);

   int ret = ope_encoder_write_float(m_encoder.enc, m_inputFloatBuffer.data(), nb_samples);
   if (ret != OPE_OK)
   {
      m_lastError.Format(
         _T("Error: Encoding failed: %hs"),
         ope_strerror(ret));
      return false;
   }

   return true; // no errors
}

std::string OpusOutputModule::GetMetadataBlockPicture(const std::vector<unsigned char>& imageData)
{
   OggOpusComments* comments = ope_comments_create();
   std::shared_ptr<OggOpusComments> spComments(comments, ope_comments_destroy);

   ope_comments_add_picture_from_memory(comments, reinterpret_cast<const char*>(imageData.data()), imageData.size(), -1, nullptr);

   const unsigned char* data = *reinterpret_cast<const unsigned char**>(comments);
   data += 8; // OpusTags
   data += 4 + data[0] + 4; // offset + length + number of comments

   unsigned int length = data[0] |
      (static_cast<unsigned int>(data[1]) << 8) |
      (static_cast<unsigned int>(data[2]) << 16) |
      (static_cast<unsigned int>(data[3]) << 24);

   const unsigned int prefixLength = sizeof("METADATA_BLOCK_PICTURE=") - 1;
   data += 4 + prefixLength;

   std::string base64image(reinterpret_cast<const char*>(data), length - prefixLength);

   return base64image;
}

// Note: The stupid_matrix table and the code in SetupDownmix() and
// DownmixSamples() is taken from opus-tools' audio-in.c file:
// https://github.com/xiph/opus-tools/blob/master/src/audio-in.c
// The following copyright header appears in the file
//
/* Copyright 2000-2002, Michael Smith <msmith@xiph.org>
             2010, Monty <monty@xiph.org>
   AIFF/AIFC support from OggSquish, (c) 1994-1996 Monty <xiphmont@xiph.org>
   (From GPL code in oggenc relicensed by permission from Monty and Msmith)
   File: audio-in.c
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/// \brief matrix for downsampling from N channels to 2 channels
static const float stupid_matrix[7][8][2] =
{
   /*2*/  {{1,0}, {0,1}},
   /*3*/  {{1,0}, {0.7071f,0.7071f}, {0,1}},
   /*4*/  {{1,0}, {0,1},{0.866f,0.5f}, {0.5f,0.866f}},
   /*5*/  {{1,0}, {0.7071f,0.7071f}, {0,1}, {0.866f,0.5f}, {0.5f,0.866f}},
   /*6*/  {{1,0}, {0.7071f,0.7071f}, {0,1}, {0.866f,0.5f}, {0.5f,0.866f}, {0.7071f,0.7071f}},
   /*7*/  {{1,0}, {0.7071f,0.7071f}, {0,1}, {0.866f,0.5f}, {0.5f,0.866f}, {0.6123f,0.6123f}, {0.7071f,0.7071f}},
   /*8*/  {{1,0}, {0.7071f,0.7071f}, {0,1}, {0.866f,0.5f}, {0.5f,0.866f}, {0.866f,0.5f}, {0.5f,0.866f}, {0.7071f,0.7071f}},
};

bool OpusOutputModule::SetupDownmix(size_t inputNumChannels, size_t outputNumChannels)
{
   if (inputNumChannels <= outputNumChannels || outputNumChannels > 2 || inputNumChannels <= 0 || outputNumChannels <= 0)
   {
      m_lastError = _T("Downmix must actually downmix and only knows mono/stereo out.");
      return false;
   }

   if (outputNumChannels == 2 && inputNumChannels > 8)
   {
      m_lastError = _T("Downmix only knows how to mix >8ch to mono.");
      return false;
   }

   m_downmixMatrix.resize(inputNumChannels * outputNumChannels);

   if (outputNumChannels == 1 && inputNumChannels > 8)
   {
      for (size_t i = 0; i < inputNumChannels; i++)
         m_downmixMatrix[i] = 1.0f / inputNumChannels;
   }
   else if (outputNumChannels == 2)
   {
      for (size_t j = 0; j < outputNumChannels; j++)
         for (size_t i = 0; i < inputNumChannels; i++)
            m_downmixMatrix[inputNumChannels * j + i] = stupid_matrix[inputNumChannels - 2][i][j];
   }
   else
   {
      for (size_t i = 0; i < inputNumChannels; i++)
         m_downmixMatrix[i] = stupid_matrix[inputNumChannels - 2][i][0] + stupid_matrix[inputNumChannels - 2][i][1];
   }

   float sum = 0.f;
   for (size_t i = 0; i < inputNumChannels * outputNumChannels; i++)
      sum += m_downmixMatrix[i];

   sum = (float)outputNumChannels / sum;
   for (size_t i = 0; i < inputNumChannels * outputNumChannels; i++)
      m_downmixMatrix[i] *= sum;

   return true;
}

void OpusOutputModule::DownmixSamples(opus_int32& numSamplesPerChannel)
{
   ATLASSERT(m_downmix == 1 || m_downmix == 2); // downmix value must be 1 or 2

   size_t inputNumChannels = m_channels;
   size_t outputNumChannels = m_downmix;

   for (opus_int32 i = 0; i < numSamplesPerChannel; i++)
   {
      for (size_t j = 0; j < outputNumChannels; j++)
      {
         float* sample = &m_downmixFloatBuffer[i * outputNumChannels + j];
         *sample = 0.f;

         for (size_t k = 0; k < inputNumChannels; k++)
         {
            *sample += m_inputFloatBuffer[i * inputNumChannels + k] * m_downmixMatrix[inputNumChannels * j + k];
         }
      }
   }

   numSamplesPerChannel = numSamplesPerChannel * m_downmix / m_channels;

   std::swap(m_downmixFloatBuffer, m_inputFloatBuffer);
}
