//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2018 Michael Fink
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
/// \file ClassicModeStartPage.cpp
/// \brief Start page for Classic UI
//
#include "StdAfx.h"
#include "ClassicModeStartPage.hpp"
#include "WizardPageHost.hpp"
#include "InputFilesPage.hpp"
#include "InputCDPage.hpp"

using namespace UI;

LRESULT ClassicModeStartPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   return 1;
}

LRESULT ClassicModeStartPage::OnButtonOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (m_encodeFiles)
      m_pageHost.SetWizardPage(std::make_shared<InputFilesPage>(m_pageHost, std::vector<CString>()));
   else
      m_pageHost.SetWizardPage(std::make_shared<InputCDPage>(m_pageHost));

   return 0;
}

LRESULT ClassicModeStartPage::OnButtonInputFilesReadCD(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_encodeFiles = wID != IDC_START_BUTTON_INPUT_CD;

   GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED));

   return 0;
}
