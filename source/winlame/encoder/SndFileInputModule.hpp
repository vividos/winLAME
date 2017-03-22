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
/// \file SndFileInputModule.hpp
/// \brief contains the libsndfile input module definition
//
#pragma once

#include "ModuleInterface.hpp"
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#include <sndfile.h>

namespace Encoder
{
   /// libsndfile input module
   class SndFileInputModule : public InputModule
   {
   public:
      /// ctor
      SndFileInputModule();
      /// dtor
      virtual ~SndFileInputModule() throw() {}

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("libsndfile Audio File Decoder"); }

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
         TrackInfo& trackInfo, SampleContainer& samples) override;

      /// returns info about the input file
      virtual void GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const override;

      /// decodes samples and stores them in the sample container
      virtual int DecodeSamples(SampleContainer& samples) override;

      /// returns the number of percent done
      virtual float PercentDone() const override;

      /// called when done with decoding
      virtual void DoneInput() override;

   private:
      /// searches for id3 tag chunk in the wave file
      bool WaveGetID3Tag(LPCTSTR wavfile, TrackInfo &trackinfo);

      /// retrieves track infos from tags read by libsndfile
      void GetTrackInfos(TrackInfo& trackInfo);

   private:
      /// file handle
      SNDFILE* m_sndfile;

      /// sndfile info
      SF_INFO m_sfinfo;

      /// counts the samples already decoded
      int m_sampleCount;

      /// last error occured
      CString m_lastError;

      /// sample buffer
      std::vector<unsigned char> m_buffer;

      /// number of output bits
      int m_numOutputBits;

      /// filter string
      mutable CString m_filterString;
   };

} // namespace Encoder
