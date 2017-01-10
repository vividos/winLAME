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
/// \file EncoderErrorHandler.hpp
/// \brief encoder error handler interface
//
#pragma once

namespace Encoder
{
   /// error handler interface
   class EncoderErrorHandler
   {
   public:
      /// dtor
      virtual ~EncoderErrorHandler() throw() {}

      /// action to perform when error is handled
      enum ErrorAction
      {
         Continue = 0,  ///< continues encoding
         SkipFile,      ///< skips the current file
         StopEncode     ///< stops encoding
      };

      /// error handler function
      virtual ErrorAction HandleError(
         LPCTSTR inputFilename,
         LPCTSTR moduleName,
         int errorNumber, LPCTSTR errorMessage, bool skipDisabled = false) = 0;
   };

} // namespace Encoder
