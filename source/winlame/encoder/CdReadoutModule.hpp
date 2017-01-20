//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2005-2016 Michael Fink
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
/// \file CDReadoutModule.hpp
/// \brief contains the Bass input module definition
//
#pragma once

#include "ModuleInterface.hpp"
#include "SndFileInputModule.hpp"

namespace Encoder
{
   /// CD readout input module
   class CDReadoutModule : public SndFileInputModule
   {
   public:
      // ctor
      CDReadoutModule();
      /// dtor
      virtual ~CDReadoutModule() throw() {}

      /// clones input module
      virtual InputModule* CloneModule() override;

      /// returns the module name
      virtual CString GetModuleName() const override { return _T("CD Audio Extraction"); }

      /// returns description of current file
      virtual CString GetDescription() const override;

      /// returns filter string
      virtual CString GetFilterString() const override;

      /// resolves possibly encoded filenames
      virtual void ResolveRealFilename(CString& filename) override;

      /// initializes the input module
      virtual int InitInput(LPCTSTR infilename, SettingsManager& mgr,
         TrackInfo& trackInfo, SampleContainer& samples) override;

      /// called when done with decoding
      virtual void DoneInput(bool isTrackCompleted) override;

   private:
      /// track index
      unsigned int m_trackIndex;
   };

} // namespace Encoder
