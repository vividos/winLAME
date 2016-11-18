//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2016 Michael Fink
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

/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include <opus/opus.h>
#include <opus/opus_multistream.h>
extern "C"
{
#include "opusenc.h"
#include "opus_header.h"
}

/// Opus output module
class OpusOutputModule: public OutputModule
{
public:
   // ctor
   OpusOutputModule();
   // dtor
   ~OpusOutputModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("Opus Encoder"); }

   // returns the last error
   virtual CString getLastError(){ return m_lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns version string
   virtual void getVersionString(CString& version, int special=0);

   // returns the extension the output module produces
   virtual LPCTSTR getOutputExtension(){ return _T("opus"); }

   // initializes the output module
   virtual int initOutput(LPCTSTR outfilename, SettingsManager &mgr,
      const TrackInfo& trackinfo, SampleContainer &samplecont);

   // encodes samples from the sample container
   virtual int encodeSamples(SampleContainer &samples);

   // cleans up the output module
   virtual void doneOutput();

private:
   /// initializes inopt struct
   void InitInopt();

   // set options from input stream
   void SetInputStreamInfos(SampleContainer& samplecont);

   /// stores track infos in comments
   bool StoreTrackInfos(const TrackInfo& trackinfo);

   /// initializes encoder
   bool InitEncoder();

   /// sets encoder options
   bool SetEncoderOptions();

   /// opens output file
   bool OpenOutputFile(LPCTSTR outputFilename);

   /// writes Opus header
   bool WriteHeader();

   /// reads float samples from 16-bit or 32-bit buffer
   static long ReadFloatSamplesStatic(void* src, float* buffer, int samples);

   /// reads float samples from 16-bit buffer
   long ReadFloatSamples16(float* buffer, int samples);

   /// reads float samples from 32-bit buffer
   long ReadFloatSamples32(float* buffer, int samples);

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
   CString m_lasterror;

   /// encoding options, from opusenc
   oe_enc_opt inopt;

   /// chosen bitrate, in bps (not Kbps)
   opus_int32 m_bitrateInBps;

   /// complexity value, from 0-10
   int m_complexity;

   /// bitrate mode: 0 VBR, 1 CVBR, 2 CBR
   int m_opusBitrateMode;

   /// output coding rate
   opus_int32 m_codingRate;

   /// indicates if input stream is downmixed; contains number of channels
   // after downmixing, or 0 for no downmixing
   int m_downmix;

   /// Opus header written
   OpusHeader header;

   /// Opus Multichannel Surround encoder instance
   std::shared_ptr<OpusMSEncoder> m_encoder;

   /// Ogg stream state
   ogg_stream_state os;

   /// current Ogg page
   ogg_page og;

   /// current Ogg packet
   ogg_packet op;

   /// buffer for packets created by encoder
   std::vector<unsigned char> m_packetBuffer;

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

   /// number of bytes written so far
   opus_int64 m_numBytesWritten;

   /// number of pages written so far
   opus_int64 m_numPagesWritten;

   /// output file
   std::shared_ptr<FILE> m_outputFile;

   /// indicates if the input module supports 32-bit samples
   bool m_32bitMode;

   /// end of stream flag
   int eos;

   /// total number of samples processed
   opus_int64 total_samples;

   /// current Ogg packet number, starting with 0
   ogg_int32_t m_currentPacketNumber;

   /// number of original samples, before padding
   ogg_int64_t original_samples;

   /// encoder granule pos
   ogg_int64_t enc_granulepos;

   /// last granule pos
   ogg_int64_t last_granulepos;

   /// last segments pos (?)
   int last_segments;

   /// maximum delay in number of samples
   int max_ogg_delay;
};

/// @}
