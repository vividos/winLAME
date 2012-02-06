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

   $Id: wlPageBase.h,v 1.17 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlPageBase.h

   \brief wlPageBase is a common base class for all wizard child dialogs

   a bit of tweaking must be used to get this work; IDD is not an enum
   but must be filled in with the real dialog id in the constructor of
   the derived class

*/
/*! \ingroup userinterface */
/*! @{ */

// prevent multiple including
#ifndef __wlpagebase_h_
#define __wlpagebase_h_

// needed includes
#include "wlUIinterface.h"


//! common page base class

class wlPageBase:
   public CDialogImpl<wlPageBase>,
   public CWinDataExchange<wlPageBase>
{
public:
   //! ctor
   wlPageBase(){ pui=NULL; helpID=0; }

   //! dtor
   virtual ~wlPageBase(){}

   //! dialog id
   int IDD;

   //! caption string resource id
   int captionID;

   //! description string resource id
   int descID;

   //! html help path string id
   int helpID;

   //! pointer to the ui interface
   wlUIinterface *pui;

   //! called when leaving the page; returns if we really should leave
   virtual bool OnLeavePage(){ return true; }

   //! called on entering the page
   virtual void OnEnterPage(){}

   //! returns if page should be removed
   virtual bool ShouldRemovePage() const { return false; }

   //! creates the dialog; overlaps the version in CDialogImpl<T>
   HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
   {
      // copied from ATL
      ATLASSERT(m_hWnd == NULL);
      _Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT<CWindow>*)this);
#ifdef _DEBUG
      m_bModal = false;
#endif //_DEBUG
      HWND hWnd = ::CreateDialogParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD),
         hWndParent, (DLGPROC)CDialogImplBaseT<CWindow>::StartDialogProc, dwInitParam);
      ATLASSERT(m_hWnd == hWnd);
      return hWnd;
   }
};


//@}

#endif
