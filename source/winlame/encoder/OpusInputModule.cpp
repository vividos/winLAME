//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2015 Michael Fink
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

// includes
#include "stdafx.h"
#include "OpusInputModule.hpp"
#include "resource.h"

#pragma comment(lib, "libopusfile-0.lib")
#pragma comment(lib, "libopus-0.lib")


OpusInputModule::OpusInputModule()
:m_lTotalSamples(0)
{
   module_id = ID_IM_OPUS;

   m_callbacks.read = &OpusInputModule::ReadStream;
   m_callbacks.seek = &OpusInputModule::SeekStream;
   m_callbacks.tell = &OpusInputModule::PosStream;
   m_callbacks.close = &OpusInputModule::CloseStream;
}

InputModule* OpusInputModule::cloneModule()
{
   return new OpusInputModule;
}

bool OpusInputModule::isAvailable()
{
   // we don't do delay-loading anymore, so it's always available
   return true;
}

void OpusInputModule::getDescription(CString& desc)
{
   ATLASSERT(m_spInputFile != nullptr);

   const OpusHead* pHeader = op_head(m_spInputFile.get(), 0);

   desc.Format(IDS_FORMAT_INFO_OPUS_INPUT,
      pHeader->channel_count,
      pHeader->input_sample_rate);
}

void OpusInputModule::getVersionString(CString& version, int /*special*/)
{
   version = opus_get_version_string();
}

CString OpusInputModule::getFilterString()
{
   CString cszFilter;
   cszFilter.LoadString(IDS_FILTER_OPUS_INPUT);
   return cszFilter;
}

int OpusInputModule::initInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackinfo, SampleContainer& samples)
{
   FILE* fd = nullptr;
   errno_t err = _tfopen_s(&fd, infilename, _T("rb"));

   if (err != 0 || fd == nullptr)
   {
      m_cszLastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   int iErrorCode = 0;
   OggOpusFile* pFile = op_open_callbacks(fd, &m_callbacks, nullptr, 0, &iErrorCode);

   if (pFile != nullptr)
      m_spInputFile.reset(pFile, op_free);

   if (pFile == nullptr || iErrorCode < 0)
   {
      m_cszLastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      m_cszLastError.AppendFormat(_T(" (%s)"), ErrorTextFromCode(iErrorCode));
      return -1;
   }

   const OpusHead* pHeader = op_head(m_spInputFile.get(), 0);

   // set up input traits
   samples.setInputModuleTraits(16, SamplesInterleaved,
      48000, pHeader->channel_count);

   m_lTotalSamples = op_pcm_total(m_spInputFile.get(), -1);

   return 0;
}

void OpusInputModule::getInfo(int& channels, int& bitrate, int& length, int& samplerate)
{
   bitrate = op_bitrate(m_spInputFile.get(), 0);
   samplerate = 48000;

   ogg_int64_t total = op_pcm_total(m_spInputFile.get(), -1);

   length = static_cast<int>(total / 48000);

   const OpusHead* pHeader = op_head(m_spInputFile.get(), 0);
   if (pHeader == nullptr)
      return;

   channels = pHeader->channel_count;
}

int OpusInputModule::decodeSamples(SampleContainer& samples)
{
   const OpusHead* pHeader = op_head(m_spInputFile.get(), 0);
   if (pHeader == nullptr)
      return -1;

   short aSamples[48000];

   int iCurrentLink = 0;
   int iSamplesPerChannel = op_read(m_spInputFile.get(), aSamples, sizeof(aSamples) / sizeof(*aSamples), &iCurrentLink);

   if (iSamplesPerChannel == 0)
      return 0;

   samples.putSamplesInterleaved(aSamples, iSamplesPerChannel);

   return iSamplesPerChannel * pHeader->channel_count;
}

float OpusInputModule::percentDone()
{
   if (m_lTotalSamples == 0 || m_spInputFile == nullptr)
      return 0.0f;

   ogg_int64_t lCurrent = op_pcm_tell(m_spInputFile.get());
   float fValue = lCurrent / float(m_lTotalSamples);

   return fValue * 100.f;
}

void OpusInputModule::doneInput()
{
   m_spInputFile.reset();
}

LPCTSTR OpusInputModule::ErrorTextFromCode(int iErrorCode)
{
   switch (iErrorCode)
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

int OpusInputModule::ReadStream(void *_stream, unsigned char *_ptr, int _nbytes)
{
   FILE* fd = reinterpret_cast<FILE*>(_stream);
   return fread(_ptr, 1, _nbytes, fd);
}

int OpusInputModule::SeekStream(void *_stream, opus_int64 _offset, int _whence)
{
   FILE* fd = reinterpret_cast<FILE*>(_stream);
   return _fseeki64(fd, _offset, _whence);
}

opus_int64 OpusInputModule::PosStream(void *_stream)
{
   FILE* fd = reinterpret_cast<FILE*>(_stream);
   return _ftelli64(fd);
}

int OpusInputModule::CloseStream(void *_stream)
{
   FILE* fd = reinterpret_cast<FILE*>(_stream);
   return fclose(fd);
}
