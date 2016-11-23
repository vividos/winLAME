/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005-2007 Michael Fink

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
/// \file CDRipDlg.h
/// \brief contains the dialog for cd ripping
/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "resource.h"
#include "freedb.hpp"
#include "CommonStuff.h"
#include "UIinterface.h"
#include "TrackEditListCtrl.h"

/// timer id for timer checking for cd in drive
#define IDT_CDRIP_CHECK 66

/// cd rip dialog
class CDRipDlg:
   public CDialogImpl<CDRipDlg>,
   public CWinDataExchange<CDRipDlg>,
   public CDialogResize<CDRipDlg>
{
public:
   /// ctor
   CDRipDlg(UISettings& uiSettings, UIinterface& UIinterface);

   static bool IsCDExtractionAvail() throw();

   /// dialog id
   enum { IDD = IDD_CDSELECT };

   // resize map
BEGIN_DLGRESIZE_MAP(CDRipDlg)
   DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_CDSELECT_COMBO_DRIVES, DLSZ_SIZE_X)
   DLGRESIZE_CONTROL(IDC_CDSELECT_LIST_TRACKS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_EDIT_TITLE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_EDIT_ARTIST, DLSZ_SIZE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_EDIT_YEAR, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_COMBO_GENRE, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_CHECK_VARIOUS_ARTISTS, DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_STATIC_TITLE, DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_STATIC_ARTIST, DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_STATIC_YEAR, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_STATIC_GENRE, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   DLGRESIZE_CONTROL(IDC_CDSELECT_BUTTON_PLAY, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_CDSELECT_BUTTON_STOP, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_CDSELECT_BUTTON_FREEDB, DLSZ_MOVE_X)
   DLGRESIZE_CONTROL(IDC_CDSELECT_BUTTON_OPTIONS, DLSZ_MOVE_X)
END_DLGRESIZE_MAP()

   // message map
BEGIN_MSG_MAP(CDRipDlg)
   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   MESSAGE_HANDLER(WM_TIMER, OnTimer)
   COMMAND_HANDLER(IDOK, BN_CLICKED, OnExit)
   COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnExit)
   COMMAND_HANDLER(IDC_CDSELECT_COMBO_DRIVES, CBN_SELENDOK, OnDriveSelEndOk)
   NOTIFY_HANDLER(IDC_CDSELECT_LIST_TRACKS, NM_DBLCLK, OnListDoubleClick)
   COMMAND_HANDLER(IDC_CDSELECT_BUTTON_PLAY, BN_CLICKED, OnClickedButtonPlay)
   COMMAND_HANDLER(IDC_CDSELECT_BUTTON_STOP, BN_CLICKED, OnClickedButtonStop)
   COMMAND_HANDLER(IDC_CDSELECT_BUTTON_FREEDB, BN_CLICKED, OnClickedButtonFreedb)
   COMMAND_HANDLER(IDC_CDSELECT_BUTTON_OPTIONS, BN_CLICKED, OnClickedButtonOptions)
   COMMAND_HANDLER(IDC_CDSELECT_CHECK_VARIOUS_ARTISTS, BN_CLICKED, OnClickedCheckVariousArtists)
   COMMAND_HANDLER(IDC_CDSELECT_EDIT_TITLE, EN_CHANGE, OnChangedEditCtrl)
   COMMAND_HANDLER(IDC_CDSELECT_EDIT_ARTIST, EN_CHANGE, OnChangedEditCtrl)
   COMMAND_HANDLER(IDC_CDSELECT_EDIT_YEAR, EN_CHANGE, OnChangedEditCtrl)
   MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT, OnEndLabelEdit)
   CHAIN_MSG_MAP(CDialogResize<CDRipDlg>)
   REFLECT_NOTIFICATIONS()
END_MSG_MAP()

   // dialog data exchange map
BEGIN_DDX_MAP(CDRipDlg)
   DDX_CONTROL_HANDLE(IDC_CDSELECT_COMBO_DRIVES, m_cbDrives)
   DDX_CONTROL(IDC_CDSELECT_LIST_TRACKS, m_lcTracks)
   DDX_CONTROL_HANDLE(IDC_CDSELECT_COMBO_GENRE, m_cbGenre)
END_DDX_MAP()

   /// called to init the dialog
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called on exiting the dialog
   LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   {
      KillTimer(IDT_CDRIP_CHECK);

      if (wID == IDOK)
      {
         if (m_bEditedTrack)
            StoreInCdplayerIni(GetCurrentDrive());

         UpdateTrackManager();
      }

      // ends the modal dialog
      EndDialog(wID);
      return 0;
   }

   LRESULT OnDriveSelEndOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnListDoubleClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
   LRESULT OnClickedButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedButtonStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedButtonFreedb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedButtonOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedCheckVariousArtists(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnChangedEditCtrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// called when user presses a key
   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if (VK_RETURN == (int)wParam)
      {
         return 0;
      }

      bHandled = FALSE;
      return 0;
   }

   DWORD GetCurrentDrive();

   void RefreshCDList();
   bool ReadCdplayerIni(bool& bVarious);
   void ReadCDText(bool& bVarious);
   void CheckCD();
   void UpdateTrackManager();
   void StoreInCdplayerIni(unsigned int nDrive);
   void FreedbLookup();
   void FillListFreedbInfo(const Freedb::CDInfo& info);


protected:
   CComboBox m_cbDrives;

   TrackEditListCtrl m_lcTracks;

   CComboBox m_cbGenre;

   bool m_bEditedTrack;

   bool m_bDriveActive;

   bool m_bAcquiredDiscInfo;

   UISettings& m_uiSettings;

   UIinterface& m_UIinterface;
};


/// @}
