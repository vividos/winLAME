//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2004 Michael Fink
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
/// \file PresetsPage.hpp
/// \brief contains the presets page
//
#pragma once

#include "resource.h"
#include "PageBase.hpp"
#include "CommonStuff.hpp"
#include "PresetManagerInterface.hpp"

namespace ClassicUI
{
   /// presets selection page
   class PresetsPage :
      public PageBase,
      public CDialogResize<PresetsPage>
   {
   public:
      /// ctor
      PresetsPage()
         :m_presetManager(IoCContainer::Current().Resolve<PresetManagerInterface>())
      {
         IDD = IDD_DLG_PRESETS;
         captionID = IDS_DLG_CAP_PRESETS;
         descID = IDS_DLG_DESC_PRESETS;
         helpID = IDS_HTML_PRESETS;
      }

      // resize map
      BEGIN_DLGRESIZE_MAP(PresetsPage)
         DLGRESIZE_CONTROL(IDC_PRE_BEVEL1, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_PRE_LIST_PRESET, DLSZ_SIZE_X | DLSZ_SIZE_Y)
         DLGRESIZE_CONTROL(IDC_PRE_DESC, DLSZ_MOVE_Y | DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      // message map
      BEGIN_MSG_MAP(PresetsPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDC_PRE_LIST_PRESET, LBN_SELCHANGE, OnSelItemChanged)
         COMMAND_HANDLER(IDC_PRE_LIST_PRESET, LBN_DBLCLK, OnLButtonDblClk)
         CHAIN_MSG_MAP(CDialogResize<PresetsPage>)
      END_MSG_MAP()
      // Handler prototypes:
      //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
      //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

         /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      /// called when a presets listbox item selection changes
      LRESULT OnSelItemChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
      /// called when the user double-clicks on a preset item
      LRESULT OnLButtonDblClk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      // virtual functions from PageBase

      // called on entering the page
      virtual void OnEnterPage();

      // called on leaving the page
      virtual bool OnLeavePage();

   protected:
      /// bevel lines
      BevelLine bevel1;

      /// last selected index
      static int lastindex;

   private:
      /// preset manager
      PresetManagerInterface& m_presetManager;
   };

} // namespace ClassicUI
