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

*/
/// \file WaveMp3Header.h
/// \brief contains functions to write riff wave mp3 header
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include <iosfwd>


// global functions

/// writes riff wave mp3 header to output stream
/// \param ostr output file stream to write to
/// \param channels number of channels, either 1 or 2
/// \param samplerate mp3 file sample rate
/// \param bitrate bitrate of the mp3
/// \param codec_delay codec sample delay when decoding
void WriteWaveMp3Header(std::ofstream &ostr,unsigned int channels,
   unsigned int samplerate,unsigned int bitrate,unsigned short codec_delay);

/// fixes fact chunk and riff header lengths; seeks around in the file
/// \param ostr output file stream to write to
/// \param datalen number of mp3 data bytes written
/// \param numsamples number of samples written
void FixupWaveMp3Header(std::ofstream &ostr,unsigned int datalen,
   unsigned int numsamples);

/// @}
