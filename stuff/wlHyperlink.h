/*
   winLAME - a frontend for the LAME encoding engine
   copyright (c) 2000-2002 Michael Fink

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
/* file wlHyperlink.h

   contains a hyperlink control, that makes a static control underlined and
   clickable; the standard browser will be opened with the assigned URL.
   the mouse cursor changes when the user points it over the control.

   to use it, make sure that in every window's message map is the entry
   REFLECT_NOTIFICATIONS()
   so that notifications will be routed back to the child controls.

*/

// prevent multiple including
#ifndef __wlhyperlink_h_
#define __wlhyperlink_h_

// needed includes
#include <atlmisc.h>
#include "shellapi.h"
#undef ExtractIcon    // darn macros :(


// make IDC_HAND known to non-win2k-systems
#if(WINVER < 0x0500)
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif


// wlHyperlink class

class wlHyperlink: public CWindowImpl<wlHyperlink>
{
public:
   wlHyperlink()
   {
      underlinedFont = NULL;
      handCursor = NULL;
      firstMessage = true;
      visited = false;
      mouseCaptured = false;
   }

   // message map
BEGIN_MSG_MAP(wlHyperlink)
   MESSAGE_HANDLER(WM_PAINT, OnPaint)
   MESSAGE_HANDLER(OCM_COMMAND, OnReflected) // reflected command messages
   MESSAGE_HANDLER(OCM_CTLCOLORSTATIC, OnCtlColor)
   MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
   MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
END_MSG_MAP()

   LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // little trick to initalize the hyperlink ctrl: wait for the first
      // paint msg; that ensures that the ctrl is already created
      if (firstMessage)
      {
         firstMessage = false;

         // initialize colors
         colorHover = ::GetSysColor(COLOR_HIGHLIGHT);
         colorLink = RGB(0,0,255);
         colorVisited = RGB(128,0,128);

         // set SS_NOTIFY flag on static control
         SetWindowLong(GWL_STYLE, GetStyle() | SS_NOTIFY);

         // get font and logfont struct
         HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
         LOGFONT lf;
         ::GetObject(hFont,sizeof(LOGFONT), &lf);

         // create an underlined font
         lf.lfUnderline = TRUE;
         underlinedFont = ::CreateFontIndirect(&lf);
         SetFont(underlinedFont,FALSE);

         // try to load hand cursor
         handCursor = ::LoadCursor(NULL,MAKEINTRESOURCE(IDC_HAND));
         
         if (handCursor==NULL)
         {
            // IDC_HAND only exists on >= NT5, so do this ugly construct:
            // get the windows directory
            char path[MAX_PATH+14];
            ::GetWindowsDirectory(path,MAX_PATH);
            strcat(path,"\\winhlp32.exe");

            // retrieve cursor #106 from winhlp32.exe, a hand pointer
            HMODULE hModule = ::LoadLibrary(path);
            if (hModule)
            {
               // load the cursor
               HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
               if (hHandCursor)
                  handCursor = CopyCursor(hHandCursor);
            }
            ::FreeLibrary(hModule);
         }
      }
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnReflected(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // we got a reflected message

      UINT wNotifyCode = HIWORD(wParam);
      switch (wNotifyCode)
      {
         // user clicked on the control
      case STN_CLICKED:
         ::ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOW);
         visited = true;
         break;

      default:
         bHandled = FALSE;
         break;
      }
      return 0;
   }

   LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // set text color and background mode
      HDC hDC = (HDC)wParam;
      ::SetTextColor(hDC,visited ? colorVisited : mouseCaptured ? colorHover : colorLink);
      ::SetBkMode(hDC,TRANSPARENT);
      return (LRESULT)::GetStockObject(NULL_BRUSH);
   }

   LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (mouseCaptured)
      {
         // we already captured the mouse

         // now check if the mouse is still over the ctrl
         RECT rc;
         GetClientRect(&rc);
         
         POINT pt={ LOWORD(lParam), HIWORD(lParam) };

         if (FALSE==::PtInRect(&rc,pt))
         {
            // no, release capture
            mouseCaptured=false;
            ReleaseCapture();
            RedrawWindow();
         }
      }
      else
      {
         // capture the mouse
         SetCapture();
         mouseCaptured = true;
         RedrawWindow();
      }

      return 0;
   }

   LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (handCursor)
      {
         ::SetCursor(handCursor);
         return TRUE;
      }
      return 0;
   }

   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // get rid of the font
      if (underlinedFont!=NULL)
         ::DeleteObject(underlinedFont);

      // and of the cursor
      if (handCursor!=NULL)
         ::DestroyCursor(handCursor);

      firstMessage = true;

      return 0;
   }

   // sets the hyperlink URL
   void SetHyperlink(const char*str){ url = str; }

protected:
   // underlined font
   HFONT underlinedFont;

   // hand cursor
   HCURSOR handCursor;

   // contains the hyperlink URL
   CString url;

   // indicates if the mouse is captured
   bool mouseCaptured;

   // indicates if the control already got a message
   bool firstMessage;

   // indicates if the link was already visited
   bool visited;

   // color for mouse hover
   COLORREF colorHover;

   // color for unvisited link
   COLORREF colorLink;

   // color for visited link
   COLORREF colorVisited;
};

#endif
