//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2018-2021 Michael Fink
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
/// \file LibMpg123InputModule.hpp
/// \brief libmpg123 input module
//
#pragma once

#include "ModuleInterface.hpp"
#define MPG123_ENUM_API
#include <mpg123.h>

namespace Encoder
{
   /// input module for MPEG Layer 1-2-3 encoded files
   class LibMpg123InputModule : public InputModule
   {
   public:
      /// ctor
      LibMpg123InputModule();
      /// dtor
      virtual ~LibMpg123InputModule() {}

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("mpg123 Audio File Decoder"); }

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
      /// reads file size from opened input file
      bool GetFileSize();

      /// opens mü3 stream from input file
      bool OpenStream();

      /// reads track info from input file
      bool GetTrackInfo(const CString& filename, TrackInfo& trackInfo);

      /// reads ID3v2 tag infos, if available
      bool GetId3v2TagInfos(const CString& filename, TrackInfo& trackInfo);

      /// sets up decoder
      bool SetupDecoder();

      /// sets sample container format
      bool SetFormat(SampleContainer& samples);

   private:
      /// last error text
      CString m_lastError;

      /// input file
      std::shared_ptr<FILE> m_inputFile;

      /// file size of input file
      long m_fileSize;

      /// handle to the mpg123 decoder
      std::shared_ptr<mpg123_handle> m_decoder;

      /// indicates if the decoder is at the end of the file
      bool m_isAtEndOfFile;
   };

} // namespace Encoder
