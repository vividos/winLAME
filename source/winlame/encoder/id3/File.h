/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2006-2014 Michael Fink

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
/// \file File.h
/// \brief id3 file class
#pragma once

#include <boost/shared_ptr.hpp>

#include "Tag.h"

struct id3_file;

namespace ID3
{

class File
{
public:
   /// opens a file
   File(const CString& cszFilename, bool bReadOnly);
   ~File(){}

   /// returns if a ID3v1 tag is present
   bool HasID3v1Tag() const throw();

   /// returns if a ID3v2 tag is present
   bool HasID3v2Tag() const throw();

   /// returns primary tag, ID3v2 when present, else ID3v1
   Tag GetTag();

   /// returns primary tag, ID3v2 when present, else ID3v1; const version
   const Tag GetTag() const;

   /// updates file with changes in tag
   bool Update();

private:
   boost::shared_ptr<id3_file> m_spFile;
   bool m_bReadOnly;
};

}

/*
   // Usage example:

   ID3::File file(cszFilename, true);

   if (file.HasID3v1Tag())
      ATLTRACE(_T("has ID3v1 tag\n"));

   if (file.HasID3v2Tag())
      ATLTRACE(_T("has ID3v2 tag\n"));

   ID3::Tag tag = file.GetTag();

   ATLTRACE(_T("frame count: %u\n"), tag.GetFrameCount());
   for (unsigned int i=0,iMax=tag.GetFrameCount(); i<iMax; i++)
   {
      ID3::Frame frame = tag.GetByIndex(i);

      ATLTRACE(_T("frame %u: name=[%s] desc=[%s] text=[%s]\n"),
         i,
         frame.GetId(),
         frame.GetDescription(),
         frame.AsString());

      ID3::Field field = frame.GetField(0);
   }

   if (tag.IsFrameAvail(ID3::FrameId::Genre))
   {
      ID3::Frame frame = tag.FindFrame(ID3::TagName::Genre);
      ATLTRACE(_T("genre: [%s]\n"), frame.AsString());
   }

*/
