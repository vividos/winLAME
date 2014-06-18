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
/// \file OutputSettingsPage.hpp
/// \brief Output settings page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "BevelLine.hpp"
#include "resource.h"

// forward references
struct UISettings;

namespace UI
{

/// \brief Output settings page
class OutputSettingsPage:
   public WizardPage,
   public CWinDataExchange<OutputSettingsPage>,
   public CDialogResize<OutputSettingsPage>
{
public:
   /// ctor
   OutputSettingsPage(WizardPageHost& pageHost) throw();
   /// dtor
   ~OutputSettingsPage() throw()
   {
   }

   /// sets new wizard page by given output module id
   static void SetWizardPageByOutputModule(WizardPageHost& pageHost, int modid);

private:
   friend CDialogResize<OutputSettingsPage>;

   BEGIN_DDX_MAP(OutputSettingsPage)
      DDX_CONTROL(IDC_OUT_BEVEL1, m_bevel1)
      DDX_CONTROL(IDC_OUT_BEVEL2, m_bevel2)
      DDX_CONTROL(IDC_OUT_BEVEL3, m_bevel3)
      DDX_CONTROL_HANDLE(IDC_OUT_COMBO_OUTMODULE, m_cbOutputModule)
      DDX_CONTROL_HANDLE(IDC_OUT_USE_INDIR, m_checkUseInputDir)
      DDX_CONTROL_HANDLE(IDC_OUT_OUTPATH, m_cbOutputPath)
      DDX_CONTROL_HANDLE(IDC_OUT_DELAFTER, m_checkDeleteAfter)
      DDX_CONTROL_HANDLE(IDC_OUT_CHECK_OVERWRITE, m_checkOverwrite)
      DDX_CONTROL_HANDLE(IDC_OUT_CREATEPLAYLIST, m_checkCreatePlaylist)
      DDX_CONTROL_HANDLE(IDC_OUT_PLAYLISTNAME, m_ecPlaylistName)
      DDX_CONTROL_HANDLE(IDC_OUT_SELECTPATH, m_btnSelectPath)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(OutputSettingsPage)
      DLGRESIZE_CONTROL(IDC_OUT_BEVEL1, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_OUT_BEVEL2, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_OUT_BEVEL3, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_OUT_COMBO_OUTMODULE, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_OUT_OUTPATH, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_OUT_SELECTPATH, DLSZ_MOVE_X)
      DLGRESIZE_CONTROL(IDC_OUT_PLAYLISTNAME, DLSZ_SIZE_X)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(OutputSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
      COMMAND_HANDLER(IDC_OUT_SELECTPATH, BN_CLICKED, OnButtonSelectOutputPath)
      COMMAND_HANDLER(IDC_OUT_CREATEPLAYLIST, BN_CLICKED, OnCheckCreatePlaylist)
      COMMAND_HANDLER(IDC_OUT_USE_INDIR, BN_CLICKED, OnCheckUseInputFolder)
      COMMAND_HANDLER(IDC_OUT_OUTPATH, CBN_SELENDOK, OnOutPathSelEndOk)
      CHAIN_MSG_MAP(CDialogResize<OutputSettingsPage>)
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

   /// called when the user clicks on the button to select the output path
   LRESULT OnButtonSelectOutputPath(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when the state of the "create playlist" check changes
   LRESULT OnCheckCreatePlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when check "use input dir as output location" is clicked
   LRESULT OnCheckUseInputFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when output path combo box selection ends
   LRESULT OnOutPathSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// sets up output modules list
   void SetupOutputModulesList();

   /// loads settings data into controls
   void LoadData();

   /// saves settings data from controls
   bool SaveData(bool bSilent);

   /// refreshes output dir history combobox
   void RefreshHistory();

   /// sets new page depending on selected module
   void SetWizardPage();

private:
   // controls

   BevelLine m_bevel1; ///< bevel line
   BevelLine m_bevel2; ///< bevel line
   BevelLine m_bevel3; ///< bevel line

   /// output module combobox
   CComboBox m_cbOutputModule;

   CComboBox m_cbOutputPath;     ///< output path combobox
   CButton m_checkUseInputDir;   ///< "use input dir" checkbox
   CButton m_checkDeleteAfter;   ///< "delete after encoding" checkbox
   CButton m_checkOverwrite;     ///< "overwrite" checkbox
   CButton m_checkCreatePlaylist;///< "create playlist" checkbox
   CEdit m_ecPlaylistName;       ///< playlist filename
   CButton m_btnSelectPath;      ///< "select path" button

   /// icon list for image buttons
   CImageList m_ilIcons;

   // model

   /// settings
   UISettings& m_uiSettings;
};

} // namespace UI
