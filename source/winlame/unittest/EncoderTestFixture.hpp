//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file EncoderTestFixture.hpp
/// \brief Test fixture for testing Encoder classes
//
#pragma once

#include "TestEncoderErrorHandler.hpp"

namespace Encoder
{
   class EncoderImpl;
   class LameNogapInstanceManager;
   class ModuleManager;
}

namespace unittest
{

   class EncoderTestFixture
   {
   public:
      EncoderTestFixture() {}

      static void SetUp();

      static void ExtractFromResource(UINT resourceId, LPCTSTR filename);

      static void StartEncodeAndWaitForFinish(Encoder::EncoderImpl& encoder);

   private:
      /// instance of static LAME NoGap instance manager
      static std::shared_ptr<Encoder::LameNogapInstanceManager> m_spLameNogapInstanceManager;

      /// instance of static module manager
      static std::shared_ptr<Encoder::ModuleManager> m_spModuleManager;

      /// instance of static encoding error handler
      static TestEncoderErrorHandler m_encodingErrorHandler;
   };

} // namespace unittest
