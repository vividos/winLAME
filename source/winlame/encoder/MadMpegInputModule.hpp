//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file MadMpegInputModule.hpp
/// \brief contains the MAD mpeg input module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include <iosfwd>
#include "mad.h"
//#include "dither/dither.h"

namespace Encoder
{
   // constants

   /// input buffer size
   const int mad_inbufsize = 4096;

   /// MAD MPEG input module
   class MadMpegInputModule : public InputModule
   {
   public:
      // ctor
      MadMpegInputModule();

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("MAD MPEG Audio File Decoder"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual void GetDescription(CString& desc) const override;

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
      virtual float PercentDone() const override
      {
         return m_numMaxSamples == 0 ? 0.0f : float(m_numCurrentSamples * 100) / m_numMaxSamples;
      }

      /// called when done with decoding
      virtual void DoneInput() override;

   private:
      int ReadFileLength(LPCTSTR infilename);

      /// checks for Xing / VBR info tag and adjusts bitrate
      bool CheckVBRInfoTag(unsigned char *buffer);

      /// retrieves id3v2 tag infos
      static bool GetId3v2TagInfos(const CString& cszFilename, TrackInfo& trackinfo);

   private:
      // mad structs

      /// first decoded frame header
      struct mad_header m_initHeader;

      /// stream struct
      struct mad_stream m_stream;
      /// frame struct
      struct mad_frame m_frame;
      /// struct for synthesizing output samples
      struct mad_synth m_synth;

      // dither structs
      //struct audio_dither left_dither, right_dither;
      //struct audio_stats stats;

      /// end of MPEG stream
      bool m_endOfStream;

      /// input file stream
      std::ifstream m_inputFile;

      /// file input buffer
      unsigned char m_inputBuffer[mad_inbufsize];

      /// number of samples decoded
      __int64 m_numCurrentSamples;

      /// number of samples in the mpeg file
      __int64 m_numMaxSamples;

      /// last error occured
      CString m_lastError;
   };

} // namespace Encoder
