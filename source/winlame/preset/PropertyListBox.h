/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2004 Michael Fink

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
/// \file PropertyListBox.h
/// \brief contains the property editing list box
/// \ingroup preset
/// @{

// include guard
#pragma once

// needed includes
#include <string>
#include <list>

// constants

const int LEFT_BORDER = 15;


/// property manager interface

class PropertyManagerInterface
{
public:
   /// ctor
   PropertyManagerInterface(){}

   /// returns number of groups
   virtual int GetGroupCount()=0;
   /// returns specific group name
   virtual std::tstring GetGroupName(int group)=0;

   /// returns count of items in specific group
   virtual int GetItemCount(int group)=0;
   /// returns name of item in group
   virtual std::tstring GetItemName(int group, int index)=0;

   /// returns value of item
   virtual std::tstring GetItemValue(int group, int index)=0;
   /// assigns an item a new value
   virtual void SetItemValue(int group, int index, std::tstring val)=0;
};


/// property list box class

class PropertyListBox: public CWindowImpl<PropertyListBox, CListBox>
{
public:
   /// ctor
   PropertyListBox()
      :inplaceCtrl(NULL),
       readonly(true),
       propmanager(NULL)
   {
   }

   /// sets the property manager
   void SetPropertyManager(PropertyManagerInterface *mgr){ propmanager = mgr; }

   /// initializes list box
   void InitListBox(bool readonly=true);

protected:
   // message map
BEGIN_MSG_MAP(PropertyListBox)
   MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
   MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
   MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
   MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   MESSAGE_HANDLER(OCM_COMMAND, OnReflectedCommand)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// called when a command was reflected back to the control
   LRESULT OnReflectedCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when an item is drawn
   LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when the control is destroyed
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // remove inplace control
      if (inplaceCtrl)
      {
         inplaceCtrl->DestroyWindow();
         delete inplaceCtrl;
         inplaceCtrl = NULL;
      }

      return 0;
   }

   /// called when user doubleclicks on an item
   LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      ChangeGroupState(SendMessage(LB_ITEMFROMPOINT,0,lParam));
      bHandled = FALSE;
      return 0;
   }

   /// called when user presses down button on item
   LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (LOWORD(lParam) < LEFT_BORDER)
         ChangeGroupState(SendMessage(LB_ITEMFROMPOINT, 0, lParam));
      bHandled = FALSE;
      return 0;
   }

   /// called when user presses a key
   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (wParam == VK_RETURN)
         ChangeGroupState(GetCurSel());
//      else if (wParam == VK_TAB)
//         m_oPropertyManager.SetSIFocus();
//      else if (wParam == VK_DELETE)
//         OnValueChanged(L"");
      bHandled = FALSE;
      return 0;
   }

protected:
   // drawing functions

   /// draws grid line around item
   void DrawGridLine(HDC hDC, RECT &rcGrid, UINT nPropWidth);

   /// draws property texts
   void DrawPropText(HDC hDC, LPCTSTR name, RECT &rcName, LPCTSTR value,
      RECT &rcValue, UINT nItemState, UINT nItemAction, bool thin);

   /// draws cross, when a group item
   void DrawCross(HDC hDC, RECT& rect, LONG nSize, bool bPlus);

   /// called when a group state changes
   void ChangeGroupState(int nIndex);

protected:
   /// property manager
   PropertyManagerInterface *propmanager;

   /// inplace control
   CWindow *inplaceCtrl;

   /// property index of current inplace control value
   int inplaceIndex;

   /// property group of current inplace control value
   int inplaceGroup;

   /// indicates if the list box is read only
   bool readonly;
};


/// @}
