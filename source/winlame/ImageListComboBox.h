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
/*! \file ImageListComboBox.h

   \brief image list combobox

*/

// include guard
#pragma once

// includes
#include <atlcrack.h>
#include <map>

/// combobox with image list for entries
class CImageListComboBox :
   public CWindowImpl<CImageListComboBox, CComboBox>,
   public COwnerDraw<CImageListComboBox>
{
public:
   CImageListComboBox()
      :m_uiItemWidth(0),
       m_uiItemHeight(0)
   {
   }

   /// adds new string with icon
   int AddString(LPCTSTR pszString, int iIconIndex)
   {
      int iRet = CComboBox::AddString(pszString);
      m_mapItemsToIndex[iRet] = iIconIndex;
      return iRet;
   }

   /// inserts new string with icon
   int InsertString(int iIndex, LPCTSTR pszString, int iIconIndex)
   {
      int iRet = CComboBox::InsertString(iIndex, pszString);
      m_mapItemsToIndex[iRet] = iIconIndex;
      return iRet;
   }

   /// sets image list
   void SetImageList(const CImageList& ilIcons)
   {
      int cx = 0, cy = 0;
      ilIcons.GetIconSize(cx, cy);
      SetSize(cx, cy);

      m_ilIcons = ilIcons;
   }

private:
   DECLARE_WND_SUPERCLASS(0, CComboBox::GetWndClassName())

   BEGIN_MSG_MAP_EX(CImageListComboBox)
      MESSAGE_HANDLER(WM_PAINT, OnPaint)
      //MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChanged)
      REFLECTED_COMMAND_CODE_HANDLER(CBN_SELENDOK, OnSelEndOK)
      CHAIN_MSG_MAP(COwnerDraw<CImageListComboBox>)
      CHAIN_MSG_MAP_ALT(COwnerDraw<CImageListComboBox>, 1)
      DEFAULT_REFLECTION_HANDLER()
   END_MSG_MAP()

   LRESULT OnSelEndOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
   {
      Invalidate();
      return 0;
   }

   LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

   LRESULT OnThemeChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
//      CloseThemeData(m_hTheme);
//      m_hTheme = ::OpenThemeData(m_hWnd, VSCLASS_COMBOBOX);
      return 0;
   }

   void DrawItem(LPDRAWITEMSTRUCT lpDIS);

   void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
   {
      lpMeasureItemStruct->itemWidth = m_uiItemWidth + 6;
      lpMeasureItemStruct->itemHeight = m_uiItemHeight + 7;
   }

   /// draws bitmap (and string) in item
   void OutputBitmap(LPDRAWITEMSTRUCT lpDIS);

   /// sets item size
   void SetSize(UINT width, UINT height)
   {
      if (width > m_uiItemWidth)
         m_uiItemWidth = width;
      if (height > m_uiItemHeight)
         m_uiItemHeight = height;
      for (int i = -1; i < GetCount(); i++) 
         SetItemHeight(i, m_uiItemHeight + 6);
   }

private:
   CImageList m_ilIcons;
   std::map<int, unsigned int> m_mapItemsToIndex;

   UINT m_uiItemWidth;
   UINT m_uiItemHeight;
};
