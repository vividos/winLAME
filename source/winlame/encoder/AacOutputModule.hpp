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
/// \file AacOutputModule.hpp
/// \brief contains the AAC output module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include <iosfwd>
#include "faac.h"

namespace Encoder
{
   /// AAC output module
   class AacOutputModule : public OutputModule
   {
   public:
      /// ctor
      AacOutputModule();

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("AAC Audio File Encoder (libfaac)"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual CString GetDescription() const override;

      /// returns version string
      virtual void GetVersionString(CString& version, int special = 0) const override;

      /// returns the extension the output module produces
      virtual CString GetOutputExtension() const override { return _T("aac"); }

      /// initializes the output module
      virtual int InitOutput(LPCTSTR outfilename, SettingsManager& mgr,
         const TrackInfo& trackInfo, SampleContainer& samples) override;

      /// encodes samples from the sample container
      virtual int EncodeSamples(SampleContainer& samples) override;

      /// cleans up the output module
      virtual void DoneOutput() override;

   private:
      /// encoder handle
      faacEncHandle m_handle;

      /// size of input buffer in bytes
      unsigned long m_inputBufferSize;

      /// size of output buffer in bytes
      unsigned long m_outputBufferSize;

      /// output buffer
      std::vector<unsigned char> m_outputBuffer;

      /// sample buffer
      std::vector<short> m_sampleBuffer;

      /// sample buffer high watermark
      int m_sampleBufferHigh;

      /// bitrate control method
      int m_bitrateControlMethod;

      /// output file stream
      std::ofstream m_outputFile;

      /// last error occured
      CString m_lastError;
   };

} // namespace Encoder
