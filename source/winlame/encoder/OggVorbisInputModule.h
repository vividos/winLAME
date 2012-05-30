/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink

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
/*! \file OggVorbisInputModule.h

   \brief contains the ogg vorbis input module definition

*/
/*! \ingroup encoder */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include <cstdio>
#include "vorbis/vorbisfile.h"


//! ogg vorbis input module

class OggVorbisInputModule: public InputModule
{
public:
   // ctor
   OggVorbisInputModule();
   // dtor
   virtual ~OggVorbisInputModule();

   //! clones input module
   virtual InputModule *cloneModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("Ogg Vorbis Decoder"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns filter string
   virtual CString getFilterString();

   // initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager &mgr,
      TrackInfo &trackinfo, SampleContainer &samples);

   // returns info about the input file
   virtual void getInfo(int &channels, int &bitrate, int &length, int &samplerate);

   // decodes samples and stores them in the sample container
   virtual int decodeSamples(SampleContainer &samples);

   // returns the number of percent done
   virtual float percentDone()
   {
      return maxsamples>0 ? float(numsamples)*100.f/maxsamples : 0.f;
   }

   // called when done with decoding
   virtual void doneInput();

protected:
   //! last error occured
   CString lasterror;

   //! number of samples decoded
   __int64 numsamples;

   //! maximum number of samples
   __int64 maxsamples;

   //! input file
   FILE *infile;

   //! decoding file struct
   OggVorbis_File vf;

   //! module handle for DLL containing vorbisfile functions
   HMODULE dll;

   //! indicates if dll handle should be free'd
   bool free_handle;

   // vorbisfile function pointers

   vorbis_info *(*ov_info_func)(OggVorbis_File *vf,int link);

   int (*ov_open_func)(FILE *f,OggVorbis_File *vf,char *initial,long ibytes);

   int *(*ov_clear_func)(OggVorbis_File *vf);

   ogg_int64_t (*ov_pcm_total_func)(OggVorbis_File *vf,int i);

   long (*ov_read_func)(OggVorbis_File *vf,char *buffer,int length,
      int bigendianp,int word,int sgned,int *bitstream);

   // msvcrt function pointers
   FILE* (*msvcrt_wfopen)(const wchar_t*, const wchar_t*);
   int (*msvcrt_fclose)(FILE*);
};


//@}
