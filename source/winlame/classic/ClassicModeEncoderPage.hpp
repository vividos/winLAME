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
/// \file ClassicModeEncoderPage.hpp
/// \brief Start page for Classic UI
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "resource.h"

namespace UI
{
   /// \brief Encoder page for classic mode
   /// Lets the user encode the files or CD tracks that were selected
   class ClassicModeEncoderPage :
      public WizardPage,
      public CWinDataExchange<ClassicModeEncoderPage>,
      public CDialogResize<ClassicModeEncoderPage>
   {
   public:
      /// ctor
      ClassicModeEncoderPage(WizardPageHost& pageHost)
         :WizardPage(pageHost, IDD_PAGE_CLASSIC_ENCODE, WizardPage::typeCancelBackFinish)
      {
      }
      /// dtor
      ~ClassicModeEncoderPage()
      {
      }

   private:
      friend CDialogResize<ClassicModeEncoderPage>;

      BEGIN_DDX_MAP(ClassicModeEncoderPage)
      END_DDX_MAP()

      BEGIN_DLGRESIZE_MAP(ClassicModeEncoderPage)
      END_DLGRESIZE_MAP()

      BEGIN_MSG_MAP(ClassicModeEncoderPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
         CHAIN_MSG_MAP(CDialogResize<ClassicModeEncoderPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()

      /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when page is left with "Finish" button
      LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   private:
      // model

      // controls

   };

} // namespace UI
