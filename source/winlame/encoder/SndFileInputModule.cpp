/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2014 Michael Fink
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
/// \file SndFileInputModule.cpp
/// \brief contains the implementation of the libsndfile input module

// needed includes
#include "stdafx.h"
#include "resource.h"
#include "SndFileInputModule.h"
#include "Id3v1Tag.h"


// constants

/// sndfile input buffer size
const int sndfile_inbufsize = 512;


// SndFileInputModule methods

SndFileInputModule::SndFileInputModule()
:m_buffer(NULL)
{
   module_id = ID_IM_SNDFILE;
}

InputModule *SndFileInputModule::cloneModule()
{
   return new SndFileInputModule;
}

bool SndFileInputModule::isAvailable()
{
   HMODULE dll = ::LoadLibrary(_T("libsndfile-1.dll"));
   bool avail = dll != NULL;

   // check for old libsndfile.dll (pre-1.x)
   if (::GetProcAddress(dll,"sf_get_lib_version")!=NULL)
      avail=false;

   ::FreeLibrary(dll);

   return avail;
}

void SndFileInputModule::getDescription(CString& desc)
{
   // find out type and subtype description
   SF_FORMAT_INFO types,subtypes;
   types.name = "???";
   subtypes.name = "???";

   // find out type
   {
      // get format length
      int count=0;
      sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof(count));

      int i;
      for(i=0; i<count; i++)
      {
         // get format info
         types.format = i;
         sf_command(NULL,SFC_GET_FORMAT_MAJOR,&types,sizeof(types));

         if (types.format == (sfinfo.format&SF_FORMAT_TYPEMASK))
            break;
      }

      if (i>=count)
         types.name = "unknown format";
   }

   // find out subtype
   {
      // get format length
      int count=0;
      sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof(count));

      int i;
      for(i=0; i<count; i++)
      {
         // get format info
         subtypes.format = i;
         sf_command(NULL,SFC_GET_FORMAT_SUBTYPE,&subtypes,sizeof(subtypes));

         if (subtypes.format == (sfinfo.format&SF_FORMAT_SUBMASK))
            break;
      }

      if (i>=count)
         types.name = "unknown subformat";
   }

   // format string
   desc.Format(IDS_FORMAT_INFO_SNDFILE,
      types.name, subtypes.name, sfinfo.samplerate, sfinfo.channels);
}

void SndFileInputModule::getVersionString(CString& version, int special)
{
   // retrieve libsndfile version
   if (isAvailable())
   {
      char buffer[32];
      sf_command(NULL, SFC_GET_LIB_VERSION, buffer, sizeof(buffer));

      version = CString(buffer+11);
   }
}

CString SndFileInputModule::getFilterString()
{
   // build filter string when not already done
   if (filterstring.IsEmpty())
   {
      SF_FORMAT_INFO format_info;
      int count=0;

      // get format length
      sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof(count));

      for(int i=0; i<count; i++)
      {
         // get format info
         format_info.format = i;
         sf_command(NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (format_info));

         // do format string
         CString temp;
         temp.Format(
            _T("%hs [*.%hs]|*.%hs|"),
            format_info.name,
            format_info.extension,
            format_info.extension);

         filterstring += temp;
      }
   }

   return filterstring;
}

int SndFileInputModule::initInput(LPCTSTR infilename,
   SettingsManager &mgr, TrackInfo &trackinfo,
   SampleContainer &samplecont)
{
   // resets the sample count
   samplecount = 0;

   // opens the file for reading
   memset(&sfinfo, 0, sizeof (sfinfo));
#ifdef UNICODE
   sndfile = sf_wchar_open(infilename,SFM_READ,&sfinfo);
#else
   sndfile = sf_open(CStringA(GetAnsiCompatFilename(infilename)),SFM_READ,&sfinfo);
#endif

   if (sndfile==NULL)
   {
      char buffer[512];
      sf_error_str(sndfile,buffer,512);

      lasterror.LoadString(IDS_ENCODER_INPUT_FILE_OPEN_ERROR);

      lasterror += _T(" (");
      lasterror += CString(buffer);
      lasterror += _T(")");
      return -1;
   }

   // when RIFF wave format, check for id3 tag info chunk
   if ((sfinfo.format&SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV)
   {
      waveGetId3(infilename, trackinfo);
   }

   switch(sfinfo.format&SF_FORMAT_SUBMASK)
   {
      case SF_FORMAT_PCM_S8:
      case SF_FORMAT_PCM_U8:
      case SF_FORMAT_PCM_16:
      case SF_FORMAT_DWVW_16:
         outbits = 16;
         break;
      case SF_FORMAT_PCM_24:
      case SF_FORMAT_DWVW_24:
      case SF_FORMAT_PCM_32:
      case SF_FORMAT_FLOAT:
      case SF_FORMAT_DOUBLE:
         outbits = 32;
         break;
     default:
         outbits = 16;
         break;
   }

   // prepare input buffer
   if (outbits == 32)
      m_buffer = new int[sndfile_inbufsize*sfinfo.channels];
   else
      m_buffer = new short[sndfile_inbufsize*sfinfo.channels];

   // set up input traits
   samplecont.setInputModuleTraits(outbits,SamplesInterleaved,
      sfinfo.samplerate, sfinfo.channels);

   return 0;
}

void SndFileInputModule::getInfo(int &channels, int &bitrate, int &length, int &samplerate)
{
   if (sndfile!=NULL)
   {
      // number of channels
      channels = sfinfo.channels;

      // determine bitrate
      bitrate = -1;

      switch(sfinfo.format&SF_FORMAT_SUBMASK)
      {
      case SF_FORMAT_PCM_S8:
      case SF_FORMAT_PCM_U8:
         bitrate = sfinfo.samplerate*8;
         break;

      case SF_FORMAT_PCM_16:
      case SF_FORMAT_DWVW_16:
         bitrate = sfinfo.samplerate*16;
         break;

      case SF_FORMAT_PCM_24:
      case SF_FORMAT_DWVW_24:
         bitrate = sfinfo.samplerate*24;
         break;

      case SF_FORMAT_PCM_32:
      case SF_FORMAT_FLOAT:
         bitrate = sfinfo.samplerate*32;
         break;

      case SF_FORMAT_DOUBLE:
         bitrate = sfinfo.samplerate*64;
         break;

      case SF_FORMAT_G721_32:
         bitrate = 32000;
         break;

      case SF_FORMAT_G723_24:
         bitrate = 24000;
         break;

      case SF_FORMAT_G723_40:
         bitrate = 40000;
         break;

      case SF_FORMAT_DWVW_12:
         bitrate = sfinfo.samplerate*12;
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
      if (sfinfo.samplerate != 0)
         length = int(sfinfo.frames / sfinfo.samplerate);

      // sample rate
      samplerate = sfinfo.samplerate;
   }
}

int SndFileInputModule::decodeSamples(SampleContainer &samples)
{
   // read samples
   sf_count_t ret;

   if (outbits == 32)
      ret = sf_readf_int(sndfile,(int *)m_buffer,sndfile_inbufsize);
   else
      ret = sf_readf_short(sndfile,(short *)m_buffer,sndfile_inbufsize);

   int iret = static_cast<int>(ret);

   if (ret<0)
   {
      char buffer[512];
      sf_error_str(sndfile,buffer,512);

      lasterror = CString(buffer);
      return iret;
   }

   // put samples in container
   samples.putSamplesInterleaved(m_buffer,iret);

   // count samples
   samplecount += iret;

   return iret;
}

void SndFileInputModule::doneInput()
{
   // closes the file
   sf_close(sndfile);

   // free buffer
   delete[] m_buffer;
}

bool SndFileInputModule::waveGetId3(LPCTSTR wavfile, TrackInfo &trackinfo)
{
   FILE* wav = _tfopen(wavfile, _T("rb"));
   if (!wav) return false;

   // first check for last 0x80 bytes for id3 tag
   {
      fseek(wav,-128,SEEK_END);

      // read in id3 tag
      Id3v1Tag id3tag;
      fread(id3tag.getData(), 128, 1, wav);

      if (id3tag.isValidTag())
         id3tag.toTrackInfo(trackinfo);

      fseek(wav,0,SEEK_SET);
   }

   char buffer[5]; buffer[4]=0;

   // RIFF header
   fread(buffer,4,1,wav);
   if (strcmp(buffer,"RIFF")!=0)
      return false;

   // file length
   fread(buffer,4,1,wav);
   int length = *((int*)buffer)+8;

   // RIFF format type
   fread(buffer,4,1,wav);
//   printf("RIFF type: %s, length 0x%08x\n",buffer,length);
   if (stricmp(buffer,"wave")!=0)
      return false;

   // now all chunks follow
   while (!feof(wav))
   {
      // chunk type
      fread(buffer,4,1,wav);
//      printf("chunk %s, ",buffer);

      if (strncmp(buffer,"TAG",3)==0)
         break;

      bool isid3 = strcmp(buffer,"id3 ")==0;

      // chunk length
      fread(buffer,4,1,wav);
      int clength = *((int*)buffer);
//      printf("length 0x%08x\n",clength);

      if (isid3)
      {
         // check tag length
         if (clength!=128) return false;

         // read in id3 tag
         Id3v1Tag id3tag;
         fread(id3tag.getData(), 128, 1, wav);

         if (id3tag.isValidTag())
            id3tag.toTrackInfo(trackinfo);
      }
      else
      {
         // jump over chunk
         if (0!=fseek(wav,clength,SEEK_CUR))
            break;
      }

      // check if we are on the end
      int now = ftell(wav);
      if (now>=length)
         break;
   }

   // close file
   fclose(wav);

   return true;
}
