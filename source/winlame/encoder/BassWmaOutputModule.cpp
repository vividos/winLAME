//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2004 DeXT
// Copyright (c) 2009-2018 Michael Fink
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
/// \file BassWmaOutputModule.cpp
/// \brief contains the implementation of the basswma output module
//
#include "stdafx.h"
#include "resource.h"
#include "BassWmaOutputModule.hpp"
#include <ulib/DynamicLibrary.hpp>
#include <ulib/UTF8.hpp>
#include "App.hpp"
#include <wmsdk.h> // for WM_PICTURE

using Encoder::BassWmaOutputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

extern std::atomic<unsigned int> s_bassApiusageCount;

BassWmaOutputModule::BassWmaOutputModule()
   :m_handle(0),
   m_samplerateInHz(0),
   m_numChannels(0),
   m_bitrateInBps(0),
   m_quality(0),
   m_bitrateMode(0)
{
   m_moduleId = ID_OM_BASSWMA;
}

bool BassWmaOutputModule::IsAvailable() const
{
   DynamicLibrary bassLib(_T("bass.dll"));
   DynamicLibrary bassWmaLib(_T("basswma.dll"));

   return bassLib.IsLoaded() && bassWmaLib.IsLoaded();
}

CString BassWmaOutputModule::GetDescription() const
{
   CString desc;
   desc.Format(IDS_FORMAT_INFO_BASS_WMA_OUTPUT,
      m_bitrateMode == 1 ? _T("Quality ") : _T(""),
      m_bitrateMode == 1 ? m_quality : m_bitrateInBps / 1000,
      m_bitrateMode == 1 ? _T("%") : _T(" kbps"),
      m_bitrateMode == 1 ? _T("VBR") : _T("CBR"),
      m_samplerateInHz,
      m_numChannels);

   return desc;
}

void BassWmaOutputModule::GetVersionString(CString& version, int special) const
{
   version.Empty();

   HPLUGIN plugin = BASS_PluginLoad(_T("basswma"), 0);
   const BASS_PLUGININFO* pluginInfo = BASS_PluginGetInfo(plugin);

   DWORD bassVersion = pluginInfo->version;

   BASS_PluginFree(plugin);

   version.Format(_T("%u.%u.%u.%u"),
      HIBYTE(HIWORD(bassVersion)),
      LOBYTE(HIWORD(bassVersion)),
      HIBYTE(LOWORD(bassVersion)),
      LOBYTE(LOWORD(bassVersion)));
}

void BassWmaOutputModule::PrepareOutput(SettingsManager& mgr)
{
   m_bitrateMode = mgr.QueryValueInt(WmaBitrateMode);
   switch (m_bitrateMode)
   {
   case 1: // VBR
      m_quality = mgr.QueryValueInt(WmaQuality);
      break;

   case 0: // CBR
   default:
      m_bitrateInBps = mgr.QueryValueInt(WmaBitrate) * 1000;
      break;
   }
}

int BassWmaOutputModule::InitOutput(LPCTSTR outfilename,
   SettingsManager& mgr, const TrackInfo& trackInfo,
   SampleContainer& samples)
{
   m_samplerateInHz = samples.GetInputModuleSampleRate();
   m_numChannels = samples.GetInputModuleChannels();

   // check that BASS 2.4 was loaded
   if ((HIWORD(BASS_GetVersion()) != BASSVERSION))
   {
      m_lastError.LoadString(IDS_BASS_DLL_ERROR_VERSION_MISMATCH);
      return -1;
   }

   // not playing anything, so don't need an update thread
   BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);

   // setup output - "no sound" device, 44100hz, stereo, 16 bits
   if (s_bassApiusageCount++ == 0)
   {
      if (!BASS_Init(0, m_samplerateInHz, 0, 0, NULL))
      {
         m_lastError.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);
         return -1;
      }
   }

   // find nearest valid bitrate
   DWORD bitrateOrQuality = m_bitrateMode == 1 ? m_bitrateInBps : m_quality;
   bool validRate = false;
   DWORD nearestRate = 0;
   const DWORD* availableRates = BASS_WMA_EncodeGetRates(m_samplerateInHz, m_numChannels, m_bitrateMode == 1 ? BASS_WMA_ENCODE_RATES_VBR : 0);
   while (availableRates && *availableRates) {
      if (bitrateOrQuality == *availableRates)
      {
         validRate = true;
         break;
      }

      if (bitrateOrQuality < *availableRates || nearestRate == 0)
         nearestRate = *availableRates;
      availableRates++;
   }

   if (!validRate)
      bitrateOrQuality = nearestRate;

   // opens the file for writing
   m_handle = BASS_WMA_EncodeOpenFile(m_samplerateInHz, m_numChannels, 0, bitrateOrQuality, CStringA(outfilename));
   if (m_handle == NULL)
   {
      CString errorText;
      int errorCode = BASS_ErrorGetCode();
      switch (errorCode)
      {
      case BASS_ERROR_ILLPARAM:
         errorText = _T("BASS_ERROR_ILLPARAM");
         break;
      case BASS_ERROR_NOTAVAIL:
         errorText = _T("BASS_ERROR_NOTAVAIL");
         break;
      case BASS_ERROR_CREATE:
         errorText = _T("BASS_ERROR_CREATE");
         break;
      case BASS_ERROR_UNKNOWN:
         errorText = _T("BASS_ERROR_UNKNOWN");
         break;
      default:
         errorText.Format(_T("BASS_ErrorCode: %i"), errorCode);
         break;
      }

      m_lastError.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);
      m_lastError.AppendFormat(_T(" (%s)"), errorText.GetString());
      return -1;
   }

   // add track properties
   AddTrackInfo(trackInfo);

   // set up output traits
   samples.SetOutputModuleTraits(16, SamplesInterleaved);

   return 0;
}

int BassWmaOutputModule::EncodeSamples(SampleContainer& samples)
{
   // get samples
   int numSamples = 0;
   void* sampleBuffer = samples.GetSamplesInterleaved(numSamples);

   // write samples
   int ret = numSamples * sizeof(short) * m_numChannels; // bytes
   if (!BASS_WMA_EncodeWrite(m_handle, sampleBuffer, ret))
   {
      m_lastError.Format(IDS_ENCODER_ERROR_WRITE_OUTPUT);
      return -1;
   }

   return ret;
}

void BassWmaOutputModule::DoneOutput()
{
   BASS_WMA_EncodeClose(m_handle);

   if (--s_bassApiusageCount == 0)
   {
      BASS_Free();
   }
}

void BassWmaOutputModule::AddTrackInfo(const TrackInfo& trackInfo)
{
   bool isAvail = false;

   std::vector<char> utf8Buffer;

   CString textValue = trackInfo.GetTextInfo(TrackInfoTitle, isAvail);
   if (isAvail && !textValue.IsEmpty())
   {
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "Title", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   textValue = trackInfo.GetTextInfo(TrackInfoArtist, isAvail);
   if (isAvail && !textValue.IsEmpty())
   {
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "Author", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   textValue = trackInfo.GetTextInfo(TrackInfoDiscArtist, isAvail);
   if (isAvail && !textValue.IsEmpty())
   {
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "WM/AlbumArtist", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   textValue = trackInfo.GetTextInfo(TrackInfoComposer, isAvail);
   if (isAvail && !textValue.IsEmpty())
   {
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "WM/Composer", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   textValue = trackInfo.GetTextInfo(TrackInfoAlbum, isAvail);
   if (isAvail && !textValue.IsEmpty())
   {
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "WM/AlbumTitle", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   int intValue = trackInfo.GetNumberInfo(TrackInfoYear, isAvail);
   if (isAvail && intValue != -1)
   {
      textValue.Format(_T("%i"), intValue);
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "WM/Year", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   textValue = trackInfo.GetTextInfo(TrackInfoComment, isAvail);
   if (isAvail && !textValue.IsEmpty())
   {
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "Description", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   intValue = trackInfo.GetNumberInfo(TrackInfoTrack, isAvail);
   if (isAvail && intValue != -1)
   {
      textValue.Format(_T("%i"), intValue);
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "WM/TrackNumber", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   intValue = trackInfo.GetNumberInfo(TrackInfoDiscNumber, isAvail);
   if (isAvail && intValue != -1)
   {
      textValue.Format(_T("%i"), intValue);
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "WM/PartOfSet", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   textValue = trackInfo.GetTextInfo(TrackInfoGenre, isAvail);
   if (isAvail && !textValue.IsEmpty())
   {
      StringToUTF8(textValue, utf8Buffer);
      BASS_WMA_EncodeSetTag(m_handle, "WM/Genre", utf8Buffer.data(), BASS_WMA_TAG_UTF8);
   }

   BASS_WMA_EncodeSetTag(m_handle, "WM/ToolName", "winLAME", BASS_WMA_TAG_UTF8);

   CStringA version(App::Version());
   BASS_WMA_EncodeSetTag(m_handle, "WM/ToolVersion", version, BASS_WMA_TAG_UTF8);

   std::vector<unsigned char> binaryInfo;
   isAvail = trackInfo.GetBinaryInfo(TrackInfoFrontCover, binaryInfo);
   if (isAvail)
   {
      // BASS WMA just takes a WM_PICTURE struct and hands it over to the WMA
      // SDK, so just prepare that struct, call BASS_WMA_EncodeSetTag with the
      // tag type binary and the size of the WM_PICTURE struct. The WMA SDK
      // then writes the infos from the struct as the correct byte stream.
      WM_PICTURE wmPictureData = { 0 };

      wmPictureData.bPictureType = 3; // 3: front cover
      wmPictureData.pwszMIMEType = L"image/jpeg";
      wmPictureData.pwszDescription = L"";
      wmPictureData.pbData = binaryInfo.data();
      wmPictureData.dwDataLen = binaryInfo.size();

      DWORD type = MAKELONG(BASS_WMA_TAG_BINARY, sizeof(wmPictureData));

      BASS_WMA_EncodeSetTag(m_handle,
         "WM/Picture",
         reinterpret_cast<const char*>(&wmPictureData),
         type);
   }

   BASS_WMA_EncodeSetTag(m_handle, 0, 0, BASS_WMA_TAG_UTF8);
}
