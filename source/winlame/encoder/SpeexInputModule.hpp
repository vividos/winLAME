//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014 Michael Fink
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

// includes
#include "ModuleInterface.h"
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <ogg/ogg.h>
#include "OggInputStream.hpp"

/// input module for Speex encoded files
class SpeexInputModule : public InputModule
{
public:
   /// ctor
   SpeexInputModule();
   // dtor
   virtual ~SpeexInputModule() throw() {}

   /// clones input module
   virtual InputModule* cloneModule() override;

   /// returns the module name
   virtual CString getModuleName() override { return _T("Speex Audio File Decoder"); }

   /// returns the last error
   virtual CString getLastError() override { return m_cszLastError; }

   /// returns if the module is available
   virtual bool isAvailable() override;

   /// returns description of current file
   virtual void getDescription(CString& desc) override;

   /// returns version string
   virtual void getVersionString(CString& version, int special = 0) override;

   /// returns filter string
   virtual CString getFilterString() override;

   /// initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager& mgr,
      TrackInfo& trackinfo, SampleContainer& samples) override;

   /// returns info about the input file
   virtual void getInfo(int& channels, int& bitrate, int& length, int& samplerate) override;

   /// decodes samples and stores them in the sample container
   virtual int decodeSamples(SampleContainer& samples) override;

   /// returns the number of percent done
   virtual float percentDone() override;

   /// called when done with decoding
   virtual void doneInput() override;

private:
   /// inits decoder
   void InitDecoder();

   /// reads next packet from stream
   bool ReadNextPacket(ogg_packet& packet);

   /// decodes packet to 16-bit samples
   int DecodePacket(ogg_packet& packet, std::vector<short>& vecSamples);

private:
   /// last error text
   CString m_cszLastError;

   /// input stream
   std::shared_ptr<OggInputStream> m_spInputStream;

   /// file size
   long m_lFileSize;

   /// packet count
   unsigned int m_packetCount;

   /// speex header
   std::shared_ptr<SpeexHeader> m_spHeader;

   /// speex bit-packing struct
   SpeexBits m_bits;

   /// stereo state
   SpeexStereoState m_stereo;

   /// encoder state
   std::shared_ptr<void> m_spDecoderState;
};
