/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2017 Michael Fink

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
/// \file winlame.cpp
/// \brief contains the main function which just pops up the MainDlg

// needed includes
#include "stdafx.h"
#include "App.hpp"

/// win main function
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
   LPTSTR lpCmdLine, int nCmdShow)
{
   try
   {
      App::InitCrashReporter();

      App app(hInstance);
      return app.Run(lpCmdLine, nCmdShow);
   }
   catch (...)
   {
      ATLTRACE(_T("Exception while running winLAME\n"));
      return -1;
   }
}
