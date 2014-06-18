//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2014 Michael Fink
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
/// \file FinishPage.hpp
/// \brief Finish page
//
#pragma once

// includes
#include "WizardPage.h"
#include "resource.h"

// forward references
struct UISettings;

namespace UI
{

/// \brief Finish page
class FinishPage:
   public WizardPage,
   public CWinDataExchange<FinishPage>,
   public CDialogResize<FinishPage>
{
public:
   /// ctor
   FinishPage(WizardPageHost& pageHost) throw();
   /// dtor
   ~FinishPage() throw()
   {
   }

private:
   friend CDialogResize<FinishPage>;

   BEGIN_DDX_MAP(FinishPage)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(FinishPage)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(FinishPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
      CHAIN_MSG_MAP(CDialogResize<FinishPage>)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

   // Handler prototypes:
   // LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   // LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   // LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is left with Next button
   LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when page is left with Back button
   LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
   // controls

   // model

   /// settings
   UISettings& m_uiSettings;
};

} // namespace UI
