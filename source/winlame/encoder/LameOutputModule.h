/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2009 Michael Fink

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
/*! \file LameOutputModule.h

   \brief contains the LAME output module definition

*/
/*! \ingroup encoder */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include <iosfwd>
#include "nlame.h"

// forward references
struct Id3v1Tag;

//! lame output module

class LameOutputModule: public OutputModule
{
public:
   // ctor
   LameOutputModule();

   virtual ~LameOutputModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("LAME mp3 Encoder"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc){ desc = description; }

   // returns version string
   virtual void getVersionString(CString& version, int special=0);

   // returns the extension the output module produces
   virtual LPCTSTR getOutputExtension(){ return waveheader ? _T("wav") : _T("mp3"); }

   // lets the output module fetch some settings, right after module creation
   virtual void prepareOutput(SettingsManager &mgr);

   // initializes the output module
   virtual int initOutput(LPCTSTR outfilename, SettingsManager &mgr,
      const TrackInfo& trackinfo, SampleContainer &samplecont);

   // encodes samples from the sample container
   virtual int encodeSamples(SampleContainer &samples);

   // cleans up the output module
   virtual void doneOutput();

private:
   //! encodes one frame
   int encodeFrame();

   //! adds id3v2 tag infos to nlame instance
   void AddLameID3v2Tag(const TrackInfo& trackinfo);

   //! estimate ID3v2 padding length
   unsigned int GetID3v2PaddingLength();

   //! writes out ID3v2 tag
   void WriteID3v2Tag();

protected:
   //! nlame instance
   nlame_instance_t *inst;

   //! output file stream
   std::ofstream ostr;

   //! output mp3 filename
   CString mp3filename;

   //! indicates if we should write a vbr info tag
   bool infotag;

   //! encode buffer type
   nlame_encode_buffer_type buftype;

   //! current input buffer size for LAME
   unsigned int lame_input_buffer_size;

   //! lame input buffer
   unsigned char* inbuffer;

   //! inbuffer filled indicator
   unsigned int inbuffer_fill;

   //! mp3 output buffer
   unsigned char *mp3buf;

   //! last error occured
   CString lasterror;

   //! encoding description
   CString description;

   //! possible id3 tag to append to the file
   Id3v1Tag* id3tag;

   //! track info for writing ID3v2 tag
   TrackInfo m_trackInfoID3v2;

   //! indicates if we encode for gapless output
   bool nogap;

   //! indicates if we encode the last file
   bool nogap_lastfile;

   //! saved nlame instance for nogap encoding
   static nlame_instance_t *nogap_inst;

   //! indicates if we should write a wave header
   bool waveheader;

   //! number of samples encoded
   unsigned int samplecount;

   //! number of data bytes written
   unsigned int datalen;
};


//@}
