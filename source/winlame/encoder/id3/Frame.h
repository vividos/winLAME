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
/// \file Frame.h
/// \brief id3 frame class

#pragma once

#include <memory>
#include "Field.h"

struct id3_frame;
enum id3_field_textencoding;

namespace ID3
{
   namespace FrameId
   {
      static TCHAR TrackNumber[] =        _T("TRCK");
      static TCHAR EncodedBy[] =          _T("TENC");
      static TCHAR UrlLink[] =            _T("WXXX"); // user defined URL link frame
      static TCHAR DiscNumber[] =         _T("TPOS"); // part of a set
      static TCHAR Copyright[] =          _T("TCOP");
      static TCHAR OriginalArtist[] =     _T("TOPE"); // original performer
      static TCHAR Composer[] =           _T("TCOM");
      static TCHAR Comment[] =            _T("COMM");
      static TCHAR Genre[] =              _T("TCON"); // content type
      static TCHAR Obsolete[] =           _T("ZOBS"); // obsolete frame
      static TCHAR AlbumTitle[] =         _T("TALB");
      static TCHAR AlbumArtist[] =        _T("TPE2"); // band/orchestra/accompaniment
      static TCHAR Artist[] =             _T("TPE1"); // lead performer(s)/soloist(s)
      static TCHAR Title[] =              _T("TIT2");
      static TCHAR RecordingTime[] =      _T("TDRC");
      static TCHAR AttachedPicture[] =    _T("APIC"); ///< attached picture
      //static TCHAR Xxx[] = _T("");
   }


class Frame
{
public:
   /// creates a new frame not attached to a tag
   Frame(const CString& cszFrameId);
   ~Frame();

   // read functions

   /// returns number of fields
   unsigned int GetFieldCount();

   /// returns field by index
   Field GetField(unsigned int uiIndex);

   /// returns frame id
   CString GetId() const;

   /// returns frame description, based on id
   CString GetDescription() const;

   /// returns string field
   CString GetString(unsigned int fieldIndex) const;

   /// returns frame value as integer
   signed long AsInteger();

   /// returns frame value as string
   CString AsString();

   // write functions

   /// sets text encoding for field
   bool SetTextEncoding(unsigned int fieldIndex, id3_field_textencoding textEncoding);

   /// write text as string entry
   bool SetString(unsigned int fieldIndex, const CString& cszText);

   /// write integer entry
   bool SetInt8(unsigned int fieldIndex, char fieldValue);

   /// write binary data entry
   bool SetBinaryData(unsigned int fieldIndex, const std::vector<unsigned char>& binaryData);

private:
   friend class Tag;
   std::shared_ptr<id3_frame> GetFrame(){ return m_spFrame; }

   Frame(std::shared_ptr<id3_frame> spFrame)
      :m_spFrame(spFrame),
       m_bAttached(true)
   {
   }

private:
   bool m_bAttached;
   std::shared_ptr<id3_frame> m_spFrame;
};

} // namespace ID3
