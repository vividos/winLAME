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
/// \file BevelLine.cpp
/// \brief Bevel line control
//
#include "stdafx.h"
#include "BevelLine.hpp"

LRESULT UI::BevelLine::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // begin painting
   CPaintDC dc(m_hWnd);

   // find out rects of control and text
   CRect rectWnd;
   GetWindowRect(&rectWnd);
   ScreenToClient(&rectWnd);

   // draw bevel line
   CRect rectLine = rectWnd;
   rectLine.top = (rectWnd.bottom - rectWnd.top) >> 1;
   rectLine.bottom = rectLine.top + 2;

   dc.SetBkColor(::GetSysColor(COLOR_3DHIGHLIGHT));
   dc.ExtTextOut(0, 0, ETO_OPAQUE, &rectLine, NULL, 0, NULL);

   rectLine.bottom--;
   rectLine.right--;

   dc.SetBkColor(::GetSysColor(COLOR_3DSHADOW));
   dc.ExtTextOut(0, 0, ETO_OPAQUE, &rectLine, NULL, 0, NULL);

   // add space at the end so that bevel starts further right
   CString cszText;
   GetWindowText(cszText);
   cszText += _T("  ");

   // draw text
   dc.SetBkColor(::GetSysColor(COLOR_BTNFACE));

   HFONT hFont = (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
   HFONT hObj = dc.SelectFont(hFont);

   dc.DrawText(cszText, cszText.GetLength(), &rectWnd, DT_TOP | DT_LEFT);

   dc.SelectFont(hObj);

   return 0;
}
