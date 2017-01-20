//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2016 Michael Fink
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
/// \file OpusInputModule.cpp
/// \brief Opus input module
//
#include "stdafx.h"
#include "OpusInputModule.hpp"
#include "resource.h"

using Encoder::OpusInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

#pragma comment(lib, "libopusfile-0.lib")
#pragma comment(lib, "libopus-0.lib")

OpusInputModule::OpusInputModule()
   :m_numTotalSamples(0)
{
   m_moduleId = ID_IM_OPUS;

   m_callbacks.read = &OpusInputModule::ReadStream;
   m_callbacks.seek = &OpusInputModule::SeekStream;
   m_callbacks.tell = &OpusInputModule::PosStream;
   m_callbacks.close = &OpusInputModule::CloseStream;
}

Encoder::InputModule* OpusInputModule::CloneModule()
{
   return new OpusInputModule;
}

bool OpusInputModule::IsAvailable() const
{
   // we don't do delay-loading anymore, so it's always available
   return true;
}

CString OpusInputModule::GetDescription() const
{
   ATLASSERT(m_inputFile != nullptr);

   const OpusHead* header = op_head(m_inputFile.get(), 0);

   CString desc;
   desc.Format(IDS_FORMAT_INFO_OPUS_INPUT,
      header->channel_count,
      header->input_sample_rate);

   return desc;
}

void OpusInputModule::GetVersionString(CString& version, int special) const
{
   version = opus_get_version_string();
}

CString OpusInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_OPUS_INPUT);
   return filterString;
}

int OpusInputModule::InitInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackInfo, SampleContainer& samples)
{
   FILE* fd = nullptr;
   errno_t err = _tfopen_s(&fd, infilename, _T("rb"));

   if (err != 0 || fd == nullptr)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   int errorCode = 0;
   OggOpusFile* file = op_open_callbacks(fd, &m_callbacks, nullptr, 0, &errorCode);

   if (file != nullptr)
      m_inputFile.reset(file, op_free);

   if (file == nullptr || errorCode < 0)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      m_lastError.AppendFormat(_T(" (%s)"), ErrorTextFromCode(errorCode));
      return -1;
   }

   const OpusHead* header = op_head(m_inputFile.get(), 0);

   // set up input traits
   samples.SetInputModuleTraits(16, SamplesInterleaved,
      48000, header->channel_count);

   m_numTotalSamples = op_pcm_total(m_inputFile.get(), -1);

   return 0;
}

void OpusInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   bitrateInBps = op_bitrate(m_inputFile.get(), 0);
   samplerateInHz = 48000;

   ogg_int64_t numTotalSamples = op_pcm_total(m_inputFile.get(), -1);

   lengthInSeconds = static_cast<int>(numTotalSamples / 48000);

   const OpusHead* header = op_head(m_inputFile.get(), 0);
   if (header == nullptr)
      return;

   numChannels = header->channel_count;
}

int OpusInputModule::DecodeSamples(SampleContainer& samples)
{
   const OpusHead* header = op_head(m_inputFile.get(), 0);
   if (header == nullptr)
      return -1;

   short sampleBuffer[48000];

   int currentLink = 0;
   int numSamplesPerChannel = op_read(m_inputFile.get(), sampleBuffer, sizeof(sampleBuffer) / sizeof(*sampleBuffer), &currentLink);

   if (numSamplesPerChannel == 0)
      return 0;

   samples.PutSamplesInterleaved(sampleBuffer, numSamplesPerChannel);

   return numSamplesPerChannel * header->channel_count;
}

float OpusInputModule::PercentDone() const
{
   if (m_numTotalSamples == 0 || m_inputFile == nullptr)
      return 0.0f;

   ogg_int64_t currentSamplePos = op_pcm_tell(m_inputFile.get());
   float percentValue = currentSamplePos / float(m_numTotalSamples);

   return percentValue * 100.f;
}

void OpusInputModule::DoneInput()
{
   m_inputFile.reset();
}

LPCTSTR OpusInputModule::ErrorTextFromCode(int errorCode)
{
   switch (errorCode)
   {
   case OP_FALSE: return _T("A request did not succeed");
   case OP_EOF: return _T("End of file"); // unused
   case OP_HOLE: return _T("Hole in page sequence numbers");
   case OP_EREAD: return _T("Read, seek or tell operation failed");
   case OP_EFAULT: return _T("NULL pointer, memory allocation or internal library error");
   case OP_EIMPL: return _T("Not implemented");
   case OP_EINVAL: return _T("Invalid parameter");
   case OP_ENOTFORMAT: return _T("Wrong Ogg or Opus format");
   case OP_EBADHEADER: return _T("Bad heder format");
   case OP_EVERSION: return _T("Unrecognited ID header version number");
   case OP_ENOTAUDIO: return _T("Not an audio stream"); // unused
   case OP_EBADPACKET: return _T("Bad audio packet");
   case OP_EBADLINK: return _T("Bad link in data");
   case OP_ENOSEEK: return _T("Seek on unseekable stream");
   case OP_EBADTIMESTAMP: return _T("Bad timestamp");
   default:
      ATLASSERT(false);
      break;
   }

   return _T("???");
}

int OpusInputModule::ReadStream(void* stream, unsigned char* buffer, int numBytes)
{
   FILE* fd = reinterpret_cast<FILE*>(stream);
   return fread(buffer, 1, numBytes, fd);
}

int OpusInputModule::SeekStream(void* stream, opus_int64 offset, int whence)
{
   FILE* fd = reinterpret_cast<FILE*>(stream);
   return _fseeki64(fd, offset, whence);
}

opus_int64 OpusInputModule::PosStream(void* stream)
{
   FILE* fd = reinterpret_cast<FILE*>(stream);
   return _ftelli64(fd);
}

int OpusInputModule::CloseStream(void* stream)
{
   FILE* fd = reinterpret_cast<FILE*>(stream);
   return fclose(fd);
}
