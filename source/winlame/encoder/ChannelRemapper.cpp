//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2020 Michael Fink
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
/// \file ChannelRemapper.cpp
/// \brief channel remapping implementation
//
#include "stdafx.h"
#include "ChannelRemapper.hpp"

using Encoder::ChannelRemapper;

const int MAX_CHANNELS = 6; ///< make this higher to support files with more channels

/// channel remapping map
const int g_channelMap[4][MAX_CHANNELS][MAX_CHANNELS] =
{
   // aacInputChannelMap
   {
      { 0, },               // mono
      { 0, 1, },            // l, r
      { 1, 2, 0, },         // c, l, r -> l, r, c
      { 1, 2, 0, 3, },      // c, l, r, bc -> l, r, c, bc
      { 1, 2, 0, 3, 4, },   // c, l, r, bl, br -> l, r, c, bl, br
      { 1, 2, 0, 5, 3, 4 }  // c, l, r, bl, br, lfe -> l, r, c, lfe, bl, br
   },
   // aacOutputChannelMap
   {
      { 0, },               // mono
      { 0, 1, },            // l, r
      { 2, 0, 1, },         // l, r, c -> c, l, r
      { 2, 0, 1, 3, },      // l, r, c, bc -> c, l, r, bc
      { 2, 0, 1, 3, 4, },   // l, r, c, bl, br -> c, l, r, bl, br
      { 2, 0, 1, 4, 5, 3 }  // l, r, c, lfe, bl, br -> c, l, r, bl, br, lfe
   },
   // oggVorbisInputChannelMap
   {
      { 0, },               // mono
      { 0, 1, },            // l, r
      { 0, 2, 1, },         // l, c, r -> l, r, c
      { 0, 1, 2, 3, },      // l, r, bl, br
      { 0, 2, 1, 3, 4, },   // l, c, r, bl, br -> l, r, c, bl, br
      { 0, 2, 1, 5, 3, 4 }  // l, c, r, bl, br, lfe -> l, r, c, lfe, bl, br
   },
   // oggVorbisOutputChannelMap
   {
      { 0, },               // mono
      { 0, 1, },            // l, r
      { 0, 2, 1, },         // l, r, c -> l, c, r
      { 0, 1, 2, 3, },      // l, r, bl, br
      { 0, 2, 1, 3, 4, },   // l, r, c, bl, br -> l, c, r, bl, br
      { 0, 2, 1, 4, 5, 3 }  // l, r, c, lfe, bl, br -> l, c, r, bl, br, lfe
   }
};

size_t ChannelRemapper::GetMaxMappedChannel()
{
   return MAX_CHANNELS;
}

size_t ChannelRemapper::GetMappedChannel(T_enChannelMapType channelMapType, size_t numChannels, size_t inputChannel)
{
   if (numChannels >= MAX_CHANNELS ||
      inputChannel >= MAX_CHANNELS)
      return 0;

   return g_channelMap[channelMapType][numChannels - 1][inputChannel];
}

void ChannelRemapper::ConvertSamplesToFloat(
   short** sampleBuffer, size_t numSamples, size_t numChannels, float** outputBuffer)
{
   for (size_t channelIndex = 0; channelIndex < numChannels; channelIndex++)
      for (size_t sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
         outputBuffer[channelIndex][sampleIndex] = float(sampleBuffer[channelIndex][sampleIndex]) / 32768.f;
}

void ChannelRemapper::RemapInterleaved(T_enChannelMapType channelMapType,
   short* sampleBuffer, size_t numSamples, size_t numChannels, short* outputBuffer)
{
   auto channelMap = g_channelMap[channelMapType];

   for (size_t sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
      for (size_t channelIndex = 0; channelIndex < std::min<size_t>(numChannels, MAX_CHANNELS); channelIndex++)
         outputBuffer[sampleIndex * numChannels + channelIndex] =
         sampleBuffer[sampleIndex * numChannels + (channelMap[numChannels - 1][channelIndex])];

   // copy the remaining channels
   if (numChannels > MAX_CHANNELS)
   {
      for (size_t sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
         for (size_t channelIndex = MAX_CHANNELS; channelIndex < numChannels; channelIndex++)
            outputBuffer[sampleIndex * numChannels + channelIndex] = sampleBuffer[sampleIndex * numChannels + channelIndex];
   }
}

void ChannelRemapper::RemapArrayToFloat(T_enChannelMapType channelMapType,
   short** sampleBuffer, size_t numSamples, size_t numChannels, float** outputBuffer)
{
   auto channelMap = g_channelMap[channelMapType];

   for (size_t channelIndex = 0; channelIndex < std::min<size_t>(numChannels, MAX_CHANNELS); channelIndex++)
      for (size_t sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
         outputBuffer[channelIndex][sampleIndex] =
         float(sampleBuffer[channelMap[numChannels - 1][channelIndex]][sampleIndex]) / 32768.f;

   if (numChannels > MAX_CHANNELS)
   {
      for (size_t channelIndex = MAX_CHANNELS; channelIndex < numChannels; channelIndex++)
         for (size_t sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
            outputBuffer[channelIndex][sampleIndex] = float(sampleBuffer[channelIndex][sampleIndex]) / 32768.f;
   }
}
