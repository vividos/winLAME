//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2016 Michael Fink
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
/// \file BassWmaOutputModule.hpp
/// \brief contains the basswma output module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include "bass.h"
#include "basswma.h"

namespace Encoder
{
   /// Bass WMA output module
   class BassWmaOutputModule : public OutputModule
   {
   public:
      /// ctor
      BassWmaOutputModule();

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("Windows Media Audio Encoder"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual void GetDescription(CString& desc) const override;

      /// returns the extension the output module produces
      virtual CString GetOutputExtension() const override { return _T("wma"); }

      /// lets the output module fetch some settings, right after module creation
      virtual void PrepareOutput(SettingsManager& mgr) override;

      /// initializes the output module
      virtual int InitOutput(LPCTSTR outfilename, SettingsManager& mgr,
         const TrackInfo& trackInfo, SampleContainer& samples) override;

      /// encodes samples from the sample container
      virtual int EncodeSamples(SampleContainer& samples) override;

      /// cleans up the output module
      virtual void DoneOutput() override;

   private:
      /// adds track info to output file
      void AddTrackInfo(const TrackInfo& trackInfo);

   private:
      /// last error occured
      CString m_lastError;

      HWMENCODE m_handle;     ///< encoder handle
      int m_samplerateInHz;   ///< sample rate
      int m_numChannels;      ///< number of channels
      int m_bitrateInBps;     ///< bitrate; in bitrate mode 0
      int m_quality;          ///< quality; in bitrate mode 1
      int m_bitrateMode;      ///< bitrate mode
   };

} // namespace Encoder
