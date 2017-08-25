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
/// \file SndFileInputModule.cpp
/// \brief contains the implementation of the libsndfile input module
//
#include "stdafx.h"
#include "SndFileInputModule.hpp"
#include "resource.h"
#include "Id3v1Tag.hpp"
#include "SndFileFormats.hpp"
#include <ulib/DynamicLibrary.hpp>

using Encoder::SndFileInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

/// sndfile input buffer size
const int c_sndfileInputBufferSize = 512;

SndFileInputModule::SndFileInputModule()
   :m_sndfile(nullptr),
   m_sampleCount(0),
   m_numOutputBits(0)
{
   m_moduleId = ID_IM_SNDFILE;
   memset(&m_sfinfo, 0, sizeof(m_sfinfo));
}

Encoder::InputModule* SndFileInputModule::CloneModule()
{
   return new SndFileInputModule;
}

bool SndFileInputModule::IsAvailable() const
{
   DynamicLibrary dll(_T("libsndfile-1.dll"));

   bool avail = dll.IsLoaded();

   // check for old libsndfile.dll (pre-1.x); not supported
   if (dll.IsFunctionAvail("sf_get_lib_version"))
      avail = false;

   return avail;
}

CString SndFileInputModule::GetDescription() const
{
   CString formatName, outputExtension;
   SndFileFormats::GetFormatInfo(m_sfinfo.format & SF_FORMAT_TYPEMASK, formatName, outputExtension);

   CString subTypeName = SndFileFormats::GetSubTypeName(m_sfinfo.format & SF_FORMAT_SUBMASK);

   CString desc;
   desc.Format(IDS_FORMAT_INFO_SNDFILE,
      formatName.GetString(),
      subTypeName.GetString(),
      m_sfinfo.samplerate,
      m_sfinfo.channels);

   return desc;
}

void SndFileInputModule::GetVersionString(CString& version, int special) const
{
   if (IsAvailable())
   {
      char buffer[32];
      sf_command(nullptr, SFC_GET_LIB_VERSION, buffer, sizeof(buffer));

      version = CString(buffer + 11);
   }
}

CString SndFileInputModule::GetFilterString() const
{
   // build filter string when not already done
   if (m_filterString.IsEmpty())
   {
      std::vector<int> formatsList = SndFileFormats::EnumFormats();

      for (size_t formatIndex = 0, maxFormatIndex = formatsList.size(); formatIndex < maxFormatIndex; formatIndex++)
      {
         int format = formatsList[formatIndex];

         CString formatName, outputExtension;
         SndFileFormats::GetFormatInfo(format, formatName, outputExtension);

         // do format string
         CString temp;
         temp.Format(
            _T("%s [*.%s]|*.%s|"),
            formatName.GetString(),
            outputExtension.GetString(),
            outputExtension.GetString());

         m_filterString += temp;
      }
   }

   return m_filterString;
}

int SndFileInputModule::InitInput(LPCTSTR infilename,
   SettingsManager& mgr,
   TrackInfo& trackInfo,
   SampleContainer& samples)
{
   // resets the sample count
   m_sampleCount = 0;

   // opens the file for reading
   memset(&m_sfinfo, 0, sizeof(m_sfinfo));
#ifdef UNICODE
   m_sndfile = sf_wchar_open(infilename, SFM_READ, &m_sfinfo);
#else
   m_sndfile = sf_open(CStringA(GetAnsiCompatFilename(infilename)), SFM_READ, &m_sfinfo);
#endif

   if (m_sndfile == nullptr)
   {
      char buffer[512];
      sf_error_str(m_sndfile, buffer, 512);

      m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      m_lastError.AppendFormat(_T(" (%hs)"), buffer);

      return -1;
   }

   // when RIFF wave format, check for id3 tag info chunk
   if ((m_sfinfo.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV)
   {
      WaveGetID3Tag(infilename, trackInfo);
   }

   GetTrackInfos(trackInfo);

   switch (m_sfinfo.format & SF_FORMAT_SUBMASK)
   {
   case SF_FORMAT_PCM_S8:
   case SF_FORMAT_PCM_U8:
   case SF_FORMAT_PCM_16:
   case SF_FORMAT_DWVW_16:
      m_numOutputBits = 16;
      break;
   case SF_FORMAT_PCM_24:
   case SF_FORMAT_DWVW_24:
   case SF_FORMAT_PCM_32:
   case SF_FORMAT_FLOAT:
   case SF_FORMAT_DOUBLE:
      m_numOutputBits = 32;
      break;
   default:
      m_numOutputBits = 16;
      break;
   }

   // prepare input buffer
   if (m_numOutputBits == 32)
      m_buffer.resize(sizeof(int) * c_sndfileInputBufferSize * m_sfinfo.channels);
   else
   if (m_numOutputBits == 16)
      m_buffer.resize(sizeof(short) * c_sndfileInputBufferSize * m_sfinfo.channels);
   else
   {
      m_lastError.LoadString(IDS_ENCODER_INVALID_FILE_FORMAT);
      return -1;
   }

   // set up input traits
   samples.SetInputModuleTraits(m_numOutputBits, SamplesInterleaved,
      m_sfinfo.samplerate, m_sfinfo.channels);

   return 0;
}

void SndFileInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   if (m_sndfile == nullptr)
      return;

   numChannels = m_sfinfo.channels;

   // determine bitrate
   bitrateInBps = -1;

   switch (m_sfinfo.format & SF_FORMAT_SUBMASK)
   {
   case SF_FORMAT_PCM_S8:
   case SF_FORMAT_PCM_U8:
      bitrateInBps = m_sfinfo.samplerate * 8;
      break;

   case SF_FORMAT_PCM_16:
   case SF_FORMAT_DWVW_16:
      bitrateInBps = m_sfinfo.samplerate * 16;
      break;

   case SF_FORMAT_PCM_24:
   case SF_FORMAT_DWVW_24:
      bitrateInBps = m_sfinfo.samplerate * 24;
      break;

   case SF_FORMAT_PCM_32:
   case SF_FORMAT_FLOAT:
      bitrateInBps = m_sfinfo.samplerate * 32;
      break;

   case SF_FORMAT_DOUBLE:
      bitrateInBps = m_sfinfo.samplerate * 64;
      break;

   case SF_FORMAT_G721_32:
      bitrateInBps = 32000;
      break;

   case SF_FORMAT_G723_24:
      bitrateInBps = 24000;
      break;

   case SF_FORMAT_G723_40:
      bitrateInBps = 40000;
      break;

   case SF_FORMAT_DWVW_12:
      bitrateInBps = m_sfinfo.samplerate * 12;
      break;

   case SF_FORMAT_ULAW:
   case SF_FORMAT_ALAW:
   case SF_FORMAT_IMA_ADPCM:
   case SF_FORMAT_MS_ADPCM:
   case SF_FORMAT_GSM610:
   case SF_FORMAT_DWVW_N:
      break;
   }

   // calculate length of audio file
   if (m_sfinfo.samplerate != 0)
      lengthInSeconds = int(m_sfinfo.frames / m_sfinfo.samplerate);

   samplerateInHz = m_sfinfo.samplerate;
}

int SndFileInputModule::DecodeSamples(SampleContainer& samples)
{
   // read samples
   sf_count_t ret;

   int* intBuffer = reinterpret_cast<int*>(m_buffer.data());
   short* shortBuffer = reinterpret_cast<short*>(m_buffer.data());

   if (m_numOutputBits == 32)
      ret = sf_readf_int(m_sndfile, intBuffer, c_sndfileInputBufferSize);
   else
      ret = sf_readf_short(m_sndfile, shortBuffer, c_sndfileInputBufferSize);

   int iret = static_cast<int>(ret);

   if (ret < 0)
   {
      char buffer[512];
      sf_error_str(m_sndfile, buffer, 512);

      m_lastError = CString(buffer);
      return iret;
   }

   // put samples in container
   samples.PutSamplesInterleaved(m_buffer.data(), iret);

   // count samples
   m_sampleCount += iret;

   return iret;
}

float SndFileInputModule::PercentDone() const
{
   return m_sfinfo.frames != 0 ? float(m_sampleCount)*100.f / float(m_sfinfo.frames) : 0.f;
}

void SndFileInputModule::DoneInput()
{
   sf_close(m_sndfile);
}

bool SndFileInputModule::WaveGetID3Tag(LPCTSTR wavfile, TrackInfo& trackInfo)
{
   FILE* wav = _tfopen(wavfile, _T("rb"));
   if (!wav)
      return false;

   std::shared_ptr<FILE> wavFd(wav, fclose);

   // first check for last 0x80 bytes for id3 tag
   {
      int ret = fseek(wav, -128, SEEK_END);
      if (ret != 0)
      {
         ret = fseek(wav, 0, SEEK_SET);
         ATLASSERT(ret == 0); ret;
         return false;
      }

      // read in id3 tag
      Id3v1Tag id3tag;
      size_t sizeTagRead = fread(id3tag.GetData(), 1, 128, wav);
      if (sizeTagRead != 128)
      {
         ret = fseek(wav, 0, SEEK_SET);
         ATLASSERT(ret == 0); ret;
         return false;
      }

      if (id3tag.IsValidTag())
         id3tag.ToTrackInfo(trackInfo);

      ret = fseek(wav, 0, SEEK_SET);
      ATLASSERT(ret == 0); ret;
   }

   char buffer[5];
   buffer[4] = 0;

   // RIFF header
   size_t sizeRead = fread(buffer, 1, 4, wav);
   if (sizeRead != 4 || strcmp(buffer, "RIFF") != 0)
      return false;

   // file length
   sizeRead = fread(buffer, 1, 4, wav);
   if (sizeRead != 4)
      return false;

   int length = *((int*)buffer) + 8;

   // RIFF format type
   sizeRead = fread(buffer, 1, 4, wav);
   ATLTRACE(_T("RIFF type: %hs, length 0x%08x\n"), buffer, length);
   if (sizeRead != 4 || stricmp(buffer, "wave") != 0)
      return false;

   // now all chunks follow
   while (!feof(wav))
   {
      // chunk type
      sizeRead = fread(buffer, 1, 4, wav);
      ATLTRACE(_T("chunk %hs, "), buffer);

      if (sizeRead != 4 || strncmp(buffer, "TAG", 3) == 0)
         break;

      bool isid3 = strcmp(buffer, "id3 ") == 0;

      // chunk length
      sizeRead = fread(buffer, 1, 4, wav);
      if (sizeRead != 4)
         break;

      int clength = *((int*)buffer);
      ATLTRACE(_T("length 0x%08x\n"), clength);

      if (isid3)
      {
         // check tag length
         if (clength != 128)
            return false;

         // read in id3 tag
         Id3v1Tag id3tag;
         sizeRead = fread(id3tag.GetData(), 1, 128, wav);
         if (sizeRead != 128)
            break;

         if (id3tag.IsValidTag())
         {
            id3tag.ToTrackInfo(trackInfo);
            return true;
         }
      }
      else
      {
         // jump over chunk
         if (0 != fseek(wav, clength, SEEK_CUR))
            break;
      }

      // check if we are on the end
      int now = ftell(wav);
      if (now >= length)
         break;
   }

   return false;
}

void SndFileInputModule::GetTrackInfos(TrackInfo& trackInfo)
{
   CString text = sf_get_string(m_sndfile, SF_STR_TITLE);
   if (!text.IsEmpty())
      trackInfo.TextInfo(TrackInfoTitle, text);

   text = sf_get_string(m_sndfile, SF_STR_ARTIST);
   if (!text.IsEmpty())
      trackInfo.TextInfo(TrackInfoArtist, text);

   text = sf_get_string(m_sndfile, SF_STR_ALBUM);
   if (!text.IsEmpty())
      trackInfo.TextInfo(TrackInfoAlbum, text);

   text = sf_get_string(m_sndfile, SF_STR_DATE);
   if (!text.IsEmpty())
   {
      int year = _ttoi(text);
      if (year > 0)
         trackInfo.NumberInfo(TrackInfoYear, year);
   }

   text = sf_get_string(m_sndfile, SF_STR_COMMENT);
   if (!text.IsEmpty())
      trackInfo.TextInfo(TrackInfoComment, text);

   text = sf_get_string(m_sndfile, SF_STR_TRACKNUMBER);
   if (!text.IsEmpty())
   {
      int trackNumber = _ttoi(text);
      if (trackNumber > 0)
         trackInfo.NumberInfo(TrackInfoTrack, trackNumber);
   }

   text = sf_get_string(m_sndfile, SF_STR_GENRE);
   if (!text.IsEmpty())
      trackInfo.TextInfo(TrackInfoGenre, text);

   text = sf_get_string(m_sndfile, SF_STR_SOFTWARE);
   ATLTRACE(_T("File produced by: %s\n"), text.GetString());
}
