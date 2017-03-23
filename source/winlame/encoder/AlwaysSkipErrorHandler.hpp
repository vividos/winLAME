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
/// \file AlwaysSkipErrorHandler.hpp
/// \brief error handler that always skips file
//
#pragma once

#include <vector>
#include "EncoderErrorHandler.hpp"

namespace Encoder
{
   /// error handler that always skips file and stores error info
   class AlwaysSkipErrorHandler : public EncoderErrorHandler
   {
   public:
      /// error info
      struct ErrorInfo
      {
         /// ctor
         ErrorInfo()
            :m_errorNumber(0)
         {
         }

         CString m_inputFilename;   ///< input filename
         CString m_moduleName;      ///< module name
         int m_errorNumber;         ///< error number
         CString m_errorMessage;    ///< error message
      };

      /// dtor
      virtual ~AlwaysSkipErrorHandler() {}

      /// error handler function
      virtual ErrorAction HandleError(LPCTSTR inputFilename,
         LPCTSTR moduleName, int errorNumber, LPCTSTR errorMessage, bool skipDisabled) override
      {
         ErrorInfo errorInfo;
         errorInfo.m_inputFilename = inputFilename;
         errorInfo.m_moduleName = moduleName;
         errorInfo.m_errorNumber = errorNumber;
         errorInfo.m_errorMessage = errorMessage;

         m_allErrorsList.push_back(errorInfo);

         return skipDisabled ? EncoderErrorHandler::Continue : EncoderErrorHandler::SkipFile;
      }

      /// returns list of all errors
      const std::vector<ErrorInfo>& AllErrors() const { return m_allErrorsList; }

   private:
      /// list of all errors
      std::vector<ErrorInfo> m_allErrorsList;
   };

} // namespace Encoder
