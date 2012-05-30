/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2004 Michael Fink

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
/// \file HtmlHelper.h
/// \brief contains a HTML help API wrapper class
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include <htmlhelp.h>


/// html help support class

class HtmlHelper
{
public:
   /// ctor
   HtmlHelper()
   {
      // initialize html help; get cookie
      ::HtmlHelp(NULL,NULL,HH_INITIALIZE,(DWORD)&cookie);
      chmfile = NULL;
   }

   /// dtor
   ~HtmlHelper()
   {
      // uninitialize; return cookie
      ::HtmlHelp(NULL,NULL,HH_UNINITIALIZE,cookie);
      delete chmfile;
   }

   /// initializes the html help api
   void Init(HWND parent, LPCTSTR chmhelpfile)
   {
      hParent = parent;
      chmfile = new TCHAR[_tcslen(chmhelpfile)+1];
      lstrcpy(chmfile,chmhelpfile);
   }

   /// displays a topic in the html help file
   void DisplayTopic(LPCTSTR topic)
   {
      ::HtmlHelp(hParent,chmfile,HH_DISPLAY_TOPIC,(DWORD)topic);
   }

protected:
   /// parent window
   HWND hParent;

   /// location of chm file
   LPTSTR chmfile;

   /// good tasting cookie
   DWORD cookie;
};


/// @}
