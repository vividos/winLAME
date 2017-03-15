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
/// \file OutputModule.hpp
/// \brief contains the interface definition for input and output modules
//
#pragma once

#include "ModuleBase.hpp"

class SettingsManager;

namespace Encoder
{
   class TrackInfo;
   class SampleContainer;

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

} // namespace Encoder
