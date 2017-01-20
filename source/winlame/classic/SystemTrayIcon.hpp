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
/// \file SystemTrayIcon.hpp
/// \brief contains a wrapper class for handling a system tray icon
//
#pragma once

#include "shellapi.h"
#undef ExtractIcon

/// wrapper class for a system tray icon
class SystemTrayIcon
{
public:
   /// ctor
   SystemTrayIcon()
      :active(false)
   {
      memset(&nid, 0, sizeof(nid));
   }

   /// dtor
   ~SystemTrayIcon()
   {
      if (active) RemoveIcon();
   }

   /// creates icon in the system tray
   bool Init(HWND hParent, UINT uID, HICON hIcon, UINT notifyMessage, LPCTSTR pszTooltip)
   {
      nid.cbSize = sizeof(NOTIFYICONDATA);
      nid.hWnd   = hParent;
      nid.uID    = uID;
      nid.hIcon  = hIcon;

      CString cszAppCaption(MAKEINTRESOURCE(IDS_APP_CAPTION));
      _tcsncpy(nid.szTip, cszAppCaption, 64);
      nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP/* | NIF_INFO*/;
//      nid.uVersion = NOTIFYICON_VERSION;
//      nid.dwInfoFlags = NIIF_INFO;
      nid.uCallbackMessage = notifyMessage;

      int size = lstrlen(pszTooltip)+1;
      if (size>64) size=64;
      lstrcpyn(nid.szTip,pszTooltip,size);

      active=true;

      return ::Shell_NotifyIcon(NIM_ADD, &nid)==TRUE;
   }

   /// removes the system tray icon
   bool RemoveIcon()
   {
      active=false;
      return ::Shell_NotifyIcon(NIM_DELETE, &nid)==TRUE;
   }

   /// sets icon for tray
   bool SetIcon(HICON hIcon)
   {
      nid.hIcon = hIcon;
      nid.uFlags = NIF_ICON;
      return ::Shell_NotifyIcon(NIM_MODIFY, &nid)==TRUE;
   }


   /// sets a new tooltip text
   bool SetTooltipText(LPCTSTR pszTooltip)
   {
      nid.uFlags = NIF_TIP;
      int size = lstrlen(pszTooltip)+1;
      if (size>64) size=64;
      lstrcpyn(nid.szTip,pszTooltip,size);
      return ::Shell_NotifyIcon(NIM_MODIFY, &nid)==TRUE;
   }

protected:
   /// notify icon data
   NOTIFYICONDATA nid;

   /// indicates if icon is active
   bool active;
};
