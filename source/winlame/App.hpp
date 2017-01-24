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
/// \file App.hpp
/// \brief App class

// include guard
#pragma once

// includes
#include "resource.h"
#include "UISettings.hpp"
#include "LanguageResourceManager.hpp"
#include "TaskManager.hpp"

// forward references
class PresetManagerInterface;
namespace Encoder
{
   class ModuleManager;
   class LameNogapInstanceManager;
}

/// main application class
class App
{
public:
   /// ctor
   App(HINSTANCE hInstance);
   /// dtor
   ~App();

   /// initialzes crash reporter
   static void InitCrashReporter();

   /// returns if help file is available
   bool IsHelpAvailable() const throw() { return m_helpAvailable; }

   /// returns help filename
   CString HelpFilename() const { return m_helpFilename; }

   /// indicates if command line was already read by main dialog
   bool AlreadyReadCommandLine() const throw() { return m_alreadyReadCommandLine; }

   /// sets flag that command line was already read by MainDlg or MainFrame
   void SetAlreadyReadCommandLine() throw() { m_alreadyReadCommandLine = true; }

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
   /// runs "classic" winLAME dialog
   void RunClassicDialog();

   /// runs new main frame based winLAME window
   int RunMainFrame(int nCmdShow);

   /// loads presets file
   void LoadPresetFile();

private:
   /// current app object
   static App* s_pApp;

   /// language resource manager
   LanguageResourceManager m_langResourceManager;

   /// user interface settings
   UISettings m_settings;

   /// task manager
   TaskManager m_taskManager;

   /// LAME nogap instance manager
   std::shared_ptr<Encoder::LameNogapInstanceManager> m_spLameNogapInstanceManager;

   /// preset manager
   std::shared_ptr<PresetManagerInterface> m_spPresetManager;

   /// module manager
   std::shared_ptr<Encoder::ModuleManager> m_spModuleManager;

   /// indicates if help file is available
   bool m_helpAvailable;

   /// help filename
   CString m_helpFilename;

   /// indicates if command line was already read by main dialog
   bool m_alreadyReadCommandLine;

   /// indicates if app should be exited
   bool m_exit;
};
