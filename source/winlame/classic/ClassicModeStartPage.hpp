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
/// \file ClassicModeStartPage.hpp
/// \brief Start page for Classic UI
//
#pragma once

#include "WizardPage.hpp"
#include "resource.h"

namespace UI
{
   /// \brief Start page for classic mode
   /// Lets the user choose between file input and CD input
   class ClassicModeStartPage :
      public WizardPage,
      public CWinDataExchange<ClassicModeStartPage>,
      public CDialogResize<ClassicModeStartPage>
   {
   public:
      /// ctor
      explicit ClassicModeStartPage(WizardPageHost& pageHost)
         :WizardPage(pageHost, IDD_PAGE_CLASSIC_START, WizardPage::typeCancelNext),
         m_encodeFiles(true)
      {
      }
      /// dtor
      ~ClassicModeStartPage()
      {
      }

   private:
      friend CDialogResize<ClassicModeStartPage>;

      BEGIN_DDX_MAP(ClassicModeStartPage)
         DDX_CONTROL_HANDLE(IDC_START_BUTTON_INPUT_FILES, m_inputFilesButton)
         DDX_CONTROL_HANDLE(IDC_START_BUTTON_INPUT_CD, m_inputCDButton)
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(ClassicModeStartPage)
         DLGRESIZE_CONTROL(IDC_START_BUTTON_INPUT_FILES, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_START_BUTTON_INPUT_CD, DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(ClassicModeStartPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
         COMMAND_HANDLER(IDC_START_BUTTON_INPUT_FILES, BN_CLICKED, OnButtonInputFilesReadCD)
         COMMAND_HANDLER(IDC_START_BUTTON_INPUT_CD, BN_CLICKED, OnButtonInputFilesReadCD)
         CHAIN_MSG_MAP(CDialogResize<ClassicModeStartPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

      /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when page is left with "Next" button
      LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when page is left with "Input files" or "Read CD" button
      LRESULT OnButtonInputFilesReadCD(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   private:
      // model

      /// indicates that files should be encoded
      bool m_encodeFiles;

      // controls

      /// button to select "Input Files" option
      CButton m_inputFilesButton;

      /// button to select "Read CD" option
      CButton m_inputCDButton;
   };

} // namespace UI
