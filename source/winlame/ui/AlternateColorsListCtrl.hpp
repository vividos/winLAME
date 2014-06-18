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
/// \file FixedValueSpinButtonCtrl.hpp
/// \brief Fixed value spin button control
//
#pragma once

namespace UI
{
   class AlternateColorsListCtrl :
      public CWindowImpl<AlternateColorsListCtrl, CListViewCtrl>,
      public CCustomDraw<AlternateColorsListCtrl>
   {
   public:
      AlternateColorsListCtrl() {}

   private:
      BEGIN_MSG_MAP(AlternateColorsListCtrl)
         CHAIN_MSG_MAP_ALT(CCustomDraw<AlternateColorsListCtrl>, 1)
      END_MSG_MAP()

      DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
      {
         return CDRF_NOTIFYITEMDRAW;
      }

      DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
      {
         LPNMLVCUSTOMDRAW lpNMLVCustomDraw = reinterpret_cast<LPNMLVCUSTOMDRAW>(lpNMCustomDraw);
         DWORD nItem = lpNMCustomDraw->dwItemSpec;

         extern COLORREF g_clrAlternateListColor;

         // color every other line
         if ((nItem & 1) && nItem < static_cast<DWORD>(GetItemCount()))
            lpNMLVCustomDraw->clrTextBk = g_clrAlternateListColor;

         return CDRF_DODEFAULT;
      }
   };

} // namespace UI
