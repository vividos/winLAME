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
/// \file OggVorbisInputModule.hpp
/// \brief contains the ogg vorbis input module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include <cstdio>
#include "vorbis/vorbisfile.h"

namespace Encoder
{
   /// Ogg Vorbis input module
   class OggVorbisInputModule : public InputModule
   {
   public:
      // ctor
      OggVorbisInputModule();
      // dtor
      virtual ~OggVorbisInputModule() throw();

      /// clones input module
      virtual InputModule* CloneModule() override;

      // returns the module name
      virtual CString GetModuleName() const override { return _T("Ogg Vorbis Decoder"); }

      // returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      // returns if the module is available
      virtual bool IsAvailable() const override;

      // returns description of current file
      virtual CString GetDescription() const override;

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
         return m_numMaxSamples > 0 ? float(m_numCurrentSamples)*100.f / m_numMaxSamples : 0.f;
      }

      // called when done with decoding
      virtual void DoneInput() override;

   private:
      /// last error occured
      CString m_lastError;

      /// number of samples decoded
      __int64 m_numCurrentSamples;

      /// maximum number of samples
      __int64 m_numMaxSamples;

      /// input file
      FILE* m_inputFile;

      /// decoding file struct
      OggVorbis_File m_vf;
   };

} // namespace Encoder
