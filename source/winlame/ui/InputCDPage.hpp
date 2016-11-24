//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2014 Michael Fink
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file InputCDPage.hpp
/// \brief Input CD page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "TrackEditListCtrl.h"
#include "resource.h"

// forward references
struct UISettings;
namespace Freedb
{
   struct CDInfo;
}

namespace UI
{

/// timer id for timer that checks for cd in drive
const UINT IDT_CDRIP_CHECK = 66;

/// \brief Input CD page
class InputCDPage:
   public WizardPage,
   public CWinDataExchange<InputCDPage>,
   public CDialogResize<InputCDPage>
{
public:
   /// ctor
   InputCDPage(WizardPageHost& pageHost) throw();
   /// dtor
   ~InputCDPage() throw()
   {
   }

   /// returns if CD extraction is avail
   static bool IsCDExtractionAvail() throw();

private:
   friend CDialogResize<InputCDPage>;

   BEGIN_DDX_MAP(InputCDPage)
      DDX_CONTROL_HANDLE(IDC_CDSELECT_COMBO_DRIVES, m_cbDrives)
      DDX_CONTROL(IDC_CDSELECT_LIST_TRACKS, m_lcTracks)
      DDX_CONTROL_HANDLE(IDC_CDSELECT_BUTTON_PLAY, m_buttonPlay)
      DDX_CONTROL_HANDLE(IDC_CDSELECT_BUTTON_STOP, m_buttonStop)
      DDX_CONTROL_HANDLE(IDC_CDSELECT_COMBO_GENRE, m_cbGenre)
      DDX_CONTROL_HANDLE(IDC_CDSELECT_CHECK_VARIOUS_ARTISTS, m_checkVariousArtists)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(InputCDPage)
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

   BEGIN_MSG_MAP(InputCDPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      MESSAGE_HANDLER(WM_TIMER, OnTimer)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
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
      NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT, OnEndLabelEdit)
      CHAIN_MSG_MAP(CDialogResize<InputCDPage>)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is left with Next button
   LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when resizing the dialog
   LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   LRESULT OnDriveSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnListDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
   LRESULT OnClickedButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedButtonStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedButtonFreedb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedButtonOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnClickedCheckVariousArtists(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnChangedEditCtrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
   void SetupDriveCombobox();
   void HideDriveCombobox();
   void SetupTracksList();

   /// resizes list view columns
   void ResizeListCtrlColumns(int cx);

   DWORD GetCurrentDrive();
   void RefreshCDList();
   bool ReadCdplayerIni(bool& bVarious);
   void ReadCDText(bool& bVarious);
   void CheckCD();
   void UpdateTrackManager();
   void StoreInCdplayerIni(unsigned int nDrive);
   void FreedbLookup();
   void FillListFreedbInfo(const Freedb::CDInfo& info);

private:
   // controls

   /// drives combobox (hidden when only one drive is present)
   CComboBox m_cbDrives;

   /// tracks list
   TrackEditListCtrl m_lcTracks;

   /// button to start playing
   CButton m_buttonPlay;

   /// button to stop playing
   CButton m_buttonStop;

   /// genre combobox
   CComboBox m_cbGenre;

   /// various artists checkbox
   CButton m_checkVariousArtists;

   /// current page width
   int m_pageWidth;

   // Model

   /// settings
   UISettings& m_uiSettings;

   /// indicates if a track title was edited
   bool m_bEditedTrack;

   /// indicates if drive is active (has a disc)
   bool m_bDriveActive;

   /// indicates if disc info has already been acquired
   bool m_bAcquiredDiscInfo;
};

} // namespace UI
