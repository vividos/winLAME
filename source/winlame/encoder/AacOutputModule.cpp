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
/// \file AacOutputModule.cpp
/// \brief contains the implementation of the AAC output module
//
#include "stdafx.h"
#include <fstream>
#include "resource.h"
#include "AacOutputModule.hpp"
#include "neaacdec.h"
#include <ulib/DynamicLibrary.hpp>
#include "ChannelRemapper.hpp"

using Encoder::AacOutputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

// AacOutputModule methods

AacOutputModule::AacOutputModule()
{
   m_moduleId = ID_OM_AAC;
}

bool AacOutputModule::IsAvailable() const
{
   return DynamicLibrary(_T("faac-1.dll")).IsLoaded();
}

CString AacOutputModule::GetDescription() const
{
   if (m_handle == nullptr)
      return CString();

   faac_encoder_info encoderInfo = {};
   encoderInfo.struct_size = sizeof(faac_encoder_info);

   faac_encoder_get_info(m_handle.get(), &encoderInfo);

   CString desc;
   desc.Format(IDS_FORMAT_INFO_AAC_OUTPUT,
      m_params.mpeg_version == faac_mpeg_version::FAAC_MPEG4 ? 4 : 2,
      (m_bitrateControlMethod == 0) ? _T("Quality ") : _T(""),
      (m_bitrateControlMethod == 0) ? m_params.quant_quality : m_params.bit_rate / 1000,
      (m_bitrateControlMethod == 0) ? _T("") : _T(" kbps/channel"),
      m_params.num_channels,
      m_params.bandwidth,
      m_params.joint_mode == faac_joint_mode::FAAC_JOINT_MS ? _T(", Mid/Side") : _T(""),
      m_params.use_tns == 1 ? _T(", Temporal Noise Shaping") : _T(""),
      m_params.use_lfe == 1 ? _T(", LFE channel") : _T(""));

   return desc;
}

void AacOutputModule::GetVersionString(CString& version, int special) const
{
   faac_library_info libraryInfo = {};
   libraryInfo.struct_size = sizeof(faac_library_info);

   faac_get_library_info(&libraryInfo);

   version.Format(_T("%hs"), libraryInfo.version);
}

int AacOutputModule::InitOutput(LPCTSTR outfilename,
   SettingsManager& mgr, const TrackInfo& trackInfo,
   SampleContainer& samples)
{
   m_outputFile.open(outfilename, std::ios::out | std::ios::binary);
   if (!m_outputFile.is_open())
   {
      m_lastError.LoadString(IDS_ENCODER_OUTPUT_FILE_CREATE_ERROR);
      return -1;
   }

   m_samplerate = samples.GetInputModuleSampleRate();
   m_channels = samples.GetInputModuleChannels();

   faac_params_init(&m_params);

   m_params.sample_rate = samples.GetInputModuleSampleRate();
   m_params.num_channels = samples.GetInputModuleChannels();
   m_params.mpeg_version =
      mgr.QueryValueInt(AacMpegVersion) == 4
      ? faac_mpeg_version::FAAC_MPEG4
      : faac_mpeg_version::FAAC_MPEG2;

   int objectType = mgr.QueryValueInt(AacObjectType);
   m_params.object_type =
      objectType == 1
      ? faac_object_type::FAAC_OBJ_LOW
      : faac_object_type::FAAC_OBJ_AUTO;

   int allowMidside = mgr.QueryValueInt(AacAllowMS);
   m_params.joint_mode = allowMidside
      ? faac_joint_mode::FAAC_JOINT_MS
      : faac_joint_mode::FAAC_JOINT_NONE;

   m_params.use_lfe = mgr.QueryValueInt(AacUseLFEChan) == 1;
   m_params.use_tns = mgr.QueryValueInt(AacUseTNS) == 1;
   m_params.short_control = faac_shortctl_mode::FAAC_SHORTCTL_NORMAL;
   m_params.input_format = faac_input_format::FAAC_INPUT_16BIT;

   // set bandwidth
   if (mgr.QueryValueInt(AacAutoBandwidth))
   {
      m_params.bandwidth = 0;
   }
   else
   {
      m_params.bandwidth = mgr.QueryValueInt(AacBandwidth);
   }

   // set bitrate/quality
   m_bitrateControlMethod = mgr.QueryValueInt(AacBRCMethod);
   if (m_bitrateControlMethod == 0) // Quality
   {
      m_params.quant_quality = mgr.QueryValueInt(AacQuality);
      m_params.bit_rate = 0;
   }
   else // Bitrate
   {
      m_params.quant_quality = 0;
      m_params.bit_rate = mgr.QueryValueInt(AacBitrate) * 1000 / m_params.num_channels;
   }

   // channel remap
   size_t maxChannelIndex = std::min<size_t>(
      m_params.num_channels,
      ChannelRemapper::GetMaxMappedChannel());

   std::vector< int32_t> channelMap(maxChannelIndex);

   for (size_t channelIndex = 0; channelIndex < maxChannelIndex; channelIndex++)
   {
      channelMap[channelIndex] = (int32_t)ChannelRemapper::GetMappedChannel(
         T_enChannelMapType::aacOutputChannelMap,
         m_params.num_channels,
         channelIndex);
   }

   m_params.channel_map = channelMap.data();
   m_params.channel_map_count = (uint32_t)channelMap.size();

   faac_encoder* handle = nullptr;
   faac_status status = faac_encoder_open(
      &m_params, &handle);

   if (handle != nullptr)
      m_handle = std::shared_ptr<faac_encoder>(
         handle,
         [](faac_encoder* handle) { faac_encoder_close(&handle); });

   if (m_handle == nullptr ||
      status < FAAC_OK)
   {
      m_lastError.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);

      const char* error = faac_strerror(status);
      m_lastError.AppendFormat(_T(" (%hs)"), error);

      return status;
   }

   faac_encoder_info encoderInfo = {};
   encoderInfo.struct_size = sizeof(faac_encoder_info);

   status = faac_encoder_get_info(m_handle.get(), &encoderInfo);

   m_inputBufferSize = encoderInfo.frame_samples * m_params.num_channels;
   m_outputBufferSize = encoderInfo.max_output_bytes;

   // alloc memory for input buffer
   m_sampleBuffer.resize(m_inputBufferSize);
   m_sampleBufferHigh = 0;

   // alloc memory for output buffer
   m_outputBuffer.resize(m_outputBufferSize);

   // set up output traits
   samples.SetOutputModuleTraits(16, SamplesInterleaved);

   return 0;
}

int AacOutputModule::EncodeSamples(SampleContainer& samples)
{
   // get input samples
   int numInputSamplesPerChannel = 0;
   short* inputSampleBuffer = (short*)samples.GetSamplesInterleaved(numInputSamplesPerChannel);

   // as faac_encoder_encode() always wants 'm_inputBufferSize' number of samples, we
   // have to store samples until a whole block of samples can be passed to
   // the function; otherwise faac_encoder_encode() would pad the buffer with 0's.

   size_t numInputSamples = numInputSamplesPerChannel * m_params.num_channels;
   size_t inputSamplePos = 0;

   //ATLTRACE(_T("AacOutputModule: encoding 0x%04x fresh input samples\n"), numInputSamples);

   // fill the sample buffer from the input samples
   do
   {
      size_t numSamplesToTransfer = std::min((size_t)m_inputBufferSize - m_sampleBufferHigh, numInputSamples - inputSamplePos);
      if (numSamplesToTransfer > 0)
      {
         //ATLTRACE(_T("AacOutputModule: copying 0x%04x bytes to sample buffer\n"), numSamplesToTransfer);
         memcpy(m_sampleBuffer.data() + m_sampleBufferHigh, inputSampleBuffer + inputSamplePos, numSamplesToTransfer * sizeof(short));
         m_sampleBufferHigh += numSamplesToTransfer;
         inputSamplePos += numSamplesToTransfer;
      }

      // if the sample buffer is full, encode one frame
      if (m_inputBufferSize == m_sampleBufferHigh)
      {
         // encode the samples
         EncodeAndWrite(m_inputBufferSize);
      }

      // loop while the input samples are exhausted
   } while (inputSamplePos < numInputSamples);

   //ATLTRACE(_T("AacOutputModule: finished encoding samples, 0x%04x samples are left in buffer\n"), m_sampleBufferHigh);

   return numInputSamples;
}

int AacOutputModule::EncodeAndWrite(unsigned long inputBufferSize)
{
   uint32_t bytesWritten = 0;
   faac_status status = faac_encoder_encode(
      m_handle.get(),
      reinterpret_cast<int*>(m_sampleBuffer.data()),
      inputBufferSize,
      m_outputBuffer.data(),
      m_outputBuffer.size(),
      &bytesWritten);

   //ATLTRACE(_T("AacOutputModule: encoding the samples to %i output bytes\n"), bytesWritten);

   if (status < FAAC_OK)
   {
      const char* error = faac_strerror(status);
      m_lastError.Format(_T("%hs"), error);

      return status;
   }

   // write the output buffer
   if (bytesWritten > 0)
   {
      m_outputFile.write(
         reinterpret_cast<char*>(m_outputBuffer.data()),
         bytesWritten);
   }

   m_sampleBufferHigh = 0;

   return FAAC_OK;
}

void AacOutputModule::DoneOutput()
{
   uint32_t bytesWritten = 0;

   // encode the last samples in sample buffer
   if (m_sampleBufferHigh > 0)
   {
      //ATLTRACE(_T("AacOutputModule: encoding remaining 0x%04x samples that were left in buffer\n"), m_sampleBufferHigh);
      EncodeAndWrite(m_sampleBufferHigh);
   }

   // finish encoding and write the last aac frames
   while (faac_encoder_encode(
      m_handle.get(),
      nullptr,
      0,
      m_outputBuffer.data(),
      m_outputBuffer.size(),
      &bytesWritten) >= FAAC_OK &&
      bytesWritten > 0)
   {
      m_outputFile.write(
         reinterpret_cast<char*>(m_outputBuffer.data()),
         bytesWritten);
   }

   m_outputFile.close();

   m_handle.reset();
}
