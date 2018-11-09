//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file FreeDbDiscListDlg.hpp
/// \brief FreeDB disc list dialog
//
#pragma once

#include <vector>
#include "FreedbInfo.hpp"
#include "AlternateColorsListCtrl.hpp"

namespace UI
{
   /// dialog that provides a list of found freedb disc entries
   class FreeDbDiscListDlg :
      public CDialogImpl<FreeDbDiscListDlg>,
      public CWinDataExchange<FreeDbDiscListDlg>
   {
   public:
      /// ctor
      explicit FreeDbDiscListDlg(const std::vector<FreedbInfo>& entriesList)
         :m_entriesList(entriesList),
         m_selectedItem(0)
      {
      }

      /// dialog id
      enum { IDD = IDD_FREEDB_LIST };

      BEGIN_MSG_MAP(FreeDbDiscListDlg)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_ID_HANDLER(IDOK, OnOK)
      END_MSG_MAP()

      LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
      LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

      /// returns the item that was selected by the user
      unsigned int GetSelectedItem() const { return m_selectedItem; }

      /// updates list
      void UpdateList();

   private:
      /// search results to choose from
      const std::vector<FreedbInfo>& m_entriesList;

      /// selected item
      unsigned int m_selectedItem;

      /// list of found freedb entries
      AlternateColorsListCtrl m_list;
   };

} // namespace UI
