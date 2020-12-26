//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2020 Michael Fink
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
/// \file ChannelRemapper.hpp
/// \brief channel remapping
//
#pragma once

namespace Encoder
{
   /// pre-defined channel map types
   enum T_enChannelMapType
   {
      aacInputChannelMap = 0,
      aacOutputChannelMap = 1,
      oggVorbisInputChannelMap = 2,
      oggVorbisOutputChannelMap = 3,
   };

   /// channel remapper helper class
   class ChannelRemapper
   {
   public:
      /// returns number of mappable channels
      static size_t GetMaxMappedChannel();

      /// returns mapped output channel for a given input channel
      static size_t GetMappedChannel(T_enChannelMapType channelMapType, size_t numChannels, size_t inputChannel);

      /// converts a sample buffer to float, without remapping
      static void ConvertSamplesToFloat(
         short** sampleBuffer, size_t numSamples, size_t numChannels, float** outputBuffer);

      /// remaps an interleaved sample buffer with number of samples and channels to a stereo output buffer
      static void RemapInterleaved(T_enChannelMapType channelMapType,
         short* sampleBuffer, size_t numSamples, size_t numChannels, short* outputBuffer);

      /// remaps an array sample buffer with number of samples and channels to a float stereo output buffer
      static void RemapArrayToFloat(T_enChannelMapType channelMapType,
         short** sampleBuffer, size_t numSamples, size_t numChannels, float** outputBuffer);
   };

} // namespace Encoder
