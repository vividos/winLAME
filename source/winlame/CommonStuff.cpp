/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2005 Michael Fink

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/// \file CommonStuff.cpp
/// \brief common UI function implementations
/// \details contains the drawing code for the bevel line, and the tooltips and
/// folder-browse function

// needed includes
#include "stdafx.h"
#include "CommonStuff.h"
#include "resource.h"

// functions

LRESULT BevelLine::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // begin painting
   PAINTSTRUCT ps;
   HDC hDC = ::BeginPaint(m_hWnd,&ps);

   // find out rects of control and text
   RECT rectWnd;
   GetWindowRect(&rectWnd);
   ScreenToClient(&rectWnd);

   // draw bevel line
   RECT rectLine = rectWnd;
   rectLine.top = (rectWnd.bottom-rectWnd.top)>>1;
   rectLine.bottom = rectLine.top+2;

   ::SetBkColor(hDC, ::GetSysColor(COLOR_3DHIGHLIGHT) );
   ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rectLine, NULL, 0, NULL);
  
   rectLine.bottom--; rectLine.right--;

   ::SetBkColor(hDC, ::GetSysColor(COLOR_3DSHADOW) );
   ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rectLine, NULL, 0, NULL);

   // add two spaces at the end
   TCHAR buffer[512];
   GetWindowText(buffer,510);
   _tcscat(buffer,_T("  "));

   // draw the text
   ::SetBkColor(hDC, ::GetSysColor(COLOR_BTNFACE) );

   HFONT hFont = (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
   HGDIOBJ hObj = SelectObject(hDC,hFont);

   ::DrawText(hDC, buffer, _tcslen(buffer), &rectWnd, DT_TOP | DT_LEFT );

   ::SelectObject(hDC, hObj);

   // pink panther has finished
   ::EndPaint(m_hWnd,&ps);

   return 0;
}

void AddTooltips(HWND hWnd, CToolTipCtrl &ctrl)
{
   // get first child window
   hWnd = ::GetWindow(hWnd,GW_CHILD);

   CString tooltext;

   // go through all child windows
   while (hWnd!=NULL)
   {
      // get ctrl id
      int id = ::GetDlgCtrlID(hWnd);

      if (id!=-1 && id !=0)
      {
         // try to load a text
         tooltext.LoadString(id);

         // when successful, add text as tooltip text
         if (tooltext.GetLength()!=0)
            ctrl.AddTool(hWnd,(LPCTSTR)tooltext);
      }

      // get next window
      hWnd = GetWindow(hWnd,GW_HWNDNEXT);
   }
}

#ifndef BIF_USENEWUI
#define BIF_USENEWUI 0x0050   ///< constant to use new UI when selecting folders
#endif

bool BrowseForFolder(HWND hParentWnd, CString& cszPathname, UINT nCaptionID)
{
   if (nCaptionID==0)
      nCaptionID = IDS_COMMON_SELECTDIR;

   // get caption string
   CString cszCaption;
   cszCaption.LoadString(nCaptionID);

   // add last slash to dir name
   if (!cszPathname.IsEmpty() && cszPathname.Right(1)!=_T('\\'))
      cszPathname += _T("\\");

   CFolderDialog dlg(hParentWnd, cszCaption, BIF_RETURNONLYFSDIRS | BIF_USENEWUI);
   dlg.SetInitialFolder(cszPathname);
   if (IDOK == dlg.DoModal())
   {
      cszPathname = dlg.GetFolderPath();
      if (!cszPathname.IsEmpty() && cszPathname.Right(1)!=_T('\\'))
         cszPathname += _T("\\");
      return true;
   }
   else
      return false;
}
