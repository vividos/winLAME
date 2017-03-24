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
   /// wrapper for a spin button control that "snaps" to fixed values when the
   /// up or down button is pressed
   class FixedValueSpinButtonCtrl : public CWindowImpl<FixedValueSpinButtonCtrl>
   {
   public:
      /// ctor
      FixedValueSpinButtonCtrl() {}

      BEGIN_MSG_MAP(FixedValueSpinButtonCtrl)
         MESSAGE_HANDLER(OCM_NOTIFY, OnReflectedNotify) // reflected notifications
      END_MSG_MAP()

      /// called when a notification was reflected back to the control
      LRESULT OnReflectedNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);

      /// sets the fixed integer values to use
      void SetFixedValues(const int* pvalues, int size);

      /// finds nearest value of the value passed
      int FindNearest(int value);

      /// sets the buddy window for a spin button control
      HWND SetBuddy(HWND hWnd)
      {
         return (HWND)::SendMessage(m_hWnd, UDM_SETBUDDY, (WPARAM)hWnd, 0L);
      }

      /// sets the current position for a spin button control
      int SetPos(int nPos)
      {
         return (int)(short)LOWORD(::SendMessage(m_hWnd, UDM_SETPOS, 0, MAKELPARAM(nPos, 0)));
      }

   private:
      /// vector containing all possible values
      std::vector<int> m_vecValues;
   };

} // namespace UI
