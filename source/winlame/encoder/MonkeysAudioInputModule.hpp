//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
// Copyright (c) Kjetil Haga
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
/// \file MonkeysAudioInputModule.hpp
/// \author Kjetil Haga (programmer.khaga@start.no)
/// \brief contains the Monkey's Audio input module
//
#pragma once

#include "ModuleInterface.hpp"
#include <string>

namespace MonkeysAudio
{
   struct MonkeysAudioDll;
   typedef void* APE_DECOMPRESS_HANDLE;
}

namespace Encoder
{
   /// monkey's audio input module
   class MonkeysAudioInputModule : public InputModule
   {
   public:
      /// ctor
      MonkeysAudioInputModule();
      /// dtor
      virtual ~MonkeysAudioInputModule() throw();

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("Monkey's Audio File Decoder"); }

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
      virtual float PercentDone() const override
      {
         return m_numTotalSamples != 0 ? float(m_numCurrentSamples)*100.f / float(m_numTotalSamples) : 0.f;
      }

      /// called when done with decoding
      virtual void DoneInput() override;

   private:
      /// gets the id3 tag in ape files
      bool GetID3Tag(LPCTSTR filename, TrackInfo& trackInfo);

   private:
      /// pointers to functions
      static MonkeysAudio::MonkeysAudioDll s_dll;

      /// handle to APE decompress data
      void* m_handle;

      /// counts the samples already decoded
      int m_numCurrentSamples;

      /// number of samples in file
      int m_numTotalSamples;

      /// last error occured
      CString m_lastError;
   };

} // namespace Encoder
