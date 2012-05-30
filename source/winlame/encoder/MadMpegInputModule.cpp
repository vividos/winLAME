/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink
   Copyright (c) 2004 DeXT

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
/*! \file MadMpegInputModule.cpp

   \brief contains the implementation of the MAD mpeg input module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "MadMpegInputModule.h"
#include "Id3v1Tag.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "id3/File.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// linker options

#if _MSC_VER < 1400
#pragma comment(linker, "/delayload:libmad.dll")
#endif


// MadMpegInputModule methods

MadMpegInputModule::MadMpegInputModule()
{
   module_id = ID_IM_MAD;

   memset(&init_header, 0, sizeof(init_header));
   memset(&stream, 0, sizeof(stream));
   memset(&frame, 0, sizeof(frame));
   memset(&synth, 0, sizeof(synth));
}

InputModule *MadMpegInputModule::cloneModule()
{
   return new MadMpegInputModule;
}

bool MadMpegInputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("libmad.dll"));
   bool avail = dll != NULL;
   ::FreeLibrary(dll);

   return avail;

}

void MadMpegInputModule::getDescription(CString& desc)
{
   // mpeg version
   LPCTSTR mpegver = _T("1");
   int srate = init_header.samplerate;
   if (srate<=24000 && srate>=16000 ) mpegver = _T("2");
   if (srate<=12000 ) mpegver = _T("2.5");

   // special options
   LPCTSTR option = _T("");
   if (init_header.flags & MAD_FLAG_MC_EXT)
      option = _T(", Multichannel");

   LPCTSTR stereo = _T("Stereo");
   switch(init_header.mode)
   {
   case MAD_MODE_SINGLE_CHANNEL: stereo = _T("Mono"); break;
   case MAD_MODE_DUAL_CHANNEL: stereo = _T("Dual Channel Stereo"); break;
   case MAD_MODE_JOINT_STEREO:
      stereo = _T("Joint Stereo");
      if (init_header.layer==3)
      {
         if (init_header.mode_extension & (MAD_FLAG_I_STEREO | MAD_FLAG_MS_STEREO))
            stereo = _T("Joint Stereo (Mixed)");
         else
            if (init_header.mode_extension & MAD_FLAG_I_STEREO)
               stereo = _T("Joint Stereo (Intensity)");
            else
               if (init_header.mode_extension & MAD_FLAG_MS_STEREO)
                  stereo = _T("Joint Stereo (Mid/Side)");
      }
   }

   // format output string
   desc.Format(IDS_FORMAT_INFO_MAD_MPEG,
      mpegver, init_header.layer, init_header.bitrate/1000,
      init_header.samplerate, stereo, option);
}

void MadMpegInputModule::getVersionString(CString& version, int special)
{
   HMODULE lib = ::LoadLibrary(_T("libmad.dll"));
   if (lib != NULL)
   {
      LPCSTR str = "mad_version";
      switch(special)
      {
      case 1: str = "mad_copyright"; break;
      case 2: str = "mad_author"; break;
      case 3: str = "mad_build"; break;
      }

      // version has to be retrieved that way, because mad_version isn't
      // really the string, it points to a delayload helper routine
      USES_CONVERSION;
      version = reinterpret_cast<const char*>(::GetProcAddress(lib,str));
      ::FreeLibrary(lib);
   }
}

CString MadMpegInputModule::getFilterString()
{
   CString cszFilter;
   cszFilter.LoadString(IDS_FILTER_MAD_MPEG_INPUT);
   return cszFilter;
}

int MadMpegInputModule::initInput(LPCTSTR infilename,
   SettingsManager &mgr, TrackInfo &trackinfo,
   SampleContainer &samplecont)
{
   // reset number of samples
   numsamples = 0;
   maxsamples = 1;

   // open infile
   USES_CONVERSION;
   istr.open(T2CA(GetAnsiCompatFilename(infilename)),std::ios::in|std::ios::binary);
   if (!istr.is_open())
   {
      lasterror.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   // search for id3v2 tag
   bool bFound = GetId3v2TagInfos(infilename, trackinfo);

   // search for id3v1 tag
   if (!bFound)
   {
      istr.seekg(-128,std::ios::end);

      Id3v1Tag id3tag;
      istr.read(reinterpret_cast<char*>(id3tag.getData()), 128);
      if (istr.gcount() == 128 && id3tag.isValidTag())
      {
         // store found id3 tag infos
         id3tag.toTrackInfo(trackinfo);
      }

      istr.seekg(0,std::ios::beg);
   }

   memset(&init_header,0,sizeof(init_header));

   {
      struct mad_stream stream;

      bool retry_vbrtag_search = false;
      bool first_private_header = true;

      // init structs
      mad_stream_init(&stream);

      // search sync signal of first mpeg frame
      int syncstart = 0;

   do
   {
      bool syncfound = false;
      int synctries = 32; // search in (32 * mad_inbufsize) bytes of input
      do
      {
         // read in first few bytes
         istr.read(reinterpret_cast<char*>(inbuffer),mad_inbufsize);
         std::streamsize read = istr.gcount();

         // skip ID3v2 tag
         int tagsize = 0;
         if (syncstart==0 && !memcmp(inbuffer, "ID3", 3))
         {
            /* high bit is not used */
            tagsize = (inbuffer[6] << 21) | (inbuffer[7] << 14) |
               (inbuffer[8] <<  7) | (inbuffer[9] <<  0) + 10;
            istr.seekg(tagsize,std::ios::beg);
            syncstart += tagsize;
            istr.read(reinterpret_cast<char*>(inbuffer),mad_inbufsize);
            read = istr.gcount();
         }

         // search for sync bits
         for(int i=0; i<read-1; i++)
         if (inbuffer[i]==0xff && (inbuffer[i+1]&0xe0)==0xe0) // first 11 bits are set?
         {
            // sync found!
            syncfound = true;
            syncstart += i;

            if (i>0)
            {
               memmove(inbuffer,inbuffer+i,static_cast<size_t>(read-i));

               // read in more of the frame
               istr.read(reinterpret_cast<char*>(inbuffer+i),mad_inbufsize-i);
            }

            // try to decode frame header
            mad_stream_buffer(&stream,inbuffer,mad_inbufsize);
            int ret = mad_header_decode(&init_header, &stream);

            bool is_first_private_header = first_private_header &&
               (init_header.private_bits & MAD_PRIVATE_HEADER) != 0;

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
            // seek back, we could miss a sync at the last byte of the inbuffer
            istr.seekg(-1,std::ios::cur);
            syncstart += mad_inbufsize-1;
         }
      }
      while (!syncfound && --synctries>0);

      if (!syncfound)
      {
         // d'oh! no sync found; probably no mpeg file
         lasterror.Format(IDS_ENCODER_INVALID_FILE_FORMAT);
         return -1;
      }

      // reposition stream to the sync start
      istr.seekg(syncstart,std::ios::beg);

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
      mad_stream_buffer(&stream,inbuffer,mad_inbufsize);
      if (-1 != mad_header_decode(&init_header, &stream))
      {
         // when we searched for the second header, we are finished here
         if (retry_vbrtag_search)
         {
            retry_vbrtag_search = false;
            break;
         }

         // find out filelength
         struct _stat statbuf;
         ::_tstat(infilename, &statbuf);
         unsigned __int64 filelen = statbuf.st_size; // 32 bit max.

         // find out playing time
         unsigned long playtime=0;
         if (init_header.bitrate != 0)
            playtime = (unsigned long)((filelen << 3) / init_header.bitrate);

         // find out max. number of samples
         if (!retry_vbrtag_search)
            maxsamples = __int64(playtime) * init_header.samplerate;
      }

      // check for Xing / VBR info tag
      if (checkVbrInfoTag(inbuffer))
      {
         // we found one; so this frame header wasn't a normal one

         // reposition stream
         syncstart+=1;
         istr.seekg(syncstart,std::ios::beg);

         // retry reading next header
         retry_vbrtag_search = true;
      }

      // finished
   } while (retry_vbrtag_search);

      mad_stream_finish(&stream);
   }

   samplecont.setInputModuleTraits(32,
      SamplesChannelArray, init_header.samplerate,
      init_header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2);

   // init that mad encoder
   mad_stream_init(&stream);
   mad_frame_init(&frame);
   mad_synth_init(&synth);
/*
   audio_init_stats(&stats);
   audio_init_dither(&left_dither);
   audio_init_dither(&right_dither);
*/

   eos = false;

   return 0;
}

/*! finds out tag; inspired by LAME's GetVbrTag() */
bool MadMpegInputModule::checkVbrInfoTag(unsigned char *buffer)
{
   // mpeg version
   int id = (buffer[1] >> 3) & 1;
   int mode = (buffer[3] >> 6) & 3;

   // calculate offset
   if(id)
      buffer += mode!=3 ? (32+4) : (17+4); // MPEG-1
   else
      buffer += mode!=3 ? (17+4) : (9+4); // MPEG-2

   if (strncmp((const char*)buffer,"Info",4)==0 ||
       strncmp((const char*)buffer,"Xing",4)==0)
   {
      // tag found
      buffer+=4;

      // retrieve flags
      int flags = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
      buffer+=4;

      // vbr tag flags
      const int FRAMES_FLAG = 1;
      const int BYTES_FLAG = 2;

      if (flags & FRAMES_FLAG)
      {
         int frames = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];

         // calculate number of samples
         if (init_header.layer == 1)
            maxsamples = frames * 384;
         else if (!id && init_header.layer == 3)
            maxsamples = frames * 576;
         else
            maxsamples = frames * 1152;
      }

      return true;
   }
   return false;
}

bool MadMpegInputModule::GetId3v2TagInfos(const CString& cszFilename, TrackInfo& trackinfo)
{
   ID3::File file(cszFilename, true);

   if (!file.HasID3v2Tag())
      return false;

   // get primary tag
   ID3::Tag tag = file.GetTag();

   // retrieve field values
   CString cszValue;
   if (tag.IsFrameAvail(ID3::FrameId::Title))
   {
      cszValue = tag.FindFrame(ID3::FrameId::Title).AsString();
      trackinfo.TextInfo(TrackInfoTitle, cszValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Artist))
   {
      cszValue = tag.FindFrame(ID3::FrameId::Artist).AsString();
      trackinfo.TextInfo(TrackInfoArtist, cszValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Comment))
   {
      cszValue = tag.FindFrame(ID3::FrameId::Comment).AsString();
      trackinfo.TextInfo(TrackInfoComment, cszValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::AlbumTitle))
   {
      cszValue = tag.FindFrame(ID3::FrameId::AlbumTitle).AsString();
      trackinfo.TextInfo(TrackInfoAlbum, cszValue);
   }

   if (tag.IsFrameAvail(ID3::FrameId::RecordingTime))
   {
      cszValue = tag.FindFrame(ID3::FrameId::RecordingTime).AsString();
      trackinfo.NumberInfo(TrackInfoYear, _ttoi(cszValue));
   }

   if (tag.IsFrameAvail(ID3::FrameId::TrackNumber))
   {
      cszValue = tag.FindFrame(ID3::FrameId::TrackNumber).AsString();
      trackinfo.NumberInfo(TrackInfoTrack, _ttoi(cszValue));
   }

   if (tag.IsFrameAvail(ID3::FrameId::Genre))
   {
      cszValue = tag.FindFrame(ID3::FrameId::Genre).AsString();
      if (!cszValue.IsEmpty())
         trackinfo.TextInfo(TrackInfoGenre, cszValue);
   }

   return true;
}

void MadMpegInputModule::getInfo(int &channels, int &bitrate, int &length, int &samplerate)
{
   channels = init_header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2;
   bitrate = init_header.bitrate;
   length = int(maxsamples / init_header.samplerate);
   samplerate = init_header.samplerate;
}

int MadMpegInputModule::decodeSamples(SampleContainer &samples)
{
   if (eos) return 0;

   int step=0,ret=0;
   do
   {
      // do all steps
      switch (step)
      {
      case 0:
         // try to decode header
         ret = mad_header_decode((struct mad_header*)&frame.header, &stream);
         if (ret!=-1) step++;
         break;

      case 1:
         // try to decode frame
         ret = mad_frame_decode(&frame,&stream);
         if (ret!=-1) step++;
         break;
      }

      // do error checking/handling
      std::streamsize read=0;
      int fill=0;
      if (ret==-1)
      switch(stream.error)
      {
      case MAD_ERROR_BUFLEN:
         // copy the remaining bytes of the buffer to the front
         fill = stream.bufend-stream.next_frame;
         if (fill!=0)
            memmove(inbuffer, stream.next_frame, fill);
         // and ...

      case MAD_ERROR_BUFPTR:
         // fill the buffer
         istr.read(reinterpret_cast<char*>(inbuffer+fill),mad_inbufsize-fill);
         read = istr.gcount();
         if (read==0)
         {
            // were we already here?
            if (eos) return 0;

            // finished with stream
            eos = true;

            // add MAD_BUFFER_GUARD bytes to buffer
            memset(inbuffer+fill,0,MAD_BUFFER_GUARD);
            read = MAD_BUFFER_GUARD;
         }

         mad_stream_buffer(&stream,inbuffer,static_cast<unsigned long>(read+fill));
         break;

      case MAD_ERROR_BADCRC:
         // crc error; mute this frame
         mad_frame_mute(&frame);
         step=2;
         break;

      case MAD_ERROR_LOSTSYNC:
         if (strncmp(reinterpret_cast<const char*>(stream.this_frame), "ID3", 3) == 0)
         {
            // id3v2 tag
            int tagsize = (stream.this_frame[6] << 21) | (stream.this_frame[7] << 14) |
               (stream.this_frame[8] <<  7) | (stream.this_frame[9] <<  0) + 10;
            mad_stream_skip(&stream, tagsize);
         }
         else if (strncmp(reinterpret_cast<const char*>(stream.this_frame), "TAG", 3) == 0)
         {
            // id3v1 tag
            mad_stream_skip(&stream, 128);
         }
         break;
      case MAD_ERROR_BADHUFFDATA:
         // skip this bad frame
         mad_frame_mute(&frame);
         step=2;
         break;

      default:
         if (!MAD_RECOVERABLE(stream.error))
         {
            lasterror.LoadString(IDS_ENCODER_INTERNAL_DECODE_ERROR);
            return -stream.error;
         }
         break;
      }

      // loop until all steps are made
   } while (step<2);

   // do pcm synthesis
   mad_synth_frame(&synth,&frame);

   // we have output
   ret=synth.pcm.length;

   // clip and rescale output

   // note: ignore frame.header.mode, since it cannot (or should not) change between frames
   ATLASSERT(frame.header.mode == init_header.mode);
   channels = init_header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2;

   // use rounding and clipping
   for(int ch=0; ch<channels; ch++)
   for(int i=0; i<ret; i++)
   {
      mad_fixed_t sample = synth.pcm.samples[ch][i];

      // round
      sample += (1L << (32 - MAD_F_FRACBITS));

      // clip
      if (sample >= MAD_F_ONE) sample = MAD_F_ONE - 1; 
      else if (sample < -MAD_F_ONE) sample = -MAD_F_ONE;

      // scale to 32 bits
      //synth.pcm.samples[ch][i] = sample >> (MAD_F_FRACBITS + 1 - MadOutputBits);
      synth.pcm.samples[ch][i] = sample << (32 - MAD_F_FRACBITS - 1);
   }

/*
   // use linear dither
   for(int i=0; i<ret; i++)
   {
      // dither left
      mad_fixed_t sample = synth.pcm.samples[0][i];
      synth.pcm.samples[0][i] = audio_linear_dither(MadOutputBits,sample,&left_dither,&stats);
   }

   // second channel?
   if (frame.header.mode != MAD_MODE_SINGLE_CHANNEL)
   for(int i=0; i<ret; i++)
   {
      // dither right
      mad_fixed_t sample = synth.pcm.samples[1][i];
      synth.pcm.samples[1][i] = audio_linear_dither(MadOutputBits,sample,&right_dither,&stats);
   }
*/

   // fill sample container
   mad_fixed_t* channel_array[2] = { synth.pcm.samples[0], synth.pcm.samples[1] };
   samples.putSamplesArray((void**)channel_array,ret);

   // count samples
   numsamples += ret;

   return ret;
}

void MadMpegInputModule::doneInput()
{
   if (istr.is_open())
   {
      // close file
      istr.close();

      // finish structs
      mad_synth_finish(&synth);
      mad_frame_finish(&frame);
      mad_stream_finish(&stream);
   }
}
