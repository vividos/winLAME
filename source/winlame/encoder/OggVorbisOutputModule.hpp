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
/// \file OggVorbisOutputModule.hpp
/// \brief contains the ogg vorbis output module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include <iosfwd>
#include "vorbis/codec.h"

namespace Encoder
{
   /// Ogg Vorbis output module
   class OggVorbisOutputModule : public OutputModule
   {
   public:
      /// ctor
      OggVorbisOutputModule();
      /// dtor
      ~OggVorbisOutputModule();

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("Ogg Vorbis Encoder"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual CString GetDescription() const override;

      /// returns version string
      virtual void GetVersionString(CString& version, int special = 0) const override;

      /// returns the extension the output module produces
      virtual CString GetOutputExtension() const override { return _T("ogg"); }

      /// initializes the output module
      virtual int InitOutput(LPCTSTR outfilename, SettingsManager& mgr,
         const TrackInfo& trackInfo, SampleContainer& samples) override;

      /// encodes samples from the sample container
      virtual int EncodeSamples(SampleContainer& samples) override;

      /// cleans up the output module
      virtual void DoneOutput() override;

   private:
      /// initializes vorbis info struct
      int InitVorbisInfo(SettingsManager& mgr);

      /// adds track info to vorbis info struct
      void AddTrackInfo(const TrackInfo& trackInfo);

      /// initializes encoder
      void InitEncoder();

      /// writes Ogg Vorbis header
      void WriteHeader();

   private:
      /// output file stream
      std::ofstream m_outputStream;

      /// last error occured
      CString m_lastError;

      /// chosen bitrate mode
      int m_bitrateMode;

      /// base quality (when m_bitrateMode==0)
      float m_baseQuality;

      /// end of stream marker
      bool m_endOfStream;

      /// take physical pages, weld into a logical stream of packets
      ogg_stream_state m_os;

      /// one ogg bitstream page. vorbis packets are inside
      ogg_page         m_og;

      /// one raw packet of data for decode
      ogg_packet       m_op;

      /// struct that stores all the static vorbis bitstream settings
      vorbis_info      m_vi;

      /// struct that stores all the bitstream user comments
      vorbis_comment   m_vc;

      /// central working state for the packet->PCM decoder
      vorbis_dsp_state m_vd;

      /// local working space for packet->PCM decode
      vorbis_block     m_vb;
   };

} // namespace Encoder
