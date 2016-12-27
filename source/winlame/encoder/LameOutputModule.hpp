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
/// \file LameOutputModule.hpp
/// \brief contains the LAME output module
//
#pragma once

#include "ModuleInterface.hpp"
#include <iosfwd>
#include "nlame.h"

namespace Encoder
{
   // forward references
   struct Id3v1Tag;

   /// LAME output module
   class LameOutputModule : public OutputModule
   {
   public:
      /// ctor
      LameOutputModule();
      /// dtor
      virtual ~LameOutputModule() throw();

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("LAME mp3 Encoder"); }

      /// returns the last error
      virtual CString GetLastError() const override { return m_lastError; }

      /// returns if the module is available
      virtual bool IsAvailable() const override;

      /// returns description of current file
      virtual void GetDescription(CString& desc) const override { desc = m_description; }

      /// returns version string
      virtual void GetVersionString(CString& version, int special = 0) const override;

      /// returns the extension the output module produces
      virtual CString GetOutputExtension() const override { return m_writeWaveHeader ? _T("wav") : _T("mp3"); }

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
      /// sets all encoding parameters from settings
      int SetEncodingParameters(SettingsManager& mgr);

      /// generatse a description text
      void GenerateDescription(SettingsManager& mgr);

      /// encodes one frame
      int EncodeFrame();

      /// adds id3v2 tag infos to nlame instance
      void AddLameID3v2Tag(const TrackInfo& trackinfo);

      /// estimate ID3v2 padding length
      unsigned int GetID3v2PaddingLength();

      /// writes out ID3v2 tag
      void WriteID3v2Tag();

      /// Writes VBR Info tag
      static void WriteVBRInfoTag(nlame_instance_t* inst, LPCTSTR mp3filename);

   private:
      /// nlame instance
      nlame_instance_t* m_instance;

      /// output file stream
      std::ofstream m_outputFile;

      /// output mp3 filename
      CString m_mp3Filename;

      /// indicates if we should write a vbr info tag
      bool m_writeInfoTag;

      /// encode buffer type
      nlame_encode_buffer_type m_bufferType;

      /// current input buffer size for LAME
      unsigned int m_inputBufferSize;

      /// LAME input buffer
      std::vector<unsigned char> m_inputBuffer;

      /// inbuffer filled indicator
      unsigned int m_inputBufferFill;

      /// mp3 output buffer
      std::vector<unsigned char> m_mp3OutputBuffer;

      /// last error occured
      CString m_lastError;

      /// encoding description
      CString m_description;

      /// possible id3 tag to append to the file
      std::unique_ptr<Id3v1Tag> m_ID33v1Tag;

      /// track info for writing ID3v2 tag
      TrackInfo m_trackInfoID3v2;

      /// indicates if we encode for gapless output
      bool m_nogapEncoding;

      /// indicates if we encode the last file
      bool m_nogapIsLastFile;

      /// saved nlame instance for nogap encoding
      static nlame_instance_t* m_nogapInstance;

      /// indicates if we should write a wave header
      bool m_writeWaveHeader;

      /// number of samples encoded so gar
      unsigned int m_numSamplesEncoded;

      /// number of data bytes written
      unsigned int m_numDataBytesWritten;
   };

} // namespace Encoder
