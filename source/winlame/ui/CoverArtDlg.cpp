//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file CoverArtDlg.cpp
/// \brief Cover art dialog
//
#include "stdafx.h"
#include "resource.h"
#include "CoverArtDlg.hpp"

using UI::CoverArtDlg;

CoverArtDlg::CoverArtDlg(const ATL::CImage& image)
   :m_image(image)
{
}

LRESULT CoverArtDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   CenterWindow();

   // set icons
   HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME),
      IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
   SetIcon(hIcon, TRUE);
   HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME),
      IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
   SetIcon(hIconSmall, FALSE);

   // set image
   m_staticCoverArtImage.ModifyStyle(SS_BLACKFRAME, SS_BITMAP | SS_REALSIZECONTROL);
   m_staticCoverArtImage.SetBitmap(m_image);

   return TRUE;
}

LRESULT CoverArtDlg::OnExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   EndDialog(wID);
   return 0;
}

LRESULT CoverArtDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
   int cxWidth = GET_X_LPARAM(lParam);
   int cyHeight = GET_Y_LPARAM(lParam);

   double aspectRatio = double(m_image.GetWidth()) / m_image.GetHeight();

   int cyNewHeight = int(cxWidth / aspectRatio);
   int cxNewWidth = int(1.0 / (aspectRatio / cyHeight));

   cxWidth = std::min(cxWidth, cxNewWidth);
   cyHeight = std::min(cyHeight, cyNewHeight);

   m_staticCoverArtImage.MoveWindow(0, 0, cxWidth, cyHeight, FALSE);

   RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

   return 0;
}
