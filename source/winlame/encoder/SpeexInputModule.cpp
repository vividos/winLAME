//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2017 Michael Fink
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
/// \file SpeexInputModule.cpp
/// \brief Speex input module
//
#include "stdafx.h"
#include "SpeexInputModule.hpp"
#include "resource.h"
#include <speex/speex_callbacks.h>
#include <initializer_list>

using Encoder::SpeexInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

#pragma comment(lib, "libspeex.lib")

// define some symbols that seem to be missing from the lib file
extern "C" void speex_header_free(void* ptr)
{
   // note: speex_free isn't exported by the lib as well
   //speex_free(ptr);
}

SpeexInputModule::SpeexInputModule()
   :m_fileSize(0),
   m_packetCount(0),
   m_sampleCount(-1), // -1 means "not calculated yet"
   m_stereo(SPEEX_STEREO_STATE_INIT)
{
   m_moduleId = ID_IM_SPEEX;

   memset(&m_bits, 0, sizeof(m_bits));
}

Encoder::InputModule* SpeexInputModule::CloneModule()
{
   return new SpeexInputModule;
}

bool SpeexInputModule::IsAvailable() const
{
   // we don't do delay-loading anymore, so it's always available
   return true;
}

CString SpeexInputModule::GetDescription() const
{
   if (m_header == nullptr)
      return CString();

   CString desc;
   desc.Format(IDS_FORMAT_INFO_SPEEX_INPUT,
      m_header->mode == 0 ? _T("narrow-band") : m_header->mode == 1 ? _T("wide-band") : _T("???"),
      m_header->vbr == 1 ? _T(" VBR") : _T(""),
      m_header->nb_channels,
      m_header->rate,
      m_header->speex_version);

   return desc;
}

void SpeexInputModule::GetVersionString(CString& version, int special) const
{
   const char* fullVersion = nullptr;
   speex_lib_ctl(SPEEX_LIB_GET_VERSION_STRING, &fullVersion);

   unsigned int uiVersion[4] = { 0 };
   speex_lib_ctl(SPEEX_LIB_GET_MAJOR_VERSION, uiVersion + 0);
   speex_lib_ctl(SPEEX_LIB_GET_MINOR_VERSION, uiVersion + 1);
   speex_lib_ctl(SPEEX_LIB_GET_MICRO_VERSION, uiVersion + 2);

   const char* extraVersion = nullptr;
   speex_lib_ctl(SPEEX_LIB_GET_EXTRA_VERSION, &extraVersion);

   version.Format(_T("%hs (%u.%u.%u%s%hs)"),
      fullVersion,
      uiVersion[0], uiVersion[1], uiVersion[2],
      extraVersion != nullptr && strlen(extraVersion) > 0 ? _T(" ") : _T(""),
      extraVersion);
}

CString SpeexInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_SPEEX_INPUT);
   return filterString;
}

int SpeexInputModule::InitInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackInfo, SampleContainer& samples)
{
   FILE* fd = nullptr;
   errno_t err = _tfopen_s(&fd, infilename, _T("rb"));

   if (err != 0 || fd == nullptr)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   err = fseek(fd, 0, SEEK_END);
   if (err != 0)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      fclose(fd);
      return -1;
   }

   m_fileSize = ftell(fd);
   err = fseek(fd, 0, SEEK_SET);
   if (err != 0)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      fclose(fd);
      return -1;
   }

   m_inputStream.reset(new OggInputStream(fd, false));

   size_t numTryReads = 3;
   while (!m_inputStream->IsEndOfStream() && numTryReads > 0)
   {
      m_inputStream->ReadInput(200); // read some more

      ogg_packet header;

      if (m_inputStream->ReadNextPacket(header))
      {
         SpeexHeader* speexHeader = speex_packet_to_header(reinterpret_cast<char*>(header.packet), header.bytes);
         m_header.reset(speexHeader, speex_header_free);

         break;
      }
      else
         --numTryReads;
   }

   if (m_header == nullptr)
   {
      m_lastError = _T("Couldn't read Speex header");
      fclose(fd);
      return -1;
   }

   InitDecoder();

   // set up input traits
   samples.SetInputModuleTraits(16, SamplesInterleaved,
      m_header->rate, m_header->nb_channels);

   // re-init file
   err = fseek(fd, 0, SEEK_SET);
   if (err != 0)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      fclose(fd);
      return -1;
   }

   m_inputStream.reset(new OggInputStream(fd, true));

   m_packetCount = 0;

   speex_bits_init(&m_bits);

   return 0;
}

void SpeexInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   numChannels = m_header->nb_channels;
   bitrateInBps = m_header->bitrate;
   samplerateInHz = m_header->rate;

   if (m_sampleCount < 0)
   {
      CalcSampleCount();
   }

   lengthInSeconds = m_header->rate == 0 ? 0 : static_cast<int>(m_sampleCount / m_header->rate);
}

int SpeexInputModule::DecodeSamples(SampleContainer& samples)
{
   do
   {
      // read packet
      ogg_packet packet = { 0 };
      if (!ReadNextPacket(packet))
         return 0;

      if (m_packetCount == 0 || m_packetCount == 1)
      {
         // header or comment packet; header already read in InitInput()
      }
      else
         if (m_packetCount <= static_cast<unsigned int>(1 + m_header->extra_headers))
         {
            // ignore extra headers
         }
         else
         {
            std::vector<short> sampleBuffer;

            int iRet = DecodePacket(packet, sampleBuffer);

            if (iRet == 0)
               return 0;

            ATLASSERT(!sampleBuffer.empty());

            samples.PutSamplesInterleaved(sampleBuffer.data(), sampleBuffer.size() / m_header->nb_channels);

            return sampleBuffer.size();
         }

      m_packetCount++;

   } while (true);

   return 0;
}

float SpeexInputModule::PercentDone() const
{
   if (m_inputStream == nullptr ||
      m_fileSize == 0)
      return 0.0f;

   if (m_inputStream->IsEndOfStream())
      return 100.0f;

   long lPos = ftell(m_inputStream->GetFd());

   return float(lPos) * 100.0f / m_fileSize;
}

void SpeexInputModule::DoneInput()
{
   speex_bits_destroy(&m_bits);
   m_header.reset();
   m_decoderState.reset();
}

void SpeexInputModule::InitDecoder()
{
   bool bWideband = m_header->mode == 1;
   const bool perceptualEnhancer = true;

   const SpeexMode* mode = speex_lib_get_mode(bWideband ? SPEEX_MODEID_WB : SPEEX_MODEID_NB);

   void* p = speex_decoder_init(mode);
   m_decoderState.reset(p, speex_decoder_destroy);

   int enh = perceptualEnhancer ? 1 : 0;
   speex_decoder_ctl(m_decoderState.get(), SPEEX_SET_ENH, &enh);

   // init stereo
   if (m_header->nb_channels != 1)
   {
      SpeexCallback callback;

      callback.callback_id = SPEEX_INBAND_STEREO;
      callback.func = speex_std_stereo_request_handler;
      callback.data = &m_stereo;
      speex_decoder_ctl(m_decoderState.get(), SPEEX_SET_HANDLER, &callback);
   }
}

bool SpeexInputModule::ReadNextPacket(ogg_packet& packet)
{
   while (!m_inputStream->ReadNextPacket(packet))
   {
      if (m_inputStream->IsEndOfStream())
         return false;

      m_inputStream->ReadInput(4000); // read some more
   }

   return true;
}

int SpeexInputModule::DecodePacket(ogg_packet& packet, std::vector<short>& samples)
{
   spx_int32_t frameSize = 0;
   speex_decoder_ctl(m_decoderState.get(), SPEEX_GET_FRAME_SIZE, &frameSize);

   // copy Ogg packet to Speex bitstream
   speex_bits_read_from(&m_bits, reinterpret_cast<char*>(packet.packet), packet.bytes);

   int iMaxFrames = m_header->frames_per_packet;
   int numChannels = m_header->nb_channels;

   for (int i = 0; i < iMaxFrames; i++)
   {
#define MAX_FRAME_SIZE 2000
      short outputBuffer[MAX_FRAME_SIZE];

      int iRet = speex_decode_int(m_decoderState.get(), &m_bits, outputBuffer);

      if (iRet == -1)
         break;

      if (iRet == -2 ||
         speex_bits_remaining(&m_bits) < 0)
      {
         m_lastError = _T("Decoding error; corrupted stream");
         return 0;
      }

      if (numChannels == 2)
         speex_decode_stereo_int(outputBuffer, frameSize, &m_stereo);

      samples.insert(samples.end(),
         std::initializer_list<short>(outputBuffer, outputBuffer + frameSize * numChannels));
   }

   return samples.size();
}

/// \see http://lists.xiph.org/pipermail/speex-dev/2003-September/001885.html
void SpeexInputModule::CalcSampleCount() const
{
   if (m_inputStream == nullptr)
      return;

   FILE* fd = m_inputStream->GetFd();
   long currentPos = ftell(fd);

   ogg_int64_t sampleCount = 0;
   ogg_int64_t samplesPerPacket = 0;

   while (!m_inputStream->IsEndOfStream())
   {
      m_inputStream->ReadInput(2048); // read some more

      ogg_packet header;

      while (m_inputStream->ReadNextPacket(header))
      {
         // updated values?
         SpeexHeader* speexHeader = speex_packet_to_header(reinterpret_cast<char*>(header.packet), header.bytes);
         if (speexHeader != nullptr)
         {
            samplesPerPacket = speexHeader->frames_per_packet * speexHeader->frame_size;

            speex_header_free(speexHeader);
         }

         if (samplesPerPacket > 0)
         {
            sampleCount += samplesPerPacket;
         }
      }
   }

   fseek(fd, currentPos, SEEK_SET);

   m_sampleCount = sampleCount;
}
