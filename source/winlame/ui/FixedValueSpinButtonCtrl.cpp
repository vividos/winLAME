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
/// \file FixedValueSpinButtonCtrl.cpp
/// \brief Fixed value spin button control
//
#include "stdafx.h"
#include "FixedValueSpinButtonCtrl.hpp"

LRESULT UI::FixedValueSpinButtonCtrl::OnReflectedNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
   // we got a reflected notification
   NMHDR* lpnmh = (LPNMHDR)lParam;

   switch (lpnmh->code)
   {
      // the position changed by delta
   case UDN_DELTAPOS:
      {
         NMUPDOWN* lpnmud = (LPNMUPDOWN)lpnmh;

         bool up = lpnmud->iDelta > 0;
         int value = lpnmud->iPos;
         size_t max = m_vecValues.size();
         size_t lastIndex = 0;

         // search position in the values vector
         for (size_t i = 0; i < max; i++)
            if (m_vecValues[i] <= value)
               lastIndex = i;

         // up or down?
         if (up)
            lastIndex++;
         else
            if (m_vecValues[lastIndex] == value)
               lastIndex--;

         // adjust iDelta value
         if (lastIndex < max)
         {
            value = m_vecValues[lastIndex];
            lpnmud->iDelta = value - lpnmud->iPos;
         }
      }
      break;

   default:
      bHandled = FALSE;
      break;
   }
   return 0;
}

void UI::FixedValueSpinButtonCtrl::SetFixedValues(const int *pvalues, int size)
{
   m_vecValues.assign(pvalues, pvalues + size);

   // search biggest and smallest value
   int smallest = 0, biggest = 0;

   for (int i = 0; i < size; i++)
   {
      if (m_vecValues[i] < m_vecValues[smallest]) smallest = i;
      if (m_vecValues[i] > m_vecValues[biggest]) biggest = i;
   }

   // set 32 bit range
   ::SendMessage(m_hWnd, UDM_SETRANGE32, (WPARAM)pvalues[smallest], (LPARAM)pvalues[biggest]);
}

int UI::FixedValueSpinButtonCtrl::FindNearest(int value)
{
   if (m_vecValues.empty())
      return value;

   int newval = value, dist = 0x7fffffff;
   for (size_t i = 0; i < m_vecValues.size(); i++)
   {
      if (abs(m_vecValues[i] - value) < dist)
      {
         dist = abs(m_vecValues[i] - value);
         newval = m_vecValues[i];

         if (dist == 0)
            break;
      }
   }

   return newval;
}
