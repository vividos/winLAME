//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file AacInputModule.cpp
/// \brief contains the implementation of the AAC input module
//
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "AacInputModule.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <ulib/DynamicLibrary.hpp>

using Encoder::AacInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

// channel remap stuff

const int MAX_CHANNELS = 6; ///< make this higher to support files with more channels

/// channel remapping map
const int chmap[MAX_CHANNELS][MAX_CHANNELS] = {
   { 0, },               // mono
   { 0, 1, },            // l, r
   { 1, 2, 0, },         // c, l, r -> l, r, c
   { 1, 2, 0, 3, },      // c, l, r, bc -> l, r, c, bc
   { 1, 2, 0, 3, 4, },   // c, l, r, bl, br -> l, r, c, bl, br
   { 1, 2, 0, 5, 3, 4 }  // c, l, r, bl, br, lfe -> l, r, c, lfe, bl, br
};

AacInputModule::AacInputModule()
   :m_decoder(nullptr),
   m_inputBufferHigh(0),
   m_inputFileLength(0),
   m_currentFilePos(0)
{
   m_moduleId = ID_IM_AAC;

   memset(&m_info, 0, sizeof(m_info));
}

Encoder::InputModule* AacInputModule::CloneModule()
{
   return new AacInputModule;
}

bool AacInputModule::IsAvailable() const
{
   return DynamicLibrary(_T("libfaad2.dll")).IsLoaded();
}

CString AacInputModule::GetDescription() const
{
   // determine object type
   LPCTSTR object = _T("???");
   switch (m_info.object_type)
   {
   case 0:
      object = _T("MAIN");
      break;
   case 1:
      object = _T("LC");
      break;
   case 2:
      object = _T("SSR");
      break;
   case 3:
      object = _T("LTP");
      break;
   case 4:
      object = _T("HE AAC");
      break;
   case 16:
      object = _T("ER_LC");
      break;
   case 18:
      object = _T("ER_LTP");
      break;
   case 22:
      object = _T("LD");
      break;
   case 26:
      object = _T("DRM_ER_LC");
      break;
   }

   // determine header type
   LPCTSTR header = _T("???");
   switch (m_info.headertype)
   {
   case 0:
      header = _T("RAW");
      break;
   case 1:
      header = _T("ADIF");
      break;
   case 2:
      header = _T("ADTS");
      break;
   case 3:
      header = _T("LATM");
      break;
   }

   CString desc;
   desc.Format(IDS_FORMAT_INFO_AAC_INFO,
      m_info.version,
      object,
      m_info.bitrate / 1000,
      m_info.sampling_rate,
      m_info.channels,
      header);

   return desc;
}

void AacInputModule::GetVersionString(CString& version, int special) const
{
   version = "libfaad2 " FAAD2_VERSION;
}

CString AacInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_AAC_INPUT);
   return filterString;
}

int AacInputModule::InitInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackInfo, SampleContainer& samples)
{
   // open infile
   m_inputFile.open(infilename, std::ios::in | std::ios::binary);
   if (!m_inputFile.is_open())
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   // find out AAC file infos
   {
      unsigned long* seekTable = nullptr;
      int seekTableLength = 0;

      get_AAC_format(infilename, &m_info, &seekTable, &seekTableLength, 1);
      free(seekTable);
   }

   // find out length of aac file
   struct _stat statbuf;
   ::_tstat(infilename, &statbuf);
   m_inputFileLength = statbuf.st_size; // 32 bit max.
   m_currentFilePos = 0;

   // search for begin of aac stream, skipping id3v2 tags; modifies m_currentFilePos
   // ...

   // retrieve id3 tag
   // ...

   // seek to begin
   m_inputFile.seekg(m_currentFilePos, std::ios::beg);

   // grab decoder instance
   m_decoder = NeAACDecOpen();

   // set output format
   {
      NeAACDecConfigurationPtr config;
      config = NeAACDecGetCurrentConfiguration(m_decoder);
      config->outputFormat = FAAD_FMT_16BIT;  // 32 bit sounds bad for some reason
      NeAACDecSetConfiguration(m_decoder, config);
   }

   // read first frame(s) and get infos about the aac file
   m_inputFile.read(reinterpret_cast<char*>(m_inputBuffer), c_aacInputBufferSize);

   unsigned long dummy;
   unsigned char dummy2;
   int result = NeAACDecInit(m_decoder, m_inputBuffer, sizeof(m_inputBuffer), &dummy, &dummy2);

   if (result < 0)
   {
      m_lastError.LoadString(IDS_ENCODER_ERROR_INIT_DECODER);
      return -2;
   }

   m_inputBufferHigh = 0;

   // seek to the next start
   m_currentFilePos += result;
   m_inputFile.seekg(m_currentFilePos, std::ios::beg);

   // get right file info (for HE AAC files)
   NeAACDecFrameInfo frameInfo;
   NeAACDecDecode(m_decoder, &frameInfo, m_inputBuffer, sizeof(m_inputBuffer));
   if (frameInfo.error > 0)
   {
      m_lastError = CString(NeAACDecGetErrorMessage(frameInfo.error));
      return -frameInfo.error;
   }
   else
   {
      m_info.sampling_rate = frameInfo.samplerate;
      m_info.channels = frameInfo.channels;
      m_info.object_type = frameInfo.object_type - 1;
   }

   // set up input traits
   samples.SetInputModuleTraits(
      16,
      SamplesInterleaved,
      m_info.sampling_rate,
      m_info.channels);

   return 0;
}

void AacInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   numChannels = m_info.channels;
   bitrateInBps = m_info.bitrate;
   lengthInSeconds = (m_inputFileLength << 3) / m_info.bitrate;
   samplerateInHz = m_info.sampling_rate;
}

int AacInputModule::DecodeSamples(SampleContainer& samples)
{
   // frame decoding info
   NeAACDecFrameInfo frameInfo;

   // temporary sample buffer
   short* outputBuffer;
   short tempBuffer[2048 * c_aacNumMaxChannels];

   // fill input buffer
   if (m_inputBufferHigh < c_aacInputBufferSize)
   {
      m_inputFile.read(reinterpret_cast<char*>(m_inputBuffer + m_inputBufferHigh), c_aacInputBufferSize - m_inputBufferHigh);
      int read = static_cast<int>(m_inputFile.gcount());

      if (read == 0 && m_inputBufferHigh == 0)
         return 0;

      m_inputBufferHigh += read;
      m_currentFilePos += read;
   }

   // decode buffer
   short* sampleBuffer = (short *)NeAACDecDecode(m_decoder, &frameInfo, m_inputBuffer, m_inputBufferHigh); //sizeof(m_inputBuffer));

   m_inputBufferHigh -= frameInfo.bytesconsumed;
   memmove(m_inputBuffer, m_inputBuffer + frameInfo.bytesconsumed,
      c_aacInputBufferSize - frameInfo.bytesconsumed);

   // check for return codes
   if (frameInfo.error > 0)
   {
      m_lastError = NeAACDecGetErrorMessage(frameInfo.error);
      return -frameInfo.error;
   }

   int numSamples = frameInfo.samples / frameInfo.channels;

   // remap channels
   if (frameInfo.channels > 2)
   {
      outputBuffer = tempBuffer;
      for (int i = 0; i < numSamples; i++)
         for (int j = 0; j < std::min(static_cast<int>(frameInfo.channels), MAX_CHANNELS); j++)
            outputBuffer[i * frameInfo.channels + j] = sampleBuffer[i * frameInfo.channels + (chmap[frameInfo.channels - 1][j])];

      // copy the remaining channels
      if (frameInfo.channels > MAX_CHANNELS)
      {
         for (int i = 0; i < numSamples; i++)
            for (int j = MAX_CHANNELS; j < frameInfo.channels; j++)
               outputBuffer[i * frameInfo.channels + j] = sampleBuffer[i * frameInfo.channels + j];
      }
   }
   else
      outputBuffer = sampleBuffer;

   // copy the samples to the sample container
   samples.PutSamplesInterleaved(outputBuffer, numSamples);

   return numSamples;
}

void AacInputModule::DoneInput()
{
   NeAACDecClose(m_decoder);
   m_inputFile.close();
}
