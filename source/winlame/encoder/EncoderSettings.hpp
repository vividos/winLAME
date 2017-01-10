//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file EncoderSettings.hpp
/// \brief encoder settings
//
#pragma once

namespace Encoder
{
   /// settings for the encoder
   struct EncoderSettings
   {
      CString m_inputFilename;      ///< input filename
      CString m_outputFolder;       ///< output folder
      bool m_outputSameFolder;      ///< indicates if output should be in same folder as input folder
      CString m_outputFilename;     ///< output filename
      CString m_playlistFilename;   ///< playlist filename, without path
      int m_outputModuleID;         ///< output module id that should be used
      bool m_overwriteExisting;     ///< indicates if existing output files can be overwritten
      bool m_deleteInputAfterEncode;///< indicates if input file should be deleted after encoding

      /// track info to store in output
      TrackInfo m_trackInfo;

      /// indicates if the provided track info should be used instead of track info read from
      /// the input file
      bool m_useTrackInfo;

      /// indicates if encoder should warn about lossy transcoding
      bool m_warnLossyTranscoding;

      /// default ctor
      EncoderSettings()
         :m_outputSameFolder(false),
         m_outputModuleID(-1),
         m_overwriteExisting(false),
         m_deleteInputAfterEncode(false),
         m_useTrackInfo(false),
         m_warnLossyTranscoding(true)
      {
      }
   };

} // namespace Encoder
