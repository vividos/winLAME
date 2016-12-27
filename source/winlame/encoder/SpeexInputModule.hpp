//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2016 Michael Fink
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
/// \file SpeexInputModule.hpp
/// \brief Speex input module
//
#pragma once

#include "ModuleInterface.hpp"
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <ogg/ogg.h>
#include "OggInputStream.hpp"

namespace Encoder
{
   /// input module for Speex encoded files
   class SpeexInputModule : public InputModule
   {
   public:
      /// ctor
      SpeexInputModule();
      /// dtor
      virtual ~SpeexInputModule() throw() {}

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("Speex Audio File Decoder"); }

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
      virtual float PercentDone() const override;

      /// called when done with decoding
      virtual void DoneInput() override;

   private:
      /// inits decoder
      void InitDecoder();

      /// reads next packet from stream
      bool ReadNextPacket(ogg_packet& packet);

      /// decodes packet to 16-bit samples
      int DecodePacket(ogg_packet& packet, std::vector<short>& samples);

   private:
      /// last error text
      CString m_lastError;

      /// input stream
      std::shared_ptr<OggInputStream> m_inputStream;

      /// file size
      long m_fileSize;

      /// packet count
      unsigned int m_packetCount;

      /// speex header
      std::shared_ptr<SpeexHeader> m_header;

      /// speex bit-packing struct
      SpeexBits m_bits;

      /// stereo state
      SpeexStereoState m_stereo;

      /// encoder state
      std::shared_ptr<void> m_decoderState;
   };

} // namespace Encoder
