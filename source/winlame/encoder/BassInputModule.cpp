/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2004 DeXT
   Copyright (c) 2009 Michael Fink

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
/*! \file BassInputModule.cpp

   \brief contains the implementation of the Bass input module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include "BassInputModule.h"
#include <sys/types.h>
#include <sys/stat.h>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// linker options

#if _MSC_VER < 1400
#pragma comment(linker, "/delayload:bass.dll")
#pragma comment(linker, "/delayload:basswma.dll")
#endif

// constants

/// buffer size
const int BUF_SIZE = 4096;

// BassInputModule methods

BassInputModule::BassInputModule()
{
   module_id = ID_IM_BASS;
}

InputModule *BassInputModule::cloneModule()
{
   return new BassInputModule;
}

bool BassInputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("bass.dll"));
   bool avail = dll != NULL;
   if (avail) ::FreeLibrary(dll);

   HMODULE wmadll = ::LoadLibrary(_T("basswma.dll"));
   basswma = wmadll != NULL;
   if (basswma) ::FreeLibrary(wmadll);

   return avail;
}

void BassInputModule::getDescription(CString& desc)
{
   // format string
   if (!is_str) // MOD file
   {
      desc.Format(IDS_FORMAT_INFO_BASS_INPUT_MUSIC,
         BASS_ChannelGetTags(chan, BASS_TAG_MUSIC_NAME),
         info.freq, info.chans);
   }
   else // stream
   {
      CString cszTypeWMA, cszTypeBassFileStream;
      cszTypeWMA.LoadString(IDS_FORMAT_INFO_BASS_WMA);
      cszTypeBassFileStream.LoadString(IDS_FORMAT_INFO_BASS_FILESTREAM);

      desc.Format(IDS_FORMAT_INFO_BASS_INPUT_AUDIO,
         basswma ? cszTypeWMA : cszTypeBassFileStream, brate, info.freq, info.chans);
   }
}

void BassInputModule::getVersionString(CString& version, int special)
{
   version.Empty();

   HMODULE dll = ::LoadLibrary(_T("bass.dll"));
   if (dll != NULL)
   {
      DWORD dwVersion = BASS_GetVersion();
      ::FreeLibrary(dll);

      version.Format(_T("%u.%u.%u.%u"),
         HIBYTE(HIWORD(dwVersion)),
         LOBYTE(HIWORD(dwVersion)),
         HIBYTE(LOWORD(dwVersion)),
         LOBYTE(LOWORD(dwVersion)));
   }
}

CString BassInputModule::getFilterString()
{
   // build filter string when not already done
   if (filterstring.IsEmpty())
   {
      filterstring.LoadString(IDS_FILTER_BASS_INPUT);

      if (basswma)
      {
         CString cszTemp;
         cszTemp.LoadString(IDS_FILTER_BASS_WMA_INPUT);
         filterstring += cszTemp;
      }
   }

   return filterstring;
}

/// converts from UTF-8 encoded text to CString
CString UTF8ToCString(const char* pszUtf8Text)
{
   int iBufferLength = MultiByteToWideChar(CP_UTF8, 0,
      pszUtf8Text, -1,
      NULL, 0);

   CString cszText;
   MultiByteToWideChar(CP_UTF8, 0,
      pszUtf8Text, -1,
      cszText.GetBuffer(iBufferLength), iBufferLength);

   cszText.ReleaseBuffer();
   return cszText;
}

int BassInputModule::initInput(LPCTSTR infilename, SettingsManager &mgr,
   TrackInfo &trackinfo, SampleContainer &samplecont)
{
   USES_CONVERSION;

   /* check that correct BASS version was loaded */
   if ((HIWORD(BASS_GetVersion()) != BASSVERSION))
   {
      lasterror.LoadString(IDS_BASS_DLL_ERROR_VERSION_MISMATCH);
      return -1;
   }

   /* not playing anything, so don't need an update thread */
   BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD,0);

   /* setup output - "no sound" device, 44100hz, stereo, 16 bits */
   if (!BASS_Init(0,44100,0,0,NULL))
   {
      lasterror.LoadString(IDS_ENCODER_ERROR_INIT_DECODER);
      return -1;
   }

   /* try streaming the file/url */
   CString cszAnsiFilename = GetAnsiCompatFilename(infilename);
   if ((chan=BASS_StreamCreateFile(FALSE,T2CA(cszAnsiFilename),0,0,BASS_STREAM_DECODE)) ||
      (chan=BASS_StreamCreateURL(T2CA(cszAnsiFilename),0,BASS_STREAM_DECODE,0,0)) ||
      (basswma && (chan=BASS_WMA_StreamCreateFile(FALSE,T2CA(cszAnsiFilename),0,0,BASS_STREAM_DECODE)))) {
      length=BASS_ChannelGetLength(chan, BASS_POS_BYTE);
      is_str=TRUE;
   /* try loading the MOD (with sensitive ramping, and calculate the duration) */
   } else if ((chan=BASS_MusicLoad(FALSE,T2CA(cszAnsiFilename),0,0,
         BASS_MUSIC_DECODE | BASS_MUSIC_RAMPS | BASS_MUSIC_SURROUND |
         BASS_MUSIC_CALCLEN | BASS_MUSIC_STOPBACK,0))) {
      modlen=BASS_ChannelGetLength(chan, BASS_POS_MUSIC_ORDER);
      length=BASS_ChannelGetLength(chan, BASS_POS_BYTE);
      is_str=FALSE;
   }
   else
   {
      lasterror.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);
      return -1;
   }

   time=BASS_ChannelBytes2Seconds(chan,length);

   if (is_str)
   {
      filelen=BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END); // file length
      if (time)
         brate=(DWORD)(filelen*8/time+0.5); // bitrate (bps)
      else
         brate=0;
   }
   else // not applicable for modules
   {
      filelen=0;
      brate=0;
   }

   BASS_ChannelGetInfo(chan,&info);

   // get tags
   if (is_str && (info.ctype & BASS_CTYPE_STREAM_WMA))
   {
      const char* comments = BASS_WMA_GetTags(T2CA(cszAnsiFilename), 0);

      std::vector<size_t> vecCommentIndices;
      if (comments)
      {
         size_t cur_index = 0;
         const char* index = comments;
         while (index && *index) {
            vecCommentIndices.push_back(cur_index);
            cur_index += strlen(index)+1;
            index += strlen(index)+1;
         }
      }

      const TCHAR* WMA_BITRATE_TAG = _T("Bitrate");
      const TCHAR* WMA_TRACKNO_TAG = _T("WM/TrackNumber");
      const TCHAR* WMA_TITLE_TAG = _T("Title");
      const TCHAR* WMA_AUTHOR_TAG = _T("Author");
      const TCHAR* WMA_ALBUM_TAG = _T("WM/AlbumTitle");
      const TCHAR* WMA_YEAR_TAG = _T("WM/Year");
      const TCHAR* WMA_GENRE_TAG = _T("WM/Genre");
      const TCHAR* WMA_DESC_TAG = _T("Description");

      int wma_bitrate = 0;
      CString wma_genre;

      for (size_t i = 0; i < vecCommentIndices.size(); i++)
      {
         CString cszTag = UTF8ToCString(comments + vecCommentIndices[i]);

         // search for delimiting colon
         int iPos = cszTag.Find(_T('='));

         CString cszName = cszTag.Left(iPos);
         cszName.TrimRight();

         CString cszValue = cszTag.Mid(iPos+1);
         cszValue.TrimLeft();

         if (cszName == WMA_TITLE_TAG)    trackinfo.TextInfo(TrackInfoTitle, cszValue);    else
         if (cszName == WMA_AUTHOR_TAG)   trackinfo.TextInfo(TrackInfoArtist, cszValue);   else
         if (cszName == WMA_ALBUM_TAG)    trackinfo.TextInfo(TrackInfoAlbum, cszValue);    else
         if (cszName == WMA_DESC_TAG)     trackinfo.TextInfo(TrackInfoComment, cszValue);  else
         if (cszName == WMA_GENRE_TAG)    trackinfo.TextInfo(TrackInfoGenre, cszValue);    else
         if (cszName == WMA_YEAR_TAG)
         {
            int iYear = _ttoi(cszValue);
            trackinfo.NumberInfo(TrackInfoYear, iYear);
         }
         else
         if (cszName == WMA_TRACKNO_TAG)
         {
            int iTrackNo = _ttoi(cszValue);
            trackinfo.NumberInfo(TrackInfoTrack, iTrackNo);
         }
         else
         if (cszName == WMA_BITRATE_TAG)
         {
            int iBitrate = _ttoi(cszValue);
            brate = iBitrate;
         }
      }
   }

   // create buffer
   buffer = new char[BUF_SIZE];

   // set up input traits
   samplecont.setInputModuleTraits(16,SamplesInterleaved,
      info.freq,info.chans);

   return 0;
}

void BassInputModule::getInfo(int &channels, int &bitrate, int &length, int &samplerate)
{
   channels = info.chans;
   bitrate = (brate > 0 ? brate : -1 );
   length = static_cast<int>(time);
   samplerate = info.freq;
}

int BassInputModule::decodeSamples(SampleContainer &samples)
{
   int ret;

   if (!BASS_ChannelIsActive(chan)) return 0;

   // loop to wait for input data, if available
   do {
      ret = BASS_ChannelGetData(chan,buffer,BUF_SIZE);
   } while (!ret && BASS_ChannelIsActive(chan));

   ret /= info.chans * (16 >> 3); // samples

   // copy the samples to the sample container
   samples.putSamplesInterleaved(buffer, ret);

   return ret;
}

float BassInputModule::percentDone()
{
   const int PATLEN = 64;
   float done;

   pos = BASS_ChannelGetPosition(chan, BASS_POS_BYTE);

   if (!is_str)
      done = (((LOWORD(pos) * PATLEN) + HIWORD(pos)) / 64.f) * 100.f / modlen;
   else
      done = __int64(pos) * 100.f / __int64(length);

   return done;
}

void BassInputModule::doneInput()
{
   BASS_Free();
   delete[] buffer;
}

