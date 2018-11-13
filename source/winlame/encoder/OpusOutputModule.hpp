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
/// \file OpusOutputModule.hpp
/// \brief Opus output module
//
#pragma once

#include "ModuleInterface.hpp"
#include <opus/opusenc.h>


namespace Encoder
{
   /// Opus encoding data
   struct OpusEncData
   {
      /// ctor
      OpusEncData();
      /// dtor; also cleans up OggOpusEnc
      ~OpusEncData();

      bool Create(opus_int32 inputSampleRate, int channels, CString& lastError);

      void Close();

      /// libopusenc encoding instance
      OggOpusEnc* enc;

      /// Opus comments
      std::shared_ptr<OggOpusComments> m_comments;

      /// output file
      std::shared_ptr<FILE> m_outputFile;

      opus_int64 total_bytes;
      opus_int64 bytes_written;
      opus_int64 nb_encoded;
      opus_int64 pages_out;
      opus_int64 packets_out;
      opus_int32 peak_bytes;
      opus_int32 min_bytes;
      opus_int32 last_length;
      opus_int32 nb_streams;
      opus_int32 nb_coupled;

      static int write_callback(void* user_data, const unsigned char* ptr, opus_int32 len);
      static int close_callback(void* user_data);
      static void packet_callback(void* user_data, const unsigned char* packet_ptr, opus_int32 packet_len, opus_uint32 flags);
   };

   /// Opus output module
   class OpusOutputModule : public OutputModule
   {
   public:
      /// ctor
      OpusOutputModule();
      /// dtor
      virtual ~OpusOutputModule();

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("Opus Encoder"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual CString GetDescription() const override;

      /// returns version string
      virtual void GetVersionString(CString& version, int special = 0) const override;

      /// returns the extension the output module produces
      virtual CString GetOutputExtension() const override { return _T("opus"); }

      /// initializes the output module
      virtual int InitOutput(LPCTSTR outfilename, SettingsManager& mgr,
         const TrackInfo& trackInfo, SampleContainer& samples) override;

      /// encodes samples from the sample container
      virtual int EncodeSamples(SampleContainer& samples) override;

      /// cleans up the output module
      virtual void DoneOutput() override;

      /// generates text for a picture metadata block from an image
      static std::string GetMetadataBlockPicture(const std::vector<unsigned char>& imageData);

   private:
      /// stores track infos in comments
      bool StoreTrackInfos(const TrackInfo& trackInfo);

      /// initializes encoder
      bool InitEncoder();

      /// sets up downmixing to stereo or mono
      bool SetupDownmix(size_t inputNumChannels, size_t outputNumChannels);

      /// sets encoder options
      bool SetEncoderOptions();

      /// opens output file
      bool OpenOutputFile(LPCTSTR outputFilename);

      /// reads float samples from 16-bit buffer
      long ReadFloatSamples16(float* buffer, int samples);

      /// reads float samples from 32-bit buffer
      long ReadFloatSamples32(float* buffer, int samples);

      /// downmix samples in input float sample buffer
      void DownmixSamples(opus_int32& numSamplesPerChannel);

      /// refills input sample buffer from sample container
      int RefillInputSampleBuffer(SampleContainer& samples);

      /// feeds the input buffer to the encoder until it doesn't hold a complete frame anymore
      bool EncodeInputBufferUntilEmpty();

      /// encodes remaining input buffer, even when it isn't a full frame anymore
      void EncodeRemainingInputBuffer();

      /// encodes input buffer samples for one frame with encoder
      bool EncodeInputBufferFrame();

   private:
      /// last error occured
      CString m_lastError;

      /// encoding options and data
      OpusEncData m_encoder;

      /// chosen bitrate, in bps (not Kbps)
      opus_int32 m_bitrateInBps;

      /// complexity value, from 0-10
      int m_complexity;

      /// bitrate mode: 0 VBR, 1 CVBR, 2 CBR
      int m_opusBitrateMode;

      opus_int32 m_inputSampleRate;

      /// number of bits in input samples
      unsigned int m_inputSampleSize;

      /// output coding rate
      opus_int32 m_codingRate;

      /// indicates if input stream is downmixed; contains number of channels
      /// after downmixing, or 0 for no downmixing
      /// 0: default, off; 1: --downmix-mono; 2: --downmix-stereo; -1: --no-downmix
      int m_downmix;

      /// number of samples for one frame, per channel
      opus_int32 m_frameSize;

      /// number of samples per frame we should feed the encoder with, for all channels
      opus_int32 m_numSamplesPerFrame;

      /// input buffer for 16-bit samples
      std::vector<opus_int16> m_inputInt16Buffer;

      /// input buffer for 32-bit samples
      std::vector<opus_int32> m_inputInt32Buffer;

      /// input buffer for float samples; contains at most one frame
      std::vector<float> m_inputFloatBuffer;

      /// matrix for factors for downmixing channels
      std::vector<float> m_downmixMatrix;

      /// buffer for downmixed float samples
      std::vector<float> m_downmixFloatBuffer;

      /// indicates if the input module supports 32-bit samples
      bool m_32bitMode;

      /// indicates if the output stream is at the end
      bool m_outputStreamAtEnd;
   };

} /// namespace Encoder
