/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2006-2009 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/// \file Frame.cpp
/// \brief id3 frame class

#include "StdAfx.h"
#include "Frame.h"
#include <id3tag.h>
extern "C"
{
#include "../src/libid3tag/utf16.h"
}

using ID3::Frame;

/// deletor template that does nothing
template <typename T> void do_nothing(T*){}

Frame::Frame(const CString& cszFrameId)
:m_bAttached(false)
{
   id3_frame* frame = id3_frame_new(CStringA(cszFrameId));
   m_spFrame = std::shared_ptr<id3_frame>(frame, do_nothing<id3_frame>);
}

Frame::~Frame()
{
   if (!m_bAttached)
      id3_frame_delete(m_spFrame.get());
}

unsigned int Frame::GetFieldCount()
{
   return m_spFrame->nfields;
}


ID3::Field Frame::GetField(unsigned int uiIndex)
{
   id3_field* field = id3_frame_field(m_spFrame.get(), uiIndex);
//   id3_field_addref(field);

   std::shared_ptr<id3_field> spField(field, do_nothing<id3_field>);
   return ID3::Field(spField);
}

CString Frame::GetId() const
{
   return m_spFrame->id;
}

CString Frame::GetDescription() const
{
   return m_spFrame->description;
}

signed long Frame::AsInteger()
{
   ATLASSERT(m_spFrame != NULL);

   id3_field* field = id3_frame_field(m_spFrame.get(), 0);

   // check if it's a proper type
   enum id3_field_type type = id3_field_type(field);

   switch (type)
   {
   case ID3_FIELD_TYPE_INT8:
   case ID3_FIELD_TYPE_INT16:
   case ID3_FIELD_TYPE_INT24:
   case ID3_FIELD_TYPE_INT32:
   case ID3_FIELD_TYPE_INT32PLUS:
      return id3_field_getint(field);

   default:
      ATLASSERT(false);
   }
   return 0;
}

/// converts from UCS-4 format to CString
CString Ucs4ToString(const id3_ucs4_t* str)
{
   // convert to utf-16, then copy to CString
   id3_utf16_t* str2 = id3_ucs4_utf16duplicate(str);

   CString cszText(reinterpret_cast<WCHAR*>(str2));

   free(str2);

   return cszText;
}

/// converts from Latin-1 format to CString
CString Latin1ToString(const id3_latin1_t* str)
{
   // convert to utf-16, then copy to CString
//   const id3_utf16_t* str2 = id3_ucs4_utf16duplicate(str2);
//   return CString(str2);
   return CString(str);
}

/// converts from CString to UCS-4 format
void StringToUcs4(CString text, std::vector<id3_ucs4_t>& str)
{
   // convert to utf-16, then copy to CString
   const id3_utf16_t* str1 = reinterpret_cast<const id3_utf16_t*>(text.GetString());
   id3_ucs4_t* str2 = id3_utf16_ucs4duplicate(str1);

   // find end
   id3_ucs4_t* current = str2;
   while (*current++ != 0);

   str.assign(str2, current);

   free(str2);
}

CString Frame::GetString(unsigned int fieldIndex) const
{
   ATLASSERT(m_spFrame != NULL);

   id3_field* field = id3_frame_field(m_spFrame.get(), fieldIndex);

   CString cszText;

   switch (id3_field_type(field))
   {
   case ID3_FIELD_TYPE_LATIN1:
   {
      const id3_latin1_t* latin1 = id3_field_getlatin1(field);
      cszText = Latin1ToString(latin1);
   }
   break;

   case ID3_FIELD_TYPE_LATIN1FULL:
   {
      const id3_latin1_t* latin1 = id3_field_getfulllatin1(field);
      cszText = Latin1ToString(latin1);
   }
   break;

   case ID3_FIELD_TYPE_LATIN1LIST:
      // TODO
      ATLASSERT(false);
      break;

   case ID3_FIELD_TYPE_STRING:
   {
      const id3_ucs4_t* str = id3_field_getstring(field);
      cszText = Ucs4ToString(str);
   }
   break;

   case ID3_FIELD_TYPE_STRINGFULL:
   {
      const id3_ucs4_t* str = id3_field_getfullstring(field);
      cszText = Ucs4ToString(str);
   }
   break;

   case ID3_FIELD_TYPE_STRINGLIST:
   {
      for (unsigned int i = 0, iMax = id3_field_getnstrings(field); i<iMax; i++)
      {
         const id3_ucs4_t* str = id3_field_getstrings(field, i);
         cszText += Ucs4ToString(str);
      }
   }
   break;

   default:
      ATLASSERT(false);
      break;
   }

   return cszText;
}

std::vector<unsigned char> Frame::GetBinaryData(unsigned int fieldIndex) const
{
   ATLASSERT(m_spFrame != NULL);

   id3_field* field = id3_frame_field(m_spFrame.get(), fieldIndex);

   std::vector<unsigned char> binaryData;

   enum id3_field_type type = id3_field_type(field);
   if (type == ID3_FIELD_TYPE_BINARYDATA)
   {
      id3_length_t size = 0;
      const id3_byte_t* data = id3_field_getbinarydata(field, &size);

      binaryData.assign(data, data + size);
   }

   return binaryData;
}

CString Frame::AsString()
{
   ATLASSERT(m_spFrame != NULL);

   CString cszText;
   enum id3_field_textencoding encoding = ID3_FIELD_TEXTENCODING_UTF_8;

   // read first field
   unsigned int fieldIndex = 0;
   id3_field* field = id3_frame_field(m_spFrame.get(), fieldIndex);

   // check if it's a proper type
   enum id3_field_type type = id3_field_type(field);

   if (type == ID3_FIELD_TYPE_TEXTENCODING && m_spFrame->nfields >= 2)
   {
      // first one is a text encoding field, so get encoding, then get second one
      encoding = id3_field_gettextencoding(field);
      field = id3_frame_field(m_spFrame.get(), ++fieldIndex);
   }

   switch (id3_field_type(field))
   {
   case ID3_FIELD_TYPE_TEXTENCODING:
      // should not appear a second time, or at second position
      ATLASSERT(false);
      break;

   case ID3_FIELD_TYPE_LATIN1:
   case ID3_FIELD_TYPE_LATIN1FULL:
   case ID3_FIELD_TYPE_LATIN1LIST:
   case ID3_FIELD_TYPE_STRING:
   case ID3_FIELD_TYPE_STRINGFULL:
   case ID3_FIELD_TYPE_STRINGLIST:
      cszText = GetString(fieldIndex);
      break;

   case ID3_FIELD_TYPE_LANGUAGE:
      cszText = field->immediate.value;
      break;
   case ID3_FIELD_TYPE_FRAMEID:
      cszText = id3_field_getframeid(field);
      break;

   case ID3_FIELD_TYPE_DATE:
      ATLASSERT(false);
      // TODO
      break;

   case ID3_FIELD_TYPE_INT8:
   case ID3_FIELD_TYPE_INT16:
   case ID3_FIELD_TYPE_INT24:
   case ID3_FIELD_TYPE_INT32:
   case ID3_FIELD_TYPE_INT32PLUS:
      {
         signed long value = AsInteger();
         cszText.Format(_T("%li"), value);
      }
      break;

   case ID3_FIELD_TYPE_BINARYDATA:
   default:
      ATLASSERT(false);
   }

   return cszText;
}

bool Frame::SetTextEncoding(unsigned int fieldIndex, id3_field_textencoding textEncoding)
{
   id3_field* field = id3_frame_field(m_spFrame.get(), fieldIndex);

   enum id3_field_type type = id3_field_type(field);

   ATLASSERT(type == ID3_FIELD_TYPE_TEXTENCODING); type;

   int ret = id3_field_settextencoding(field, textEncoding);

   return ret == 0;
}

bool Frame::SetString(unsigned int fieldIndex, const CString& cszText)
{
   id3_field* field = id3_frame_field(m_spFrame.get(), fieldIndex);

   int ret = 1;

   switch (id3_field_type(field))
   {
   case ID3_FIELD_TYPE_LATIN1:
   {
      CStringA text(cszText);
      const id3_latin1_t* latin1 = reinterpret_cast<const id3_latin1_t*>(text.GetString());

      ret = id3_field_setlatin1(field, latin1);
   }
   break;

   case ID3_FIELD_TYPE_LATIN1FULL:
   {
      CStringA text(cszText);
      const id3_latin1_t* latin1 = reinterpret_cast<const id3_latin1_t*>(text.GetString());

      ret = id3_field_setfulllatin1(field, latin1);
   }
   break;

   case ID3_FIELD_TYPE_LATIN1LIST:
      // TODO
      ATLASSERT(false);
      break;

   case ID3_FIELD_TYPE_STRING:
   {
      std::vector<id3_ucs4_t> ucs4text;
      StringToUcs4(cszText, ucs4text);
      ret = id3_field_setstring(field, ucs4text.data());
   }
   break;

   case ID3_FIELD_TYPE_STRINGFULL:
   {
      std::vector<id3_ucs4_t> ucs4text;
      StringToUcs4(cszText, ucs4text);
      ret = id3_field_setfullstring(field, ucs4text.data());
   }
   break;

   case ID3_FIELD_TYPE_STRINGLIST:
   {
      std::vector<id3_ucs4_t> ucs4text;
      StringToUcs4(cszText, ucs4text);
      ret = id3_field_addstring(field, ucs4text.data());
   }
   break;

   case ID3_FIELD_TYPE_LANGUAGE:
   {
      CStringA text(cszText);
      id3_field_setlanguage(field, text.GetString());
   }
   break;

   default:
      // TODO
      ATLASSERT(false);
      break;
   }

   return ret == 0;
}

bool Frame::SetInt8(unsigned int fieldIndex, char fieldValue)
{
   id3_field* field = id3_frame_field(m_spFrame.get(), fieldIndex);

   enum id3_field_type type = id3_field_type(field); type;

   ATLASSERT(type == ID3_FIELD_TYPE_INT8 ||
      type == ID3_FIELD_TYPE_INT16 ||
      type == ID3_FIELD_TYPE_INT24 ||
      type == ID3_FIELD_TYPE_INT32);

   int ret = ::id3_field_setint(field, fieldValue);

   return ret == 0;
}

bool Frame::SetBinaryData(unsigned int fieldIndex, const std::vector<unsigned char>& binaryData)
{
   id3_field* field = id3_frame_field(m_spFrame.get(), fieldIndex);

   enum id3_field_type type = id3_field_type(field); type;

   ATLASSERT(type == ID3_FIELD_TYPE_BINARYDATA);

   int ret = ::id3_field_setbinarydata(field, binaryData.data(), binaryData.size());

   return ret == 0;
}
