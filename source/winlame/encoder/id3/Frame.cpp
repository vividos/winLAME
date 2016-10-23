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

CString Frame::AsString()
{
   ATLASSERT(m_spFrame != NULL);

   CString cszText;
   enum id3_field_textencoding encoding = ID3_FIELD_TEXTENCODING_UTF_8;

   // read first field
   id3_field* field = id3_frame_field(m_spFrame.get(), 0);

   // check if it's a proper type
   enum id3_field_type type = id3_field_type(field);

   if (type == ID3_FIELD_TYPE_TEXTENCODING && m_spFrame->nfields >= 2)
   {
      // first one is a text encoding field, so get encoding, then get second one
      encoding = id3_field_gettextencoding(field);
      field = id3_frame_field(m_spFrame.get(), 1);
   }

   switch (id3_field_type(field))
   {
   case ID3_FIELD_TYPE_TEXTENCODING:
      // should not appear a second time, or at second position
      ATLASSERT(false);
      break;

   case ID3_FIELD_TYPE_LATIN1:
      {
         ATLASSERT(encoding == ID3_FIELD_TEXTENCODING_ISO_8859_1 ||
            encoding == ID3_FIELD_TEXTENCODING_UTF_8);
         const id3_latin1_t* latin1 = id3_field_getlatin1(field);
         cszText = Latin1ToString(latin1);
      }
      break;

   case ID3_FIELD_TYPE_LATIN1FULL:
      {
         ATLASSERT(encoding == ID3_FIELD_TEXTENCODING_ISO_8859_1 ||
            encoding == ID3_FIELD_TEXTENCODING_UTF_8);
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
         for (unsigned int i=0,iMax=id3_field_getnstrings(field); i<iMax; i++)
         {
            const id3_ucs4_t* str = id3_field_getstrings(field, i);
            cszText += Ucs4ToString(str);
         }
      }
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
         cszText.Format(_T("%i"), value);
      }
      break;

   case ID3_FIELD_TYPE_BINARYDATA:
   default:
      ATLASSERT(false);
   }

   return cszText;
}

void Frame::SetString(const CString& cszText)
{
   // TODO
}
