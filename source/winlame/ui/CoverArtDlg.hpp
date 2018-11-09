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
/// \file CoverArtDlg.hpp
/// \brief Cover art dialog
//
#pragma once

namespace UI
{
   /// Dialog to show cover art image in full
   class CoverArtDlg :
      public CDialogImpl<CoverArtDlg>,
      public CWinDataExchange<CoverArtDlg>
   {
   public:
      /// ctor
      explicit CoverArtDlg(const ATL::CImage& image);

      /// dialog id
      enum { IDD = IDD_DLG_COVERART };

      BEGIN_DDX_MAP(CoverArtDlg)
         DDX_CONTROL_HANDLE(IDC_STATIC_COVERART, m_staticCoverArtImage)
      END_DDX_MAP()

      BEGIN_MSG_MAP(CoverArtDlg)
         MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
         COMMAND_ID_HANDLER(IDOK, OnExit)
         COMMAND_ID_HANDLER(IDCANCEL, OnExit)
         COMMAND_HANDLER(IDC_STATIC_COVERART, STN_CLICKED, OnExit)
         MESSAGE_HANDLER(WM_SIZE, OnSize)
      END_MSG_MAP()

      /// called to init the dialog
      LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

      /// called on exiting the dialog
      LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

      /// called when dialog is resized
      LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   private:
      /// cover art image
      const ATL::CImage& m_image;

      /// cover art image control
      CStatic m_staticCoverArtImage;
   };

} // namespace UI
