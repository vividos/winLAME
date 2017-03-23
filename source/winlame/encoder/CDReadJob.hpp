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
/// \file CDReadJob.hpp
/// \brief CD read job
//
#pragma once

#include "CDRipDiscInfo.hpp"
#include "CDRipTrackInfo.hpp"

namespace Encoder
{
   /// infos about a CD read job
   class CDReadJob
   {
   public:
      /// ctor
      CDReadJob(const CDRipDiscInfo& discInfo, const CDRipTrackInfo& trackInfo)
         :m_discInfo(discInfo),
         m_trackInfo(trackInfo)
      {
      }

      // getter

      /// returns output filename
      const CString& OutputFilename() const { return m_outputFilename; }

      /// returns disc info
      const CDRipDiscInfo& DiscInfo() const { return m_discInfo; }

      /// returns track info
      const CDRipTrackInfo& TrackInfo() const { return m_trackInfo; }

      /// returns track info; non-const version
      CDRipTrackInfo& TrackInfo() { return m_trackInfo; }

      /// returns title
      const CString& Title() const { return m_title; }

      /// return front cover art image
      const std::vector<unsigned char>& FrontCoverArtImage() const
      {
         return m_covertArtImageData;
      }

      // setter

      /// sets output filename
      void OutputFilename(const CString& outputFilename) { m_outputFilename = outputFilename; }

      /// sets title
      void Title(const CString& title) { m_title = title; }

      /// sets front cover art image
      void FrontCoverArtImage(const std::vector<unsigned char>& covertArtImageData)
      {
         m_covertArtImageData = covertArtImageData;
      }

   private:
      CString m_outputFilename;     ///< output filename
      CDRipDiscInfo m_discInfo;     ///< disc info
      CDRipTrackInfo m_trackInfo;   ///< track info
      CString m_title;              ///< track title

      /// front cover art image; empty when not set
      std::vector<unsigned char> m_covertArtImageData;
   };

} // namespace Encoder
