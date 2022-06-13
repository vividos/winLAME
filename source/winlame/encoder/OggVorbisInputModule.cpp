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
/// \file OggVorbisInputModule.cpp
/// \brief contains the implementation of the ogg vorbis input module
//
#include "stdafx.h"
#include "OggVorbisInputModule.hpp"
#include "resource.h"
#include <fstream>
#include <ulib/UTF8.hpp>
#include <../include/opus/opusfile.h>
#include "ChannelRemapper.hpp"

using Encoder::OggVorbisInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

extern CString GetOggVorbisVersionString();

/// ogg vorbis input buffer size
const int c_oggInputBufferSize = 512 * 6; // must be a multiple of max channels

static size_t ReadDataSource(void* buffer, size_t size, size_t count, void* dataSource)
{
   return fread(buffer, size, count, reinterpret_cast<FILE*>(dataSource));
}

static int SeekDataSource(void* dataSource, ogg_int64_t offset, int whence)
{
   return _fseeki64(reinterpret_cast<FILE*>(dataSource), offset, whence);
}

static int CloseDataSource(void* dataSource)
{
   return fclose(reinterpret_cast<FILE*>(dataSource));
}

static long FilePosDataSource(void* dataSource)
{
   return ftell(reinterpret_cast<FILE*>(dataSource));
}

/// ogg vorbis reading callbacks
ov_callbacks c_callbacks =
{
   &::ReadDataSource,
   &::SeekDataSource,
   &::CloseDataSource,
   &::FilePosDataSource
};

OggVorbisInputModule::OggVorbisInputModule()
   :m_numCurrentSamples(0),
   m_numMaxSamples(0),
   m_inputFile(nullptr)
{
   m_moduleId = ID_IM_OGGV;

   memset(&m_vf, 0, sizeof(m_vf));
}

OggVorbisInputModule::~OggVorbisInputModule()
{
}

Encoder::InputModule* OggVorbisInputModule::CloneModule()
{
   return new OggVorbisInputModule;
}

bool OggVorbisInputModule::IsAvailable() const
{
   // we don't do delay-loading anymore, so it's available always
   return true;
}

CString OggVorbisInputModule::GetDescription() const
{
   const vorbis_info* vi = ov_info(&m_vf, -1);

   if (vi == nullptr)
      return CString(); // file not open

   CString desc;
   if (vi->bitrate_upper != -1 && vi->bitrate_lower != -1)
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_VBR,
         vi->bitrate_lower / 1000,
         vi->bitrate_upper / 1000,
         m_channels,
         m_samplerate);
   }
   else if (vi->bitrate_nominal == -1)
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_FREE,
         m_channels,
         m_samplerate);
   }
   else
   {
      desc.Format(IDS_FORMAT_INFO_OGGV_INPUT_NOMINAL,
         vi->bitrate_nominal / 1000,
         m_channels,
         m_samplerate);
   }

   return desc;
}

void OggVorbisInputModule::GetVersionString(CString& version, int special) const
{
   if (!IsAvailable())
      return;

   version = GetOggVorbisVersionString();
}

CString OggVorbisInputModule::GetFilterString() const
{
   CString filterString;
   filterString.LoadString(IDS_FILTER_OGG_VORBIS_INPUT);
   return filterString;
}

int OggVorbisInputModule::InitInput(LPCTSTR m_inputFilename,
   SettingsManager& mgr, TrackInfo& trackInfo,
   SampleContainer& samplecont)
{
   IsAvailable();

   m_inputFile = _wfopen(m_inputFilename, _T("rb"));

   if (m_inputFile == NULL)
   {
      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   // open ogg vorbis file
   if (ov_open_callbacks(m_inputFile, &m_vf, NULL, 0, c_callbacks) < 0)
   {
      m_lastError.Format(IDS_ENCODER_INVALID_FILE_FORMAT);
      return -2;
   }

   // retrieve file infos
   m_numCurrentSamples = 0;
   m_numMaxSamples = ov_pcm_total(&m_vf, -1);

   vorbis_info *vi = ov_info(&m_vf, -1);
   m_channels = vi->channels;
   m_samplerate = vi->rate;

   // set up input traits
   samplecont.SetInputModuleTraits(sizeof(short) * 8, SamplesInterleaved, m_samplerate, m_channels);

   GetTrackInfo(trackInfo);

   return 0;
}

void OggVorbisInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   vorbis_info *vi = ov_info(&m_vf, -1);

   numChannels = vi->channels;
   bitrateInBps = vi->bitrate_nominal;
   lengthInSeconds = int(m_numMaxSamples / vi->rate);
   samplerateInHz = vi->rate;
}

int OggVorbisInputModule::DecodeSamples(SampleContainer& samples)
{
   short buffer[c_oggInputBufferSize];
   int bitstream;

   // temporary sample buffer
   short* outputBuffer;
   short tempBuffer[c_oggInputBufferSize];

   // read in samples
   int ret = ov_read(&m_vf,
      reinterpret_cast<char*>(buffer),
      c_oggInputBufferSize * sizeof(short),
      0,
      sizeof(short),
      1,
      &bitstream);

   if (ret < 0)
   {
      m_lastError.LoadString(IDS_ENCODER_INTERNAL_DECODE_ERROR);
      return ret;
   }

   ret /= m_channels * sizeof(short);

   // channel remap
   if (m_channels > 2 && ret > 0)
   {
      outputBuffer = tempBuffer;
      ChannelRemapper::RemapInterleaved(T_enChannelMapType::oggVorbisInputChannelMap,
         buffer, ret, m_channels, outputBuffer);
   }
   else
      outputBuffer = buffer;

   if (ret > 0)
      samples.PutSamplesInterleaved(outputBuffer, ret);

   m_numCurrentSamples += ret;

   return ret;
}

void OggVorbisInputModule::DoneInput()
{
   ov_clear(&m_vf);

   fclose(m_inputFile);
}

void OggVorbisInputModule::GetTrackInfo(TrackInfo& trackInfo)
{
   vorbis_comment* comment = ov_comment(&m_vf, 0);

   const char* utf8text = nullptr;
   CString text;
   if (vorbis_comment_query_count(comment, "artist") > 0)
   {
      utf8text = vorbis_comment_query(comment, "artist", 0);
      text = UTF8ToString(utf8text);

      trackInfo.SetTextInfo(TrackInfoArtist, text);
   }

   if (vorbis_comment_query_count(comment, "title") > 0)
   {
      utf8text = vorbis_comment_query(comment, "title", 0);
      text = UTF8ToString(utf8text);

      trackInfo.SetTextInfo(TrackInfoTitle, text);
   }

   if (vorbis_comment_query_count(comment, "album") > 0)
   {
      utf8text = vorbis_comment_query(comment, "album", 0);
      text = UTF8ToString(utf8text);

      trackInfo.SetTextInfo(TrackInfoAlbum, text);
   }

   if (vorbis_comment_query_count(comment, "albumartist") > 0)
   {
      utf8text = vorbis_comment_query(comment, "albumartist", 0);
      text = UTF8ToString(utf8text);

      trackInfo.SetTextInfo(TrackInfoDiscArtist, text);
   }

   if (vorbis_comment_query_count(comment, "composer") > 0)
   {
      utf8text = vorbis_comment_query(comment, "composer", 0);
      text = UTF8ToString(utf8text);

      trackInfo.SetTextInfo(TrackInfoComposer, text);
   }

   if (vorbis_comment_query_count(comment, "comment") > 0)
   {
      utf8text = vorbis_comment_query(comment, "comment", 0);
      text = UTF8ToString(utf8text);

      trackInfo.SetTextInfo(TrackInfoComment, text);
   }

   if (vorbis_comment_query_count(comment, "genre") > 0)
   {
      utf8text = vorbis_comment_query(comment, "genre", 0);
      text = UTF8ToString(utf8text);

      trackInfo.SetTextInfo(TrackInfoGenre, text);
   }

   if (vorbis_comment_query_count(comment, "date") > 0)
   {
      utf8text = vorbis_comment_query(comment, "date", 0);
      text = UTF8ToString(utf8text);

      int year = _ttoi(text);

      if (year > 0)
         trackInfo.SetNumberInfo(TrackInfoYear, year);
   }

   if (vorbis_comment_query_count(comment, "tracknumber") > 0)
   {
      utf8text = vorbis_comment_query(comment, "tracknumber", 0);
      text = UTF8ToString(utf8text);

      int trackNumber = _ttoi(text);

      if (trackNumber > 0)
         trackInfo.SetNumberInfo(TrackInfoTrack, trackNumber);
   }

   if (vorbis_comment_query_count(comment, "discnumber") > 0)
   {
      utf8text = vorbis_comment_query(comment, "discnumber", 0);
      text = UTF8ToString(utf8text);

      int discNumber = _ttoi(text);

      if (discNumber > 0)
         trackInfo.SetNumberInfo(TrackInfoDiscNumber, discNumber);
   }

   if (vorbis_comment_query_count(comment, "METADATA_BLOCK_PICTURE") > 0)
   {
      // reuse the code that is using opusfile functions
      OpusPictureTag pictureTag = { 0 };
      opus_picture_tag_init(&pictureTag);

      const char* metadataBlock = vorbis_comment_query(comment, "METADATA_BLOCK_PICTURE", 0);
      int errorCode = opus_picture_tag_parse(&pictureTag, metadataBlock);

      if (errorCode == 0 &&
         pictureTag.data_length > 0)
      {
         const std::vector<unsigned char> binaryData(
            pictureTag.data, pictureTag.data + pictureTag.data_length);

         trackInfo.SetBinaryInfo(TrackInfoFrontCover, binaryData);
      }

      opus_picture_tag_clear(&pictureTag);
   }

   vorbis_comment_clear(comment);
}
