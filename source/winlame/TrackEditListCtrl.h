/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005 Michael Fink
   Copyright (c) 2004 DeXT

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
/// \file TrackEditListCtrl.h
/// \brief edit control for track list
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes


#define WM_DELETEME (WM_APP+11)

typedef CWinTraits<(WS_VISIBLE | WS_CHILD), 0> InplaceEditCtrlTraits;

// classes

/// in-place edit control for editable list view control
class InplaceEditCtrl: public CWindowImpl<InplaceEditCtrl, CEdit, InplaceEditCtrlTraits>
{
public:
   /// ctor
   InplaceEditCtrl(int nItem, int nColumn)
      :m_bFinished(false), m_nItem(nItem), m_nColumn(nColumn){}

   // message map
   BEGIN_MSG_MAP(CEditListViewCtrl)
      MESSAGE_HANDLER(WM_CHAR, OnChar)
      MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
      MESSAGE_HANDLER(WM_NCDESTROY, OnNcDestroy)
   END_MSG_MAP()

protected:
   /// called to check if changes are accepted
   bool AcceptChanges()
   {
      _TCHAR szBuffer[256];
      GetWindowText(szBuffer, 256);

      NMLVDISPINFO dispinfo;
      dispinfo.hdr.hwndFrom = GetParent();
      dispinfo.hdr.idFrom = 0;
      dispinfo.hdr.code = LVN_ENDLABELEDIT;

      dispinfo.item.mask = LVIF_TEXT;
      dispinfo.item.iItem = m_nItem;
      dispinfo.item.iSubItem = m_nColumn;
      dispinfo.item.pszText = szBuffer;

      HWND hWnd = ::GetParent(GetParent());
      int nRet = ::SendMessage(hWnd, WM_NOTIFY, 0, (LPARAM)&dispinfo);

      if (nRet == 1)
         ListView_SetItemText(GetParent(), m_nItem, m_nColumn, szBuffer);

      return nRet != 0;
   }

   /// called when finished editing
   void Finish()
   {
      if (!m_bFinished)
      {
         m_bFinished = true;
         ::SetFocus(GetParent());
         DestroyWindow();
      }
   }

   // message handler

   /// called when a key is pressed
   LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      switch(wParam)
      {
      case VK_RETURN:
         if (!AcceptChanges())
         {
            bHandled = true;
            break;
         }

      case VK_ESCAPE:
         Finish();
         break;

      default:
         bHandled = false;
         break;
      }

      return 0;
   }

   /// called when edit control loses focus
   LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (!m_bFinished)
      {
         AcceptChanges();
         Finish();
      }
      return 0;
   }

   /// called as last message
   LRESULT OnNcDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      ::PostMessage(GetParent(), WM_DELETEME, 0, reinterpret_cast<LPARAM>(this));
      m_hWnd = NULL;
      return 0;
   }

protected:
   bool m_bFinished; ///< indicates if finished editing
   int m_nItem;      ///< item row
   int m_nColumn;    ///< item column
};

/// list control to let edit titles
class TrackEditListCtrl:
   public CWindowImpl<TrackEditListCtrl, CListViewCtrl>,
   public CCustomDraw<TrackEditListCtrl>
{
public:
protected:
   // message map
BEGIN_MSG_MAP(TrackEditListCtrl)
   MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLeftButtonDblClick)
   MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   MESSAGE_HANDLER(WM_DELETEME, OnDeleteMe)
   REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINLABELEDIT, OnBeginLabelEdit) // item editing
   CHAIN_MSG_MAP_ALT(CCustomDraw<TrackEditListCtrl>, 1)
END_MSG_MAP()

   /// called before painting
	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_NOTIFYITEMDRAW;
	}

   /// called before painting an item
   DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
   {
      LPNMLVCUSTOMDRAW lpNMLVCustomDraw = reinterpret_cast<LPNMLVCUSTOMDRAW>(lpNMCustomDraw);
      DWORD nItem = lpNMCustomDraw->dwItemSpec;

      extern COLORREF g_clrAlternateListColor;
      // color every other line
      if ((nItem & 1) && nItem < static_cast<DWORD>(GetItemCount()))
         lpNMLVCustomDraw->clrTextBk = g_clrAlternateListColor;

      return CDRF_DODEFAULT;
   }

   /// called when user does a left-button double-click
   LRESULT OnLeftButtonDblClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
   {
      SetFocus();

      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);

      UINT flags = 0;
      int item = HitTest(pt, &flags);

      if ((flags & LVHT_ONITEMLABEL) == 0)
         return 0;

      unsigned int xpos = 0;

      int xlparam = GET_X_LPARAM(lParam);
      int column = 0;

      while(xpos < (unsigned)xlparam)
         xpos += GetColumnWidth(column++);
      column--;

      EditItem(column, item);

      return 0;
   }

   /// called when user presses a key when the list ctrl has focus
   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // do we have the F2 key? then edit first selected item
      if (VK_F2 == (int)wParam)
      {
         int item = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
         if (item != -1)
            EditItem(1, item);
      }

      bHandled = FALSE;
      return 0;
   }

   /// starts editing an item
   void EditItem(int column, int item)
   {
      NMLVDISPINFO dispInfo;
      dispInfo.hdr.hwndFrom = m_hWnd;
      dispInfo.hdr.idFrom = static_cast<UINT_PTR>(-1);
      dispInfo.hdr.code = LVN_BEGINLABELEDIT;
      dispInfo.item.mask = 0;
      dispInfo.item.iItem = item;
      dispInfo.item.iSubItem = column;

      HWND hWnd = GetParent();
      int nRet = SendMessage(hWnd, WM_NOTIFY, 0, (LPARAM)&dispInfo);

      if (nRet != 0)
         return;

      // create edit-control
      InplaceEditCtrl* pEdit = new InplaceEditCtrl(item,column);

      RECT rect;
      GetItemRect(item, &rect, LVIR_LABEL);

      unsigned int startx = 0;
      for(int n=0; n<column; n++)
         startx += GetColumnWidth(n);

      rect.left = column == 0 ? rect.left : startx+3;
      rect.right = startx + GetColumnWidth(column);
      rect.bottom--;

      _TCHAR szBuffer[256];
      GetItemText(item,column,szBuffer,256);

      pEdit->Create(m_hWnd, rect, _T(""));
      pEdit->SetRedraw(FALSE);
      pEdit->SetWindowText(szBuffer);

      pEdit->SetFocus();
      pEdit->SetSel(0,-1);

      HFONT hFont = GetFont();
      pEdit->SetFont(hFont);

      pEdit->SetRedraw(TRUE);
   }

   /// called to dispose of the inplace edit control
   LRESULT OnDeleteMe(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      InplaceEditCtrl* pEdit = reinterpret_cast<InplaceEditCtrl*>(lParam);
      delete pEdit;
      return 0;
   }

   /// called to check if label can be started to edit
   LRESULT OnBeginLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
   {
      NMLVDISPINFO* pLvDispInfo = reinterpret_cast<NMLVDISPINFO*>(pnmh);

      int nSubItem = pLvDispInfo->item.iSubItem;
      if (nSubItem == -1)
         return 0;

      // disallow editing columns that are non-editable
      if (nSubItem != 1)
      {
         // when column 0, at least select item
         if (nSubItem == 0 && (GetStyle() & LVS_SINGLESEL) != 0)
            SelectItem(pLvDispInfo->item.iItem);
         return 1;
      }

      return 0;
   }
};


/// @}
