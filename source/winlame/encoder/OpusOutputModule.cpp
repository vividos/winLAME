//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2016 Michael Fink
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
/// \file OpusOutputModule.cpp
/// \brief Opus output module
//

// The next copyright and license block appears because this file uses a lot
// of code from opus-tools code from opusenc.c

/* Copyright (C)2002-2011 Jean-Marc Valin
   Copyright (C)2007-2013 Xiph.Org Foundation
   Copyright (C)2008-2013 Gregory Maxwell
   File: opusenc.c

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "OpusOutputModule.hpp"
#include "..\App.h"

#pragma comment(lib, "libopusfile-0.lib")
#pragma comment(lib, "libopus-0.lib")

#pragma warning (disable: 4706) // assignment within conditional expression

static inline int oe_write_page(ogg_page *page, FILE *fp);
static void comment_init(char **comments, int* length, const char *vendor_string);
void comment_add(char **comments, int* length, char *tag, char *val);
static void comment_pad(char **comments, int* length, int amount);

/*Write an Ogg page to a file pointer*/
static inline int oe_write_page(ogg_page *page, FILE *fp)
{
   int written;
   written = fwrite(page->header, 1, page->header_len, fp);
   written += fwrite(page->body, 1, page->body_len, fp);
   return written;
}

#define MAX_FRAME_BYTES 61295
#define IMIN(a,b) ((a) < (b) ? (a) : (b))   /**< Minimum int value.   */
#define IMAX(a,b) ((a) > (b) ? (a) : (b))   /**< Maximum int value.   */


// OpusOutputModule methods

OpusOutputModule::OpusOutputModule()
   :m_bitrateInBps(-1),
   m_complexity(10),
   m_opusBitrateMode(0),
   m_codingRate(48000),
   m_downmix(0),
   m_numBytesWritten(0),
   m_numPagesWritten(0),
   m_floatMode(false)
{
   module_id = ID_OM_OPUS;
}

OpusOutputModule::~OpusOutputModule()
{
}

bool OpusOutputModule::isAvailable()
{
   // always available
   return true;
}

void OpusOutputModule::getDescription(CString& desc)
{
}

void OpusOutputModule::getVersionString(CString& version, int special)
{
   ATLASSERT(false);
}

int OpusOutputModule::initOutput(LPCTSTR outfilename,
   SettingsManager& mgr, const TrackInfo& trackinfo,
   SampleContainer& samplecont)
{
   InitInopt();
   SetInputStreamInfos(samplecont);

   // set options from UI
   m_bitrateInBps = mgr.queryValueInt(OpusTargetBitrate) * 1000;
   m_complexity = mgr.queryValueInt(OpusComplexity);
   m_opusBitrateMode = mgr.queryValueInt(OpusBitrateMode);

   if (!StoreTrackInfos(trackinfo))
      return -1;

   if (!OpenOutputFile(outfilename))
      return -1;

   if (!WriteHeader())
      return -1;

   if (!InitEncoder())
      return -1;

   if (!SetEncoderOptions())
      return -1;

   m_samplerate = m_codingRate;

   // set up output traits
   samplecont.setOutputModuleTraits(m_floatMode ? 32 : 16, SamplesInterleaved, m_samplerate, m_channels);

   return 0;
}

int OpusOutputModule::encodeSamples(SampleContainer& samples)
{
   // TODO implement
   return 0;
}

void OpusOutputModule::doneOutput()
{
   m_encoder.reset();

   ogg_stream_clear(&os);

   if (inopt.rate != m_codingRate)
      clear_resample(&inopt);

   if (m_downmix)
      clear_downmix(&inopt);

   m_outputFile.reset();
}

void OpusOutputModule::InitInopt()
{
   inopt.channels = 2;
   inopt.rate = 48000;
   // 0 dB gain is recommended unless you know what you're doing
   inopt.gain = 0;
   inopt.samplesize = 16;
   inopt.endianness = 0;
   inopt.rawmode = 0;
   inopt.ignorelength = 0;
   inopt.copy_comments = 1;
   inopt.copy_pictures = 1;

   m_bitrateInBps = -1;
   m_complexity = 10;
}

void OpusOutputModule::SetInputStreamInfos(SampleContainer& samplecont)
{
   inopt.rate = samplecont.getInputModuleSampleRate();
   inopt.channels = m_channels = samplecont.getInputModuleChannels();
   inopt.samplesize = samplecont.getInputModuleBitsPerSample();
   inopt.total_samples_per_channel = 0;

   m_floatMode = samplecont.getInputModuleBitsPerSample() == 32;
}

bool OpusOutputModule::StoreTrackInfos(const TrackInfo& trackinfo)
{
   char ENCODER_string[1024];
   const char* opus_version = opus_get_version_string();

   // Vendor string should just be the encoder library,
   // the ENCODER comment specifies the tool used.
   comment_init(&inopt.comments, &inopt.comments_length, opus_version);
   snprintf(ENCODER_string, sizeof(ENCODER_string), "winLAME %ls", App::Version().GetString());
   comment_add(&inopt.comments, &inopt.comments_length, "ENCODER", ENCODER_string);

   CString text;
   if (trackinfo.TextInfo(TrackInfoArtist, text))
      comment_add(&inopt.comments, &inopt.comments_length, "artist", const_cast<char*>(CStringA(text).GetString()));

   if (trackinfo.TextInfo(TrackInfoTitle, text))
      comment_add(&inopt.comments, &inopt.comments_length, "title", const_cast<char*>(CStringA(text).GetString()));

   if (trackinfo.TextInfo(TrackInfoAlbum, text))
      comment_add(&inopt.comments, &inopt.comments_length, "album", const_cast<char*>(CStringA(text).GetString()));

   bool avail = false;
   int year = trackinfo.NumberInfo(TrackInfoYear, avail);
   if (avail && year != -1)
   {
      text.Format(_T("%i"), year);
      comment_add(&inopt.comments, &inopt.comments_length, "date", const_cast<char*>(CStringA(text).GetString()));
   }

   if (trackinfo.TextInfo(TrackInfoGenre, text))
      comment_add(&inopt.comments, &inopt.comments_length, "genre", const_cast<char*>(CStringA(text).GetString()));

   return true;
}

bool OpusOutputModule::InitEncoder()
{
   const opus_int32 bitrate = m_bitrateInBps;
   const int complexity = m_complexity;
   int& downmix = m_downmix;

   if (downmix == 0 && inopt.channels > 2 && bitrate > 0 && bitrate < (16000 * inopt.channels))
   {
      ATLTRACE("Notice: Surround bitrate less than 16kbit/sec/channel, downmixing.\n");
      downmix = inopt.channels > 8 ? 1 : 2;
   }

   if (downmix > 0 && downmix < inopt.channels) downmix = setup_downmix(&inopt, downmix);
   else downmix = 0;

   inopt.skip = 0;

   const opus_int32 rate = inopt.rate;
   opus_int32& coding_rate = m_codingRate;

   coding_rate = 48000;
   if (rate > 24000)coding_rate = 48000;
   else if (rate > 16000)coding_rate = 24000;
   else if (rate > 12000)coding_rate = 16000;
   else if (rate > 8000)coding_rate = 12000;
   else coding_rate = 8000;

   opus_int32 frame_size = 960;
   frame_size = frame_size / (48000 / coding_rate);

   // Scale the resampler complexity, but only for 48000 output because
   // the near-cutoff behavior matters a lot more at lower rates.
   if (rate != coding_rate)setup_resample(&inopt, coding_rate == 48000 ? (complexity + 1) / 2 : 5, coding_rate);

   if (rate != coding_rate && complexity != 10)
   {
      ATLTRACE("Notice: Using resampling with complexity<10.\n");
      ATLTRACE("Opusenc is fastest with 48, 24, 16, 12, or 8kHz input.\n\n");
   }

   // OggOpus headers FIXME: broke forcemono
   header.channels = inopt.channels;
   header.channel_mapping = header.channels > 8 ? 255 : header.channels > 2;
   header.input_sample_rate = rate;
   header.gain = inopt.gain;

   // Initialize OPUS encoder
   // Framesizes <10ms can only use the MDCT modes, so we switch on RESTRICTED_LOWDELAY
   // to save the extra 2.5ms of codec lookahead when we'll be using only small frames.
   OpusMSEncoder* st;
   int ret = 0;
   st = opus_multistream_surround_encoder_create(coding_rate, header.channels, header.channel_mapping, &header.nb_streams, &header.nb_coupled,
      header.stream_map, frame_size < 480 / (48000 / coding_rate) ? OPUS_APPLICATION_RESTRICTED_LOWDELAY : OPUS_APPLICATION_AUDIO, &ret);

   if (ret != OPUS_OK)
   {
      m_lasterror.Format(_T("Error cannot create encoder: %hs"), opus_strerror(ret));
      return false;
   }

   m_encoder.reset(st, opus_multistream_encoder_destroy);

   opus_int32 min_bytes;
   int max_frame_bytes;
   min_bytes = max_frame_bytes = (1275 * 3 + 7)*header.nb_streams;
   m_packetBuffer.resize(max_frame_bytes);

   return true;
}

bool OpusOutputModule::SetEncoderOptions()
{
   int& bitrate = m_bitrateInBps;
   const int complexity = m_complexity;
   const int rate = header.input_sample_rate;
   const opus_int32 coding_rate = m_codingRate;
   OpusMSEncoder* st = m_encoder.get();

   int with_hard_cbr = 0;
   int with_cvbr = 0;

   switch (m_opusBitrateMode)
   {
   case 0: // --vbr
      with_cvbr = 0;
      with_hard_cbr = 0;
      break;

   case 1: // --cvbr
      with_cvbr = 1;
      with_hard_cbr = 0;
      break;

   case 2: // --hard-cbr
      with_hard_cbr = 1;
      with_cvbr = 0;
      break;
   }

   if (bitrate < 0)
   {
      // Lower default rate for sampling rates [8000-44100) by a factor of (rate+16k)/(64k)
      bitrate = ((64000 * header.nb_streams + 32000 * header.nb_coupled)*
         (std::min(48, std::max(8, ((rate < 44100 ? rate : 48000) + 1000) / 1000)) + 16) + 32) >> 6;
   }

   if (bitrate > (1024000 * inopt.channels) || bitrate < 500) {
      m_lasterror.Format(_T("Error: Bitrate %d bits/sec is insane.\nDid you mistake bits for kilobits?\n"), bitrate);
      m_lasterror.Append(_T("--bitrate values from 6-256 kbit/sec per channel are meaningful.\n"));
      return false;
   }

   bitrate = std::min(inopt.channels * 256000, bitrate);

   int ret = opus_multistream_encoder_ctl(st, OPUS_SET_BITRATE(bitrate));
   if (ret != OPUS_OK)
   {
      m_lasterror.Format(_T("Error OPUS_SET_BITRATE returned: %hs\n"), opus_strerror(ret));
      return false;
   }

   ret = opus_multistream_encoder_ctl(st, OPUS_SET_VBR(!with_hard_cbr));
   if (ret != OPUS_OK)
   {
      m_lasterror.Format(_T("Error OPUS_SET_VBR returned: %hs\n"), opus_strerror(ret));
      exit(1);
   }

   if (!with_hard_cbr)
   {
      ret = opus_multistream_encoder_ctl(st, OPUS_SET_VBR_CONSTRAINT(with_cvbr));
      if (ret != OPUS_OK)
      {
         fprintf(stderr, "Error OPUS_SET_VBR_CONSTRAINT returned: %hs\n", opus_strerror(ret));
         exit(1);
      }
   }

   ret = opus_multistream_encoder_ctl(st, OPUS_SET_COMPLEXITY(complexity));
   if (ret != OPUS_OK)
   {
      m_lasterror.Format(_T("Error OPUS_SET_COMPLEXITY returned: %hs\n"), opus_strerror(ret));
      return false;
   }

   // Regardless of the rate we're coding at the ogg timestamping/skip is
   // always timed at 48000.

   header.preskip = (int)(inopt.skip*(48000. / coding_rate));
   // Extra samples that need to be read to compensate for the pre-skip
   inopt.extraout = (int)(header.preskip*(rate / 48000.));

   return true;
}

bool OpusOutputModule::OpenOutputFile(LPCTSTR outputFilename)
{
   FILE* fout = _tfopen(outputFilename, _T("wb"));

   if (fout != nullptr)
   {
      m_outputFile.reset(fout, ::fclose);
      return true;
   }

   return false;
}

bool OpusOutputModule::WriteHeader()
{
   FILE* fout = m_outputFile.get();

   // Initialize Ogg stream struct
   time_t start_time = time(NULL);
   srand((unsigned int)(((getpid() & 65535) << 15) ^ start_time));
   int serialno = rand();

   if (ogg_stream_init(&os, serialno) == -1)
   {
      m_lasterror = _T("Error: stream init failed\n");
      return false;
   }

   // Write header
   opus_int64& bytes_written = m_numBytesWritten;
   opus_int64& pages_out = m_numPagesWritten;
   int ret;
   {
      unsigned char header_data[100];
      int packet_size = opus_header_to_packet(&header, header_data, 100);
      op.packet = header_data;
      op.bytes = packet_size;
      op.b_o_s = 1;
      op.e_o_s = 0;
      op.granulepos = 0;
      op.packetno = 0;
      ogg_stream_packetin(&os, &op);

      while ((ret = ogg_stream_flush(&os, &og)))
      {
         if (!ret) break;
         ret = oe_write_page(&og, fout);
         if (ret != og.header_len + og.body_len)
         {
            m_lasterror = "Error: failed writing header to output stream\n";
            return false;
         }

         bytes_written += ret;
         pages_out++;
      }

      int comment_padding = 512;
      comment_pad(&inopt.comments, &inopt.comments_length, comment_padding);
      op.packet = (unsigned char *)inopt.comments;
      op.bytes = inopt.comments_length;
      op.b_o_s = 0;
      op.e_o_s = 0;
      op.granulepos = 0;
      op.packetno = 1;
      ogg_stream_packetin(&os, &op);
   }

   /* writing the rest of the opus header packets */
   while ((ret = ogg_stream_flush(&os, &og)))
   {
      if (!ret)break;
      ret = oe_write_page(&og, fout);
      if (ret != og.header_len + og.body_len)
      {
         m_lasterror = "Error: failed writing header to output stream\n";
         return false;
      }
      bytes_written += ret;
      pages_out++;
   }

   free(inopt.comments);

   return true;
}

// Note: The following code is from opusenc.c

/*
 Comments will be stored in the Vorbis style.
 It is describled in the "Structure" section of
    http://www.xiph.org/ogg/vorbis/doc/v-comment.html

 However, Opus and other non-vorbis formats omit the "framing_bit".

The comment header is decoded as follows:
  1) [vendor_length] = read an unsigned integer of 32 bits
  2) [vendor_string] = read a UTF-8 vector as [vendor_length] octets
  3) [user_comment_list_length] = read an unsigned integer of 32 bits
  4) iterate [user_comment_list_length] times {
     5) [length] = read an unsigned integer of 32 bits
     6) this iteration's user comment = read a UTF-8 vector as [length] octets
     }
  7) done.
*/

#define readint(buf, base) (((buf[base+3]<<24)&0xff000000)| \
                           ((buf[base+2]<<16)&0xff0000)| \
                           ((buf[base+1]<<8)&0xff00)| \
                           (buf[base]&0xff))
#define writeint(buf, base, val) do{ buf[base+3]=((val)>>24)&0xff; \
                                     buf[base+2]=((val)>>16)&0xff; \
                                     buf[base+1]=((val)>>8)&0xff; \
                                     buf[base]=(val)&0xff; \
                                 }while(0)

static void comment_init(char **comments, int* length, const char *vendor_string)
{
   /*The 'vendor' field should be the actual encoding library used.*/
   int vendor_length = strlen(vendor_string);
   int user_comment_list_length = 0;
   int len = 8 + 4 + vendor_length + 4;
   char *p = (char*)malloc(len);
   if (p == NULL) {
      fprintf(stderr, "malloc failed in comment_init()\n");
      exit(1);
   }
   memcpy(p, "OpusTags", 8);
   writeint(p, 8, vendor_length);
   memcpy(p + 12, vendor_string, vendor_length);
   writeint(p, 12 + vendor_length, user_comment_list_length);
   *length = len;
   *comments = p;
}

void comment_add(char **comments, int* length, char *tag, char *val)
{
   char* p = *comments;
   int vendor_length = readint(p, 8);
   int user_comment_list_length = readint(p, 8 + 4 + vendor_length);
   int tag_len = (tag ? strlen(tag) + 1 : 0);
   int val_len = strlen(val);
   int len = (*length) + 4 + tag_len + val_len;

   p = (char*)realloc(p, len);
   if (p == NULL) {
      fprintf(stderr, "realloc failed in comment_add()\n");
      exit(1);
   }

   writeint(p, *length, tag_len + val_len);      /* length of comment */
   if (tag) {
      memcpy(p + *length + 4, tag, tag_len);        /* comment tag */
      (p + *length + 4)[tag_len - 1] = '=';           /* separator */
   }
   memcpy(p + *length + 4 + tag_len, val, val_len);  /* comment */
   writeint(p, 8 + 4 + vendor_length, user_comment_list_length + 1);
   *comments = p;
   *length = len;
}

static void comment_pad(char **comments, int* length, int amount)
{
   if (amount > 0) {
      int i;
      int newlen;
      char* p = *comments;
      /*Make sure there is at least amount worth of padding free, and
         round up to the maximum that fits in the current ogg segments.*/
      newlen = (*length + amount + 255) / 255 * 255 - 1;
      p = (char*)realloc(p, newlen);
      if (p == NULL) {
         fprintf(stderr, "realloc failed in comment_pad()\n");
         exit(1);
      }
      for (i = *length; i < newlen; i++)p[i] = 0;
      *comments = p;
      *length = newlen;
   }
}
#undef readint
#undef writeint
