/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2004 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: wlSampleContainer.cpp,v 1.8 2009/10/25 14:03:26 vividos Exp $

*/
/*! \file wlSampleContainer.cpp

   \brief functions to convert samples from and to interleaved and channel array format

*/

// needed includes
#include "stdafx.h"
#include "wlSampleContainer.h"
#include <cstring>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


wlSampleContainer::wlSampleContainer()
{
   channel_array = NULL;
   interleaved = NULL;
   samples_avail = 0;
   mem_avail = 0;
   source.format = (wlSampleFormatType)-1;
   target.format = (wlSampleFormatType)-1;
}

wlSampleContainer::~wlSampleContainer()
{
   // clean up memory areas
   if (channel_array!=NULL)
   {
      for(int i=0; i<target.channels; i++)
         delete channel_array[i];

      delete channel_array;
   }

   if (interleaved!=NULL)
      delete[] interleaved;
}

void wlSampleContainer::setInputModuleTraits(int bits_per_sample,
   wlSampleFormatType format, int samplerate, int channels)
{
   // set up traits struct
   source.bits_per_sample = bits_per_sample;
   source.format = format;
   source.samplerate = samplerate;
   source.channels = channels;
}

void wlSampleContainer::setOutputModuleTraits(int bits_per_sample,
   wlSampleFormatType format, int samplerate, int channels)
{
   if (samplerate==-1) samplerate = source.samplerate;
   if (channels==-1) channels = source.channels;

   // set up traits struct
   target.bits_per_sample = bits_per_sample;
   target.format = format;
   target.samplerate = samplerate;
   target.channels = channels;

   // initial value
   mem_avail = 512;

   // set up channel array or interleaved memory
   switch(format)
   {
   case wlSamplesChannelArray: // channel array
      {
         channel_array = new void*[channels];
         for(int i=0; i<channels; i++)
         {
            channel_array[i] = new unsigned char[mem_avail * (bits_per_sample>>3)];
         }
      }
      break;

   case wlSamplesInterleaved: // interleaved
      interleaved = new unsigned char[mem_avail * (bits_per_sample>>3) * channels];
      break;
   }
}

void wlSampleContainer::putSamplesInterleaved(void *samples, int numsamples)
{
   // check if there is enough space in the buffer
   if (numsamples>mem_avail)
      reallocMemory(numsamples);

   // conversion from interleaved to ...

   switch(target.format)
   {
   case wlSamplesChannelArray: // channel array
      {
         for(int i=0; i<source.channels; i++)
         {
            deinterleaveChannel((unsigned char*)(samples)+i*(source.bits_per_sample>>3),
               numsamples, i, source.channels*source.bits_per_sample>>3);
         }
      }
      break;
   case wlSamplesInterleaved: // interleaved
      {
         for(int i=0; i<source.channels; i++)
         {
            interleaveChannel((unsigned char*)(samples)+i*(source.bits_per_sample>>3),
               numsamples, i, source.channels*(source.bits_per_sample>>3));
         }
      }
      break;
   }
   samples_avail = numsamples;
}

void wlSampleContainer::putSamplesArray(void **samples, int numsamples)
{
   // check if there is enough space in the buffer
   if (numsamples>mem_avail)
      reallocMemory(numsamples);

   // conversion from channel array to ...

   switch(target.format)
   {
   case wlSamplesChannelArray: // channel array
      {
         for(int i=0; i<source.channels; i++)
         {
            deinterleaveChannel( (unsigned char*)(samples[i]), numsamples,
               i, source.bits_per_sample>>3);
         }
      }
      break;
   case wlSamplesInterleaved: // interleaved
      {
         for(int i=0; i<source.channels; i++)
         {
            interleaveChannel((unsigned char*)(samples[i]),numsamples,
               i, source.bits_per_sample>>3);
         }
      }
      break;
   }
   samples_avail = numsamples;
}

void *wlSampleContainer::getSamplesInterleaved(int &numsamples)
{
   numsamples = samples_avail;
   return interleaved;
}

void **wlSampleContainer::getSamplesArray(int &numsamples)
{
   numsamples = samples_avail;
   return channel_array;
}

void wlSampleContainer::reallocMemory(int new_samples)
{
   mem_avail = new_samples;
   int new_size = mem_avail * (target.bits_per_sample>>3);

   switch(target.format)
   {
   case 0: // channel array
      {
         for(int i=0; i<target.channels; i++)
         {
            delete channel_array[i];
            channel_array[i] = new unsigned char[new_size];
         }
      }
      break;
   case 1: // interleaved
      delete[] interleaved;
      interleaved = new unsigned char[new_size * target.channels];
      break;
   }
}

void wlSampleContainer::interleaveChannel(unsigned char*samples, int numsamples, int channel, int source_step)
{
   int sshift = 32-source.bits_per_sample;
   int dbps = target.bits_per_sample>>3;
   int dshift = 32-target.bits_per_sample;
   int dest_step = target.channels * (target.bits_per_sample>>3);

   int roundbit = dshift>0 ? (1<<(dshift-1)) : 0;
   int dest_high = (1<<31)-roundbit;

   unsigned char *destbuf =
      ((unsigned char*)interleaved)+dbps*channel;

   for(int i=0; i<numsamples; i++)
   {
      register signed int sample;

      sample = *((unsigned int*)samples);

      // normalize

      // shift up
      sample <<= sshift;

      // do sth with the sample

      // add rounding value
      if (sample<dest_high)
         sample += roundbit;

      // shift to target bps
      sample >>= dshift;

      // store sample in destbuf
      _asm {
         mov   edi, destbuf
         mov   eax, sample
         mov   ecx, dbps
      label1:
         stosb
         shr   eax, 8
         loop  label1
      }

      destbuf += dest_step;
      samples += source_step;
   }
}

void wlSampleContainer::deinterleaveChannel(unsigned char*samples, int numsamples, int channel, int source_step)
{
   int sshift = 32-source.bits_per_sample;
   int dbps = target.bits_per_sample>>3;
   int dshift = 32-target.bits_per_sample;
   int dest_step = dbps;

   int roundbit = dshift>0 ? (1<<(dshift-1)) : 0;
   int dest_high = (1<<31)-roundbit;

   unsigned char *destbuf = (unsigned char*)channel_array[channel];

   for(int i=0; i<numsamples; i++)
   {
      register signed int sample;

      sample = *((unsigned int*)samples);

      // normalize

      // shift up
      sample <<= sshift;

      // do sth with the sample

      // add rounding value
      if (sample<dest_high)
         sample += roundbit;

      // shift to target bps
      sample >>= dshift;

      // store sample in destbuf
      _asm {
         mov   edi, destbuf
         mov   eax, sample
         mov   ecx, dbps
      label1:
         stosb
         shr   eax, 8
         loop  label1
      }

      destbuf += dest_step;
      samples += source_step;
   }

}

/*
void wlSampleContainer::interleaveSamples(int channels,int bitpersample, int numsamples, void**samples)
//void wlInterleaveSamples(int channels, int len, short **buffer, short *destbuffer)
{
   void *destbuf = interleaved;
   if (channels==1)
   {
      // mono
      int len = numsamples * (bitpersample>>3);
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
         mov   ebx, numsamples
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
            mov   ebx, numsamples
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
