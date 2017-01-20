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
/// \file ErrorDlg.hpp
/// \brief shows an error dialog which lets the user decide what to do (try continue,
/// skip file or stop encoding)
//
#pragma once

#include "resource.h"
#include "CommonStuff.hpp"

namespace ClassicUI
{
   /// error dialog class
   class ErrorDlg : public CDialogImpl<ErrorDlg>
   {
   public:
      /// ctor
      ErrorDlg(LPCTSTR thefilename, LPCTSTR themodulename, int theerrorcode, LPCTSTR theerrormsg,
         bool bSkipDisabled)
         :filename(thefilename),
         modulename(themodulename),
         errorcode(theerrorcode),
         errormsg(theerrormsg),
         appicon(nullptr),
         icon(nullptr),
         m_bSkipDisabled(bSkipDisabled)
      {
      }

      enum { IDD = IDD_ERRORDLG };

      BEGIN_MSG_MAP(ErrorDlg)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_HANDLER(IDC_ERR_BUTTON_CONTINUE, BN_CLICKED, OnEndDialog)
         COMMAND_HANDLER(IDC_ERR_BUTTON_SKIPFILE, BN_CLICKED, OnEndDialog)
         COMMAND_HANDLER(IDC_ERR_BUTTON_STOP, BN_CLICKED, OnEndDialog)
      END_MSG_MAP()
      // Handler prototypes:
      //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
      //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
      //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

         /// inits error dialog
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
      {
         // place window
         CenterWindow();

         // subclass the bevel line
         bevel1.SubclassWindow(GetDlgItem(IDC_ERR_BEVEL1));

         // set the application icon as the dialog's item
         appicon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME));
         SetIcon(icon, TRUE);
         SetIcon(icon, FALSE);

         // set an exclamation icon on the dialog
         icon = ::LoadIcon(NULL, IDI_EXCLAMATION);
         SendDlgItemMessage(IDC_ERR_STATIC_ICON, STM_SETICON, (WPARAM)icon);

         // set all variable strings
         SetDlgItemText(IDC_ERR_STATIC_FILENAME, filename);

         CString buffer;
         buffer.Format(IDS_ERR_MODULE, modulename);
         SetDlgItemText(IDC_ERR_STATIC_MODULE, buffer);

         buffer.Format(IDS_ERR_ERRCODE, errorcode);
         SetDlgItemText(IDC_ERR_STATIC_ERRCODE, buffer);

         buffer.Format(IDS_ERR_ERRMSG, errormsg);
         SetDlgItemText(IDC_ERR_STATIC_ERRMSG, buffer);

         if (m_bSkipDisabled)
            ::EnableWindow(GetDlgItem(IDC_ERR_BUTTON_SKIPFILE), FALSE);

         return 1;  // let the system set the focus
      }

      /// called when exiting the dialog
      LRESULT OnEndDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
      {
         // ends dialog with the id of pressed button as return value
         EndDialog(wID);
         return 0;
      }

   protected:
      /// currently processing filename
      LPCTSTR filename;

      /// module which reported the error
      LPCTSTR modulename;

      /// error code returned
      int errorcode;

      /// error message
      LPCTSTR errormsg;

      /// application icon
      HICON appicon;

      /// error icon
      HICON icon;

      /// indicates if "skip" button is disabled
      bool m_bSkipDisabled;

      /// bevel line
      BevelLine bevel1;
   };

} // namespace ClassicUI
