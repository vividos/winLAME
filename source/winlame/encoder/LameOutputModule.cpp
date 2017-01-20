//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file LameOutputModule.cpp
/// \brief contains the implementation of the LAME output module
//
#include "stdafx.h"
#include <fstream>
#include "resource.h"
#include "LameOutputModule.hpp"
#include "WaveMp3Header.hpp"
#include "Id3v1Tag.hpp"
#include "id3/file.h"
#include <id3tag.h>

using Encoder::LameOutputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

// static variables

nlame_instance_t* LameOutputModule::m_nogapInstance = nullptr;

LameOutputModule::LameOutputModule()
   :m_instance(nullptr),
   m_writeInfoTag(true),
   m_inputBufferFill(0),
   m_bufferType(nle_buffer_short),
   m_inputBufferSize(1152),
   m_nogapEncoding(false),
   m_nogapIsLastFile(false),
   m_writeWaveHeader(false),
   m_numSamplesEncoded(0),
   m_numDataBytesWritten(0)
{
   m_moduleId = ID_OM_LAME;
}

LameOutputModule::~LameOutputModule() throw()
{
}

bool LameOutputModule::IsAvailable() const
{
   // need at least version with the header we compiled against
   return nlame_get_api_version() >= NLAME_CURRENT_API_VERSION;
}

void LameOutputModule::GetVersionString(CString& version, int special) const
{
   // retrieve lame version
   if (IsAvailable())
   {
      switch (special)
      {
      case 0:
         version = CString(::nlame_lame_version_get(nle_lame_version_normal));
         break;
      case 1:
         version = CString(::nlame_lame_string_get(nle_lame_string_compiler));
         break;
      case 2:
         version = CString(::nlame_lame_string_get(nle_lame_string_cpu_features));
         break;
      }
   }
   else
   {
      if (special == 1)
         version = _T("N/A");
   }
}

void LameOutputModule::PrepareOutput(SettingsManager& mgr)
{
   m_writeWaveHeader = mgr.QueryValueInt(LameWriteWaveHeader) != 0;
}

/// error callback
void LameErrorCallback(const char* format, va_list args)
{
   CStringA buffer;
   buffer.FormatV(format, args);

   ATLTRACE(_T("%hs"), buffer.GetString());
}

int LameOutputModule::InitOutput(LPCTSTR outfilename,
   SettingsManager& mgr, const TrackInfo& trackInfo,
   SampleContainer& samples)
{
   // alloc memory for output mp3 buffer
   m_mp3OutputBuffer.resize(nlame_const_maxmp3buffer);

   m_channels = samples.GetInputModuleChannels();
   m_samplerate = samples.GetInputModuleSampleRate();

   // store track info for ID3v2 tag
   m_trackInfoID3v2 = trackInfo;

   // check if we do nogap encoding
   m_nogapEncoding = mgr.QueryValueInt(LameOptNoGap) == 1;

   if (m_nogapEncoding && m_nogapInstance != nullptr)
   {
      // use last stored nlame instance
      m_instance = m_nogapInstance;
      m_nogapInstance = nullptr;

      // update tag for next track
      if (!m_writeWaveHeader)
         AddLameID3v2Tag(m_trackInfoID3v2);

      // reinit bitstream with new tag infos
      nlame_reinit_bitstream(m_instance);
   }
   else
   {
      // init nlame
      m_instance = nlame_new();

      if (m_instance == nullptr)
      {
         m_lastError = _T("nlame_new() failed");
         return -1;
      }

      if (!m_writeWaveHeader)
         AddLameID3v2Tag(m_trackInfoID3v2);

      // set callbacks
      nlame_callback_set(m_instance, nle_callback_error, LameErrorCallback);

      // set all nlame variables
      int ret = SetEncodingParameters(mgr);

      if (ret < 0)
      {
         m_lastError = _T("nlame_init_params() failed");
         return ret;
      }
   }

   // do description string
   GenerateDescription(mgr);

   m_mp3Filename = outfilename;

   // always write VBR info tag
   //m_writeInfoTag = mgr.QueryValueInt(LameVBRWriteTag) == 1;

   m_nogapIsLastFile = mgr.QueryValueInt(GeneralIsLastFile) == 1;

   // generate info tag?
   nlame_var_set_int(m_instance, nle_var_vbr_generate_info_tag, m_writeInfoTag ? 1 : 0);

   // create id3 tag data
   if (!trackInfo.IsEmpty())
      m_ID33v1Tag.reset(new Id3v1Tag(trackInfo));

   // open output file
   m_outputFile.open(outfilename, std::ios::out | std::ios::binary);
   if (!m_outputFile.is_open())
   {
      m_lastError.LoadString(IDS_ENCODER_OUTPUT_FILE_CREATE_ERROR);
      return -1;
   }

   // set up output traits
   int bitsPerSample = 16;
   m_bufferType = nle_buffer_short;

   // beginning with version 2 of the nLAME API, the encoder can handle 32-bit
   // input samples
   if (nlame_get_api_version() >= 2 && samples.GetInputModuleBitsPerSample() > 16)
   {
      // Note: This code is currently off, since LAME doesn't export the
      // function lame_encode_buffer_interleaved_int() that would be needed
      // for 32-bit sample encoding. See also comment in nlame project, in
      // function nlame_encode_buffer_interleaved().
      //bitsPerSample=32;
      //m_bufferType = nle_buffer_int;
   }

   samples.SetOutputModuleTraits(bitsPerSample, SamplesInterleaved);

   // retrieve framesize from LAME encoder; varies from MPEG version and layer number
   int frameSize = nlame_var_get_int(m_instance, nle_var_framesize);
   if (frameSize <= 0)
      return -1;

   m_inputBufferSize = static_cast<unsigned int>(frameSize);

   m_inputBuffer.resize(m_inputBufferSize * m_channels * (bitsPerSample >> 3));
   m_inputBufferFill = 0;

   m_numSamplesEncoded = 0;
   m_numDataBytesWritten = 0;

   // write wave mp3 header when requested
   if (m_writeWaveHeader)
   {
      // write wave header
      WriteWaveMp3Header(m_outputFile,
         nlame_var_get_int(m_instance, nle_var_channel_mode) == nle_mode_mono ? 1 : 2,
         nlame_var_get_int(m_instance, nle_var_out_samplerate),
         nlame_var_get_int(m_instance, nle_var_bitrate),
         static_cast<unsigned short>(nlame_var_get_int(m_instance, nle_var_encoder_delay)));
   }

   return 0;
}

/// encodes exactly one frame, consisting of 576 samples per channel. this is
/// done due to the fact that LAME expects that number of samples, or it will
/// produce different output, e.g. when feeding less than 576 samples per call
/// to nlame_encode_buffer_*().
int LameOutputModule::EncodeFrame()
{
   // encode buffer
   int ret;
   if (m_channels == 1)
   {
      ret = nlame_encode_buffer_mono(m_instance, m_bufferType,
         m_inputBuffer.data(), m_inputBufferFill, m_mp3OutputBuffer.data(), m_mp3OutputBuffer.size());
   }
   else
   {
      ret = nlame_encode_buffer_interleaved(m_instance, m_bufferType,
         m_inputBuffer.data(), m_inputBufferFill, m_mp3OutputBuffer.data(), m_mp3OutputBuffer.size());
   }

   m_numSamplesEncoded += m_inputBufferFill;
   m_inputBufferFill = 0;

   // error?
   if (ret < 0)
      return ret;

   // write out data when available
   if (ret > 0)
   {
      m_outputFile.write(reinterpret_cast<char*>(m_mp3OutputBuffer.data()), ret);
      m_numDataBytesWritten += ret;
   }

   return ret;
}

int LameOutputModule::EncodeSamples(SampleContainer& samples)
{
   // get samples
   int numSamples = 0;
   unsigned char* sampleBuffer = (unsigned char*)samples.GetSamplesInterleaved(numSamples);

   unsigned int sampleBufferCount = 0;

   int ret = 0;
   int sampleSizeInBytes = samples.GetOutputModuleBitsPerSample() >> 3;
   do
   {
      unsigned int fillsize = std::min(m_inputBufferSize - m_inputBufferFill, static_cast<unsigned int>(numSamples));

      // copy samples into inbuffer; note: m_inputBufferFill is counted in "samples"
      memcpy(
         m_inputBuffer.data() + m_inputBufferFill * m_channels * sampleSizeInBytes,
         sampleBuffer + sampleBufferCount * m_channels * sampleSizeInBytes,
         fillsize * m_channels * sampleSizeInBytes);

      m_inputBufferFill += fillsize;
      numSamples -= fillsize;
      sampleBufferCount += fillsize;

      if (m_inputBufferFill == m_inputBufferSize)
      {
         // encode one frame
         ret = EncodeFrame();
         if (ret < 0)
            break;
      }

   } while (numSamples > 0);

   return ret;
}

void LameOutputModule::DoneOutput()
{
   // encode remaining samples, if any
   EncodeFrame();

   // finish encoding
   int ret;

   if (m_nogapEncoding && !m_nogapIsLastFile)
   {
      ret = nlame_encode_flush_nogap(m_instance, m_mp3OutputBuffer.data(), nlame_const_maxmp3buffer);
   }
   else
   {
      ret = nlame_encode_flush(m_instance, m_mp3OutputBuffer.data(), nlame_const_maxmp3buffer);
   }

   if (ret > 0)
   {
      m_outputFile.write(reinterpret_cast<char*>(m_mp3OutputBuffer.data()), ret);
      m_numDataBytesWritten += ret;
   }

   // write id3 tag when available
   // note: we write id3 tag when we do gapless encoding, too, since
   //       most decoders should be aware now
   // note: we don't write id3 tags to wave files when we wrote a wave header,
   //       since that that might confuse some software
   if (!m_writeWaveHeader && /* !m_nogapEncoding && */ m_ID33v1Tag != nullptr)
   {
      m_outputFile.write((char*)m_ID33v1Tag->GetData(), 128);
   }

   if (m_writeWaveHeader)
   {
      // fix up fact chunk and riff header lengths; seeks around a bit
      FixupWaveMp3Header(m_outputFile, m_numDataBytesWritten, m_numSamplesEncoded);
   }

   // close file
   m_outputFile.close();

   // write ID3v2 tag
   // note: we re-write the ID3v2 tag here, previously written by AddLameID3v2Tag().
   // This converts the ID3v2.3 tag to ID3v2.4.
   if (!m_writeWaveHeader)
      WriteID3v2Tag();

   // add VBR info tag to mp3 file
   // note: since nlame_write_vbr_infotag() seeks to the front of the output
   //       file, the wave header might get overwritten, so we don't write a
   //       info tag when writing a wave header
   if (m_writeInfoTag && !m_nogapEncoding && !m_writeWaveHeader)
      WriteVBRInfoTag(m_instance, m_mp3Filename);

   if (m_nogapEncoding && !m_nogapIsLastFile)
   {
      m_nogapInstance = m_instance;
   }
   else
   {
      // free nlame instance
      nlame_delete(m_instance);
   }

   m_instance = nullptr;
}

int LameOutputModule::SetEncodingParameters(SettingsManager& mgr)
{
   nlame_var_set_int(m_instance, nle_var_in_samplerate, m_samplerate);
   nlame_var_set_int(m_instance, nle_var_num_channels, m_channels);

   // mono encoding?
   bool bMono = mgr.QueryValueInt(LameSimpleMono) == 1;

   // set mono encoding, else let LAME choose the default (which is joint stereo)
   if (bMono)
      nlame_var_set_int(m_instance, nle_var_channel_mode, nle_mode_mono);

   // which mode? 0: bitrate mode, 1: quality mode
   if (mgr.QueryValueInt(LameSimpleQualityOrBitrate) == 0)
   {
      // bitrate mode
      int nBitrate = mgr.QueryValueInt(LameSimpleBitrate);

      if (mgr.QueryValueInt(LameSimpleCBR) == 1)
      {
         // CBR
         nlame_var_set_int(m_instance, nle_var_vbr_mode, nle_vbr_mode_off);
         nlame_var_set_int(m_instance, nle_var_bitrate, nBitrate);
      }
      else
      {
         // ABR
         nlame_var_set_int(m_instance, nle_var_vbr_mode, nle_vbr_mode_abr);
         nlame_var_set_int(m_instance, nle_var_abr_mean_bitrate, nBitrate);
      }
   }
   else
   {
      // quality mode; value ranges from 0 to 9
      int quality = mgr.QueryValueInt(LameSimpleQuality);

      nlame_var_set_int(m_instance, nle_var_vbr_quality, quality);

      // VBR mode; LameSimpleVBRMode, 0: standard, 1: fast
      int vbrMode = mgr.QueryValueInt(LameSimpleVBRMode);

      if (vbrMode == 0)
         nlame_var_set_int(m_instance, nle_var_vbr_mode, nle_vbr_mode_old); // standard
      else
         nlame_var_set_int(m_instance, nle_var_vbr_mode, nle_vbr_mode_new); // fast
   }

   // encode quality; LameSimpleEncodeQuality, 0: fast, 1: standard, 2: high
   // fast maps to quality factor 7, high is quality factor 2
   // note: this currently is hard coded; maybe move this into nlame?
   int encodingQuality = mgr.QueryValueInt(LameSimpleEncodeQuality);

   // mapping:
   // 0: "fast", or -f
   // 1: "standard" (no -q switch)
   // 2: "high", or -h
   // note: when using "standard" encoding quality we don't set nle_var_quality,
   // since the LAME engine then chooses the default quality value.
   if (encodingQuality == 0)
      nlame_var_set_int(m_instance, nle_var_quality, nlame_var_get_int(m_instance, nle_var_quality_value_fast));
   else if (encodingQuality == 2)
      nlame_var_set_int(m_instance, nle_var_quality, nlame_var_get_int(m_instance, nle_var_quality_value_high));

   // always use replay gain, and decode on-the-fly to get the peak sample
   // the result is written into the LAME VBR Info tag
   nlame_var_set_int(m_instance, nle_var_find_replay_gain, 1);
   nlame_var_set_int(m_instance, nle_var_decode_on_the_fly, 1);

   // init more settings in nlame
   return nlame_init_params(m_instance);
}

void LameOutputModule::GenerateDescription(SettingsManager& mgr)
{
   CString text;

   int encodingQuality = mgr.QueryValueInt(LameSimpleEncodeQuality);

   text.Format(IDS_FORMAT_INFO_LAME,
      encodingQuality == 0 ? _T("Fast") : encodingQuality == 1 ? _T("Standard") : _T("High"),
      mgr.QueryValueInt(LameSimpleMono) == 0 ? _T("") : _T(" (mono)"));

   CString format;

   // quality or bitrate mode?
   int qualityOrBitrate = mgr.QueryValueInt(LameSimpleQualityOrBitrate);
   if (qualityOrBitrate == 0) // bitrate
   {
      int nBitrate = mgr.QueryValueInt(LameSimpleBitrate);
      format.Format(
         mgr.QueryValueInt(LameSimpleCBR) == 0 ? IDS_FORMAT_INFO_LAME_ABR : IDS_FORMAT_INFO_LAME_CBR,
         nBitrate);

      text += format;
   }
   else // quality
   {
      int quality = mgr.QueryValueInt(LameSimpleQuality);
      format.Format(IDS_FORMAT_INFO_LAME_VBR,
         quality,
         mgr.QueryValueInt(LameSimpleVBRMode) == 0 ? _T("Standard") : _T("Fast"));
      text += format;
   }

   // stereo mode
   int value = nlame_var_get_int(m_instance, nle_var_channel_mode);
   text += (value == nle_mode_stereo ? _T("Stereo") :
      value == nle_mode_joint_stereo ? _T("Joint Stereo") : _T("Mono"));

   // nogap option
   if (m_nogapEncoding)
      text += _T(", gapless encoding");

   m_description = text;
}

void LameOutputModule::AddLameID3v2Tag(const TrackInfo& trackInfo)
{
   // add ID3v2 tags using LAME's functions; note that this writes ID3v2 version 2.3 tags
   // with text encoded as ISO-8859-1 (Latin1) that may not be able to represent all
   // characters we have

   unsigned int paddingLength = GetID3v2PaddingLength();
   nlame_id3tag_init(m_instance, false, true, paddingLength);

   // add all tags
   bool isAvail = false;
   CString textValue = trackInfo.TextInfo(TrackInfoTitle, isAvail);
   if (isAvail)
      nlame_id3tag_setfield_latin1(m_instance, nif_title, CStringA(textValue));

   textValue = trackInfo.TextInfo(TrackInfoArtist, isAvail);
   if (isAvail)
      nlame_id3tag_setfield_latin1(m_instance, nif_artist, CStringA(textValue));

   textValue = trackInfo.TextInfo(TrackInfoComment, isAvail);
   if (isAvail)
      nlame_id3tag_setfield_latin1(m_instance, nif_comment, CStringA(textValue));

   textValue = trackInfo.TextInfo(TrackInfoAlbum, isAvail);
   if (isAvail)
      nlame_id3tag_setfield_latin1(m_instance, nif_album, CStringA(textValue));

   // numeric
   int intValue = trackInfo.NumberInfo(TrackInfoYear, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);
      nlame_id3tag_setfield_latin1(m_instance, nif_year, CStringA(textValue));
   }

   intValue = trackInfo.NumberInfo(TrackInfoTrack, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);
      nlame_id3tag_setfield_latin1(m_instance, nif_track, CStringA(textValue));
   }

   textValue = trackInfo.TextInfo(TrackInfoGenre, isAvail);
   if (isAvail)
   {
      nlame_id3tag_setfield_latin1(m_instance, nif_genre, CStringA(textValue));
   }
}

unsigned int LameOutputModule::GetID3v2PaddingLength()
{
   unsigned int length = 0;

   const TrackInfo& trackInfo = m_trackInfoID3v2;

   // add all frames
   bool isAvail = false;
   CString textValue = trackInfo.TextInfo(TrackInfoTitle, isAvail);
   if (isAvail)
      length += textValue.GetLength() + 1;

   textValue = trackInfo.TextInfo(TrackInfoArtist, isAvail);
   if (isAvail)
      length += textValue.GetLength() + 1;

   textValue = trackInfo.TextInfo(TrackInfoComment, isAvail);
   if (isAvail)
      length += textValue.GetLength() + 1;

   textValue = trackInfo.TextInfo(TrackInfoAlbum, isAvail);
   if (isAvail)
      length += textValue.GetLength() + 1;

   // numeric

   int intValue = trackInfo.NumberInfo(TrackInfoYear, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);
      length += textValue.GetLength() + 1;
   }

   intValue = trackInfo.NumberInfo(TrackInfoTrack, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);
      length += textValue.GetLength() + 1;
   }

   textValue = trackInfo.TextInfo(TrackInfoGenre, isAvail);
   if (isAvail)
   {
      length += textValue.GetLength() + 1;
   }

   return unsigned((length * 4) * 1.5); // multiply with 4 for UCS-32, and add 50% overhead due to tag headers
}

void LameOutputModule::WriteID3v2Tag()
{
   ID3::File file(m_mp3Filename, false); // read-write

   // get primary tag
   ID3::Tag tag = file.GetTag();

   tag.SetOption(ID3::Tag::foID3v1, 0);

   const TrackInfo& trackInfo = m_trackInfoID3v2;

   // add all frames
   bool isAvail = false;
   CString textValue = trackInfo.TextInfo(TrackInfoTitle, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Title);

      ID3::Frame frame(ID3::FrameId::Title);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoArtist, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Artist);

      ID3::Frame frame(ID3::FrameId::Artist);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoComment, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Comment);

      ID3::Frame frame(ID3::FrameId::Comment);
      // COMM field is layout differently: 0: ID3_FIELD_TYPE_TEXTENCODING, 1: ID3_FIELD_TYPE_LANGUAGE, 2: ID3_FIELD_TYPE_STRING, 3: ID3_FIELD_TYPE_STRINGFULL
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, _T("eng")); // it's what Mp3tag writes
      frame.SetString(3, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoAlbum, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::AlbumTitle);

      ID3::Frame frame(ID3::FrameId::AlbumTitle);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   // numeric

   int intValue = trackInfo.NumberInfo(TrackInfoYear, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);

      tag.RemoveFrame(ID3::FrameId::RecordingTime);

      ID3::Frame frame(ID3::FrameId::RecordingTime);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   intValue = trackInfo.NumberInfo(TrackInfoTrack, isAvail);
   if (isAvail)
   {
      textValue.Format(_T("%i"), intValue);

      tag.RemoveFrame(ID3::FrameId::TrackNumber);

      ID3::Frame frame(ID3::FrameId::TrackNumber);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   textValue = trackInfo.TextInfo(TrackInfoGenre, isAvail);
   if (isAvail)
   {
      tag.RemoveFrame(ID3::FrameId::Genre);

      ID3::Frame frame(ID3::FrameId::Genre);
      frame.SetTextEncoding(0, ID3_FIELD_TEXTENCODING_ISO_8859_1);
      frame.SetString(1, textValue);
      tag.AttachFrame(frame);
   }

   file.Update();
}

void LameOutputModule::WriteVBRInfoTag(nlame_instance_t* instance, LPCTSTR mp3Filename)
{
   FILE* fp = _wfopen(mp3Filename, _T("r+b"));

   nlame_write_vbr_infotag(instance, fp);

   fclose(fp);
}
