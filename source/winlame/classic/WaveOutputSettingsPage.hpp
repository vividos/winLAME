//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file WaveOutputSettingsPage.hpp
/// \brief contains the settings page for the wave output module
//
#pragma once

#include "resource.h"
#include "PageBase.hpp"
#include "CommonStuff.hpp"

namespace ClassicUI
{
   /// wave output settings page class
   class WaveOutputSettingsPage :
      public PageBase,
      public CDialogResize<WaveOutputSettingsPage>
   {
   public:
      /// ctor
      WaveOutputSettingsPage()
      {
         IDD = IDD_DLG_WAVE;
         captionID = IDS_DLG_CAP_WAVE;
         descID = IDS_DLG_DESC_WAVE;
         helpID = IDS_HTML_WAVE;
      }

   private:
      friend CDialogResize<WaveOutputSettingsPage>;

      BEGIN_DDX_MAP(WaveOutputSettingsPage)
         DDX_CONTROL(IDC_WAVE_BEVEL1, m_bevel1);
      DDX_CONTROL_HANDLE(IDC_WAVE_COMBO_FORMAT, m_cbFormat);
      DDX_CONTROL_HANDLE(IDC_WAVE_COMBO_SUBTYPE, m_cbSubType);
      END_DDX_MAP()

      // resize map
      BEGIN_DLGRESIZE_MAP(WaveOutputSettingsPage)
         DLGRESIZE_CONTROL(IDC_WAVE_COMBO_FORMAT, DLSZ_SIZE_X)
         DLGRESIZE_CONTROL(IDC_WAVE_COMBO_SUBTYPE, DLSZ_SIZE_X)
      END_DLGRESIZE_MAP()

      // message map
      BEGIN_MSG_MAP(WaveOutputSettingsPage)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDC_WAVE_COMBO_FORMAT, CBN_SELENDOK, OnFormatSelEndOk)
         CHAIN_MSG_MAP(CDialogResize<WaveOutputSettingsPage>)
         REFLECT_NOTIFICATIONS()
      END_MSG_MAP()
      // Handler prototypes:
      //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
      //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

         /// inits the page
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called when selection on format combobox has changed
      LRESULT OnFormatSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// updates file formats combobox list
      void UpdateFileFormatList();

      /// updates sub type combobox based on selected format
      void UpdateSubTypeCombobox();

      // virtual functions from PageBase

      // called on entering the page
      virtual void OnEnterPage();

      // called on leaving the page
      virtual bool OnLeavePage();

   private:
      // controls

      /// bevel line
      BevelLine m_bevel1;

      /// format combobox
      CComboBox m_cbFormat;

      /// file format combobox
      CComboBox m_cbSubType;
   };

} // namespace ClassicUI
