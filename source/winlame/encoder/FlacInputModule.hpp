//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file FlacInputModule.hpp
/// \brief contains the FLAC input module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include "FLAC/stream_decoder.h"

namespace Encoder
{
   /// flac decoding context
   struct FLAC_context
   {
      FLAC__StreamMetadata_StreamInfo streamInfo;  ///< stream info
      FLAC__int32* reservoir;                      ///< reservoir
      unsigned int numSamplesInReservoir;          ///< number of samples in reservoir
      unsigned int totalLengthInMs;                ///< total length in ms
      bool abortFlag;                              ///< abort flag
      TrackInfo* trackInfo;                        ///< track info

      /// ctor
      FLAC_context()
         :reservoir(nullptr),
         numSamplesInReservoir(0),
         totalLengthInMs(0),
         abortFlag(false),
         trackInfo(nullptr)
      {
         memset(&streamInfo, 0, sizeof(streamInfo));
      }
   };

   /// FLAC input module
   class FlacInputModule : public InputModule
   {
   public:
      /// ctor
      FlacInputModule();

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("FLAC Audio File Decoder"); }

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
      /// length of input file
      unsigned long m_fileLength;

      /// last error occured
      CString m_lastError;

      /// local variables
      FLAC__StreamDecoder* m_flacDecoder;

      /// flac context
      FLAC_context* m_flacContext;

      /// input buffer
      std::vector<FLAC__int32> m_inputBuffer;

      /// sample position
      FLAC__uint64 m_samplePosition;

      /// buffer length
      unsigned int m_pcmBufferLength;
   };

} // namespace Encoder
