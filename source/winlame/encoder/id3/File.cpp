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
/// \file File.cpp
/// \brief id3 file class

#include "StdAfx.h"
#include "File.h"
#include <id3tag.h>

using ID3::File;

File::File(const CString& cszFilename, bool bReadOnly)
:m_bReadOnly(bReadOnly)
{
   id3_file* file = id3_file_open(CStringA(cszFilename), bReadOnly ? ID3_FILE_MODE_READONLY : ID3_FILE_MODE_READWRITE);
   if (file != NULL)
      m_spFile.reset(file, id3_file_close);
}

// it's pretty ugly, but we repeat the filetag and id3_file structure here, so we have access to its internals
struct filetag {
  struct id3_tag *tag;
  unsigned long location;
  id3_length_t length;
};

struct id3_file {
  FILE *iofile;
  enum id3_file_mode mode;
  char *path;

  int flags;

  struct id3_tag *primary;

  unsigned int ntags;
  struct filetag *tags;
};

bool File::HasID3v1Tag() const throw()
{
   if (m_spFile == NULL)
      return false;

   // search for id3v1 tag
   for (unsigned int i=0,iMax=m_spFile->ntags; i<iMax; i++)
   {
      if (ID3_TAG_VERSION_MAJOR(m_spFile->tags[i].tag->version) == 1)
         return true;
   }
   return false;
}

bool File::HasID3v2Tag() const throw()
{
   if (m_spFile == NULL)
      return false;

   // search for id3v2 tag
   for (unsigned int i=0,iMax=m_spFile->ntags; i<iMax; i++)
   {
      if (m_spFile->tags[i].tag != NULL &&
          ID3_TAG_VERSION_MAJOR(m_spFile->tags[i].tag->version) > 1)
         return true;
   }
   return false;
}

/// deletor template that does nothing
template <typename T> void do_nothing(T*){}

ID3::Tag File::GetTag()
{
   ATLASSERT(m_spFile != NULL);

   std::shared_ptr<id3_tag> spTag(id3_file_tag(m_spFile.get()), do_nothing<id3_tag>);
   return ID3::Tag(spTag);
}

const ID3::Tag File::GetTag() const
{
   ATLASSERT(m_spFile != NULL);

   std::shared_ptr<id3_tag> spTag(id3_file_tag(m_spFile.get()));
   return ID3::Tag(spTag);
}

bool File::Update()
{
   ATLASSERT(m_spFile != NULL);
   ATLASSERT(m_bReadOnly == false);

   int iRet = id3_file_update(m_spFile.get());
   return iRet == 0;
}
