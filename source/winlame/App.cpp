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
}

App::~App()
{
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
