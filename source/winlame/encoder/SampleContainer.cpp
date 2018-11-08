//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file SampleContainer.cpp
/// \brief functions to convert samples from and to interleaved and channel array format
//
#include "stdafx.h"
#include "SampleContainer.hpp"
#include <cstring>

using Encoder::SampleContainer;
using Encoder::SampleFormatType;

SampleContainer::SampleContainer()
   :m_channelArray(nullptr),
   m_interleaved(nullptr),
   m_numBytesAvail(0),
   m_numSamplesAvail(0)
{
   source.format = SamplesUnknown;
   target.format = SamplesUnknown;
}

SampleContainer::~SampleContainer()
{
   DeallocMemory();
}

void SampleContainer::SetInputModuleTraits(int bitsPerSample,
   SampleFormatType format, int samplerateInHz, int numChannels)
{
   // set up traits struct
   source.bitsPerSample = bitsPerSample;
   source.format = format;
   source.samplerateInHz = samplerateInHz;
   source.numChannels = numChannels;
}

void SampleContainer::SetOutputModuleTraits(int bitsPerSample,
   SampleFormatType format, int samplerateInHz, int numChannels)
{
   if (samplerateInHz == -1)
      samplerateInHz = source.samplerateInHz;
   if (numChannels == -1)
      numChannels = source.numChannels;

   // set up traits struct
   target.bitsPerSample = bitsPerSample;
   target.format = format;
   target.samplerateInHz = samplerateInHz;
   target.numChannels = numChannels;

   // initial value
   m_numBytesAvail = 512;

   // set up channel array or interleaved memory
   switch (format)
   {
   case SamplesChannelArray: // channel array
   {
      m_channelArray = new void*[numChannels];
      for (int i = 0; i < numChannels; i++)
      {
         m_channelArray[i] = new unsigned char[m_numBytesAvail * (bitsPerSample >> 3)];
      }
   }
   break;

   case SamplesInterleaved: // interleaved
      m_interleaved = new unsigned char[m_numBytesAvail * (bitsPerSample >> 3) * numChannels];
      break;
   }
}

void SampleContainer::PutSamplesInterleaved(void* samples, int numSamples)
{
   // check if there is enough space in the buffer
   if (numSamples > m_numBytesAvail)
      ReallocMemory(numSamples);

   // conversion from interleaved to ...

   switch (target.format)
   {
   case SamplesChannelArray: // channel array
   {
      for (int i = 0; i < source.numChannels; i++)
      {
         DeinterleaveChannel((unsigned char*)(samples)+i*(source.bitsPerSample >> 3),
            numSamples, i, source.numChannels*source.bitsPerSample >> 3);
      }
   }
   break;
   case SamplesInterleaved: // interleaved
   {
      for (int i = 0; i < source.numChannels; i++)
      {
         InterleaveChannel((unsigned char*)(samples)+i*(source.bitsPerSample >> 3),
            numSamples, i, source.numChannels*(source.bitsPerSample >> 3));
      }
   }
   break;
   }

   m_numSamplesAvail = numSamples;
}

void SampleContainer::PutSamplesArray(void** samples, int numSamples)
{
   // check if there is enough space in the buffer
   if (numSamples > m_numBytesAvail)
      ReallocMemory(numSamples);

   // conversion from channel array to ...

   switch (target.format)
   {
   case SamplesChannelArray: // channel array
   {
      for (int i = 0; i < source.numChannels; i++)
      {
         DeinterleaveChannel((unsigned char*)(samples[i]), numSamples,
            i, source.bitsPerSample >> 3);
      }
   }
   break;

   case SamplesInterleaved: // interleaved
   {
      for (int i = 0; i < source.numChannels; i++)
      {
         InterleaveChannel((unsigned char*)(samples[i]), numSamples,
            i, source.bitsPerSample >> 3);
      }
   }
   break;
   }
   m_numSamplesAvail = numSamples;
}

void* SampleContainer::GetSamplesInterleaved(int& numSamples)
{
   numSamples = m_numSamplesAvail;
   return m_interleaved;
}

void** SampleContainer::GetSamplesArray(int& numSamples)
{
   numSamples = m_numSamplesAvail;
   return m_channelArray;
}

void SampleContainer::ReallocMemory(int newSamples)
{
   m_numBytesAvail = newSamples;
   int new_size = m_numBytesAvail * (target.bitsPerSample >> 3);

   switch (target.format)
   {
   case SamplesChannelArray:
   {
      for (int i = 0; i < target.numChannels; i++)
      {
         delete[](unsigned char*)m_channelArray[i];
         m_channelArray[i] = new unsigned char[new_size];
      }
   }
   break;

   case SamplesInterleaved:
      delete[](unsigned char*)m_interleaved;
      m_interleaved = new unsigned char[new_size * target.numChannels];
      break;
   }
}

void SampleContainer::DeallocMemory()
{
   if (m_channelArray != nullptr)
   {
      for (int i = 0; i < target.numChannels; i++)
         delete[](unsigned char*)m_channelArray[i];

      delete[] m_channelArray;
      m_channelArray = nullptr;
   }

   if (m_interleaved != nullptr)
   {
      delete[](unsigned char*)m_interleaved;

      m_interleaved = nullptr;
   }

   source.format = SamplesUnknown;
   target.format = SamplesUnknown;
}

void SampleContainer::InterleaveChannel(unsigned char* samples, int numSamples, int channel, int sourceStep)
{
   int sourceShift = 32 - source.bitsPerSample;
   int dbps = target.bitsPerSample >> 3;
   int destShift = 32 - target.bitsPerSample;
   int destStep = target.numChannels * (target.bitsPerSample >> 3);

   int roundbit = destShift > 0 ? (1 << (destShift - 1)) : 0;
   int destHigh = std::numeric_limits<int>::max() - roundbit + 1;

   unsigned char *destbuf =
      ((unsigned char*)m_interleaved) + dbps*channel;

   for (int i = 0; i < numSamples; i++)
   {
      signed int sample;

      sample = *((unsigned int*)samples);

      // normalize

      // shift up
      sample <<= sourceShift;

      // do sth with the sample

      // add rounding value
      if (sample < destHigh)
         sample += roundbit;

      // shift to target bps
      sample >>= destShift;

      // store sample in destbuf
      _asm
      {
         mov   edi, destbuf
         mov   eax, sample
         mov   ecx, dbps
         label1 :
         stosb
            shr   eax, 8
            loop  label1
      }

      destbuf += destStep;
      samples += sourceStep;
   }
}

void SampleContainer::DeinterleaveChannel(unsigned char*samples, int numSamples, int channel, int sourceStep)
{
   int sourceShift = 32 - source.bitsPerSample;
   int dbps = target.bitsPerSample >> 3;
   int destShift = 32 - target.bitsPerSample;
   int destStep = dbps;

   int roundbit = destShift > 0 ? (1 << (destShift - 1)) : 0;
   int destHigh = std::numeric_limits<int>::max() - roundbit + 1;

   unsigned char *destbuf = (unsigned char*)m_channelArray[channel];

   for (int i = 0; i < numSamples; i++)
   {
      signed int sample;

      sample = *((unsigned int*)samples);

      // normalize

      // shift up
      sample <<= sourceShift;

      // do sth with the sample

      // add rounding value
      if (sample < destHigh)
         sample += roundbit;

      // shift to target bps
      sample >>= destShift;

      // store sample in destbuf
      _asm
      {
         mov   edi, destbuf
         mov   eax, sample
         mov   ecx, dbps
         label1 :
         stosb
            shr   eax, 8
            loop  label1
      }

      destbuf += destStep;
      samples += sourceStep;
   }

}
