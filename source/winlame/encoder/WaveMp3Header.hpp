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
/// \file WaveMp3Header.hpp
/// \brief contains functions to write RIFF wave mp3 header
//
#pragma once

#include <iosfwd>

namespace Encoder
{
   // global functions

   /// writes RIFF wave mp3 header to output stream
   /// \param outputFile output file stream to write to
   /// \param numChannels number of channels, either 1 or 2
   /// \param samplerateInHz mp3 file sample rate
   /// \param bitrateInBps bitrate of the mp3
   /// \param codecDelay codec sample delay when decoding
   void WriteWaveMp3Header(std::ofstream& outputFile, unsigned int numChannels,
      unsigned int samplerateInHz, unsigned int bitrateInBps, unsigned short codecDelay);

   /// fixes fact chunk and riff header lengths; seeks around in the file
   /// \param outputFile output file stream to write to
   /// \param dataLength number of mp3 data bytes written
   /// \param numSamples number of samples written
   void FixupWaveMp3Header(std::ofstream& outputFile, unsigned int dataLength,
      unsigned int numSamples);

} // namespace Encoder
