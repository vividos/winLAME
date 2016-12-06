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
/// \file SndFileFormats.hpp
/// \brief libSndFile formats and subtypes

#pragma once

/// helper for libSndFile formats and subtypes
class SndFileFormats
{
public:
   /// enumerates all available formats
   static std::vector<int> EnumFormats();

   /// enumerates all sub types
   static std::vector<int> EnumSubTypes();

   /// checks if a format and sub type is a valid combination
   static bool IsValidFormatCombo(int format, int subType);

   /// returns format info for given format
   static bool GetFormatInfo(int format, CString& formatName, CString& outputExtension);

   /// returns sub type name for given sub type
   static CString GetSubTypeName(int subType);

private:
   /// ctor
   SndFileFormats() = delete;
   /// dtor
   ~SndFileFormats() = delete;
};
