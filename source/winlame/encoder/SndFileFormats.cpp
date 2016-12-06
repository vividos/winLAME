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
/// \file SndFileFormats.cpp
/// \brief libSndFile formats and subtypes

// includes
#include "stdafx.h"
#include "SndFileFormats.hpp"
#include <sndfile.h>

std::vector<int> SndFileFormats::EnumFormats()
{
   std::vector<int> allFormats;

   int count = 0;
   sf_command(nullptr, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof(int));

   SF_FORMAT_INFO formatInfo;
   for (int formatIndex = 0; formatIndex < count; formatIndex++)
   {
      formatInfo.format = formatIndex;
      sf_command(nullptr, SFC_GET_FORMAT_MAJOR, &formatInfo, sizeof(formatInfo));

      allFormats.push_back(formatInfo.format);
   }

   return allFormats;
}

std::vector<int> SndFileFormats::EnumSubTypes()
{
   std::vector<int> allSubTypes;

   int subtypeCount = 0;
   sf_command(nullptr, SFC_GET_FORMAT_SUBTYPE_COUNT, &subtypeCount, sizeof(int));

   SF_FORMAT_INFO subTypeInfo;
   for (int subtypeIndex = 0; subtypeIndex < subtypeCount; subtypeIndex++)
   {
      subTypeInfo.format = subtypeIndex;
      sf_command(nullptr, SFC_GET_FORMAT_SUBTYPE, &subTypeInfo, sizeof(subTypeInfo));

      allSubTypes.push_back(subTypeInfo.format);
   }

   return allSubTypes;
}

bool SndFileFormats::IsValidFormatCombo(int format, int subType)
{
   SF_INFO info = { 0 };
   info.channels = 2;
   info.format = format | subType;

   return sf_format_check(&info) != 0;
}

bool SndFileFormats::GetFormatInfo(int format, CString& formatName, CString& outputExtension)
{
   int count = 0;
   sf_command(nullptr, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof(int));

   SF_FORMAT_INFO formatInfo;
   for (int formatIndex = 0; formatIndex < count; formatIndex++)
   {
      formatInfo.format = formatIndex;
      sf_command(nullptr, SFC_GET_FORMAT_MAJOR, &formatInfo, sizeof(formatInfo));

      if (formatInfo.format == format)
      {
         formatName = formatInfo.name;
         outputExtension = formatInfo.extension;
         return true;
      }
   }

   return false;
}

CString SndFileFormats::GetSubTypeName(int subType)
{
   int subtypeCount = 0;
   sf_command(nullptr, SFC_GET_FORMAT_SUBTYPE_COUNT, &subtypeCount, sizeof(int));

   SF_FORMAT_INFO formatInfo;
   for (int subtypeIndex = 0; subtypeIndex < subtypeCount; subtypeIndex++)
   {
      formatInfo.format = subtypeIndex;

      sf_command(nullptr, SFC_GET_FORMAT_SUBTYPE, &formatInfo, sizeof(formatInfo));

      if (formatInfo.format == subType)
      {
         return CString(formatInfo.name);
      }
   }

   return CString();
}
