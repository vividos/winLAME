//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2020 Michael Fink
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
//
#pragma once

#include "resource.h"
#include "UISettings.hpp"
#include "LanguageResourceManager.hpp"

class PresetManagerInterface;
class TaskManager;
namespace Encoder
{
   class ModuleManager;
   class LameNogapInstanceManager;
}
namespace UI
{
   class WizardPage;
   class WizardPageHost;
}

/// main application class
class App
{
public:
   /// ctor
   explicit App(HINSTANCE hInstance);
   /// dtor
   ~App();

   /// initialzes crash reporter
   static void InitCrashReporter();

   /// shows error dialog on crash
   static void ShowCrashErrorDialog(LPCTSTR crashDumpFilename);

   /// returns if help file is available
   bool IsHelpAvailable() const { return m_helpAvailable; }

   /// returns help filename
   CString HelpFilename() const { return m_helpFilename; }

   /// indicates if command line was already read by main dialog
   bool AlreadyReadCommandLine() const { return m_alreadyReadCommandLine; }

   /// sets flag that command line was already read by MainDlg or MainFrame
   void SetAlreadyReadCommandLine() { m_alreadyReadCommandLine = true; }

   /// indicates if main dialog or mainframe should start with Input CD dialog
   bool StartInputCD() const { return m_startInputCD; }

   /// resets flag to start Input CD dialog
   void ResetStartInputCD() { m_startInputCD = false; }

   /// runs application
   int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT);


   // static methods

   /// returns current app object
   static App& Current() { ATLASSERT(s_pApp != NULL); return *s_pApp; }

   /// returns user or machine wide app data folder
   static CString AppDataFolder(bool machineWide);

   /// returns app folder; ends with backslash
   static CString AppFolder();

   /// retrieves app version name
   static CString Version();

   /// retrieves app version number
   static CString VersionNumber();

   /// retrieves the small (16x16) or the large app icon
   static HICON AppIcon(bool smallIcon = true);

private:
   /// runs "classic" winLAME dialog
   void RunClassicDialog();

   /// returns the start wizard page for classic mode
   std::shared_ptr<UI::WizardPage> GetClassicModeStartWizardPage(UI::WizardPageHost& host);

   /// runs new main frame based winLAME window
   int RunMainFrame(int nCmdShow);

   /// loads presets file
   void LoadPresetFile();

   /// checks if app is running as UWP app (via Desktop Bridge)
   static bool IsRunningAsUwpApp();

private:
   /// current app object
   static App* s_pApp;

   /// language resource manager
   LanguageResourceManager m_langResourceManager;

   /// user interface settings
   UISettings m_settings;

   /// task manager
   std::shared_ptr<TaskManager> m_spTaskManager;

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

   /// indicates if app should start Input CD dialog
   bool m_startInputCD;
};
