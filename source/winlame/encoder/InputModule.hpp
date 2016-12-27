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
/// \file ModuleInterface.hpp
/// \brief contains the interface definition for input and output modules
//
#pragma once

#include "ModuleBase.hpp"

class SettingsManager;

namespace Encoder
{
   class TrackInfo;
   class SampleContainer;

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

} // namespace Encoder
