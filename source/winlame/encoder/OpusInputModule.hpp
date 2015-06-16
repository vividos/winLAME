//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2015 Michael Fink
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
/// \file OpusInputModule.hpp
/// \brief Opus input module
//
#pragma once

// includes
#include "ModuleInterface.h"
#include <opus/opusfile.h>

/// input module for Opus encoded files
class OpusInputModule : public InputModule
{
public:
   /// ctor
   OpusInputModule();
   // dtor
   virtual ~OpusInputModule() throw() {}

   /// clones input module
   virtual InputModule* cloneModule() override;

   /// returns the module name
   virtual CString getModuleName() override { return _T("Opus Audio File Decoder"); }

   /// returns the last error
   virtual CString getLastError() override { return m_cszLastError; }

   /// returns if the module is available
   virtual bool isAvailable() override;

   /// returns description of current file
   virtual void getDescription(CString& desc) override;

   /// returns version string
   virtual void getVersionString(CString& version, int special = 0) override;

   /// returns filter string
   virtual CString getFilterString() override;

   /// initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager& mgr,
      TrackInfo& trackinfo, SampleContainer& samples) override;

   /// returns info about the input file
   virtual void getInfo(int& channels, int& bitrate, int& length, int& samplerate) override;

   /// decodes samples and stores them in the sample container
   virtual int decodeSamples(SampleContainer& samples) override;

   /// returns the number of percent done
   virtual float percentDone() override;

   /// called when done with decoding
   virtual void doneInput() override;

private:
   /// formats error text from error code
   static LPCTSTR ErrorTextFromCode(int iErrorCode);

   /// reads from stream; used for OpusFileCallbacks
   static int ReadStream(void *_stream, unsigned char *_ptr, int _nbytes);

   /// seeks in stream
   static int SeekStream(void *_stream, opus_int64 _offset, int _whence);

   /// returns current pos in stream; used for OpusFileCallbacks
   static opus_int64 PosStream(void *_stream);

   /// closes stream; used for OpusFileCallbacks
   static int CloseStream(void *_stream);

private:
   /// last error text
   CString m_cszLastError;

   /// file callbacks
   OpusFileCallbacks m_callbacks;

   /// input file
   std::shared_ptr<OggOpusFile> m_spInputFile;

   /// total number of samples in the file
   ogg_int64_t m_lTotalSamples;
};
