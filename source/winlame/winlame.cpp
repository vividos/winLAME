/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2005 Michael Fink

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

   $Id: winlame.cpp,v 1.19 2010/09/06 15:45:26 vividos Exp $

*/
/*! \file winlame.cpp

   \brief contains the main function which just pops up the wlMainDlg

*/

// needed includes
#include "stdafx.h"
#include "wlMainDlg.h"

// globals and defines
#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

//! application module instance
CAppModule _Module;



//! win main function
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
   LPSTR lpCmdLine, int nCmdShow)
{
   // remove current directory from search path of LoadLibrary(); see also
   // Microsoft Security Advisory (2269637):
   // http://www.microsoft.com/technet/security/advisory/2269637.mspx
   ATLVERIFY(TRUE == SetDllDirectory(_T("")));

#ifdef _DEBUG
   int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get current flag
   flag |= _CRTDBG_LEAK_CHECK_DF; // Turn on leak-checking bit
   _CrtSetDbgFlag(flag); // Set flag to the new value
#endif

   HRESULT hRes = ::CoInitialize(NULL);
   ATLASSERT(SUCCEEDED(hRes));

   // init common controls
   AtlInitCommonControls(ICC_WIN95_CLASSES);

   // init COM module
   hRes = _Module.Init(NULL, hInstance, &LIBID_ATLLib);
   ATLASSERT(SUCCEEDED(hRes));

#ifdef _DEBUG
   AtlAxWinInit();
#endif

   // start dialog
   wlMainDlg dlg;
   dlg.RunDialog();
   
   _Module.Term();
   ::CoUninitialize();

   return 0;
}
