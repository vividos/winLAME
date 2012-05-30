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

*/
/*! \file InputListCtrl.h

   \brief contains the input list control used on the input page

*/
/*! \ingroup userinterface */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "resource.h"


//! audio file entry

typedef struct
{
   //! file name
   CString filename;

   //! sample rate
   int samplerate;

   //! bitrate
   int bitrate;

   //! length
   int length;
} AudioFileEntry;


//! input file list ctrl

class InputListCtrl: public CWindowImpl<InputListCtrl, CListViewCtrl>
{
public:
   //! ctor
   InputListCtrl();
   //! dtor
   ~InputListCtrl();

   //! deletes all selected list items
   void DeleteSelectedListItems();

   //! inserts a file
   void InsertFile(LPCTSTR filename, int icon, int samplerate,
      int bitrate, int length);

   //! returns file name
   LPCTSTR GetFileName(int index);

   //! returns total length of files in list
   unsigned int GetTotalLength();

protected:
   // message map
BEGIN_MSG_MAP(InputListCtrl)
   MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
   MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   MESSAGE_HANDLER(WM_CHAR, OnChar)
   MESSAGE_HANDLER(OCM_NOTIFY, OnReflectedNotify)
   MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
   MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
   MESSAGE_HANDLER(WM_CONTEXTMENU, OnListContextMenu)
END_MSG_MAP()

   //! called when files are dropped on the list ctrl
   LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // hand over message to parent
      ::SendMessage( GetParent(), uMsg, wParam, lParam);
      return 0;
   }

   //! called when user presses a key when the list ctrl has focus
   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // do we have the delete key? notify parent
      if (VK_DELETE==(int)wParam || VK_INSERT == (int)wParam)
         ::SendMessage( GetParent(), uMsg, wParam, lParam);

      bHandled = FALSE;
      return 0;
   }

   //! called for every translated key
   LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      // do we have CTRL+A?
      if (1 == wParam)
      {
         // select all entries
         int max = GetItemCount();
         for(int i=0; i<max; i++)
            SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
      }
      bHandled = FALSE;
      return 0;
   }

   //! called when a reflected notification has to be processed
   LRESULT OnReflectedNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   //! called when user moves the mouse; used for dragging
   LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   //! called when user releases the mouse button; used for dragging
   LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   //! called when the context menu of the list should be activated
   LRESULT OnListContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   //! called to move a dragged item to a new pos
   void MoveItem(int moveTo);

   //! compare function for sorting
   static int CALLBACK SortCompare(LPARAM lParam1, LPARAM lParam2,
      LPARAM lParamSort);

protected:

   //! all audio file entries
   std::vector<AudioFileEntry*> allentries;

   //! indicates if user drags item
   bool dragging;

   //! item index to drag from
   int dragFrom;

   //! column to sort
   int sortcolumn;

   //! indicates if files are sorted backwards
   bool sortreverse;

   //! last sorted column
   int lastsortcolumn;
};


//@}
