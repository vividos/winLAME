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
/// \file AacInputModule.hpp
/// \brief contains the AAC input module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include <iosfwd>
#include "neaacdec.h"

extern "C"
{
#include "aacinfo/aacinfo.h"
}

namespace Encoder
{
   // constants

   /// size of one AAC frame
   const int c_aacFrameSize = 2048;

   /// max. numbers of channels the module is able to handle
   const int c_aacNumMaxChannels = 8;

   /// calculated input buffer size
   const int c_aacInputBufferSize = c_aacFrameSize * c_aacNumMaxChannels;

   /// AAC input module
   class AacInputModule : public InputModule
   {
   public:
      // ctor
      AacInputModule();

      /// clones input module
      virtual InputModule* CloneModule() override;

      // returns the module name
      virtual CString GetModuleName() const override { return _T("AAC Audio File Decoder (libfaad2)"); }

      // returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      // returns if the module is available
      virtual bool IsAvailable() const override;

      // returns description of current file
      virtual CString GetDescription() const override;

      /// returns version string
      virtual void GetVersionString(CString& version, int special = 0) const override;

      // returns filter string
      virtual CString GetFilterString() const override;

      // initializes the input module
      virtual int InitInput(LPCTSTR infilename, SettingsManager& mgr,
         TrackInfo& trackInfo, SampleContainer& samples) override;

      // returns info about the input file
      virtual void GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const override;

      // decodes samples and stores them in the sample container
      virtual int DecodeSamples(SampleContainer& samples) override;

      // returns the number of percent done
      virtual float PercentDone() const override
      {
         return m_inputFileLength == 0 ? 0.0f : float(m_currentFilePos) * 100.f / m_inputFileLength;
      }

      // called when done with decoding
      virtual void DoneInput() override;

   private:
      /// libfaad handle
      faacDecHandle m_decoder;

      /// file input buffer
      unsigned char m_inputBuffer[c_aacInputBufferSize];

      /// high watermark for the m_inputBuffer
      int m_inputBufferHigh;

      /// length of input file
      unsigned long m_inputFileLength;

      /// current position
      unsigned long m_currentFilePos;

      /// aac file info
      faadAACInfo m_info;

      /// input file stream
      std::ifstream m_inputFile;

      /// last error occured
      CString m_lastError;
   };

} // namespace Encoder
