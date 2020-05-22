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
/// \file ModuleBase.hpp
/// \brief contains the interface definition for input and output modules
//
#pragma once

#include <string>
#include "SettingsManager.hpp"
#include "TrackInfo.hpp"
#include "SampleContainer.hpp"

namespace Encoder
{
   /// module base class
   class ModuleBase : public boost::noncopyable
   {
   public:
      /// dtor
      virtual ~ModuleBase()
      {
         m_moduleId = 0;
      }

      /// returns the module name
      virtual CString GetModuleName() const = 0;

      /// returns the last error
      virtual CString GetLastError() const = 0;

      /// returns if the module is available
      virtual bool IsAvailable() const = 0;

      /// returns module id
      int GetModuleID() const { return m_moduleId; }

      /// returns description of current file
      virtual CString GetDescription() const { return CString(); }

      /// returns version string; value in special may denote special type of string
      virtual void GetVersionString(CString& version, int special = 0) const { version.Empty(); UNUSED(special); }

      /// resolves possibly encoded filenames
      virtual void ResolveRealFilename(CString& filename) { UNUSED(filename); }

   protected:
      /// module id
      int m_moduleId;

      /// number of channels
      int m_channels;

      /// sample rate
      int m_samplerate;
   };

} // namespace Encoder
