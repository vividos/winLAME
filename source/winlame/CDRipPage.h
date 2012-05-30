/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005 Michael Fink

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

   $Id: CDRipPage.h,v 1.6 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file CDRipPage.h

   \brief contains the output page

*/
/*! \ingroup userinterface */
/*! @{ */

// prevent multiple including
#ifndef wlcdrippage_h_
#define wlcdrippage_h_

// needed includes
#include "resource.h"
#include "PageBase.h"
#include "wlCommonStuff.h"


struct CDRipDiscInfo;
struct CDRipTrackInfo;


#define WM_LOCK_NEXT_BUTTON (WM_APP+2)


//! cd extraction page

class CDRipPage:
   public PageBase,
   public CDialogResize<CDRipPage>
{
public:
   /// ctor
   CDRipPage();

   /// dtor
   virtual ~CDRipPage();

   // resize map
BEGIN_DLGRESIZE_MAP(CDRipPage)
   DLGRESIZE_CONTROL(IDC_CDRIP_BEVEL1, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_CDRIP_PROGRESS, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_CDRIP_LIST_TRACKS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
   DLGRESIZE_CONTROL(IDC_CDRIP_CHECK_AUTOSTART_ENC, DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(CDRipPage)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
   COMMAND_HANDLER(IDC_CDRIP_START, BN_CLICKED, OnButtonStart)
   MESSAGE_HANDLER(WM_LOCK_NEXT_BUTTON, OnLockNextButton)
   CHAIN_MSG_MAP(CDialogResize<CDRipPage>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   //! inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when dialog is about to be destroyed
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when "next" button should be locked
   LRESULT OnLockNextButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      pui->lockWizardButtons(true, true);
      return 0;
   }

   //! called when the user clicks on the start
   LRESULT OnButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

   // called on entering the page
   virtual void OnEnterPage();

   // called on leaving the page
   virtual bool OnLeavePage();

   virtual bool ShouldRemovePage() const { return m_bFinishedAllTracks; }

protected:
   /// thread procedure
   static DWORD CALLBACK ThreadProc(void* pData);

   /// extracts audio
   void ExtractAudio();

   /// extracts single track
   bool ExtractTrack(CDRipDiscInfo& discinfo, CDRipTrackInfo& trackinfo, const CString& cszTempFilename);

protected:
   /// bevel line
   BevelLine bevel1;

   /// tracks list
   AlternateColorsListCtrl m_lcTracks;

   /// indicates if all tracks are finished
   bool m_bFinishedAllTracks;

   /// progress bar control
   CProgressBarCtrl m_pcProgress;

   /// stop event
   HANDLE m_hEventStop;

   /// worker thread
   HANDLE m_hWorkerThread;

   /// counter for destroy handler
   LONG m_lInDestroyHandler;
};


//@}

#endif
