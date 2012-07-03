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
#include "ui\MainFrame.h"

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
:m_langResourceManager(_T("winlame.*.dll"), IDS_LANG_ENGLISH, IDS_LANG_NATIVE)
{
   s_pApp = this;

   // remove current directory from search path of LoadLibrary(); see also
   // Microsoft Security Advisory (2269637):
   // http://www.microsoft.com/technet/security/advisory/2269637.mspx
   ATLVERIFY(TRUE == SetDllDirectory(_T("")));

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

   // set language to use
   if (m_langResourceManager.IsLangResourceAvail(m_settings.language_id))
      m_langResourceManager.LoadLangResource(m_settings.language_id);

   // create new preset manager
   m_scpPresetManager.reset(PresetManagerInterface::getPresetManager());
   m_settings.preset_manager = m_scpPresetManager.get();

   // get a module manager
   m_scpModuleManager.reset(ModuleManager::getNewModuleManager());
   m_settings.module_manager = m_scpModuleManager.get();
}

App::~App()
{
   // store settings in the registry
   m_settings.StoreSettings();

   // save preset file
   m_scpPresetManager->savePreset();

   s_pApp = NULL;

   _Module.Term();
   ::CoUninitialize();
}

int App::Run(LPTSTR /*lpstrCmdLine*/, int nCmdShow)
{
   // start dialog
   MainDlg dlg;
   dlg.RunDialog();

   return 0;
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
