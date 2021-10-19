//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2004 DeXT
// Copyright (c) 2009-2021 Michael Fink
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
/// \file BassInputModule.cpp
/// \brief contains the implementation of the Bass input module
//
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "BassInputModule.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <ulib/DynamicLibrary.hpp>
#include <ulib/UTF8.hpp>
#include <wmsdk.h> // for WM_PICTURE

using Encoder::BassInputModule;
using Encoder::TrackInfo;
using Encoder::SampleContainer;

/// BASS API usage count
std::atomic<unsigned int> s_bassApiusageCount = { 0 };

// constants

/// name of WMA picture tag
const TCHAR* WMA_PICTURE_TAG = _T("WM/Picture");

/// buffer size
const int c_bassInputBufferSize = 4096;

// BassInputModule methods

BassInputModule::BassInputModule()
   :m_buffer(nullptr),
   m_isStream(TRUE),
   m_fileLength(0),
   m_lengthInSeconds(0.0),
   m_modLength(0),
   m_bitrateInBps(0),
   m_channel(0)
{
   m_moduleId = ID_IM_BASS;

   memset(&m_channelInfo, 0, sizeof(m_channelInfo));
}

Encoder::InputModule* BassInputModule::CloneModule()
{
   return new BassInputModule;
}

bool BassInputModule::IsAvailable() const
{
   DynamicLibrary bassLib(_T("bass.dll"));
   DynamicLibrary bassWmaLib(_T("basswma.dll"));

   return bassLib.IsLoaded() && bassWmaLib.IsLoaded();
}

CString BassInputModule::GetDescription() const
{
   CString desc;

   if (!m_isStream) // MOD file
   {
      desc.Format(IDS_FORMAT_INFO_BASS_INPUT_MUSIC,
         BASS_ChannelGetTags(m_channel, BASS_TAG_MUSIC_NAME),
         m_channelInfo.freq,
         m_channelInfo.chans);
   }
   else // stream
   {
      CString typeBassFileStream;
      typeBassFileStream.LoadString(IDS_FORMAT_INFO_BASS_FILESTREAM);

      desc.Format(IDS_FORMAT_INFO_BASS_INPUT_AUDIO,
         typeBassFileStream.GetString(),
         m_bitrateInBps,
         m_channelInfo.freq,
         m_channelInfo.chans);
   }

   return desc;
}

void BassInputModule::GetVersionString(CString& version, int special) const
{
   version.Empty();

   DWORD bassVersion = BASS_GetVersion();

   version.Format(_T("%u.%u.%u.%u"),
      HIBYTE(HIWORD(bassVersion)),
      LOBYTE(HIWORD(bassVersion)),
      HIBYTE(LOWORD(bassVersion)),
      LOBYTE(LOWORD(bassVersion)));
}

CString BassInputModule::GetFilterString() const
{
   // build filter string when not already done
   if (m_filterString.IsEmpty())
   {
      m_filterString.LoadString(IDS_FILTER_BASS_INPUT);

      CString temp;
      temp.LoadString(IDS_FILTER_BASS_WMA_INPUT);
      m_filterString += temp;
   }

   return m_filterString;
}

int BassInputModule::InitInput(LPCTSTR infilename, SettingsManager& mgr,
   TrackInfo& trackinfo, SampleContainer& samplecont)
{
   // check that correct BASS version was loaded
   if ((HIWORD(BASS_GetVersion()) != BASSVERSION))
   {
      m_lastError.LoadString(IDS_BASS_DLL_ERROR_VERSION_MISMATCH);
      return -1;
   }

   // not playing anything, so don't need an update thread
   BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);

   // setup output - "no sound" device, 44100hz, stereo, 16 bits
   if (s_bassApiusageCount++ == 0)
   {
      if (!BASS_Init(0, 44100, 0, 0, nullptr))
      {
         m_lastError.LoadString(IDS_ENCODER_ERROR_INIT_DECODER);
         return -1;
      }
   }

   // try streaming the file/url
   m_channel = BASS_WMA_StreamCreateFile(FALSE, infilename, 0, 0, BASS_STREAM_DECODE | BASS_UNICODE);

   if (!m_channel)
      m_channel = BASS_StreamCreateFile(FALSE, infilename, 0, 0, BASS_STREAM_DECODE | BASS_UNICODE);

   if (m_channel)
   {
      m_fileLength = BASS_ChannelGetLength(m_channel, BASS_POS_BYTE);
      m_isStream = true;
   }
   else
   {
      // try loading the MOD (with sensitive ramping, and calculate the duration)
      m_channel = BASS_MusicLoad(FALSE, infilename, 0, 0,
         BASS_MUSIC_DECODE | BASS_MUSIC_RAMPS | BASS_MUSIC_SURROUND |
         BASS_MUSIC_CALCLEN | BASS_MUSIC_STOPBACK | BASS_UNICODE, 0);

      if (m_channel)
      {
         m_modLength = BASS_ChannelGetLength(m_channel, BASS_POS_MUSIC_ORDER);
         m_fileLength = BASS_ChannelGetLength(m_channel, BASS_POS_BYTE);
         m_isStream = false;
      }
      else
      {
         m_lastError.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
         return -1;
      }
   }

   m_lengthInSeconds = BASS_ChannelBytes2Seconds(m_channel, m_fileLength);

   if (m_isStream)
   {
      QWORD filelen = BASS_StreamGetFilePosition(m_channel, BASS_FILEPOS_END); // file length
      if (m_lengthInSeconds > 0)
         m_bitrateInBps = (DWORD)(filelen * 8 / m_lengthInSeconds + 0.5); // bitrate (bps)
      else
         m_bitrateInBps = 0;
   }
   else // not applicable for modules
   {
      m_bitrateInBps = 0;
   }

   BASS_ChannelGetInfo(m_channel, &m_channelInfo);

   // get tags
   if (m_isStream && (m_channelInfo.ctype & BASS_CTYPE_STREAM_WMA))
   {
      const char* comments = BASS_WMA_GetTags(infilename, BASS_UNICODE);

      std::vector<size_t> vecCommentIndices;
      if (comments)
      {
         size_t cur_index = 0;
         const char* index = comments;
         while (index && *index) {
            vecCommentIndices.push_back(cur_index);
            cur_index += strlen(index) + 1;
            index += strlen(index) + 1;
         }
      }

      const TCHAR* WMA_BITRATE_TAG = _T("Bitrate");
      const TCHAR* WMA_TRACKNO_TAG = _T("WM/TrackNumber");
      const TCHAR* WMA_TITLE_TAG = _T("Title");
      const TCHAR* WMA_AUTHOR_TAG = _T("Author");
      const TCHAR* WMA_ALBUM_ARTIST_TAG = _T("WM/AlbumArtist");
      const TCHAR* WMA_COMPOSER_TAG = _T("WM/Composer");
      const TCHAR* WMA_ALBUM_TAG = _T("WM/AlbumTitle");
      const TCHAR* WMA_YEAR_TAG = _T("WM/Year");
      const TCHAR* WMA_GENRE_TAG = _T("WM/Genre");
      const TCHAR* WMA_DESC_TAG = _T("Description");
      const TCHAR* WMA_DISCNO_TAG = _T("WM/PartOfSet");

      CString wma_genre;

      for (size_t i = 0; i < vecCommentIndices.size(); i++)
      {
         CString cszTag = UTF8ToString(comments + vecCommentIndices[i]);

         // search for delimiting colon
         int iPos = cszTag.Find(_T('='));

         CString cszName = cszTag.Left(iPos);
         cszName.TrimRight();

         CString cszValue = cszTag.Mid(iPos + 1);
         cszValue.TrimLeft();

         if (cszName == WMA_TITLE_TAG) trackinfo.SetTextInfo(TrackInfoTitle, cszValue);
         else if (cszName == WMA_AUTHOR_TAG) trackinfo.SetTextInfo(TrackInfoArtist, cszValue);
         else if (cszName == WMA_ALBUM_TAG) trackinfo.SetTextInfo(TrackInfoAlbum, cszValue);
         else if (cszName == WMA_DESC_TAG) trackinfo.SetTextInfo(TrackInfoComment, cszValue);
         else if (cszName == WMA_GENRE_TAG) trackinfo.SetTextInfo(TrackInfoGenre, cszValue);
         else if (cszName == WMA_YEAR_TAG)
         {
            int iYear = _ttoi(cszValue);
            trackinfo.SetNumberInfo(TrackInfoYear, iYear);
         }
         else if (cszName == WMA_TRACKNO_TAG)
         {
            int iTrackNo = _ttoi(cszValue);
            trackinfo.SetNumberInfo(TrackInfoTrack, iTrackNo);
         }
         else if (cszName == WMA_DISCNO_TAG)
         {
            int discNo = _ttoi(cszValue);
            trackinfo.SetNumberInfo(TrackInfoDiscNumber, discNo);
         }
         else if (cszName == WMA_BITRATE_TAG)
         {
            int bitrate = _ttoi(cszValue);
            m_bitrateInBps = bitrate;
         }
         else if (cszName == WMA_ALBUM_ARTIST_TAG)
            trackinfo.SetTextInfo(TrackInfoDiscArtist, cszValue);
         else if (cszName == WMA_COMPOSER_TAG)
            trackinfo.SetTextInfo(TrackInfoComposer, cszValue);
         else if (cszName == WMA_PICTURE_TAG)
         {
            ReadWmaPictureTag(m_channel, trackinfo);
         }
      }
   }

   // create buffer
   m_buffer = new char[c_bassInputBufferSize];

   // set up input traits
   samplecont.SetInputModuleTraits(
      16,
      SamplesInterleaved,
      m_channelInfo.freq,
      m_channelInfo.chans);

   return 0;
}

void BassInputModule::GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const
{
   numChannels = m_channelInfo.chans;
   bitrateInBps = (m_bitrateInBps > 0 ? m_bitrateInBps : -1);
   lengthInSeconds = static_cast<int>(m_lengthInSeconds);
   samplerateInHz = m_channelInfo.freq;
}

int BassInputModule::DecodeSamples(SampleContainer& samples)
{
   if (BASS_ChannelIsActive(m_channel) != BASS_ACTIVE_PLAYING)
      return 0;

   DWORD ret = BASS_ChannelGetData(m_channel, m_buffer, c_bassInputBufferSize);

   if (ret != 0 && ret != DWORD(-1))
   {
      ret /= m_channelInfo.chans * (16 >> 3); // samples

      samples.PutSamplesInterleaved(m_buffer, ret);
   }

   if (ret == DWORD(-1))
   {
      if (BASS_ChannelIsActive(m_channel) != BASS_ACTIVE_PLAYING)
         return 0;

      int errorCode = BASS_ErrorGetCode();

      m_lastError.Format(_T("BASS error: %i"), errorCode);
   }

   return ret;
}

float BassInputModule::PercentDone() const
{
   QWORD currentPosition = BASS_ChannelGetPosition(m_channel, BASS_POS_BYTE);

   const int PATLEN = 64;
   float percentDone;

   if (!m_isStream)
      percentDone = (((LOWORD(currentPosition) * PATLEN) + HIWORD(currentPosition)) / 64.f) * 100.f / m_modLength;
   else
      percentDone = __int64(currentPosition) * 100.f / __int64(m_fileLength);

   return percentDone;
}

void BassInputModule::DoneInput()
{
   if (--s_bassApiusageCount == 0)
   {
      BASS_Free();
   }

   delete[] m_buffer;
}

void BassInputModule::ReadWmaPictureTag(DWORD channel, TrackInfo& trackInfo)
{
   IWMReader* wmReader = (IWMReader*)BASS_WMA_GetWMObject(channel);
   if (wmReader != nullptr)
   {
      CComPtr<IWMHeaderInfo3> spHeaderInfo3;
      HRESULT hr = wmReader->QueryInterface<IWMHeaderInfo3>(&spHeaderInfo3);
      if (SUCCEEDED(hr))
      {
         WORD streamNumber = 0;
         WMT_ATTR_DATATYPE dataType = WMT_TYPE_DWORD;
         WORD attributeLength = 0;

         if (SUCCEEDED(spHeaderInfo3->GetAttributeByName(
            &streamNumber, WMA_PICTURE_TAG, &dataType, nullptr, &attributeLength)) &&
            dataType == WMT_TYPE_BINARY)
         {
            std::vector<BYTE> attributeData(attributeLength);

            if (SUCCEEDED(spHeaderInfo3->GetAttributeByName(
               &streamNumber, WMA_PICTURE_TAG, &dataType, attributeData.data(), &attributeLength)))
            {
               WM_PICTURE* picture = reinterpret_cast<WM_PICTURE*>(attributeData.data());

               if (picture->bPictureType == 3 &&
                  picture->dwDataLen > 0 &&
                  picture->pbData != nullptr)
               {
                  const std::vector<unsigned char> binaryInfo(
                     picture->pbData,
                     picture->pbData + picture->dwDataLen);

                  trackInfo.SetBinaryInfo(TrackInfoFrontCover, binaryInfo);
               }
            }
         }
      }
   }
}
