//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2007 Michael Fink
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
/// \file ModuleInterface.hpp
/// \brief contains the interface definition for input and output modules
//
#pragma once

#include <string>
#include "SettingsManager.h"
#include "TrackInfo.hpp"
#include "SampleContainer.hpp"

namespace Encoder
{

   // input and output module ids

#define ID_OM_LAME                      1
#define ID_OM_OGGV                      2
#define ID_OM_WAVE                      3
#define ID_OM_AAC                       4
#define ID_IM_SNDFILE                   5
#define ID_IM_MAD                       6
#define ID_IM_OGGV                      7
#define ID_IM_AAC                       8
#define ID_IM_FLAC                      10
#define ID_IM_BASS                      11
#define ID_OM_BASSWMA                   12
#define ID_IM_CDRIP                     13
#define ID_IM_SPEEX                     14
#define ID_IM_OPUS                      15
#define ID_OM_OPUS                      16


   /// module base class
   class ModuleBase
   {
   public:
      /// dtor
      virtual ~ModuleBase()
      {
         m_moduleId = 0;
      }

      /// returns the module name
      virtual CString GetModuleName() const = 0;

      /// returns the last error
      virtual CString GetLastError() const = 0;

      /// returns if the module is available
      virtual bool IsAvailable() const = 0;

      /// returns module id
      int GetModuleID() const { return m_moduleId; }

      /// returns description of current file
      virtual void GetDescription(CString& desc) const { desc.Empty(); }

      /// returns version string; value in special may denote special type of string
      virtual void GetVersionString(CString& version, int special = 0) const { version.Empty(); special; }

      /// resolves possibly encoded filenames
      virtual void ResolveRealFilename(CString& filename) { filename; }

   protected:
      /// module id
      int m_moduleId;

      /// number of channels
      int m_channels;

      /// sample rate
      int m_samplerate;
   };


   /// input module base class
   class InputModule : public ModuleBase
   {
   public:
      /// dtor
      virtual ~InputModule() {}

      /// clones input module
      virtual InputModule* CloneModule() = 0;

      /// returns filter string
      virtual CString GetFilterString() const = 0;

      /// initializes the input module
      virtual int InitInput(LPCTSTR infilename, SettingsManager& mgr,
         TrackInfo& trackinfo, SampleContainer& samples) = 0;

      /// returns info about the input file
      virtual void GetInfo(int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz) const = 0;

      /// \brief decodes samples and stores them in the sample container
      /// \details returns number of samples decoded, or 0 if finished
      /// a negative value indicates an error
      virtual int DecodeSamples(SampleContainer& samples) = 0;

      /// returns the number of percent done
      virtual float PercentDone() const { return 0.f; }

      /// called when done with decoding
      virtual void DoneInput() {}

      /// called when done with decoding
      virtual void DoneInput(bool /*completedTrack*/) { DoneInput(); }
   };


   /// output module base class
   class OutputModule : public ModuleBase
   {
   public:
      /// dtor
      virtual ~OutputModule() {}

      /// returns the extension the output module produces (e.g. "mp3")
      virtual CString GetOutputExtension() const = 0;

      /// lets the output module fetch some settings, right after module creation
      virtual void PrepareOutput(SettingsManager& mgr) { mgr; }

      /// initializes the output module
      virtual int InitOutput(LPCTSTR outfilename, SettingsManager& mgr,
         const TrackInfo& trackinfo, SampleContainer& samplecont) = 0;

      /// \brief encodes samples from the sample container
      /// \details it is required that all samples from the container will be used up;
      /// returns 0 if all was ok, or a negative value on error
      virtual int EncodeSamples(SampleContainer& samples) = 0;

      /// cleans up the output module
      virtual void DoneOutput() {}
   };


   /// returns a filename compatible for ansi APIs such as fopen()
   CString GetAnsiCompatFilename(LPCTSTR pszFilename);

} // namespace Encoder
