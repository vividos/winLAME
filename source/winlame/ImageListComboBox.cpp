/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2009 Michael Fink

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
/*! \file ImageListComboBox.cpp

   \brief image list combobox

*/

// needed includes
#include "stdafx.h"
#include "ImageListComboBox.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/// temporarily changes brush
class CBrushChanger
{
public:
   CBrushChanger(CDCHandle& dc, CBrush& br) throw()
      :m_dc(dc),
       m_hOldBrush(dc.SelectBrush(br))
   {
   }

   ~CBrushChanger() throw()
   {
      m_dc.SelectBrush(m_hOldBrush);
   }

private:
   CDCHandle& m_dc;
   HBRUSH m_hOldBrush;
};

/// temporarily changes pen
class CPenChanger
{
public:
   CPenChanger(CDCHandle& dc, CPen& br) throw()
      :m_dc(dc),
       m_hOldPen(dc.SelectPen(br))
   {
   }

   ~CPenChanger() throw()
   {
      m_dc.SelectPen(m_hOldPen);
   }

private:
   CDCHandle& m_dc;
   HPEN m_hOldPen;
};

LRESULT CImageListComboBox::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CPaintDC dc(m_hWnd);

   CRect rcClient, rcClip;
   GetClientRect(&rcClient);
/*
   if (m_hTheme)
   {
      if (IsThemeBackgroundPartiallyTransparent(m_hTheme, CP_DROPDOWNBUTTON, CBB_NORMAL))
      {
         DrawThemeParentBackground(m_hWnd, dc, &rcClip);
      }

      HRESULT hRes = DrawThemeBackground(m_hTheme, dc, CP_BACKGROUND, CBB_NORMAL, &rcClient, &rcClip);
      ATLASSERT(SUCCEEDED(hRes)); hRes;
   }
   else
*/
   {
      // draw the edit box
      bool bEnabled = true;
      if (bEnabled)
         dc.FillSolidRect(rcClient, ::GetSysColor(COLOR_WINDOW));
      else
         dc.FillSolidRect(rcClient,::GetSysColor(COLOR_BTNFACE));

      // draw the border around the edit control
      dc.Draw3dRect(rcClient, ::GetSysColor(COLOR_3DDKSHADOW),
         ::GetSysColor(COLOR_3DLIGHT));

      HFONT hOldFont = dc.SelectFont(GetFont());

      rcClient.DeflateRect(1, 1);

      // call DrawItem()
      DRAWITEMSTRUCT dis = {0};
      dis.hDC = dc;
      dis.itemID = GetCurSel();
      dis.itemState = 0/*ODS_SELECTED*/; // TODO
      dis.itemAction = /*ODA_SELECT |*/ ODA_DRAWENTIRE;
      dis.rcItem = rcClient;
      dis.rcItem.right -= 17;
      DrawItem(&dis);

      dc.SelectFont(hOldFont);

      // draw the dropdown arrow.
      CRect rcArrow(rcClient);
      rcArrow.left = rcArrow.right - 17;
      dc.DrawFrameControl(rcArrow,DFC_SCROLL,DFCS_SCROLLCOMBOBOX);
   }

   return 0;
}

void CImageListComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
   CDCHandle dc = lpDIS->hDC;

   if (!IsWindowEnabled())
   {
      CBrush brDisabled; brDisabled.CreateSolidBrush(RGB(192,192,192)); // light gray
      CBrushChanger brChanger(dc, brDisabled);

      CPen penDisabled; penDisabled.CreatePen(PS_SOLID, 1, RGB(192,192,192));
      CPenChanger penChanger(dc, penDisabled);

      OutputBitmap(lpDIS);
      return;
   }

   // selected
   if ((lpDIS->itemState & ODS_SELECTED) 
      && (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
   {
      CBrush brHighlight; brHighlight.CreateSysColorBrush(COLOR_HIGHLIGHT);
      CBrushChanger brChanger(dc, brHighlight);

      CPen penHighlight; penHighlight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHT));
      CPenChanger penChanger(dc, penHighlight);

      dc.Rectangle(&lpDIS->rcItem);
      dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
      dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
      OutputBitmap(lpDIS);
   }

   // deselected
   if (!(lpDIS->itemState & ODS_SELECTED) 
      && (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
   {
      CBrush brWindow; brWindow.CreateSysColorBrush(COLOR_WINDOW); 
      CBrushChanger brChanger(dc, brWindow);

      CPen penHighlight; penHighlight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOW));
      CPenChanger penChanger(dc, penHighlight);

      dc.Rectangle(&lpDIS->rcItem);
      dc.SetBkColor(::GetSysColor(COLOR_WINDOW));
      dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
      OutputBitmap(lpDIS);
   }

   // focused
   if (lpDIS->itemAction & ODA_FOCUS) 
      dc.DrawFocusRect(&lpDIS->rcItem);
}

void CImageListComboBox::OutputBitmap(LPDRAWITEMSTRUCT lpDIS)
{
   CDCHandle dc = lpDIS->hDC;

   std::map<int, unsigned int>::const_iterator iter = m_mapItemsToIndex.find(lpDIS->itemID);
   int iIconIndex = iter != m_mapItemsToIndex.end() ? iter->second : -1;

   HICON hIcon = NULL;
   if (iIconIndex < m_ilIcons.GetImageCount() && iIconIndex != -1)
   {
      CPoint point;
      point.x = lpDIS->rcItem.left + 2;
      point.y = lpDIS->rcItem.top + ((lpDIS->rcItem.bottom - lpDIS->rcItem.top) / 2) - (m_uiItemHeight / 2); 

      m_ilIcons.Draw(dc, iIconIndex, point.x, point.y, ILD_NORMAL);
   }

   CString cszText; 
   if (lpDIS->itemID != -1) 
      GetLBText(lpDIS->itemID, cszText); 

   if (!cszText.IsEmpty()) 
   {
      CRect rcText(lpDIS->rcItem); 
      rcText.DeflateRect(m_uiItemWidth + 4, 0, 0, 0);
      dc.DrawText(cszText, cszText.GetLength(), rcText, DT_SINGLELINE | DT_VCENTER); 
   }
}
