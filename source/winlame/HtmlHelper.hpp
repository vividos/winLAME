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
/// \file HtmlHelper.hpp
/// \brief contains a HTML help API wrapper class
//
#pragma once

#include <htmlhelp.h>

/// html help support class
class HtmlHelper
{
public:
   /// ctor
   HtmlHelper()
      :m_parentHandle(nullptr)
   {
      // initialize html help; get cookie
      ::HtmlHelp(nullptr, nullptr, HH_INITIALIZE, reinterpret_cast<DWORD>(&m_cookie));
   }

   /// dtor
   ~HtmlHelper()
   {
      // uninitialize; return cookie
      ::HtmlHelp(nullptr, nullptr, HH_UNINITIALIZE, m_cookie);
   }

   /// initializes the html help api
   void Init(HWND parent, LPCTSTR chmHelpFile)
   {
      m_parentHandle = parent;
      m_chmFile = chmHelpFile;
   }

   /// displays a topic in the html help file
   void DisplayTopic(LPCTSTR topic)
   {
      ::HtmlHelp(m_parentHandle, m_chmFile, HH_DISPLAY_TOPIC, reinterpret_cast<DWORD>(topic));
   }

protected:
   /// parent window
   HWND m_parentHandle;

   /// location of chm file
   CString m_chmFile;

   /// good tasting m_cookie
   DWORD m_cookie;
};
