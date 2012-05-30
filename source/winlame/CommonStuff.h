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

*/
/// \file CommonStuff.h
/// \brief commonly used functions and UI classes
/// \details contains commonly used functions and UI classes, such as a bevel line
/// control and functions to add tooltips and to browse for a folder
/// \ingroup userinterface
/// @{

// include guard
#pragma once

#include "Resource.h"


// functions

/// \brief adds tool tips for every child dlg item found for hWnd
/// \details the tooltip text is the string resource that has the same ID value as
/// the control
void AddTooltips(HWND hWnd, CToolTipCtrl &ctrl);


/// lets the user browse for a folder
bool BrowseForFolder(HWND hParentWnd, CString &dirname,UINT captionid=0);


inline int AppMessageBox(HWND hWnd, LPCTSTR pszText, UINT nFlags)
{
   return AtlMessageBox(hWnd, pszText, IDS_APP_CAPTION, nFlags);
}

inline int AppMessageBox(HWND hWnd, UINT nResourceId, UINT nFlags)
{
   return AtlMessageBox(hWnd, nResourceId, IDS_APP_CAPTION, nFlags);
}


/// bevel line class

class BevelLine: public CWindowImpl<BevelLine>
{
public:
BEGIN_MSG_MAP(BevelLine)
   MESSAGE_HANDLER(WM_PAINT, OnPaint)
END_MSG_MAP()

   /// paints the bevel line
   LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


/// wrapper for a spin button control that "snaps" to fixed values when the
/// up or down button is pressed
class FixedValueSpinButtonCtrl: public CWindowImpl<FixedValueSpinButtonCtrl>
{
public:
   /// ctor
   FixedValueSpinButtonCtrl(){ values = NULL; arrsize = 0; }
   /// dtor
   virtual ~FixedValueSpinButtonCtrl(){ delete values; }

   // message map
BEGIN_MSG_MAP(FixedValueSpinButtonCtrl)
   MESSAGE_HANDLER(OCM_NOTIFY, OnReflectedNotify) // reflected notifications
END_MSG_MAP()

   /// called when a notification was reflected back to the control
   LRESULT OnReflectedNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // we got a reflected notification
      NMHDR *lpnmh = (LPNMHDR)lParam;

      switch (lpnmh->code)
      {
         // the position changed by delta
      case UDN_DELTAPOS:
         {
            NMUPDOWN *lpnmud = (LPNMUPDOWN)lpnmh;

            bool up = lpnmud->iDelta > 0;
            int value = lpnmud->iPos;
            int max = arrsize;
            int lastIndex = 0;

            // search position in the values vector
            for(int i=0;i<max;i++)
               if (values[i] <= value)
                  lastIndex = i;

            // up or down?
            if (up) lastIndex++;
            else
               if (values[lastIndex] == value)
                  lastIndex--;

            // adjust iDelta value
            if (lastIndex < max && lastIndex > -1)
            {
               value = values[lastIndex];
               lpnmud->iDelta = value - lpnmud->iPos;
            }
         }
         break;

      default:
         bHandled = FALSE;
         break;
      }
      return 0;
   }

   /// sets the fixed integer values to use
   void SetFixedValues(const int *pvalues, int size)
   {
      // allocate memory
      delete[] values;
      values = new int[arrsize = size];

      memcpy(values,pvalues,size*sizeof(int));

      // search biggest and smallest value
      int smallest=0,biggest=0;

      for(int i=0;i<size;i++)
      {
         if (pvalues[i]<pvalues[smallest]) smallest=i;
         if (pvalues[i]>pvalues[biggest]) biggest=i;
      }

      // set 32 bit range
      ::SendMessage(m_hWnd, UDM_SETRANGE32, (WPARAM)pvalues[smallest], (LPARAM)pvalues[biggest]);
   }

   /// finds nearest value of the value passed
   int FindNearest(int value)
   {
      if (arrsize==0) return value;

      int newval=value,dist=0x7fffffff;
      for(int i=0;i<arrsize;i++)
      {
         if (abs(values[i]-value)<dist)
         {
            dist = abs(values[i]-value);
            newval = values[i];

            if (dist==0)
               break;
         }
      }
      return newval;
   }

   /// sets the buddy window for a spin button control
   HWND SetBuddy(HWND hWnd)
   {
      return (HWND)::SendMessage(m_hWnd, UDM_SETBUDDY, (WPARAM)hWnd, 0L);
   }

   /// sets the current position for a spin button control
   int SetPos(int nPos)
   {
      return (int)(short)LOWORD( ::SendMessage(m_hWnd, UDM_SETPOS, 0, MAKELPARAM(nPos, 0)) );
   }

protected:
   /// vector containing all possible values
   int *values;

   /// size of the vector
   int arrsize;
};


class AlternateColorsListCtrl:
   public CWindowImpl<AlternateColorsListCtrl, CListViewCtrl>,
   public CCustomDraw<AlternateColorsListCtrl>
{
public:
protected:
   // message map
BEGIN_MSG_MAP(AlternateColorsListCtrl)
   CHAIN_MSG_MAP_ALT(CCustomDraw<AlternateColorsListCtrl>, 1)
END_MSG_MAP()

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_NOTIFYITEMDRAW;
	}

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
};


/// @}
