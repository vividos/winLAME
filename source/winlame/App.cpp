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
/// \file App.cpp
/// \brief App class

#include "StdAfx.h"
#include "App.h"
#include "MainDlg.h"
#include "ui\MainFrame.hpp"
#include "preset\PresetManagerImpl.h"
#include "encoder\ModuleManagerImpl.hpp"
#include "CommonStuff.h"

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
m_exit(false)
{
   s_pApp = this;

   // remove current directory from search path of LoadLibrary(); see also
   // Microsoft Security Advisory (2269637):
   // http://www.microsoft.com/technet/security/advisory/2269637.mspx
   BOOL ret = SetDllDirectory(_T(""));
   ATLASSERT(ret == TRUE); ret;

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

   // register objects in IoC container
   IoCContainer& ioc = IoCContainer::Current();

   ioc.Register<LanguageResourceManager>(boost::ref(m_langResourceManager));
   ioc.Register<TaskManager>(boost::ref(m_taskManager));
   ioc.Register<UISettings>(boost::ref(m_settings));

   m_spPresetManager.reset(new PresetManagerImpl);
   ioc.Register<PresetManagerInterface>(boost::ref(*m_spPresetManager.get()));

   m_spModuleManager.reset(new Encoder::ModuleManagerImpl);
   ioc.Register<Encoder::ModuleManager>(boost::ref(*m_spModuleManager.get()));

   LoadPresetFile();

   // read settings from registry
   m_settings.ReadSettings();

   // set language to use
   if (m_langResourceManager.IsLangResourceAvail(m_settings.language_id))
      m_langResourceManager.LoadLangResource(m_settings.language_id);
}

App::~App()
{
   // store settings in the registry
   m_settings.StoreSettings();

   s_pApp = NULL;

   _Module.Term();
   ::CoUninitialize();
}

int App::Run(LPTSTR /*lpstrCmdLine*/, int nCmdShow)
{
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
   // start dialog
   MainDlg dlg(m_settings, m_langResourceManager);
   dlg.RunDialog();

   if (!dlg.IsAppModeChanged())
      m_exit = true;
}

int App::RunMainFrame(int nCmdShow)
{
   CMessageLoop theLoop;
   _Module.AddMessageLoop(&theLoop);

   UI::MainFrame wndMain(m_taskManager);

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

CString App::AppDataFolder(bool bMachineWide)
{
   CString cszAppData;

   // CSIDL_APPDATA - user-dependent app data folder
   // CSIDL_COMMON_APPDATA - machine-wide app data folder
   SHGetFolderPath(NULL,
      bMachineWide ? CSIDL_COMMON_APPDATA : CSIDL_APPDATA,
      NULL, SHGFP_TYPE_CURRENT,
      cszAppData.GetBuffer(MAX_PATH));
   cszAppData.ReleaseBuffer();

   cszAppData += _T("\\winLAME\\");

   return cszAppData;
}

CString App::AppFolder()
{
   CString cszModuleFilename;
   ::GetModuleFileName(NULL, cszModuleFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszModuleFilename.ReleaseBuffer();

   // remove filename
   int iPos = cszModuleFilename.ReverseFind(_T('\\'));
   ATLASSERT(iPos != -1);
   return cszModuleFilename.Left(iPos+1);
}

CString App::AppFilename()
{
   CString cszFilename;

   GetModuleFileName(NULL, cszFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszFilename.ReleaseBuffer();

   return cszFilename;
}

CString App::Version()
{
   // get exe file name
   CString cszFilename = AppFilename();

   // allocate memory for the version info struct
   DWORD nDummy=0;
   DWORD nVerInfoSize = GetFileVersionInfoSize(const_cast<LPTSTR>(static_cast<LPCTSTR>(cszFilename)), &nDummy);
   if (nVerInfoSize == 0)
      return _T("???");

   std::vector<BYTE> vecVerInfo(nVerInfoSize);

   if (0 == GetFileVersionInfo(const_cast<LPTSTR>(static_cast<LPCTSTR>(cszFilename)), 0, nVerInfoSize, &vecVerInfo[0]))
      return _T("???");

   // retrieve version language
   LPVOID pVersion = NULL;
   UINT nVersionLen;

   BOOL bRet = VerQueryValue(&vecVerInfo[0], _T("\\VarFileInfo\\Translation"), &pVersion, &nVersionLen);
   if (!bRet)
      return _T("???");

   CString cszFileVersion;
   if (bRet && nVersionLen==4)
   {
      DWORD nLang = *(DWORD*)pVersion;

      cszFileVersion.Format(_T("\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion"),
         (nLang & 0xff00)>>8, nLang & 0xff, (nLang & 0xff000000)>>24, (nLang & 0xff0000)>>16);
   }
   else
      cszFileVersion.Format(_T("\\StringFileInfo\\%04X04B0\\FileVersion"),GetUserDefaultLangID());

   CString cszVersion;
   bRet = VerQueryValue(&vecVerInfo[0], const_cast<LPTSTR>(static_cast<LPCTSTR>(cszFileVersion)), &pVersion, &nVersionLen);
   if (bRet)
      cszVersion = (LPTSTR)pVersion;

   return cszVersion;
}

void App::LoadPresetFile()
{
   // CSIDL_APPDATA - user-dependent app data folder
   // CSIDL_COMMON_APPDATA - machine-wide app data folder
   CString userSpecificAppFolder = Path::SpecialFolder(CSIDL_APPDATA);
   CString machineWideAppFolder = Path::SpecialFolder(CSIDL_COMMON_APPDATA);

   userSpecificAppFolder = Path::Combine(userSpecificAppFolder, _T("winLAME")).ToString();
   machineWideAppFolder = Path::Combine(machineWideAppFolder, _T("winLAME")).ToString();

   // first, check if user has left a presets.xml around in the .exe folder
   CString presetFilename = Path::Combine(AppFolder(), _T("presets.xml"));

   if (!Path(presetFilename).FileExists())
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
