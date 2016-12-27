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
   source.format = (SampleFormatType)-1;
   target.format = (SampleFormatType)-1;
}

SampleContainer::~SampleContainer()
{
   // clean up memory areas
   if (m_channelArray != nullptr)
   {
      for (int i = 0; i < target.numChannels; i++)
         delete m_channelArray[i];

      delete m_channelArray;
   }

   if (m_interleaved != nullptr)
      delete[] m_interleaved;
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

void *SampleContainer::GetSamplesInterleaved(int& numSamples)
{
   numSamples = m_numSamplesAvail;
   return m_interleaved;
}

void **SampleContainer::GetSamplesArray(int& numSamples)
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
         delete m_channelArray[i];
         m_channelArray[i] = new unsigned char[new_size];
      }
   }
   break;

   case SamplesInterleaved:
      delete[] m_interleaved;
      m_interleaved = new unsigned char[new_size * target.numChannels];
      break;
   }
}

void SampleContainer::InterleaveChannel(unsigned char*samples, int numSamples, int channel, int source_step)
{
   int sshift = 32 - source.bitsPerSample;
   int dbps = target.bitsPerSample >> 3;
   int dshift = 32 - target.bitsPerSample;
   int dest_step = target.numChannels * (target.bitsPerSample >> 3);

   int roundbit = dshift > 0 ? (1 << (dshift - 1)) : 0;
   int dest_high = (1 << 31) - roundbit;

   unsigned char *destbuf =
      ((unsigned char*)m_interleaved) + dbps*channel;

   for (int i = 0; i < numSamples; i++)
   {
      register signed int sample;

      sample = *((unsigned int*)samples);

      // normalize

      // shift up
      sample <<= sshift;

      // do sth with the sample

      // add rounding value
      if (sample < dest_high)
         sample += roundbit;

      // shift to target bps
      sample >>= dshift;

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

      destbuf += dest_step;
      samples += source_step;
   }
}

void SampleContainer::DeinterleaveChannel(unsigned char*samples, int numSamples, int channel, int source_step)
{
   int sshift = 32 - source.bitsPerSample;
   int dbps = target.bitsPerSample >> 3;
   int dshift = 32 - target.bitsPerSample;
   int dest_step = dbps;

   int roundbit = dshift > 0 ? (1 << (dshift - 1)) : 0;
   int dest_high = (1 << 31) - roundbit;

   unsigned char *destbuf = (unsigned char*)m_channelArray[channel];

   for (int i = 0; i < numSamples; i++)
   {
      register signed int sample;

      sample = *((unsigned int*)samples);

      // normalize

      // shift up
      sample <<= sshift;

      // do sth with the sample

      // add rounding value
      if (sample < dest_high)
         sample += roundbit;

      // shift to target bps
      sample >>= dshift;

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

      destbuf += dest_step;
      samples += source_step;
   }

}

/*
void SampleContainer::interleaveSamples(int channels,int bitpersample, int numSamples, void**samples)
//void InterleaveSamples(int channels, int len, short **buffer, short *destbuffer)
{
   void *destbuf = m_interleaved;
   if (channels==1)
   {
      // mono
      int len = numSamples * (bitpersample>>3);
      _asm
      {
         mov   ecx, len
         shr   ecx, 2

         mov   esi, samples
         mov   edi, destbuf
         rep   movsd            ; copy with 32 bit access

         mov   ecx, len
         and   ecx, 3
         test  ecx, ecx
         je    label1

         rep   movsb            ; copy remaining bytes
      label1:
      }
   }
   else
   if (channels==2)
   {
      // stereo
      void *sbuf0 = samples[0], *sbuf1 = samples[1];
      int step = (bitpersample>>3);
      _asm
      {
         mov   ebx, numSamples
         mov   esi, sbuf0
         mov   edx, sbuf1
         mov   edi, destbuf

      label2:
         mov   ecx, step   ; copy left sample
         rep   movsb

         xchg  edx, esi    ; swap source adresses

         mov   ecx, step   ; copy right sample
         rep   movsb

         xchg  edx, esi    ; swap source adresses

         dec   ebx
         jnz   label2
      }
   }
   else
   {
      // n-channel
      for(int ch=0; ch<channels; ch++)
      {
         int step = (bitpersample>>3);
         int dist = channels*step;
         void *curbuf = samples[ch];
         unsigned char *curdest = (unsigned char*)destbuf+ch*step;

         _asm
         {
            mov   ebx, numSamples
            mov   esi, curbuf
            mov   edi, curdest

         label3:
            mov   ecx, step   ; copy sample
            rep   movsb

            add   esi, dist   ; jump over samples of other channel

            dec   ebx
            jnz   label3
         }
      }
   }
}
*/
