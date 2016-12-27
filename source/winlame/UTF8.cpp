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
/// \file UTF8.cpp
/// \brief UTF-8 conversion
//
#include "stdafx.h"
#include "UTF8.hpp"

void StringToUTF8(const CString& text, std::vector<char>& utf8Buffer)
{
#if defined(UNICODE) || defined(_UNICODE)
   // calculate the bytes necessary
   unsigned int length = ::WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);

   utf8Buffer.resize(length);

   // convert
   ::WideCharToMultiByte(CP_UTF8, 0, text, -1, &utf8Buffer[0], length, nullptr, nullptr);
#else
#error non-unicode variant not implemented!
#endif
}

CString UTF8ToString(const char* utf8Text)
{
   int bufferLength = MultiByteToWideChar(CP_UTF8, 0,
      utf8Text, -1,
      nullptr, 0);

   CString text;
   MultiByteToWideChar(CP_UTF8, 0,
      utf8Text, -1,
      text.GetBuffer(bufferLength), bufferLength);

   text.ReleaseBuffer();
   return text;
}
