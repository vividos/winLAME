//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file MadMpegInputModule.cpp
/// \brief contains the implementation of the MAD mpeg input module
//
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "MadMpegInputModule.hpp"
#include "Id3v1Tag.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include "id3/File.h"
#include "DynamicLibrary.hpp"

using Encoder::MadMpegInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

MadMpegInputModule::MadMpegInputModule()
   :m_endOfStream(false),
   m_numCurrentSamples(0),
   m_numMaxSamples(0)
{
   m_moduleId = ID_IM_MAD;

   memset(&m_initHeader, 0, sizeof(m_initHeader));
   memset(&m_stream, 0, sizeof(m_stream));
   memset(&m_frame, 0, sizeof(m_frame));
   memset(&m_synth, 0, sizeof(m_synth));
}

Encoder::InputModule* MadMpegInputModule::CloneModule()
{
   return new MadMpegInputModule;
}

bool MadMpegInputModule::IsAvailable() const
{
   DynamicLibrary lib(_T("libmad.dll"));

   return lib.IsLoaded();
}

CString MadMpegInputModule::GetDescription() const
{
   // mpeg version
   LPCTSTR mpegVersion = _T("1");
   int samplerateInHz = m_initHeader.samplerate;
   if (samplerateInHz <= 24000 && samplerateInHz >= 16000)
      mpegVersion = _T("2");
   if (samplerateInHz <= 12000)
      mpegVersion = _T("2.5");

   // special options
   LPCTSTR option = _T("");
   if (m_initHeader.flags & MAD_FLAG_MC_EXT)
      option = _T(", Multichannel");

   LPCTSTR stereo = _T("Stereo");
   switch (m_initHeader.mode)
   {
   case MAD_MODE_SINGLE_CHANNEL:
      stereo = _T("Mono");
      break;

   case MAD_MODE_DUAL_CHANNEL:
      stereo = _T("Dual Channel Stereo");
      break;

   case MAD_MODE_JOINT_STEREO:
      stereo = _T("Joint Stereo");
      if (m_initHeader.layer == 3)
      {
         if (m_initHeader.mode_extension & (MAD_FLAG_I_STEREO | MAD_FLAG_MS_STEREO))
            stereo = _T("Joint Stereo (Mixed)");
         else
            if (m_initHeader.mode_extension & MAD_FLAG_I_STEREO)
               stereo = _T("Joint Stereo (Intensity)");
            else
               if (m_initHeader.mode_extension & MAD_FLAG_MS_STEREO)
                  stereo = _T("Joint Stereo (Mid/Side)");
      }
   }

   CString desc;
   desc.Format(IDS_FORMAT_INFO_MAD_MPEG,
      mpegVersion,
      m_initHeader.layer,
      m_initHeader.bitrate / 1000,
      m_initHeader.samplerate,
      stereo,
      option);

   return desc;
}

void MadMpegInputModule::GetVersionString(CString& version, int special) const
{
   DynamicLibrary lib(_T("libmad.dll"));

   if (!lib.IsLoaded())
      return;

   LPCSTR functionName = "mad_version";
   switch (special)
   {
   case 1: functionName = "mad_copyright"; break;
   case 2: functionName = "mad_author"; break;
   case 3: functionName = "mad_build"; break;
   }

   // version has to be retrieved that way, because mad_version isn't
   // really the string, it points to a delayload helper routine
   version = lib.GetFunction<const char*>(functionName);
}

CString MadMpegInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_MAD_MPEG_INPUT);
   return filterString;
}

int MadMpegInputModule::InitInput(LPCTSTR infilename,
   SettingsManager& mgr, TrackInfo& trackInfo,
   SampleContainer& samples)
{
   // reset number of samples
   m_numCurrentSamples = 0;
   m_numMaxSamples = 1;

   // open infile
   m_inputFile.open(infilename, std::ios::in | std::ios::binary);
   if (!m_inputFile.is_open())
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   // search for id3v2 tag
   bool found = GetId3v2TagInfos(infilename, trackInfo);

   // search for id3v1 tag
   if (!found)
   {
      m_inputFile.seekg(-128, std::ios::end);

      Id3v1Tag id3tag;
      m_inputFile.read(reinterpret_cast<char*>(id3tag.GetData()), 128);
      if (m_inputFile.gcount() == 128 && id3tag.IsValidTag())
      {
         // store found id3 tag infos
         id3tag.ToTrackInfo(trackInfo);
      }

      m_inputFile.seekg(0, std::ios::beg);
   }

   memset(&m_initHeader, 0, sizeof(m_initHeader));

   int ret = ReadFileLength(infilename);
   if (ret != 0)
      return ret;

   samples.SetInputModuleTraits(32,
      SamplesChannelArray,
      m_initHeader.samplerate,
      m_initHeader.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2);

   // init that mad encoder
   mad_stream_init(&m_stream);
   mad_frame_init(&m_frame);
   mad_synth_init(&m_synth);

   //audio_init_stats(&stats);
   //audio_init_dither(&left_dither);
   //audio_init_dither(&right_dither);

   m_endOfStream = false;

   return 0;
}

int MadMpegInputModule::ReadFileLength(LPCTSTR infilename)
{
   struct mad_stream checkstream;

   bool retryVBRTagSearch = false;
   bool first_private_header = true;

   // init structs
   mad_stream_init(&checkstream);

   // search sync signal of first mpeg frame
   int syncstart = 0;

   do
   {
      bool syncfound = false;
      int synctries = 32; // search in (32 * mad_inbufsize) bytes of input
      do
      {
         // read in first few bytes
         m_inputFile.read(reinterpret_cast<char*>(m_inputBuffer), mad_inbufsize);
         std::streamsize read = m_inputFile.gcount();

         // skip ID3v2 tag
         if (syncstart == 0 && !memcmp(m_inputBuffer, "ID3", 3))
         {
            // high bit is not used
            int tagsize = (m_inputBuffer[6] << 21) | (m_inputBuffer[7] << 14) |
               (m_inputBuffer[8] << 7) | (m_inputBuffer[9] << 0) + 10;
            m_inputFile.seekg(tagsize, std::ios::beg);
            syncstart += tagsize;
            m_inputFile.read(reinterpret_cast<char*>(m_inputBuffer), mad_inbufsize);
            read = m_inputFile.gcount();
         }

         // search for sync bits
         for (int i = 0; i < read - 1; i++)
            if (m_inputBuffer[i] == 0xff && (m_inputBuffer[i + 1] & 0xe0) == 0xe0) // first 11 bits are set?
            {
               // sync found!
               syncfound = true;
               syncstart += i;

               if (i > 0)
               {
                  memmove(m_inputBuffer, m_inputBuffer + i, static_cast<size_t>(read - i));

                  // read in more of the frame
                  m_inputFile.read(reinterpret_cast<char*>(m_inputBuffer + i), mad_inbufsize - i);
               }

               // try to decode frame header
               mad_stream_buffer(&checkstream, m_inputBuffer, mad_inbufsize);
               int ret = mad_header_decode(&m_initHeader, &checkstream);

               bool is_first_private_header = first_private_header &&
                  (m_initHeader.private_bits & MAD_PRIVATE_HEADER) != 0;

               if (is_first_private_header)
                  first_private_header = false;

               if (-1 == ret || is_first_private_header)
               {
                  // failed; search for next correct header
                  syncfound = false;
                  continue;
               }
               break;
            }

         if (!syncfound)
         {
            // seek back, we could miss a sync at the last byte of the input buffer
            m_inputFile.seekg(-1, std::ios::cur);
            syncstart += mad_inbufsize - 1;
         }
      } while (!syncfound && --synctries > 0);

      if (!syncfound)
      {
         // d'oh! no sync found; probably no mpeg file
         m_lastError.Format(IDS_ENCODER_INVALID_FILE_FORMAT);
         return -1;
      }

      // reposition stream to the sync start
      m_inputFile.seekg(syncstart, std::ios::beg);

      /*! \verbatim

      Rob Leslie's how-to-find-out-number-of-frames:

      For constant bitrate files, assuming no Xing, ID3, or other headers, this is
      easy:

      playing time in seconds = (file length in bytes) * 8 / frame.header.bitrate
      samples = (playing time in seconds) * frame.header.samplerate
      frames = samples / 1152  ; MPEG-1 Layer II or Layer III, MPEG-2 Layer II
      frames = samples / 576   ; MPEG-2 Layer III
      frames = samples / 384   ; MPEG-1 or MPEG-2 Layer I

      For variable bitrate files, there is no simple way to determine anything short
      of scanning the header of every frame. However, if there is a Xing header at
      the beginning of the file, it will tell you the number of frames and effective
      number of bytes in the file. You can deduce the number of samples and total
      playing time from this:

      samples = frames * 1152  ; or 576 or 384 - see above
      playing time in seconds = samples / frame.header.samplerate
      average bitrate in bps = (file bytes) * 8 / (playing time in seconds)

      If the sampling frequency is 48000, 44100, or 32000, it is MPEG-1. If the
      sampling frequency is 24000, 22050, or 16000, it is MPEG-2 (LSF). Otherwise,
      if the sampling frequency is 12000, 11025, or 8000, it is so-called MPEG 2.5.

      \endverbatim
      */

      // pass the frame to the decoder
      mad_stream_buffer(&checkstream, m_inputBuffer, mad_inbufsize);
      if (-1 != mad_header_decode(&m_initHeader, &checkstream))
      {
         // when we searched for the second header, we are finished here
         if (retryVBRTagSearch)
         {
            retryVBRTagSearch = false;
            break;
         }

         // find out filelength
         struct _stat statbuf;
         ::_tstat(infilename, &statbuf);
         unsigned __int64 filelen = statbuf.st_size; // 32 bit max.

         // find out playing time
         unsigned long playtime = 0;
         if (m_initHeader.bitrate != 0)
            playtime = (unsigned long)((filelen << 3) / m_initHeader.bitrate);

         // find out max. number of samples
         if (!retryVBRTagSearch)
            m_numMaxSamples = __int64(playtime) * m_initHeader.samplerate;
      }

      // check for Xing / VBR info tag
      if (CheckVBRInfoTag(m_inputBuffer))
      {
         // we found one; so this frame header wasn't a normal one

         // reposition stream
         syncstart += 1;
         m_inputFile.seekg(syncstart, std::ios::beg);

         // retry reading next header
         retryVBRTagSearch = true;
      }

      // finished
   } while (retryVBRTagSearch);

   mad_stream_finish(&checkstream);

   return 0;
}

/// finds out tag; inspired by LAME's GetVbrTag()
bool MadMpegInputModule::CheckVBRInfoTag(unsigned char* buffer)
{
   // mpeg version
   int id = (buffer[1] >> 3) & 1;
   int mode = (buffer[3] >> 6) & 3;

   // calculate offset
   if (id)
      buffer += mode != 3 ? (32 + 4) : (17 + 4); // MPEG-1
   else
      buffer += mode != 3 ? (17 + 4) : (9 + 4); // MPEG-2

   if (strncmp((const char*)buffer, "Info", 4) == 0 ||
      strncmp((const char*)buffer, "Xing", 4) == 0)
   {
      // tag found
      buffer += 4;

      // retrieve flags
      int flags = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
      buffer += 4;

      // vbr tag flags
      const int FRAMES_FLAG = 1;

      if (flags & FRAMES_FLAG)
      {
         int frames = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];

         // calculate number of samples
         if (m_initHeader.layer == 1)
            m_numMaxSamples = __int64(frames) * 384;
         else if (!id && m_initHeader.layer == 3)
            m_numMaxSamples = __int64(frames) * 576;
         else
            m_numMaxSamples = __int64(frames) * 1152;
      }

      return true;
   }

   return false;
}

bool MadMpegInputModule::GetId3v2TagInfos(const CString& filename, TrackInfo& trackInfo)
{
   ID3::File file(filename, true);

   if (!file.HasID3v2Tag())
      return false;

   // get primary tag
   ID3::Tag tag = file.GetTag();

   // retrieve field values
   CString textValue;
   if (tag.IsFrameAvail(ID3::FrameId::Title))
   {
      textValue = tag.FindFrame(ID3::FrameId::Title).GetString(1);
      trackInfo.TextInfo(TrackInfoTitle, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Artist))
   {
      textValue = tag.FindFrame(ID3::FrameId::Artist).GetString(1);
      trackInfo.TextInfo(TrackInfoArtist, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Comment))
   {
      textValue = tag.FindFrame(ID3::FrameId::Comment).GetString(3);
      trackInfo.TextInfo(TrackInfoComment, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::AlbumTitle))
   {
      textValue = tag.FindFrame(ID3::FrameId::AlbumTitle).GetString(1);
      trackInfo.TextInfo(TrackInfoAlbum, textValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::RecordingTime))
   {
      textValue = tag.FindFrame(ID3::FrameId::RecordingTime).GetString(1);
      trackInfo.NumberInfo(TrackInfoYear, _ttoi(textValue));
   }

   if (tag.IsFrameAvail(ID3::FrameId::TrackNumber))
   {
      textValue = tag.FindFrame(ID3::FrameId::TrackNumber).GetString(1);
      trackInfo.NumberInfo(TrackInfoTrack, _ttoi(textValue));
   }

   if (tag.IsFrameAvail(ID3::FrameId::Genre))
   {
      textValue = tag.FindFrame(ID3::FrameId::Genre).GetString(1);
      if (!textValue.IsEmpty())
         trackInfo.TextInfo(TrackInfoGenre, textValue);
   }

   return true;
}

void MadMpegInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   numChannels = m_initHeader.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2;
   bitrateInBps = m_initHeader.bitrate;
   lengthInSeconds = int(m_numMaxSamples / m_initHeader.samplerate);
   samplerateInHz = m_initHeader.samplerate;
}

int MadMpegInputModule::DecodeSamples(SampleContainer& samples)
{
   if (m_endOfStream)
      return 0;

   int step = 0, ret = 0;
   do
   {
      // do all steps
      switch (step)
      {
      case 0:
         // try to decode header
         ret = mad_header_decode((struct mad_header*)&m_frame.header, &m_stream);
         if (ret != -1)
            step++;
         break;

      case 1:
         // try to decode frame
         ret = mad_frame_decode(&m_frame, &m_stream);
         if (ret != -1)
            step++;
         break;
      }

      // do error checking/handling
      std::streamsize read = 0;
      if (ret == -1)
      {
         int fill = 0;
         switch (m_stream.error)
         {
         case MAD_ERROR_BUFLEN:
            // copy the remaining bytes of the buffer to the front
            fill = m_stream.bufend - m_stream.next_frame;
            if (fill != 0)
               memmove(m_inputBuffer, m_stream.next_frame, fill);
            // and ...

         case MAD_ERROR_BUFPTR:
            // fill the buffer
            m_inputFile.read(reinterpret_cast<char*>(m_inputBuffer + fill), mad_inbufsize - fill);
            read = m_inputFile.gcount();
            if (read == 0)
            {
               // were we already here?
               if (m_endOfStream)
                  return 0;

               // finished with stream
               m_endOfStream = true;

               // add MAD_BUFFER_GUARD bytes to buffer
               memset(m_inputBuffer + fill, 0, MAD_BUFFER_GUARD);
               read = MAD_BUFFER_GUARD;
            }

            mad_stream_buffer(&m_stream, m_inputBuffer, static_cast<unsigned long>(read + fill));
            break;

         case MAD_ERROR_BADCRC:
            // crc error; mute this frame
            mad_frame_mute(&m_frame);
            step = 2;
            break;

         case MAD_ERROR_LOSTSYNC:
            if (strncmp(reinterpret_cast<const char*>(m_stream.this_frame), "ID3", 3) == 0)
            {
               // id3v2 tag
               int tagsize = (m_stream.this_frame[6] << 21) | (m_stream.this_frame[7] << 14) |
                  (m_stream.this_frame[8] << 7) | (m_stream.this_frame[9] << 0) + 10;
               mad_stream_skip(&m_stream, tagsize);
            }
            else if (strncmp(reinterpret_cast<const char*>(m_stream.this_frame), "TAG", 3) == 0)
            {
               // id3v1 tag
               mad_stream_skip(&m_stream, 128);
            }
            break;
         case MAD_ERROR_BADHUFFDATA:
            // skip this bad frame
            mad_frame_mute(&m_frame);
            step = 2;
            break;

         default:
            if (!MAD_RECOVERABLE(m_stream.error))
            {
               m_lastError.LoadString(IDS_ENCODER_INTERNAL_DECODE_ERROR);
               return -m_stream.error;
            }
            break;
         }
      }

      // loop until all steps are made
   } while (step < 2);

   // do pcm synthesis
   mad_synth_frame(&m_synth, &m_frame);

   // we have output
   ret = m_synth.pcm.length;

   // clip and rescale output

   // note: ignore frame.header.mode, since it cannot (or should not) change between frames
   ATLASSERT(m_frame.header.mode == m_initHeader.mode);
   m_channels = m_initHeader.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2;

   // use rounding and clipping
   for (int ch = 0; ch < m_channels; ch++)
      for (int i = 0; i < ret; i++)
      {
         mad_fixed_t sample = m_synth.pcm.samples[ch][i];

         // round
         sample += (1L << (32 - MAD_F_FRACBITS));

         // clip
         if (sample >= MAD_F_ONE) sample = MAD_F_ONE - 1;
         else if (sample < -MAD_F_ONE) sample = -MAD_F_ONE;

         // scale to 32 bits
         //m_synth.pcm.samples[ch][i] = sample >> (MAD_F_FRACBITS + 1 - MadOutputBits);
         m_synth.pcm.samples[ch][i] = sample << (32 - MAD_F_FRACBITS - 1);
      }

   /*
      // use linear dither
      for(int i=0; i<ret; i++)
      {
         // dither left
         mad_fixed_t sample = m_synth.pcm.samples[0][i];
         m_synth.pcm.samples[0][i] = audio_linear_dither(MadOutputBits,sample,&left_dither,&stats);
      }

      // second channel?
      if (m_frame.header.mode != MAD_MODE_SINGLE_CHANNEL)
      for(int i=0; i<ret; i++)
      {
         // dither right
         mad_fixed_t sample = m_synth.pcm.samples[1][i];
         m_synth.pcm.samples[1][i] = audio_linear_dither(MadOutputBits,sample,&right_dither,&stats);
      }
   */

   // fill sample container
   mad_fixed_t* channel_array[2] = { m_synth.pcm.samples[0], m_synth.pcm.samples[1] };
   samples.PutSamplesArray((void**)channel_array, ret);

   // count samples
   m_numCurrentSamples += ret;

   return ret;
}

void MadMpegInputModule::DoneInput()
{
   if (m_inputFile.is_open())
   {
      // close file
      m_inputFile.close();

      // finish structs
      mad_synth_finish(&m_synth);
      mad_frame_finish(&m_frame);
      mad_stream_finish(&m_stream);
   }
}
