//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2013 Michael Fink
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
/// \file LibsndfileSettingsPage.hpp
/// \brief Input CD page

// include guard
#pragma once

// includes
#include "WizardPage.h"
#include "resource.h"

// forward references

/// \brief input files page
/// \details shows all files opened/dropped, checks them for audio infos and errors
/// and displays them; processing is done in separate thread.
class LibsndfileSettingsPage:
   public WizardPage,
   public CWinDataExchange<LibsndfileSettingsPage>,
   public CDialogResize<LibsndfileSettingsPage>
{
public:
   /// ctor
   LibsndfileSettingsPage(WizardPageHost& pageHost) throw()
      :WizardPage(pageHost, IDD_PAGE_LIBSNDFILE_SETTINGS, WizardPage::typeCancelNext)
   {
   }
   /// dtor
   ~LibsndfileSettingsPage() throw()
   {
   }

private:
   friend CDialogResize<LibsndfileSettingsPage>;

   BEGIN_DDX_MAP(LibsndfileSettingsPage)
   END_DDX_MAP()

   // resize map
   BEGIN_DLGRESIZE_MAP(LibsndfileSettingsPage)
   END_DLGRESIZE_MAP()

   // message map
   BEGIN_MSG_MAP(LibsndfileSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      CHAIN_MSG_MAP(CDialogResize<LibsndfileSettingsPage>)
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

private:
   // controls
};