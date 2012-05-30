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

   $Id: SampleContainer.h,v 1.5 2004/01/14 21:32:20 vividos Exp $

*/
/*! \file SampleContainer.h

   \brief contains a sample container class

   the sample container can convert from various formats and bits per sample,
   as well as doing per-sample-operations

*/
/*! \ingroup encoder */
/*! @{ */

// prevent multiple including
#ifndef __wlsamplecontainer_h_
#define __wlsamplecontainer_h_


//! sample format type
typedef enum
{
   SamplesChannelArray=0,
   SamplesInterleaved=1
} SampleFormatType;

//! module traits info
typedef struct
{
   //! bits per sample
   int bits_per_sample;
   //! sample format; 0 = channel array; 1 = interleaved
   SampleFormatType format;
   //! sample rate in Hz
   int samplerate;
   //! number of channels
   int channels;
} ModuleTraits;


//! sample container class

class SampleContainer
{
public:
   //! ctor
   SampleContainer();

   //! dtor
   ~SampleContainer();

   // input module functions

   //! sets traits of the input module
   void setInputModuleTraits(int bits_per_sample, SampleFormatType format,
      int samplerate, int channels);

   //! returns the input module sample rate
   int getInputModuleSampleRate(){ return source.samplerate; }

   //! returns the input module number of channels
   int getInputModuleChannels(){ return source.channels; }

   //! returns the input module bits per sample
   int getInputModuleBitsPerSample(){ return source.bits_per_sample; }

   // output module functions

   //! sets traits of the output module
   void setOutputModuleTraits(int bits_per_sample, SampleFormatType format,
      int samplerate=-1, int channels=-1);

   //! returns the input module sample rate
   int getOutputModuleSampleRate(){ return target.samplerate; }

   //! returns the input module number of channels
   int getOutputModuleChannels(){ return target.channels; }

   //! returns the input module bits per sample
   int getOutputModuleBitsPerSample(){ return target.bits_per_sample; }

   // functions to put samples in or get samples out

   //! stores samples in interleaved format in the sample container
   void putSamplesInterleaved(void *samples, int numsamples);

   //! stores samples in interleaved format in the sample container
   void putSamplesArray(void **samples, int numsamples);

   //! retrieves samples in interleaved format
   void *getSamplesInterleaved(int &numsamples);

   //! retrieves samples in channel array format
   void **getSamplesArray(int &numsamples);

protected:
   //! reallocates internal output buffers
   void reallocMemory(int new_samples);

   //! converts samples to interleaved target buffer
   void interleaveChannel(unsigned char*samples, int numsamples, int channel, int source_step);

   //! converts samples to channel array target buffer
   void deinterleaveChannel(unsigned char*samples, int numsamples, int channel, int source_step);

protected:
   //! source traits
   ModuleTraits source;

   //! target traits
   ModuleTraits target;

   //! channel array samples memory
   void **channel_array;

   //! interleaved samples memory
   void *interleaved;

   //! number of bytes available in the buffer(s)
   int mem_avail;

   //! number of available samples
   int samples_avail;
};


//@}

#endif
