//
// winLAME - a frontend for the LAME encoding engine
// Copyright(c) 2004 Kjetil Haga
// Copyright(c) 2004 DeXT
// Copyright(c) 2007-2023 Michael Fink
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
/// \file MonkeysAudioInputModule.cpp
/// \author   Kjetil Haga (programmer.khaga@start.no)
/// \brief contains the Monkey's Audio input module
//
#include "stdafx.h"
#include "MonkeysAudioInputModule.hpp"
#include "resource.h"
#include <cstdio>
#include <cassert>
#include "AudioFileTag.hpp"

#define PLATFORM_WINDOWS
#include <monkeys-audio/MACDll.h>
#include <monkeys-audio/APETag.h>

using Encoder::MonkeysAudioInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

/// monkey namespace contains internal stuff used in MonkeysAudioInputModule
namespace MonkeysAudio
{
   /// internal struct with pointers to mac dll functions
   struct MonkeysAudioDll
   {
      /// ctor
      MonkeysAudioDll()
         :m_module(nullptr),
         Create(nullptr),
         Destroy(nullptr),
         GetData(nullptr),
         Seek(nullptr),
         GetInfo(nullptr)
      {
      }

      /// dtor
      ~MonkeysAudioDll()
      {
         if (m_module)
         {
            ::FreeLibrary(m_module);
         }
      }

      /// loads library
      void Load()
      {
         m_module = ::LoadLibraryA("MACDll.dll");

         if (m_module == nullptr)
            return;

         // load the functions
         Create = (proc_APEDecompress_Create)GetProcAddress(m_module, "c_APEDecompress_Create");
         Destroy = (proc_APEDecompress_Destroy)GetProcAddress(m_module, "c_APEDecompress_Destroy");
         GetData = (proc_APEDecompress_GetData)GetProcAddress(m_module, "c_APEDecompress_GetData");
         Seek = (proc_APEDecompress_Seek)GetProcAddress(m_module, "c_APEDecompress_Seek");
         GetInfo = (proc_APEDecompress_GetInfo)GetProcAddress(m_module, "c_APEDecompress_GetInfo");

         // error check (unload dll on failure)
         if (Create == nullptr ||
            Destroy == nullptr ||
            GetData == nullptr ||
            Seek == nullptr ||
            GetInfo == nullptr)
         {
            ::FreeLibrary(m_module);
            m_module = nullptr;
         }
      }

      /// returns if library is available
      bool IsAvail() const { return m_module != nullptr; }

      /// checks interface version
      bool CheckInterfaceVersion()
      {
         ATLASSERT(IsAvail());

         int ret = -1;
         proc_GetInterfaceCompatibility fnGetInterfaceCompatibility =
            (proc_GetInterfaceCompatibility)GetProcAddress(m_module, "GetInterfaceCompatibility");
         if (fnGetInterfaceCompatibility)
         {
            ret = fnGetInterfaceCompatibility(MAC_FILE_VERSION_NUMBER, TRUE, NULL);
         }

         return ret == 0;
      }

      /// module handle
      HMODULE m_module;

      // APEDecompress functions
      proc_APEDecompress_Create        Create;
      proc_APEDecompress_Destroy       Destroy;
      proc_APEDecompress_GetData       GetData;
      proc_APEDecompress_Seek          Seek;
      proc_APEDecompress_GetInfo       GetInfo;
   };

   /// buffer size for MonkeysAudio's audio module
   const int c_macBufferSize = 8192;

   /// encodes an error string based on macdll error code
   CString EncodeMonkeyErrorString(int errorCode)
   {
      CString desc = _T("APE error (");

      switch (errorCode)
      {
      case ERROR_IO_READ:
         desc += _T("I/O read error");
         break;
      case ERROR_IO_WRITE:
         desc += _T("I/O write error");
         break;
      case ERROR_INVALID_INPUT_FILE:
         desc += _T("invalid input file");
         break;
      case ERROR_INVALID_OUTPUT_FILE:
         desc += _T("invalid output file");
         break;
      case ERROR_INPUT_FILE_TOO_LARGE:
         desc += _T("input file file too large");
         break;
      case ERROR_INPUT_FILE_UNSUPPORTED_BIT_DEPTH:
         desc += _T("input file unsupported bit depth");
         break;
      case ERROR_INPUT_FILE_UNSUPPORTED_SAMPLE_RATE:
         desc += _T("input file unsupported sample rate");
         break;
      case ERROR_INPUT_FILE_UNSUPPORTED_CHANNEL_COUNT:
         desc += _T("input file unsupported channel count");
         break;
      case ERROR_INPUT_FILE_TOO_SMALL:
         desc += _T("input file too small");
         break;
      case ERROR_INVALID_CHECKSUM:
         desc += _T("invalid checksum");
         break;
      case ERROR_DECOMPRESSING_FRAME:
         desc += _T("decompressing frame");
         break;
      case ERROR_INITIALIZING_UNMAC:
         desc += _T("initializing unmac");
         break;
      case ERROR_INVALID_FUNCTION_PARAMETER:
         desc += _T("invalid function parameter");
         break;
      case ERROR_UNSUPPORTED_FILE_TYPE:
         desc += _T("unsupported file type");
         break;
      case ERROR_INSUFFICIENT_MEMORY:
         desc += _T("insufficient memory");
         break;
      case ERROR_LOADINGAPE_DLL:
         desc += _T("loading MACDll.dll");
         break;
      case ERROR_LOADINGAPE_INFO_DLL:
         desc += _T("loading MACinfo.dll");
         break;
      case ERROR_LOADING_UNMAC_DLL:
         desc += _T("loading UnMAC.dll");
         break;
      case ERROR_USER_STOPPED_PROCESSING:
         desc += _T("user stopped processing");
         break;
      case ERROR_SKIPPED:
         desc += _T("skipped...");
         break;
      case ERROR_BAD_PARAMETER:
         desc += _T("bad parameter");
         break;
      case ERROR_APE_COMPRESS_TOO_MUCH_DATA:
         desc += _T("APE compress too much data");
         break;
      default:
         desc += _T("undefined error");
         break;
      }

      desc.AppendFormat(_T("; code: %d)"), errorCode);

      return desc;
   }
}

// static references

MonkeysAudio::MonkeysAudioDll MonkeysAudioInputModule::s_dll;

MonkeysAudioInputModule::MonkeysAudioInputModule()
   :m_handle(0),
   m_numCurrentSamples(0),
   m_numTotalSamples(0)
{
   m_moduleId = ID_IM_MONKEYSAUDIO;
}

MonkeysAudioInputModule::~MonkeysAudioInputModule()
{
}

Encoder::InputModule* MonkeysAudioInputModule::CloneModule()
{
   return new MonkeysAudioInputModule;
}

bool MonkeysAudioInputModule::IsAvailable() const
{
   using namespace MonkeysAudio;

   s_dll.Load();

   if (!s_dll.IsAvail() ||
      !s_dll.CheckInterfaceVersion())
      return false;

   return true;
}

CString MonkeysAudioInputModule::GetDescription() const
{
   ATLASSERT(s_dll.IsAvail());

   // sanity check .. ;)
   if (!m_handle)
      return CString();

   // get fileinfo
   APE::int64 samplerateInHz = s_dll.GetInfo(m_handle, APE::APE_INFO_SAMPLE_RATE, 0, 0);
   APE::int64 numChannels = s_dll.GetInfo(m_handle, APE::APE_INFO_CHANNELS, 0, 0);
   APE::int64 level = s_dll.GetInfo(m_handle, APE::APE_INFO_COMPRESSION_LEVEL, 0, 0);

   LPCTSTR compressionLevel;
   switch (level)
   {
   case MAC_COMPRESSION_LEVEL_FAST:
      compressionLevel = _T("Fast compression");
      break;
   case MAC_COMPRESSION_LEVEL_NORMAL:
      compressionLevel = _T("Normal compression");
      break;
   case MAC_COMPRESSION_LEVEL_HIGH:
      compressionLevel = _T("High compression");
      break;
   case MAC_COMPRESSION_LEVEL_EXTRA_HIGH:
      compressionLevel = _T("Extra high compression");
      break;
   case MAC_COMPRESSION_LEVEL_INSANE:
      compressionLevel = _T("Insane compression");
      break;
   default:
      compressionLevel = _T("Unknown compression level.");
      break;
   }

   CString desc;
   desc.Format(IDS_FORMAT_INFO_MONKEYS_AUDIO,
      compressionLevel,
      samplerateInHz,
      numChannels);

   return desc;
}

void MonkeysAudioInputModule::GetVersionString(CString& version, int special) const
{
   version = MAC_VERSION_STRING;
}

CString MonkeysAudioInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_MONKEYS_AUDIO_INPUT);
   return filterString;
}

int MonkeysAudioInputModule::InitInput(LPCTSTR infilename,
   SettingsManager& mgr, TrackInfo& trackInfo,
   SampleContainer& samples)
{
   ATLASSERT(s_dll.IsAvail());

   // close old file if still open
   if (m_handle)
   {
      s_dll.Destroy(m_handle);
      m_handle = nullptr;
   }

   // internal retval used by macdll
   int retval = 0;

   // resets the sample count
   m_numCurrentSamples = 0;

   // get id3 tag
   GetTrackInfo(infilename, trackInfo);

   // opens the file for reading
   m_handle = s_dll.Create(CStringA(GetAnsiCompatFilename(infilename)), &retval);
   if (m_handle == nullptr)
   {
      m_lastError = MonkeysAudio::EncodeMonkeyErrorString(retval);
      return -1;
   }

   // get some fileinfo
   APE::int64 samplerateInHz = s_dll.GetInfo(m_handle, APE::APE_INFO_SAMPLE_RATE, 0, 0);
   APE::int64 numChannels = s_dll.GetInfo(m_handle, APE::APE_INFO_CHANNELS, 0, 0);
   APE::int64 bitrateInBps = s_dll.GetInfo(m_handle, APE::APE_INFO_BITS_PER_SAMPLE, 0, 0);

   // set total samples in file (for stats update)
   m_numTotalSamples = s_dll.GetInfo(m_handle, APE::APE_DECOMPRESS_TOTAL_BLOCKS, 0, 0);

   // set up input traits
   samples.SetInputModuleTraits(
      static_cast<int>(bitrateInBps),
      SamplesInterleaved,
      static_cast<int>(samplerateInHz),
      static_cast<int>(numChannels));

   return 0;
}

void MonkeysAudioInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   ATLASSERT(s_dll.IsAvail());

   if (m_handle == nullptr)
      return;

   int bitsPerSample = static_cast<int>(s_dll.GetInfo(m_handle, APE::APE_INFO_BITS_PER_SAMPLE, 0, 0));

   numChannels = static_cast<int>(s_dll.GetInfo(m_handle, APE::APE_INFO_CHANNELS, 0, 0));

   samplerateInHz = static_cast<int>(s_dll.GetInfo(m_handle, APE::APE_INFO_SAMPLE_RATE, 0, 0));

   lengthInSeconds = static_cast<int>(s_dll.GetInfo(m_handle, APE::APE_DECOMPRESS_LENGTH_MS, 0, 0) / 1000);

   bitrateInBps = samplerateInHz * bitsPerSample;
}

int MonkeysAudioInputModule::DecodeSamples(SampleContainer& samples)
{
   ATLASSERT(s_dll.IsAvail() && m_handle != nullptr);

   unsigned char buffer[MonkeysAudio::c_macBufferSize] = {};
   APE::int64 numBlocksRetrieved = 0;

   // get data from file
   APE::int64 blockalign = s_dll.GetInfo(m_handle, APE::APE_INFO_BLOCK_ALIGN, 0, 0);
   int retval = s_dll.GetData(m_handle, reinterpret_cast<char*>(buffer), MonkeysAudio::c_macBufferSize / blockalign, &numBlocksRetrieved);

   // success?
   if (retval != 0)
   {
      m_lastError = MonkeysAudio::EncodeMonkeyErrorString(retval);
      return -retval;
   }

   // if we are dealing with 8-bit samples, we have to convert them to signed samples
   // (8-bit samples from MonkeysAudio's audio are unsigned)
   APE::int64 bitsPerSample = s_dll.GetInfo(m_handle, APE::APE_INFO_BITS_PER_SAMPLE, 0, 0);
   if (8 == bitsPerSample)
   {
      for (int i = 0; i < MonkeysAudio::c_macBufferSize; ++i)
      {
         buffer[i] = ((buffer[i] & 0x80) ^ 0x80) | (buffer[i] & 0x7f);   // invert most significant bit
      }
   }

   // update stats
   m_numCurrentSamples += numBlocksRetrieved;

   // put samples in container
   samples.PutSamplesInterleaved(buffer, static_cast<int>(numBlocksRetrieved));

   // return samples retrieved
   return static_cast<int>(numBlocksRetrieved);
}

void MonkeysAudioInputModule::DoneInput()
{
   if (m_handle)
   {
      s_dll.Destroy(m_handle);
      m_handle = nullptr;
   }
}

bool MonkeysAudioInputModule::GetTrackInfo(LPCTSTR filename, TrackInfo& trackInfo)
{
   AudioFileTag tag(trackInfo);
   return tag.ReadFromFile(filename);
}
