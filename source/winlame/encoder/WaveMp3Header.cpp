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
/// \file WaveMp3Header.cpp
/// \brief writes wave mp3 header
//
#include "stdafx.h"
#include "WaveMp3Header.hpp"
#include <mmreg.h>
#include <fstream>

/// \verbatim from mmreg.h:
/// //
/// // MPEG Layer3 WAVEFORMATEX structure
/// // for WAVE_FORMAT_MPEGLAYER3 (0x0055)
/// //
/// #define MPEGLAYER3_WFX_EXTRA_BYTES   12
///
/// // WAVE_FORMAT_MPEGLAYER3 format sructure
/// //
/// typedef struct mpeglayer3waveformat_tag {
///   WAVEFORMATEX  wfx;
///   WORD          wID;
///   DWORD         fdwFlags;
///   WORD          nBlockSize;
///   WORD          nFramesPerBlock;
///   WORD          nCodecDelay;
/// } MPEGLAYER3WAVEFORMAT;
///
/// #define MPEGLAYER3_ID_UNKNOWN            0
/// #define MPEGLAYER3_ID_MPEG               1
/// #define MPEGLAYER3_ID_CONSTANTFRAMESIZE  2
///
/// #define MPEGLAYER3_FLAG_PADDING_ISO      0x00000000
/// #define MPEGLAYER3_FLAG_PADDING_ON       0x00000001
/// #define MPEGLAYER3_FLAG_PADDING_OFF      0x00000002
///
/// typedef struct tWAVEFORMATEX
/// {
///     WORD    wFormatTag;        // format type
///     WORD    nChannels;         // number of channels (i.e. mono, stereo...)
///     DWORD   nSamplesPerSec;    // sample rate
///     DWORD   nAvgBytesPerSec;   // for buffer estimation
///     WORD    nBlockAlign;       // block size of data
///     WORD    wBitsPerSample;    // Number of bits per sample of mono data
///     WORD    cbSize;            // The count in bytes of the size of
///                                     extra information (after cbSize)
/// } WAVEFORMATEX;
///
/// \endverbatim
/// typical mp3 file, converted to wave by wavemp3.exe:
/// 
/// RIFF type: WAVE, data 0x0071463d
/// chunk fmt , data 0x0000001e
///  format:
///  type 0055 chan: 2
///  nSamplesPerSec: 44100
///  nAvgBytesPerSec: 20000 // bitrate / 8
///  nBlockAlign: 1
///  wBitsPerSample: 0
/// 
///  nExtraBytes: 12
///  wID: 1
///  fdwFlags: 0x00000002
///  nBlockSize: 0x020a   // bitrate * 144 / sample rate
///  nFramesPerBlock: 1
///  nCodecDelay: 0x0571
/// chunk fact, data 0x00000004
/// chunk data, data 0x007145f6
/// chunk LIST, data 0x00000040

void Encoder::WriteWaveMp3Header(std::ofstream& outputFile, unsigned int numChannels,
   unsigned int samplerateInHz, unsigned int bitrateInBps, unsigned short codecDelay)
{
   // write riff header
   outputFile.write("RIFF", 4);

   unsigned int data = 0xffffffff; // length of file; we don't know yet
   outputFile.write(reinterpret_cast<char*>(&data), 4);

   outputFile.write("WAVE", 4);

   // write "fmt " chunk
   outputFile.write("fmt ", 4);
   data = 16 + 2 + 12;
   outputFile.write(reinterpret_cast<char*>(&data), 4);

   // prepare and write format info with extra mp3 data
   MPEGLAYER3WAVEFORMAT fmt;
   fmt.wfx.wFormatTag = 0x0055;           // WAVE_FORMAT_MPEGLAYER3
   fmt.wfx.nChannels = static_cast<WORD>(numChannels);
   fmt.wfx.nSamplesPerSec = samplerateInHz;
   fmt.wfx.nAvgBytesPerSec = bitrateInBps * 1000 / 8; // bitrate / 8
   fmt.wfx.nBlockAlign = 1;               // 1 block
   fmt.wfx.wBitsPerSample = 0;            // unused, depends on the decoder
   fmt.wfx.cbSize = 12;                   // MPEGLAYER3_WFX_EXTRA_BYTES

   fmt.wID = 1;                  // MPEGLAYER3_ID_MPEG
   fmt.fdwFlags = 0x00000002;    // MPEGLAYER3_FLAG_PADDING_OFF
   fmt.nBlockSize = static_cast<WORD>(bitrateInBps * 1000 * 144 / samplerateInHz);   // bitrate * 144 / sample rate
   fmt.nFramesPerBlock = 1;
   fmt.nCodecDelay = codecDelay;

   outputFile.write(reinterpret_cast<char*>(&fmt), sizeof(MPEGLAYER3WAVEFORMAT));

   // write "fact" chunk
   outputFile.write("fact", 4);
   data = 4;
   outputFile.write(reinterpret_cast<char*>(&data), 4);
   data = 0xffffffff; // number of samples: we don't know yet
   outputFile.write(reinterpret_cast<char*>(&data), 4);

   // write "data" chunk
   outputFile.write("data", 4);
   data = 0xffffffff; // number of data bytes: we don't know yet
   outputFile.write(reinterpret_cast<char*>(&data), 4);
}

void Encoder::FixupWaveMp3Header(std::ofstream& outputFile, unsigned int dataLength,
   unsigned int numSamples)
{
   // whole riff file size
   outputFile.seekp(4);
   unsigned int data = dataLength + 0x0046 - 8;
   outputFile.write(reinterpret_cast<char*>(&data), 4);

   // "fact" chunk: sample size
   outputFile.seekp(0x003a);
   data = numSamples;
   outputFile.write(reinterpret_cast<char*>(&data), 4);

   // "data" chunk: length
   outputFile.seekp(0x0042);
   data = dataLength;
   outputFile.write(reinterpret_cast<char*>(&data), 4);
}
