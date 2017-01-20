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
/// \file SndFileOutputModule.hpp
/// \brief contains the libsndfile output module definition
//
#pragma once

#include "ModuleInterface.hpp"
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#include <sndfile.h>

namespace Encoder
{
   /// libsndfile output module
   class SndFileOutputModule : public OutputModule
   {
   public:
      /// ctor
      SndFileOutputModule();

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("LibSndFile Audio Output"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual CString GetDescription() const override;

      /// returns the extension the output module produces
      virtual CString GetOutputExtension() const override;

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
      /// sets track info for sndfile to write
      void SetTrackInfo(const TrackInfo& trackInfo);

   private:
      /// file handle
      SNDFILE* m_sndfile;

      /// soundfile info
      SF_INFO m_sfinfo;

      /// last error occured
      CString m_lastError;

      /// format to write
      int m_format;

      /// format subtype to write
      int m_subType;
   };

} // namespace Encoder
