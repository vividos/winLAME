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
/// \file PresetSelectionPage.hpp
/// \brief Preset selection page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "resource.h"
#include "CommonStuff.hpp"
#include "PresetManagerInterface.hpp"

// forward references
struct UISettings;

namespace UI
{

/// \brief Preset selection page
class PresetSelectionPage:
   public WizardPage,
   public CWinDataExchange<PresetSelectionPage>,
   public CDialogResize<PresetSelectionPage>
{
public:
   /// ctor
   PresetSelectionPage(WizardPageHost& pageHost) throw()
      :WizardPage(pageHost, IDD_PAGE_PRESET_SELECTION, WizardPage::typeCancelBackNext),
      m_uiSettings(IoCContainer::Current().Resolve<UISettings>()),
      m_presetManager(IoCContainer::Current().Resolve<PresetManagerInterface>())
   {
   }
   /// dtor
   ~PresetSelectionPage() throw()
   {
   }

private:
   friend CDialogResize<PresetSelectionPage>;

   BEGIN_DDX_MAP(PresetSelectionPage)
      DDX_CONTROL(IDC_PRE_BEVEL1, m_bevel1)
      DDX_CONTROL_HANDLE(IDC_PRE_LIST_PRESET, m_lbPresets)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(PresetSelectionPage)
      DLGRESIZE_CONTROL(IDC_PRE_BEVEL1, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_PRE_LIST_PRESET, DLSZ_SIZE_X | DLSZ_SIZE_Y)
      DLGRESIZE_CONTROL(IDC_PRE_DESC, DLSZ_MOVE_Y | DLSZ_SIZE_X)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(PresetSelectionPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
      COMMAND_HANDLER(IDC_PRE_LIST_PRESET, LBN_SELCHANGE, OnSelItemChanged)
      COMMAND_HANDLER(IDC_PRE_LIST_PRESET, LBN_DBLCLK, OnLButtonDblClk)
      CHAIN_MSG_MAP(CDialogResize<PresetSelectionPage>)
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

   /// called when a presets listbox item selection changes
   LRESULT OnSelItemChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when the user double-clicks on a preset item
   LRESULT OnLButtonDblClk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// loads settings data into controls
   void LoadData();

   /// saves settings data from controls
   void SaveData();

private:
   // controls

   /// bevel lines
   BevelLine m_bevel1;

   /// presets list
   CListBox m_lbPresets;

   // model

   /// settings
   UISettings& m_uiSettings;

   /// preset manager
   PresetManagerInterface& m_presetManager;
};

} // namespace UI
