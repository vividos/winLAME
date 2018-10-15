//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file App.cpp
/// \brief App class

#include "StdAfx.h"
#include "App.hpp"
#include "ui/MainFrame.hpp"
#include "ui/WizardPageHost.hpp"
#include "classic/ClassicModeStartPage.hpp"
#include "ui/InputFilesPage.hpp"
#include "ui/InputCDPage.hpp"
#include "preset/PresetManagerImpl.h"
#include "encoder/ModuleManagerImpl.hpp"
#include "encoder/LameNogapInstanceManager.hpp"
#include "TaskManager.hpp"
#include <ulib/CrashReporter.hpp>
#include "CrashSaveResultsDlg.hpp"
#include <boost/ref.hpp>
#include <ulib/win32/VersionInfoResource.hpp>
#include <ulib/CommandLineParser.hpp>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

// globals

/// application module
CAppModule _Module;

/// app instance pointer
App* App::s_pApp = NULL;


// App methods

App::App(HINSTANCE hInstance)
:m_langResourceManager(_T("winlame.*.dll"), IDS_LANG_ENGLISH, IDS_LANG_NATIVE),
m_alreadyReadCommandLine(false),
m_helpAvailable(false),
m_exit(false),
m_startInputCD(false)
{
   s_pApp = this;

   // remove current directory from search path of LoadLibrary(); see also
   // Microsoft Security Advisory (2269637):
   // http://www.microsoft.com/technet/security/advisory/2269637.mspx
   BOOL ret = SetDllDirectory(_T(""));
   ATLASSERT(ret == TRUE);
   UNUSED(ret);

#ifdef _DEBUG
   // turn on leak-checking
   int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
   _CrtSetDbgFlag(flag | _CRTDBG_LEAK_CHECK_DF);
#endif

   HRESULT hRes = ::CoInitialize(NULL);
   ATLASSERT(SUCCEEDED(hRes));

   AtlInitCommonControls(ICC_WIN95_CLASSES);

   hRes = _Module.Init(NULL, hInstance, &LIBID_ATLLib);
   ATLASSERT(SUCCEEDED(hRes));

#ifdef _DEBUG
   AtlAxWinInit();
#endif

   // read settings from registry
   m_settings.ReadSettings();

   m_spTaskManager.reset(new TaskManager(m_settings.m_taskManagerConfig));

   // register objects in IoC container
   IoCContainer& ioc = IoCContainer::Current();

   ioc.Register<LanguageResourceManager>(boost::ref(m_langResourceManager));
   ioc.Register<TaskManager>(boost::ref(*m_spTaskManager.get()));
   ioc.Register<UISettings>(boost::ref(m_settings));

   m_spLameNogapInstanceManager.reset(new Encoder::LameNogapInstanceManager);
   ioc.Register<Encoder::LameNogapInstanceManager>(boost::ref(*m_spLameNogapInstanceManager.get()));

   m_spPresetManager.reset(new PresetManagerImpl);
   ioc.Register<PresetManagerInterface>(boost::ref(*m_spPresetManager.get()));

   m_spModuleManager.reset(new Encoder::ModuleManagerImpl);
   ioc.Register<Encoder::ModuleManager>(boost::ref(*m_spModuleManager.get()));

   LoadPresetFile();

   // set language to use
   if (m_langResourceManager.IsLangResourceAvail(m_settings.language_id))
      m_langResourceManager.LoadLangResource(m_settings.language_id);

   // check if html help file is available
   m_helpFilename = Path::Combine(App::AppFolder(), "winLAME.chm").ToString();

   m_helpAvailable = Path(m_helpFilename).FileExists();
}

App::~App()
{
   // store settings in the registry
   try
   {
      m_settings.StoreSettings();
   }
   catch (...)
   {
      // ignore errors when storing settings
   }

   s_pApp = NULL;

   _Module.Term();
   ::CoUninitialize();
}

void App::InitCrashReporter()
{
   // local app-data, non-roaming
   CString folder = Path::Combine(Path::SpecialFolder(CSIDL_LOCAL_APPDATA), _T("winLAME")).ToString();

   if (!Path(folder).FolderExists())
      CreateDirectory(folder, nullptr);

   folder = Path::Combine(folder, _T("crashdumps")).ToString();

   if (!Path(folder).FolderExists())
      CreateDirectory(folder, nullptr);

   CrashReporter::Init(_T("winLAME"), folder, &App::ShowCrashErrorDialog);
}

void App::ShowCrashErrorDialog(LPCTSTR crashDumpFilename)
{
   std::vector<CString> resultFilenamesList;
   resultFilenamesList.push_back(crashDumpFilename);

   CrashSaveResultsDlg dlg(resultFilenamesList);
   dlg.DoModal();
}

int App::Run(LPTSTR lpstrCmdLine, int nCmdShow)
{
   m_startInputCD = CString(lpstrCmdLine).Find(_T("--input-cd")) != -1;
   if (m_startInputCD)
      m_alreadyReadCommandLine = true;

   int ret = 0;

   m_exit = false;

   while (!m_exit)
   {
      switch (m_settings.m_appMode)
      {
      case UISettings::classicMode:
         RunClassicDialog();
         break;

      case UISettings::modernMode:
         ret = RunMainFrame(nCmdShow);
         break;

      default:
         ATLASSERT(false);
         m_exit = true;
         break;
      }
   }

   return ret;
}

void App::RunClassicDialog()
{
   UI::WizardPageHost host(true);

   std::shared_ptr<UI::WizardPage> wizardPage = GetClassicModeStartWizardPage(host);

   host.SetWizardPage(wizardPage);
   host.Run(nullptr);

   if (!host.IsAppModeChanged())
      m_exit = true;
}

std::shared_ptr<UI::WizardPage> App::GetClassicModeStartWizardPage(UI::WizardPageHost& host)
{
   std::shared_ptr<UI::WizardPage> wizardPage;

   if (StartInputCD())
   {
      ResetStartInputCD();

      wizardPage = std::make_shared<UI::InputCDPage>(host);
   }

   if (!AlreadyReadCommandLine())
   {
      SetAlreadyReadCommandLine();

      // collect file names from the command line
      std::vector<CString> filenames;

      CommandLineParser parser(::GetCommandLine());

      // skip first string; it's the program's name
      CString param;
      parser.GetNext(param);

      while (parser.GetNext(param))
      {
         filenames.push_back(param);
      }

      // show input files page
      if (!filenames.empty())
      {
         wizardPage = std::make_shared<UI::InputFilesPage>(host, filenames);
      }
   }

   if (wizardPage == nullptr)
      wizardPage = std::make_shared<UI::ClassicModeStartPage>(host);

   return wizardPage;
}

int App::RunMainFrame(int nCmdShow)
{
   CMessageLoop theLoop;
   _Module.AddMessageLoop(&theLoop);

   UI::MainFrame wndMain(*m_spTaskManager.get());

   if (wndMain.CreateEx() == NULL)
   {
      ATLTRACE(_T("Main window creation failed!\n"));
      return 0;
   }

   wndMain.ShowWindow(nCmdShow);

   int nRet = theLoop.Run();

   _Module.RemoveMessageLoop();

   if (!wndMain.IsAppModeChanged())
      m_exit = true;

   return nRet;
}

CString App::AppDataFolder(bool machineWide)
{
   // CSIDL_APPDATA - user-dependent app data folder
   // CSIDL_COMMON_APPDATA - machine-wide app data folder
   CString appDataPath =
      Path::SpecialFolder(machineWide ? CSIDL_COMMON_APPDATA : CSIDL_APPDATA);

   return Path::Combine(appDataPath, _T("winLAME")).ToString();
}

CString App::AppFolder()
{
   CString moduleFilename = Path::ModuleFilename();

   return Path(moduleFilename).FolderName();
}

CString App::Version()
{
   Win32::VersionInfoResource versionInfo(Path::ModuleFilename());

   // retrieve version language
   std::vector<Win32::LANGANDCODEPAGE> langAndCodePagesList;
   versionInfo.GetLangAndCodepages(langAndCodePagesList);

   if (langAndCodePagesList.empty())
      return _T("???");

   CString fileVersion = versionInfo.GetStringValue(langAndCodePagesList[0], _T("FileVersion"));

   return fileVersion;
}

void App::LoadPresetFile()
{
   CString userSpecificAppFolder = AppDataFolder(false);
   CString machineWideAppFolder = AppDataFolder(true);

   // first, check if user has left a presets.xml around in the .exe folder
   CString presetFilename = Path::Combine(AppFolder(), _T("presets.xml")).ToString();

   if (!Path(presetFilename).FileExists() &&
      !IsRunningAsUwpApp())
   {
      // next try to check for user-dependend config file
      presetFilename = Path::Combine(userSpecificAppFolder, _T("presets.xml")).ToString();
      if (!Path(presetFilename).FileExists())
      {
         // not available: try to use machine-wide config file
         presetFilename = Path::Combine(machineWideAppFolder, _T("presets.xml")).ToString();
      }
   }

   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
   settings.presets_filename = presetFilename;

   PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

   if (Path(presetFilename).FileExists())
      settings.preset_avail = presetManager.loadPreset(presetFilename);
   else
      settings.preset_avail = false;
}

/// returns if this application is running as UWP app, using the kernel32.dll function
/// GetCurrentPackageFullName().
/// \see https://blogs.msdn.microsoft.com/appconsult/2016/11/03/desktop-bridge-identify-the-applications-context/
bool App::IsRunningAsUwpApp()
{
   HMODULE module = GetModuleHandle(_T("kernel32.dll"));

   typedef
      LONG(WINAPI *T_fnGetCurrentPackageFullName)
      (UINT32* packageFullNameLength, PWSTR packageFullName);

   auto fnGetCurrentPackageFullName =
      (T_fnGetCurrentPackageFullName)GetProcAddress(module, "GetCurrentPackageFullName");

   if (fnGetCurrentPackageFullName == nullptr)
      return false; // no exported function; Windows 7 or earlier, or on Wine

   UINT32 length = 0;
   LONG rc = fnGetCurrentPackageFullName(&length, nullptr);

   if (rc == APPMODEL_ERROR_NO_PACKAGE ||
       rc != ERROR_INSUFFICIENT_BUFFER)
      return false; // not running as UWP app

   CStringW packageName;
   rc = fnGetCurrentPackageFullName(&length, packageName.GetBuffer(length));
   packageName.ReleaseBuffer();

   if (rc != ERROR_SUCCESS)
      return false; // error retrieving actual package name

   ATLTRACE(_T("UWP package name: %ls"), packageName.GetString());

   return true;
}
