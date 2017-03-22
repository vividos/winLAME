//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2016 Michael Fink
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

#include "ModuleInterface.hpp"
#include <opus/opusfile.h>

namespace Encoder
{
   /// input module for Opus encoded files
   class OpusInputModule : public InputModule
   {
   public:
      /// ctor
      OpusInputModule();
      /// dtor
      virtual ~OpusInputModule() throw() {}

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("Opus Audio File Decoder"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual CString GetDescription() const override;

      /// returns version string
      virtual void GetVersionString(CString& version, int special = 0) const override;

      /// returns filter string
      virtual CString GetFilterString() const override;

      /// initializes the input module
      virtual int InitInput(LPCTSTR infilename, SettingsManager& mgr,
         TrackInfo& trackinfo, SampleContainer& samples) override;

      /// returns info about the input file
      virtual void GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const override;

      /// decodes samples and stores them in the sample container
      virtual int DecodeSamples(SampleContainer& samples) override;

      /// returns the number of percent done
      virtual float PercentDone() const override;

      /// called when done with decoding
      virtual void DoneInput() override;

   private:
      /// reads track info from Opus tags
      void GetTrackInfo(TrackInfo& trackInfo);

      /// formats error text from error code
      static LPCTSTR ErrorTextFromCode(int errorCode);

      /// reads from stream; used for OpusFileCallbacks
      static int ReadStream(void* stream, unsigned char* ptr, int nbytes);

      /// seeks in stream
      static int SeekStream(void* stream, opus_int64 offset, int whence);

      /// returns current pos in stream; used for OpusFileCallbacks
      static opus_int64 PosStream(void* stream);

      /// closes stream; used for OpusFileCallbacks
      static int CloseStream(void* stream);

   private:
      /// last error text
      CString m_lastError;

      /// file callbacks
      OpusFileCallbacks m_callbacks;

      /// input file
      std::shared_ptr<OggOpusFile> m_inputFile;

      /// total number of samples in the file
      ogg_int64_t m_numTotalSamples;
   };

} // namespace Encoder
