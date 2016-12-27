//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file SampleContainer.hpp
/// \brief contains a sample container class
/// \details the sample container can convert from various formats and bits per sample,
/// as well as doing per-sample-operations
//
#pragma once

namespace Encoder
{
   /// sample format type
   enum SampleFormatType
   {
      SamplesChannelArray = 0,
      SamplesInterleaved = 1
   };

   /// module traits info
   struct ModuleTraits
   {
      /// bits per sample
      int bitsPerSample;

      /// sample format; 0 = channel array; 1 = interleaved
      SampleFormatType format;

      /// sample rate in Hz
      int samplerateInHz;

      /// number of channels
      int numChannels;
   };

   /// sample container class
   class SampleContainer
   {
   public:
      /// ctor
      SampleContainer();

      /// dtor
      ~SampleContainer();

      // input module functions

      /// sets traits of the input module
      void SetInputModuleTraits(int bitsPerSample, SampleFormatType format,
         int samplerateInHz, int numChannels);

      /// returns the input module sample rate
      int GetInputModuleSampleRate() { return source.samplerateInHz; }

      /// returns the input module number of channels
      int GetInputModuleChannels() { return source.numChannels; }

      /// returns the input module bits per sample
      int GetInputModuleBitsPerSample() { return source.bitsPerSample; }

      // output module functions

      /// sets traits of the output module
      void SetOutputModuleTraits(int bitsPerSample, SampleFormatType format,
         int samplerateInHz = -1, int numChannels = -1);

      /// returns the input module sample rate
      int GetOutputModuleSampleRate() { return target.samplerateInHz; }

      /// returns the input module number of channels
      int GetOutputModuleChannels() { return target.numChannels; }

      /// returns the input module bits per sample
      int GetOutputModuleBitsPerSample() { return target.bitsPerSample; }

      // functions to put samples in or get samples out

      /// stores samples in interleaved format in the sample container
      void PutSamplesInterleaved(void* samples, int numSamples);

      /// stores samples in interleaved format in the sample container
      void PutSamplesArray(void **samples, int numSamples);

      /// retrieves samples in interleaved format
      void* GetSamplesInterleaved(int& numSamples);

      /// retrieves samples in channel array format
      void** GetSamplesArray(int& numSamples);

   private:
      /// reallocates internal output buffers
      void ReallocMemory(int newSampleSize);

      /// converts samples to interleaved target buffer
      void InterleaveChannel(unsigned char* samples, int numSamples, int numChannels, int sourceStep);

      /// converts samples to channel array target buffer
      void DeinterleaveChannel(unsigned char* samples, int numSamples, int numChannels, int sourceStep);

   private:
      /// source traits
      ModuleTraits source;

      /// target traits
      ModuleTraits target;

      /// channel array samples memory
      void** m_channelArray;

      /// interleaved samples memory
      void* m_interleaved;

      /// number of bytes available in the buffer(s)
      int m_numBytesAvail;

      /// number of available samples
      int m_numSamplesAvail;
   };

} // namespace Encoder
