//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2012 Michael Fink
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
/// \file App.h
/// \brief App class

// include guard
#pragma once

// includes
#include "resource.h"
#include "UISettings.h"
#include "LanguageResourceManager.hpp"
#include "TaskManager.h"
#include "ModuleManagerImpl.h"

/// main application class
class App
{
public:
   /// ctor
   App(HINSTANCE hInstance);
   /// dtor
   ~App();

   /// runs application
   int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT);


   // static methods

   /// returns current app object
   static App& Current() { ATLASSERT(s_pApp != NULL); return *s_pApp; }

   /// shows app message box
   static int MessageBox(HWND hWndOwner, UINT uiMessageId, UINT uiType)
   {
      return AtlMessageBox(hWndOwner, uiMessageId, IDS_APP_CAPTION, uiType);
   }

   /// returns user or machine wide app data folder; ends with backslash
   static CString AppDataFolder(bool bMachineWide);

   /// returns app folder; ends with backslash
   static CString AppFolder();

   /// retrieves app filename
   static CString AppFilename();

   /// retrieves app version number
   static CString Version();

private:
   /// current app object
   static App* s_pApp;

   /// language resource manager
   LanguageResourceManager m_langResourceManager;

   /// task manager
   TaskManager m_taskManager;
};
