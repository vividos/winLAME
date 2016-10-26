//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014 Michael Fink
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

// includes
#include "stdafx.h"
#include "SpeexInputModule.hpp"
#include "resource.h"
#include <speex/speex_callbacks.h>
#include <initializer_list>

#pragma comment(lib, "libspeex.lib")

// define some symbols that seem to be missing from the lib file
extern "C" void speex_header_free(void* ptr)
{
   // TODO implement
   //free(ptr);
}


SpeexInputModule::SpeexInputModule()
:m_stereo(SPEEX_STEREO_STATE_INIT),
m_packetCount(0),
m_lFileSize(0)
{
   module_id = ID_IM_SPEEX;
}

InputModule* SpeexInputModule::cloneModule()
{
   return new SpeexInputModule;
}

bool SpeexInputModule::isAvailable()
{
   // we don't do delay-loading anymore, so it's always available
   return true;
}

void SpeexInputModule::getDescription(CString& desc)
{
   ATLASSERT(m_spHeader != nullptr);

   desc.Format(IDS_FORMAT_INFO_SPEEX_INPUT,
      m_spHeader->mode == 0 ? _T("narrow-band") : m_spHeader->mode == 1 ? _T("wide-band") : _T("???"),
      m_spHeader->vbr == 1 ? _T(" VBR") : _T(""),
      m_spHeader->nb_channels,
      m_spHeader->rate,
      m_spHeader->speex_version);
}

void SpeexInputModule::getVersionString(CString& version, int special)
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

CString SpeexInputModule::getFilterString()
{
   CString cszFilter;
   cszFilter.LoadString(IDS_FILTER_SPEEX_INPUT);
   return cszFilter;
}

int SpeexInputModule::initInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackinfo, SampleContainer& samples)
{
   FILE* fd = nullptr;
   errno_t err = _tfopen_s(&fd, infilename, _T("rb"));

   if (err != 0 || fd == nullptr)
   {
      m_cszLastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   fseek(fd, 0, SEEK_END);
   m_lFileSize = ftell(fd);
   fseek(fd, 0, SEEK_SET);

   m_spInputStream.reset(new OggInputStream(fd, false));

   size_t uiTryReads = 3;
   while (!m_spInputStream->IsEndOfStream() && uiTryReads > 0)
   {
      m_spInputStream->ReadInput(200); // read some more

      ogg_packet header;

      if (m_spInputStream->ReadNextPacket(header))
      {
         SpeexHeader* pHeader = speex_packet_to_header(reinterpret_cast<char*>(header.packet), header.bytes);
         m_spHeader.reset(pHeader, speex_header_free);

         break;
      }
      else
         --uiTryReads;
   }

   if (m_spHeader == nullptr)
   {
      m_cszLastError = _T("Couldn't read Speex header");
      return -1;
   }

   InitDecoder();

   // set up input traits
   samples.setInputModuleTraits(16, SamplesInterleaved,
      m_spHeader->rate, m_spHeader->nb_channels);

   // re-init file
   fseek(fd, 0, SEEK_SET);
   m_spInputStream.reset(new OggInputStream(fd, true));

   m_packetCount = 0;

   speex_bits_init(&m_bits);

   return 0;
}

void SpeexInputModule::getInfo(int& channels, int& bitrate, int& length, int& samplerate)
{
   channels = m_spHeader->nb_channels;
   bitrate = m_spHeader->bitrate;
   samplerate = m_spHeader->rate;
   length = -1; // TODO
}

int SpeexInputModule::decodeSamples(SampleContainer& samples)
{
   do
   {
      // read packet
      ogg_packet packet = { 0 };
      if (!ReadNextPacket(packet))
         return 0;

      if (m_packetCount == 0 || m_packetCount == 1)
      {
         // header or comment packet; header already read in initInput()
      }
      else
      if (m_packetCount <= static_cast<unsigned int>(1 + m_spHeader->extra_headers))
      {
         // ignore extra headers
      }
      else
      {
         //if (packet.e_o_s)
         //   return 0;// TODO end of stream

         std::vector<short> vecSamples;

         int iRet = DecodePacket(packet, vecSamples);

         if (iRet == 0)
            return 0;

         ATLASSERT(!vecSamples.empty());

         samples.putSamplesInterleaved(&vecSamples[0], vecSamples.size() / m_spHeader->nb_channels);

         return vecSamples.size();
      }

      m_packetCount++;

   } while (true);

   return 0;
}

float SpeexInputModule::percentDone()
{
   if (m_spInputStream == nullptr ||
       m_lFileSize == 0)
      return 0.0f;

   if (m_spInputStream->IsEndOfStream())
      return 100.0f;

   long lPos = ftell(m_spInputStream->GetFd());

   return float(lPos) * 100.0f / m_lFileSize;
}

void SpeexInputModule::doneInput()
{
   speex_bits_destroy(&m_bits);
   m_spHeader.reset();
   m_spDecoderState.reset();
}

void SpeexInputModule::InitDecoder()
{
   bool bWideband = m_spHeader->mode == 1;
   bool bPerceptualEnhancer = true;

   const SpeexMode* mode = speex_lib_get_mode(bWideband ? SPEEX_MODEID_WB : SPEEX_MODEID_NB);

   void* p = speex_decoder_init(mode);
   m_spDecoderState.reset(p, speex_decoder_destroy);

   int enh = bPerceptualEnhancer ? 1 : 0;
   speex_decoder_ctl(m_spDecoderState.get(), SPEEX_SET_ENH, &enh);

   // init stereo
   if (m_spHeader->nb_channels != 1)
   {
      SpeexCallback callback;

      callback.callback_id = SPEEX_INBAND_STEREO;
      callback.func = speex_std_stereo_request_handler;
      callback.data = &m_stereo;
      speex_decoder_ctl(m_spDecoderState.get(), SPEEX_SET_HANDLER, &callback);
   }
}

bool SpeexInputModule::ReadNextPacket(ogg_packet& packet)
{
   unsigned int uiReads = 0;
   while (!m_spInputStream->ReadNextPacket(packet))
   {
      if (m_spInputStream->IsEndOfStream())
         return false;

      m_spInputStream->ReadInput(4000); // read some more
      uiReads++;
   }

   return true;
}

int SpeexInputModule::DecodePacket(ogg_packet& packet, std::vector<short>& vecSamples)
{
   spx_int32_t iFrameSize = 0;
   speex_decoder_ctl(m_spDecoderState.get(), SPEEX_GET_FRAME_SIZE, &iFrameSize);

   // copy Ogg packet to Speex bitstream
   speex_bits_read_from(&m_bits, reinterpret_cast<char*>(packet.packet), packet.bytes);

   int iMaxFrames = m_spHeader->frames_per_packet;
   int iChannels = m_spHeader->nb_channels;

   for (int i = 0; i < iMaxFrames; i++)
   {
#define MAX_FRAME_SIZE 2000
      short outputBuffer[MAX_FRAME_SIZE];

      int iRet = speex_decode_int(m_spDecoderState.get(), &m_bits, outputBuffer);

      if (iRet == -1)
         break;

      if (iRet == -2 ||
         speex_bits_remaining(&m_bits) < 0)
      {
         m_cszLastError = _T("Decoding error; corrupted stream");
         return 0;
      }

      if (iChannels == 2)
         speex_decode_stereo_int(outputBuffer, iFrameSize, &m_stereo);

      vecSamples.insert(vecSamples.end(),
         std::initializer_list<short>(outputBuffer, outputBuffer + iFrameSize * iChannels));
   }

   return vecSamples.size();
}
