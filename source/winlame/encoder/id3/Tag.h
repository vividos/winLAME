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
/// \file Tag.h
/// \brief id3 tag class

#pragma once

#include <boost/shared_ptr.hpp>
#include "Frame.h"

struct id3_tag;

namespace ID3
{

/// represents an ID3 tag (either version 1 or 2)
class Tag
{
public:
   /// tag option for SetOption()
   enum TagOption
   {
      foID3v1 = 0x100, ///< when set, writes frame to ID3v1 tag, too
   };

   Tag();
   ~Tag(){}

   // read access to tag

   /// returns if frame with id is available
   bool IsFrameAvail(const CString& cszFrameId) const;

   /// finds frame with given id
   Frame FindFrame(const CString& cszFrameId);

   /// returns frame count
   unsigned int GetFrameCount() const;

   /// gets frame by index
   Frame GetByIndex(unsigned int uiFrameIndex);

   /// gets frame by index; const version
   const Frame GetByIndex(unsigned int uiFrameIndex) const;

   // write access to tag

   /// sets option for tag
   void SetOption(TagOption enOpt, bool bSetOpt);

   /// adds a frame to tag
   void AttachFrame(Frame& f);

   /// removes frame from tag
   void DetachFrame(Frame& f);

private:
   friend class File;

   /// ctor
   Tag(boost::shared_ptr<id3_tag> spTag)
      :m_spTag(spTag)
   {
   }

private:
   /// tag data
   boost::shared_ptr<id3_tag> m_spTag;
};

} // namespace ID3
