//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file FinishPage.hpp
/// \brief Finish page
//
#pragma once

// includes
#include "WizardPage.hpp"
#include "BevelLine.hpp"
#include "TaskCreationHelper.hpp"
#include "resource.h"

// forward references
struct UISettings;
namespace Encoder
{
   class EncoderTask;
   class CDReadJob;
   class TrackInfo;
}

namespace UI
{

/// \brief Finish page
class FinishPage:
   public WizardPage,
   public CWinDataExchange<FinishPage>,
   public CDialogResize<FinishPage>
{
public:
   /// ctor
   FinishPage(WizardPageHost& pageHost) throw();
   /// dtor
   ~FinishPage() throw()
   {
   }

private:
   friend CDialogResize<FinishPage>;

   BEGIN_DDX_MAP(FinishPage)
      DDX_CONTROL_HANDLE(IDC_FINISH_ICON_WARN_LOSSY_TRANSCODING, m_iconLossy)
      DDX_CONTROL_HANDLE(IDC_FINISH_STATIC_WARN_LOSSY_TRANSCODING, m_staticLossy)
      DDX_CONTROL_HANDLE(IDC_FINISH_ICON_WARN_OVERWRITE_ORIGINAL, m_iconOverwrite)
      DDX_CONTROL_HANDLE(IDC_FINISH_STATIC_WARN_OVERWRITE_ORIGINAL, m_staticOverwrite)
      DDX_CONTROL(IDC_FINISH_BEVEL1, m_bevel1)
      DDX_CONTROL_HANDLE(IDC_FINISH_LIST_INPUT_TRACKS, m_listInputTracks)
      DDX_CONTROL_HANDLE(IDC_FINISH_EDIT_OUTPUT_MODULE, m_editOutputModule)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(FinishPage)
      DLGRESIZE_CONTROL(IDC_FINISH_STATIC_WARN_LOSSY_TRANSCODING, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_FINISH_STATIC_WARN_OVERWRITE_ORIGINAL, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_FINISH_BEVEL1, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_FINISH_LIST_INPUT_TRACKS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
      DLGRESIZE_CONTROL(IDC_FINISH_STATIC_OUTPUT_MODULE, DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_FINISH_EDIT_OUTPUT_MODULE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(FinishPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
      CHAIN_MSG_MAP(CDialogResize<FinishPage>)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is left with Next button
   LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when page is left with Back button
   LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// moves around and hides warning controls, depending on if they should appear
   void MoveAndHideWarnings(bool warnLossyTranscoding, bool warnOverwriteOriginal);

   /// sets up input tracks list
   void SetupInputTracksList();

   /// updates input tracks list
   void UpdateInputTracksList();

   /// updates output module name
   void UpdateOutputModule();

private:
   // controls

   // icon for "lossy transcoding" warning
   CStatic m_iconLossy;

   // static text for "lossy transcoding" warning
   CStatic m_staticLossy;

   // icon for "overwrite original" warning
   CStatic m_iconOverwrite;

   // static text for "overwrite original" warning
   CStatic m_staticOverwrite;

   /// bevel line for input files
   BevelLine m_bevel1;

   /// list of input tracks
   CListViewCtrl m_listInputTracks;

   /// task list images
   CImageList m_taskImages;

   /// list of output modules
   CEdit m_editOutputModule;

   // model

   /// task creation helper
   TaskCreationHelper m_helper;

   /// settings
   UISettings& m_uiSettings;
};

} // namespace UI
