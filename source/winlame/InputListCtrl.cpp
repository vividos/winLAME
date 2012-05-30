/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink

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

   $Id: InputListCtrl.cpp,v 1.14 2009/11/02 19:54:26 vividos Exp $

*/
/*! \file InputListCtrl.cpp

   \brief contains the methods of the input list control

*/

// needed includes
#include "stdafx.h"
#include "InputListCtrl.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/// darker color for alternate lines list control
COLORREF g_clrAlternateListColor = RGB(232,232,232);

// InputListCtrl methods

InputListCtrl::InputListCtrl()
{
   dragging=false;
   lastsortcolumn = -1;
}

InputListCtrl::~InputListCtrl()
{
   for(unsigned int i=0; i<allentries.size(); i++)
      delete allentries[i];

   allentries.clear();
}

void InputListCtrl::DeleteSelectedListItems()
{
   // deletes items from the list ctrl in reverse order
   std::vector<int> vItems;

   // first, collect all item indices
   int pos = GetNextItem(-1,LVIS_SELECTED);

   while (pos != -1)
   {
      vItems.push_back(pos);
      pos = GetNextItem(pos,LVIS_SELECTED);
   }

   // then delete them in the reverse order
   for(int i=vItems.size()-1;i>=0;i--)
      DeleteItem(vItems[i]);
}

void InputListCtrl::InsertFile(LPCTSTR filename, int icon, int samplerate,
   int bitrate, int length)
{
   // create new entry
   wlAudioFileEntry *entry = new wlAudioFileEntry;
   entry->filename = filename;
   entry->samplerate = samplerate;
   entry->bitrate = bitrate;
   entry->length = length;

   allentries.push_back(entry);

   LPCTSTR pos = _tcsrchr(filename,_T('\\'));
   if (pos==NULL)
      pos = filename;
   else
      pos++;

   // insert the filename
   LVITEM lvItem = {
      LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, GetItemCount(), 0,
         0, 0, const_cast<LPTSTR>(pos), 0, icon, reinterpret_cast<LPARAM>(entry)
   };
   InsertItem(&lvItem);

   // set the subitems
   TCHAR buffer[32];
   lvItem.mask = LVIF_TEXT;
   lvItem.pszText = buffer;

   if (samplerate!=-1)
   {
      _sntprintf(buffer, 32, _T("%u Hz"),samplerate);
      lvItem.iSubItem = 1;
      SetItem(&lvItem);
   }

   if (bitrate!=-1)
   {
      _sntprintf(buffer, 32, _T("%u kbps"),bitrate/1000);
      lvItem.iSubItem = 2;
      SetItem(&lvItem);
   }

   if (length!=-1)
   {
      _sntprintf(buffer, 32, _T("%u:%02u"),length/60,length%60);

      lvItem.iSubItem = 3;
      SetItem(&lvItem);
   }
}

LPCTSTR InputListCtrl::GetFileName(int index)
{
   wlAudioFileEntry *entry =
      reinterpret_cast<wlAudioFileEntry*>(GetItemData(index));

   return entry == NULL ? _T("") : entry->filename;
}

unsigned int InputListCtrl::GetTotalLength()
{
   unsigned int nLength = 0;
   unsigned int nMax = allentries.size();
   for(unsigned int n=0; n<nMax; n++)
      nLength += static_cast<unsigned int>(allentries[n]->length);
   return nLength;
}

int InputListCtrl::SortCompare(LPARAM lParam1, LPARAM lParam2,
   LPARAM lParamSort)
{
   // get pointers
   wlAudioFileEntry *entry1 = reinterpret_cast<wlAudioFileEntry*>(lParam1);
   wlAudioFileEntry *entry2 = reinterpret_cast<wlAudioFileEntry*>(lParam2);
   InputListCtrl *This = reinterpret_cast<InputListCtrl*>(lParamSort);

   bool result = true;

   // compare, according to column
   switch(This->sortcolumn)
   {
   case 0:
      result = _tcsicmp(entry1->filename, entry2->filename)>0;
      break;

   case 1:
      result = entry1->samplerate > entry2->samplerate;
      break;

   case 2:
      result = entry1->bitrate > entry2->bitrate;
      break;

   case 3:
      result = entry1->length > entry2->length;
      break;
   }

   // reverse when needed
   if (This->sortreverse)
      result = !result;

   return result? 1 : -1;
}

LRESULT InputListCtrl::OnReflectedNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   LPNMHDR pnmh = (LPNMHDR)lParam;
   switch(pnmh->code)
   {
   case LVN_BEGINDRAG:
      {
         // user begins to drag
         LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pnmh;
         dragFrom = pnmv->iItem;

         // set focus and selection to item to drag
         SetFocus();

         SetItemState(-1,0,LVIS_SELECTED|LVIS_FOCUSED);
         SetItemState(dragFrom,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);

         dragging = true;
         SetCapture();
      }
      break;

   case NM_CUSTOMDRAW:
      {
         // called when list items are drawn
         LPNMLVCUSTOMDRAW lpnmcd = (LPNMLVCUSTOMDRAW)pnmh;
         switch(lpnmcd->nmcd.dwDrawStage)
         {
         case CDDS_PREPAINT:
            // request notification for every item
            return CDRF_NOTIFYITEMDRAW;
            break;

         case CDDS_ITEMPREPAINT:
            {
               // change the color of every other item
               if ((lpnmcd->nmcd.dwItemSpec&1) == 1)
                  lpnmcd->clrTextBk = g_clrAlternateListColor;
               return CDRF_DODEFAULT;
            }
            break;
         }
      }
      break;

   case LVN_COLUMNCLICK:
      {
         // called when clicked on a column header
         LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pnmh;
         sortcolumn = pnmv->iSubItem;

         // sort reverse if clicked again
         sortreverse = sortcolumn == lastsortcolumn;

         // sort
         SortItems(InputListCtrl::SortCompare,
            reinterpret_cast<DWORD>(this));

         // remember column
         lastsortcolumn = sortreverse ? -1 : sortcolumn;
      }
      break;

   case LVN_DELETEITEM:
      {
         LPNMLISTVIEW lpnmListView = reinterpret_cast<LPNMLISTVIEW>(pnmh);

         wlAudioFileEntry* pEntry = reinterpret_cast<wlAudioFileEntry*>(lpnmListView->lParam);

         if (pEntry != NULL)
         {
            unsigned int nMax = allentries.size();
            for(unsigned int n=0; n<nMax; n++)
            {
               if (allentries[n] == pEntry)
               {
                  allentries.erase(allentries.begin()+n);
                  break;
               }
            }

            delete pEntry;
         }
      }
      break;
   }
   return 0;
}

LRESULT InputListCtrl::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   if (dragging)
   {
      POINT currPoint = { LOWORD(lParam), HIWORD(lParam) };
      POINT dropPoint = currPoint;

      // check if user drops item on us
      ClientToScreen(&dropPoint);
      HWND dropWnd = WindowFromPoint(dropPoint);

      if (dropWnd==m_hWnd)
      {
         // when moved to another item row, instantly move item to this row
         UINT flags;
         int hit = HitTest(currPoint,&flags);
         if (dragFrom != hit && hit != -1)
         {
            // ensure visibility
            if (hit<dragFrom)
               EnsureVisible(hit-1,FALSE);
            else
               EnsureVisible(hit+1,FALSE);

            if (GetSelectedCount()==1)
            {
               // move single item to new pos
               MoveItem(hit);
               dragFrom = hit;
            }
            else
            {
               std::vector<int> selitems;

               // collect selected items
               int item = GetNextItem(-1,LVIS_SELECTED);
               while (item != -1)
               {
                  selitems.push_back(item);
                  item = GetNextItem(item,LVIS_SELECTED);
               }

               // move all selected items to new pos
               if (selitems.size()!=0)
               {
                  // move items up
                  if (hit<selitems[0])
                     for(unsigned int i=0; i<selitems.size(); i++)
                     {
                        dragFrom = selitems[i];
                        MoveItem(hit+i);
                     }
                  else
                  if (hit>selitems[selitems.size()-1])
                  {
                     // move items down
                     for(int i=selitems.size()-1; i>=0; i--)
                     {
                        dragFrom = selitems[i];
                        MoveItem(hit+i-1);
                     }
                  }
               }
            }
         }
      }
   }
   return 0;
}

LRESULT InputListCtrl::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   if (dragging)
   {
      // end dragging
      ReleaseCapture();
      dragging=false;

      // redraw list ctrl
      Invalidate();
   }
   return 0;
}

LRESULT InputListCtrl::OnListContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // load popup menu
   HMENU menu = ::LoadMenu(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDM_INPUT_LIST_MENU));
   HMENU submenu = ::GetSubMenu(menu,0);

   // track popup menu
   int ret = TrackPopupMenu(submenu,
      TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
      LOWORD(lParam), HIWORD(lParam), 0, m_hWnd, NULL);

   switch (ret)
   {
   case IDC_INPUT_MENU_SELALL:
      {
         // select all by simulating ctrl+a
         BOOL dummy;
         OnChar(0,1,0,dummy);
      }
      break;

   case IDC_INPUT_BUTTON_PLAY:
   case IDC_INPUT_BUTTON_INFILESEL:
   case IDC_INPUT_BUTTON_DELETE:
      // simulate button press
      ::SendMessage(::GetParent(m_hWnd), WM_COMMAND,
         ret | (BN_CLICKED<<16), (LPARAM)::GetDlgItem(::GetParent(m_hWnd),ret));
      break;
   }

   ::DestroyMenu(menu);

   return 0;
}

void InputListCtrl::MoveItem(int moveTo)
{
   if(moveTo<0)
      moveTo = GetItemCount();

   // do nothing when dropping on the same item
   if (dragFrom==moveTo)
      return;

   // get info of dragged item
   TCHAR szLabel[MAX_PATH];
   LV_ITEM lvi;
   lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
   lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_STATEIMAGEMASK;
   lvi.pszText = szLabel;
   lvi.cchTextMax = MAX_PATH;
   lvi.iItem = dragFrom;
   lvi.iSubItem = 0;
   GetItem(&lvi);

   // adjust indices
   if (dragFrom < moveTo) moveTo++;
   else dragFrom++;

   // insert the dropped item
   lvi.iItem = moveTo;
   int newindex = InsertItem(&lvi);

   // fill in all of the columns
   HWND hdWnd = GetDlgItem(0);
   int columns = Header_GetItemCount(hdWnd);
   lvi.mask = LVIF_TEXT;
   lvi.iItem = moveTo;

   // copy subitems
   for(int n=1; n<columns; n++)
   {
      GetItemText(dragFrom, n, lvi.pszText, MAX_PATH);
      lvi.iSubItem = n;
      SetItem(&lvi);
   }

   // delete original item
   DeleteItem(dragFrom);
}
