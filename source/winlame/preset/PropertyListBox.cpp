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

   $Id: PropertyListBox.cpp,v 1.11 2009/11/02 20:30:50 vividos Exp $

*/
/*! \file PropertyListBox.cpp

   \brief contains implementation of the preset selection page

   \ingroup preset

*/

// needed includes
#include "stdafx.h"
#include "PropertyListBox.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// consts

const UINT P_GROUP = 0x80000000L;      ///< property group flag
const UINT P_COLLAPSED = 0x40000000L;  ///< collapsed property group flag
const UINT P_ITEMMASK = 0x0000ffffL;   ///< mask for item id
const UINT P_GROUPMASK = 0x0fff0000L;  ///< mask for group id


// PropertyListBox methods

void PropertyListBox::InitListBox(bool read_only)
{
   ATLASSERT(propmanager!=NULL);

   // for every group, insert a group string
   int groups = propmanager->GetGroupCount();

   for(int i=0; i<groups; i++)
   {
      int nIndex = AddString(propmanager->GetGroupName(i).c_str());
      SetItemData(nIndex, P_GROUP | P_COLLAPSED | i);
   }

   inplaceCtrl = NULL;
   inplaceIndex = -1;
   inplaceGroup = -1;
   readonly = read_only;
}

LRESULT PropertyListBox::OnReflectedCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   if (HIWORD(wParam) == LBN_SELCHANGE)
   {
      int nIndex = GetCurSel();
      RECT rc;

      if (!readonly && nIndex!=LB_ERR && GetItemRect(nIndex,&rc)!=LB_ERR)
      {
         // adjust for value field
         rc.left = (rc.right + rc.left + LEFT_BORDER) / 2 + 1;
         rc.top++; rc.bottom--;

         // read old value, store it
         if (inplaceCtrl!=NULL)
         {
            TCHAR buffer[256];
            inplaceCtrl->GetWindowText(buffer,256);

            // update value
            std::tstring value(buffer);
            propmanager->SetItemValue(inplaceGroup,inplaceIndex,value);

            // delete old inplace control
            if (inplaceCtrl->IsWindow())
               inplaceCtrl->DestroyWindow();

            delete inplaceCtrl;
            inplaceCtrl = NULL;
            inplaceIndex = -1;
            inplaceGroup = -1;
         }

         // check if group
         if ((GetItemData(nIndex) & P_GROUP) != P_GROUP)
         {
            DWORD data = GetItemData(nIndex);
            inplaceIndex = data & P_ITEMMASK;
            inplaceGroup = (data & P_GROUPMASK)>>16;

            // create new inplace control
            CEdit *editCtrl = new CEdit;
            inplaceCtrl = editCtrl;

            editCtrl->Create(m_hWnd, &rc, NULL,
               WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_LEFT|ES_NOHIDESEL,
               0, 0U/*ID*/);

            // set value for inplace control
            editCtrl->SetWindowText(propmanager->GetItemValue(inplaceGroup,inplaceIndex).c_str());
            editCtrl->SetFont(GetFont());
            editCtrl->SetSel(0,-1);

            // show control
            inplaceCtrl->ShowWindow(SW_SHOW);
            inplaceCtrl->MoveWindow(&rc);
            inplaceCtrl->SetFocus();
         }
      }
   }
   return 0;
}

LRESULT PropertyListBox::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
   HDC hDC = lpDrawItemStruct->hDC;

   // get infos about the current item
   DWORD data = lpDrawItemStruct->itemData;
   bool isgroup = (data & P_GROUP) == P_GROUP;
   bool iscollapsed = isgroup && (data & P_COLLAPSED) == P_COLLAPSED;
   int index = data & P_ITEMMASK;
   int group = (data & P_GROUPMASK)>>16;

   // calculate rects
   RECT rcItem = lpDrawItemStruct->rcItem;

   // find out prop name rect
   RECT rcName = rcItem;
   rcName.left += LEFT_BORDER;
   rcName.right = (rcName.right + rcName.left) / 2 - 1;
   rcName.bottom--;

   // find out prop value rect
   RECT rcValue = rcItem;
   rcValue.left = (rcValue.right + rcValue.left + LEFT_BORDER) / 2 + 1;
   rcValue.bottom--;

   // calc grid rect
   RECT rcGrid = rcItem;
   rcGrid.left += LEFT_BORDER - 1;

   UINT nPropWidth   = (rcGrid.right - rcGrid.left) / 2;

   // create new pen
   HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(192,192,192));
   HGDIOBJ hOldGDIObj = ::SelectObject(hDC, hPen);

   if (isgroup)
   {
      // draw the cross
      rcItem.left += 2;
      DrawCross(hDC, rcItem, 4, iscollapsed);
      rcItem.left -= 2;
   }

   // draw the grid lines
   DrawGridLine(hDC, rcGrid, nPropWidth);

   std::tstring name,value;
   if (isgroup)
   {
      name = propmanager->GetGroupName(index);
      value = _T("");
   }
   else
   {
      name = propmanager->GetItemName(group,index);
      value = propmanager->GetItemValue(group,index);
   }

   DrawPropText(hDC, name.c_str(), rcName, value.c_str(), rcValue,
      lpDrawItemStruct->itemState, lpDrawItemStruct->itemAction, !isgroup);

   // restore GDI
   ::SelectObject(hDC, hOldGDIObj);
   ::DeleteObject(hPen);

   return 0;
}

void PropertyListBox::DrawGridLine(HDC hDC, RECT &rcGrid, UINT nPropWidth)
{
   POINT pt;
   int nBottom = rcGrid.bottom - 1;

   // left vertical line
   ::MoveToEx(hDC, rcGrid.left, rcGrid.top, &pt);
   ::LineTo(hDC, rcGrid.left, rcGrid.bottom);

   // bottom line
   ::MoveToEx(hDC, rcGrid.left, nBottom, &pt);
   ::LineTo(hDC, rcGrid.right, nBottom);

   // center line
   ::MoveToEx(hDC, rcGrid.left + nPropWidth, rcGrid.top, &pt);
   ::LineTo(hDC, rcGrid.left + nPropWidth, rcGrid.bottom);
}

void PropertyListBox::DrawPropText(HDC hDC, LPCTSTR name, RECT &rcName,
      LPCTSTR value, RECT &rcValue, UINT nItemState, UINT nItemAction,
      bool thin)
{
   COLORREF clrBackground, clrText;
   COLORREF clrOldBackground, clrOldText;
   HBRUSH hBGBrush;
   HFONT hOldFont;
   bool bIsDisabled = (nItemState & ODS_DISABLED) == ODS_DISABLED;
   bool bDrawCtrl = (nItemState & ODS_SELECTED) == ODS_SELECTED && !bIsDisabled;
   
   if ((nItemState & ODS_SELECTED) == ODS_SELECTED)
   {
      clrBackground   = ::GetSysColor(COLOR_HIGHLIGHT);
      clrText         = ::GetSysColor(bIsDisabled? COLOR_GRAYTEXT : COLOR_HIGHLIGHTTEXT);
   }
   else
   {
      clrBackground   = ::GetSysColor(COLOR_WINDOW);
      clrText         = ::GetSysColor(bIsDisabled? COLOR_GRAYTEXT : COLOR_WINDOWTEXT);
   }

   if ((hBGBrush = CreateSolidBrush(clrBackground)) != NULL)
   {
      ::FillRect(hDC, &rcName, hBGBrush);
      if (!bDrawCtrl)
      {
         ::FillRect(hDC, &rcValue, hBGBrush);
      }
      ::DeleteObject(hBGBrush);
   }

   clrOldBackground = ::SetBkColor(hDC, clrBackground);
   clrOldText = ::SetTextColor(hDC, clrText);

   rcName.left += 2;
   rcName.right -= 2;

   // get default font and logfont struct
   hOldFont = NULL;
   HFONT bigfont = NULL;
   if (!thin)
   {
      bigfont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
      LOGFONT lf;
      ::GetObject(bigfont,sizeof(LOGFONT), &lf);
      lf.lfWeight = 400;
      bigfont = ::CreateFontIndirect(&lf);
      hOldFont = (HFONT)::SelectObject(hDC, bigfont);
   }

   // draw item text
   ::DrawText(hDC, name, -1, &rcName, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
   if (!bDrawCtrl)
      ::DrawText(hDC, value, -1, &rcValue, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

   if (hOldFont != NULL)
      ::SelectObject(hDC, hOldFont);

   if (bigfont != NULL)
      ::DeleteObject(bigfont);

   // restore coordinates
   rcName.left      -= 2;
   rcName.right     += 2;

   // restore GDI
   ::SetBkColor(hDC, clrOldBackground);
   ::SetTextColor(hDC, clrOldText);
}

void PropertyListBox::DrawCross(HDC hDC, RECT &rc, UINT nSize, bool bPlus)
{
   HBRUSH   hWhiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
   HBRUSH   hBlackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
   HPEN      hPen         = ::CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
   UINT      nTop         = (rc.top + rc.bottom) / 2 - nSize - 1;
   RECT      rect         = { rc.left, nTop, rc.left + 2 * nSize + 1, nTop + 2 * nSize + 1 };
   POINT      point;
   UINT      nXCenter      = rect.left + nSize;
   UINT      nYCenter      = rect.top + nSize;
   HGDIOBJ   hOldGDIObj   = ::SelectObject(hDC, hPen);

   ::FillRect(hDC, &rect, hWhiteBrush);
   ::FrameRect(hDC, &rect, hBlackBrush);

   ::MoveToEx(hDC, rect.left + 2, nYCenter, &point);
   ::LineTo(hDC, rect.right - 2, nYCenter);

   if(bPlus)
   {
      ::MoveToEx(hDC, nXCenter, rect.top + 2, &point);
      ::LineTo(hDC, nXCenter, rect.bottom - 2);
   }

   ::SelectObject(hDC, hOldGDIObj);
   ::DeleteObject(hPen);
   ::DeleteObject(hWhiteBrush);
   ::DeleteObject(hBlackBrush);
}

void PropertyListBox::ChangeGroupState(int nIndex)
{
   if (nIndex == LB_ERR)
      return;

   int nGroupIndex = nIndex;
   int data = GetItemData(nIndex);
   bool isgroup = (data & P_GROUP) == P_GROUP;
   bool iscollapsed = isgroup && (data & P_COLLAPSED) == P_COLLAPSED;
   int group = data & 0x00007fffL;

   if (!isgroup)
      return;

   if (iscollapsed)
   {
      // expand: go through all entries in the group and insert them

      int max = propmanager->GetItemCount(group);

      for(int i=0; i<max; i++)
      {
         int nIndex2 = InsertString(++nIndex,propmanager->GetItemName(group,i).c_str());
         SetItemData(nIndex2,((unsigned int)(group)<<16)|i);
      }

      data &= ~P_COLLAPSED; // expanded
   }
   else
   {
      // collapse: delete all items up to the next group
      nIndex++;
      int max = GetCount();
      do
      {
         DWORD data = GetItemData(nIndex);
         if ((data & P_GROUP) == P_GROUP)
            break;

         DeleteString(nIndex);
         max--;
      }
      while(nIndex<max);

      data |= P_COLLAPSED; // collapsed
   }

   SetItemData(nGroupIndex,data);
}
