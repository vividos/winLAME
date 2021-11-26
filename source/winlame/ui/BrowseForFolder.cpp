//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2021 Michael Fink
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
/// \file BrowseForFolder.cpp
/// \brief folder-browse function
/// \details contains code for the folder-browse function
//
#include "stdafx.h"
#include "BrowseForFolder.hpp"
#include "resource.h"

#ifndef BIF_USENEWUI
#define BIF_USENEWUI 0x0050   ///< constant to use new UI when selecting folders
#endif

bool BrowseForFolder(HWND hParentWnd, CString& cszPathname, UINT nCaptionID)
{
   if (nCaptionID == 0)
      nCaptionID = IDS_COMMON_SELECTDIR;

   // get caption string
   CString cszCaption;
   cszCaption.LoadString(nCaptionID);

   // add last slash to dir name
   Path::AddEndingBackslash(cszPathname);

   CFolderDialog dlg(hParentWnd, cszCaption, BIF_RETURNONLYFSDIRS | BIF_USENEWUI);
   dlg.SetInitialFolder(cszPathname);
   if (IDOK == dlg.DoModal())
   {
      cszPathname = dlg.GetFolderPath();

      Path::AddEndingBackslash(cszPathname);
      return true;
   }
   else
      return false;
}
