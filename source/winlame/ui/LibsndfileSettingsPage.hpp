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
/// \file LibsndfileSettingsPage.hpp
/// \brief Libsndfile settings page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "resource.h"
#include "CommonStuff.h"

// forward references
struct UISettings;

namespace UI
{

/// \brief Libsndfile settings page
class LibsndfileSettingsPage:
   public WizardPage,
   public CWinDataExchange<LibsndfileSettingsPage>,
   public CDialogResize<LibsndfileSettingsPage>
{
public:
   /// ctor
   LibsndfileSettingsPage(WizardPageHost& pageHost) throw()
      :WizardPage(pageHost, IDD_PAGE_LIBSNDFILE_SETTINGS, WizardPage::typeCancelBackNext),
      m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
   {
   }
   /// dtor
   ~LibsndfileSettingsPage() throw()
   {
   }

private:
   friend CDialogResize<LibsndfileSettingsPage>;

   BEGIN_DDX_MAP(LibsndfileSettingsPage)
      DDX_CONTROL(IDC_WAVE_BEVEL1, m_bevel1);
      DDX_CONTROL_HANDLE(IDC_WAVE_COMBO_OUTFMT, m_cbOutputFormat);
      DDX_CONTROL_HANDLE(IDC_WAVE_COMBO_FILEFMT, m_cbFileFormat);
      DDX_CONTROL_HANDLE(IDC_WAVE_CHECK_RAWAUDIO, m_checkRawAudio)
      DDX_CONTROL_HANDLE(IDC_WAVE_CHECK_WAVEX, m_checkWavex)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(LibsndfileSettingsPage)
      DLGRESIZE_CONTROL(IDC_WAVE_BEVEL1, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_WAVE_COMBO_OUTFMT, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_WAVE_COMBO_FILEFMT, DLSZ_SIZE_X)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(LibsndfileSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
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

   /// called when page is left with Cancel button
   LRESULT OnButtonCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when page is left with Back button
   LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// loads settings data into controls
   void LoadData();

   /// saves settings data from controls
   void SaveData();

private:
   // controls

   /// output format combobox
   CComboBox m_cbOutputFormat;

   /// file format combobox
   CComboBox m_cbFileFormat;

   /// Raw Audio checkbox
   CButton m_checkRawAudio;

   /// WAVEX checkbox
   CButton m_checkWavex;

   /// bevel line
   BevelLine m_bevel1;

   // model

   /// settings
   UISettings& m_uiSettings;
};

} // namespace UI
