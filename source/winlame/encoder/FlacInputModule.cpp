/*
   winLAME - a frontend for the LAME encoding engine
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

   $Id: FlacInputModule.cpp,v 1.11 2010/06/28 18:51:27 vividos Exp $

*/
/*! \file FlacInputModule.cpp

   \brief contains the implementation of the Flac input module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "FlacInputModule.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "FLAC/metadata.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// linker options

#if _MSC_VER < 1400
#pragma comment(linker, "/delayload:libFLAC.dll")
#endif

// constants

/// frame size
const unsigned int FLAC_FRAME_SIZE = 576; /* default=4608 */

// callbacks

static FLAC__StreamDecoderWriteStatus FLAC_WriteCallback(
   const FLAC__FileDecoder *decoder,
   const FLAC__Frame *frame,
   const FLAC__int32 *const buffer[],
   void *client_data)
{
   FLAC_context *context = (FLAC_context *)client_data;

   const unsigned channels = context->streaminfo.channels;
   const unsigned wide_samples = frame->header.blocksize;
   const unsigned bytes_per_sample = (context->streaminfo.bits_per_sample/8);
   unsigned wide_sample, sample, channel;
   
   if(context->abort_flag)
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

   for(sample = context->samples_in_reservoir*channels, wide_sample = 0;
       wide_sample < wide_samples;
      wide_sample++)
   {
      for(channel = 0; channel < channels; channel++, sample++)
      {
         context->reservoir[sample] = buffer[channel][wide_sample];
      }
   }
   
   context->samples_in_reservoir += wide_samples;   

   return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void FLAC_vorbis_comment_split_name_value(
   const FLAC__StreamMetadata_VorbisComment_Entry entry,
   CString& name, CString& value)
{
   int idx = 0;
   CString cszTemp(reinterpret_cast<char*>(entry.entry), entry.length);
   int iPos = cszTemp.Find(_T('='));
   name = cszTemp.Left(iPos);
   value = cszTemp.Mid(iPos+1);
}

static void FLAC_MetadataCallback(const FLAC__FileDecoder *decoder,
                          const FLAC__StreamMetadata *metadata,
                          void *client_data)
{   
   FLAC_context *context = (FLAC_context *)client_data;
   
//   context->streaminfo = metadata->data.stream_info;
   switch (metadata->type)
   {
   case FLAC__METADATA_TYPE_STREAMINFO:
   	context->streaminfo = metadata->data.stream_info;
      break;
   case FLAC__METADATA_TYPE_VORBIS_COMMENT:
      for (unsigned i = 0; i < metadata->data.vorbis_comment.num_comments; i++)
      {
         CString name, value;

         FLAC_vorbis_comment_split_name_value(
            metadata->data.vorbis_comment.comments[i],
            name, value);

         if (name.CompareNoCase(_T("title")) == 0)
         {
            context->trackInfo->TextInfo(TrackInfoTitle, value);
         }
         else if (name.CompareNoCase(_T("artist")) == 0)
         {
            context->trackInfo->TextInfo(TrackInfoArtist, value);
         }
         else if (name.CompareNoCase(_T("album")) == 0)
         {
            context->trackInfo->TextInfo(TrackInfoAlbum, value);
         }
         else if (name.CompareNoCase(_T("comment")) == 0)
         {
            context->trackInfo->TextInfo(TrackInfoComment, value);
         }
         else if (name.CompareNoCase(_T("genre")) == 0)
         {
            if (!value.IsEmpty())
               context->trackInfo->TextInfo(TrackInfoGenre, value);
         }
         else if (name.CompareNoCase(_T("tracknumber")) == 0)
         {
            context->trackInfo->NumberInfo(TrackInfoTrack, _ttoi(value));
         }
         else if (name.CompareNoCase(_T("year")) == 0)
         {
            context->trackInfo->NumberInfo(TrackInfoYear, _ttoi(value));
         }
         else if (name.CompareNoCase(_T("date")) == 0)
         {
            // Sanity check: Many FLACs seem to have 'date' actually containing
            // only 4-digit year. If this is the case, use 'date' for 'year'.
            if (value.GetLength() == 4 &&
                value.SpanIncluding(_T("0123456789")).GetLength() == 4)
            {
               context->trackInfo->NumberInfo(TrackInfoYear, _ttoi(value));
            }
         }
      }
      break;
   }

}

static void FLAC_ErrorCallback(const FLAC__FileDecoder *decoder,
                        FLAC__StreamDecoderErrorStatus status,
                        void *client_data)
{
   FLAC_context *context = (FLAC_context *)client_data;

   if(status != FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
      context->abort_flag = true;
}

/// pack function to store sample data in byte array
unsigned FLAC__pack_pcm_signed_little_endian(FLAC__byte *data, FLAC__int32 *input, unsigned wide_samples, unsigned channels, unsigned source_bps)
{
   FLAC__byte * const start = data;
   FLAC__int32 sample;
   unsigned samples = wide_samples * channels;
   const unsigned bytes_per_sample = source_bps / 8;
   
   while(samples--)
   {
      sample = *input++;
      
      switch(source_bps) 
      {
      case 8:
         data[0] = sample ^ 0x80;
         break;
      case 24:
         data[2] = (FLAC__byte)(sample >> 16);
         // fall through
      case 16:
         data[1] = (FLAC__byte)(sample >> 8);
         data[0] = (FLAC__byte)sample;
      }
      
      data += bytes_per_sample;
   }

   return data - start;
}


// FlacInputModule methods

FlacInputModule::FlacInputModule()
{
   module_id = ID_IM_FLAC;
}

InputModule *FlacInputModule::cloneModule()
{
   return new FlacInputModule;
}

bool FlacInputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("libFLAC.dll"));
   bool avail = dll != NULL;

   if (avail)
   {
      avail = GetProcAddress(dll, "FLAC__file_decoder_new") != NULL;
      ::FreeLibrary(dll);
   }

   return avail;
}

void FlacInputModule::getDescription(CString& desc)
{
   // format string
   desc.Format(IDS_FORMAT_INFO_FLAC_INPUT,
      (filelen << 3 / context->totalLenMs) / 1000,
      context->streaminfo.sample_rate,
      context->streaminfo.channels,
      context->streaminfo.bits_per_sample);
}

void FlacInputModule::getVersionString(CString& version, int special)
{
   HMODULE dll = ::LoadLibrary(_T("libFLAC.dll"));
   if (dll != NULL)
   {
      version = *(const char**)::GetProcAddress(dll, "FLAC__VERSION_STRING");
      ::FreeLibrary(dll);
   }
}

CString FlacInputModule::getFilterString()
{
   CString cszFilter;
   cszFilter.LoadString(IDS_FILTER_FLAC_INPUT);
   return cszFilter;
}

int FlacInputModule::initInput(LPCTSTR infilename, SettingsManager &mgr,
   TrackInfo &trackinfo, SampleContainer &samplecont)
{
   USES_CONVERSION;

   // find out length of file
   struct _stat statbuf;
   ::_tstat(infilename,&statbuf);
   filelen = statbuf.st_size; // 32 bit max.

   // initialize flac
   pFLACDec = FLAC__file_decoder_new();
   if(!pFLACDec)
   {
      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_DECODER);
      return -1;
   }

   context = new FLAC_context;
   memset((void *)context, 0, sizeof(FLAC_context));
   context->trackInfo = &trackinfo;

   FLAC__file_decoder_set_client_data(pFLACDec, context);

   if(!FLAC__file_decoder_set_filename(pFLACDec, T2CA(wlGetAnsiCompatFilename(infilename))))
   {
      if(pFLACDec)
      {
         FLAC__file_decoder_finish(pFLACDec);
         FLAC__file_decoder_delete(pFLACDec);
      }
      pFLACDec = NULL;

      lasterror.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   FLAC__file_decoder_set_write_callback(pFLACDec, FLAC_WriteCallback);
   FLAC__file_decoder_set_metadata_callback(pFLACDec, FLAC_MetadataCallback);
   FLAC__file_decoder_set_metadata_respond_all(pFLACDec);
   FLAC__file_decoder_set_error_callback(pFLACDec, FLAC_ErrorCallback);

   if(FLAC__file_decoder_init(pFLACDec) != FLAC__FILE_DECODER_OK)
   {
      if(pFLACDec)
      {
         FLAC__file_decoder_finish(pFLACDec);
         FLAC__file_decoder_delete(pFLACDec);
      }
      pFLACDec = NULL;

      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_ENCODER);
      return -1;
   }

   if(!FLAC__file_decoder_process_until_end_of_metadata(pFLACDec))
   {
      lasterror.LoadString(IDS_ENCODER_ERROR_GET_FILE_INFOS);
      return -1;
   }

   pos_sample = 0;
   context->totalLenMs = static_cast<unsigned int>(context->streaminfo.total_samples * 1000 / context->streaminfo.sample_rate);
   PCMBuffLen = (FLAC_FRAME_SIZE * context->streaminfo.channels * context->streaminfo.bits_per_sample);   
   context->reservoir = new FLAC__int32[context->streaminfo.max_blocksize * context->streaminfo.channels * 2];

   // set up input traits
   samplecont.setInputModuleTraits(context->streaminfo.bits_per_sample,SamplesChannelArray,
      context->streaminfo.sample_rate,context->streaminfo.channels);

   return 0;
}

void FlacInputModule::getInfo(int &channels, int &bitrate, int &length, int &samplerate)
{
   channels = context->streaminfo.channels;
   bitrate = filelen << 3 / context->totalLenMs;
   length = context->totalLenMs / 1000;
   samplerate = context->streaminfo.sample_rate;
}

int FlacInputModule::decodeSamples(SampleContainer &samples)
{
   FLAC__int32 *buff = new FLAC__int32[PCMBuffLen];

   while(context->samples_in_reservoir < FLAC_FRAME_SIZE)
   {
      if(FLAC__file_decoder_get_state(pFLACDec) == FLAC__FILE_DECODER_END_OF_FILE)
      {
         return 0;
      }
      else if(!FLAC__file_decoder_process_single(pFLACDec))
      {
         return 0;
      }
   }

   const unsigned bytes_per_sample = context->streaminfo.bits_per_sample/8;
   unsigned i, n = std::min(context->samples_in_reservoir, FLAC_FRAME_SIZE), delta;
   const int bytes = n * context->streaminfo.channels * bytes_per_sample;
   
   FLAC__pack_pcm_signed_little_endian((unsigned char *)buff,context->reservoir,n,
      context->streaminfo.channels,context->streaminfo.bits_per_sample);
   delta = i = n * context->streaminfo.channels;
   for( ; i < context->samples_in_reservoir * context->streaminfo.channels; i++)
      context->reservoir[i-delta] = context->reservoir[i];
   context->samples_in_reservoir -= n;
   pos_sample += n;
   
   // copy the samples to the sample container
   samples.putSamplesInterleaved(buff, n);

   delete[] buff;
   
   return n;
}

float FlacInputModule::percentDone()
{
   return float(__int64(pos_sample))*100.f / __int64(context->streaminfo.total_samples);
}

void FlacInputModule::doneInput()
{
   if(pFLACDec)
   {
      FLAC__file_decoder_finish(pFLACDec);
      FLAC__file_decoder_delete(pFLACDec);
   }
   pFLACDec = NULL;

   if (context)
   {
      if(context->reservoir)
         delete[] context->reservoir;
      context->reservoir = NULL;

      delete context;
      context = NULL;
   }
}

