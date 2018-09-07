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
/// \file TestEncoderErrorHandler.hpp
/// \brief EncoderErrorHandler implementation for unit tests
//
#pragma once

#include "EncoderErrorHandler.hpp"

/// impleents EncoderErrorHandler interface for unit tests
class TestEncoderErrorHandler : public Encoder::EncoderErrorHandler
{
public:
   /// dtor
   virtual ~TestEncoderErrorHandler()
   {
   }

   /// called to handle errors; always stops encoding
   virtual ErrorAction HandleError(
      LPCTSTR /*inputFilename*/, LPCTSTR /*moduleName*/, int /*errorNumber*/, LPCTSTR /*errorMessage*/, bool /*skipDisabled = false*/) override
   {
      return ErrorAction::StopEncode;
   }
};