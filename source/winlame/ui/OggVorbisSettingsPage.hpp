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
/// \file OggVorbisSettingsPage.hpp
/// \brief Ogg Vorbis settings page
//
#pragma once

// includes
#include "WizardPage.h"
#include "resource.h"
#include "BevelLine.hpp"
#include "FixedValueSpinButtonCtrl.hpp"

// forward references
struct UISettings;

namespace UI
{

/// \brief Ogg Vorbis settings page
class OggVorbisSettingsPage:
   public WizardPage,
   public CWinDataExchange<OggVorbisSettingsPage>,
   public CDialogResize<OggVorbisSettingsPage>
{
public:
   /// ctor
   OggVorbisSettingsPage(WizardPageHost& pageHost) throw()
      :WizardPage(pageHost, IDD_PAGE_OGGVORBIS_SETTINGS, WizardPage::typeCancelBackNext),
      m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
   {
   }
   /// dtor
   ~OggVorbisSettingsPage() throw()
   {
   }

private:
   friend CDialogResize<OggVorbisSettingsPage>;

   BEGIN_DDX_MAP(OggVorbisSettingsPage)
      DDX_CONTROL(IDC_OGGV_BEVEL1, m_bevel1)
      DDX_CONTROL(IDC_OGGV_BEVEL2, m_bevel2)
      DDX_CONTROL_HANDLE(IDC_OGGV_SLIDER_QUALITY, m_sliderQuality)
      DDX_CONTROL_HANDLE(IDC_OGGV_SPIN_QUICK_QUALITY, m_spinQuickQuality)
      DDX_CONTROL(IDC_OGGV_SPIN_MIN_BITRATE, m_spinBitrateMin)
      DDX_CONTROL(IDC_OGGV_SPIN_NOM_BITRATE, m_spinBitrateNominal)
      DDX_CONTROL(IDC_OGGV_SPIN_MAX_BITRATE, m_spinBitrateMax)
   END_DDX_MAP()

   BEGIN_DLGRESIZE_MAP(OggVorbisSettingsPage)
      DLGRESIZE_CONTROL(IDC_OGGV_BEVEL1, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_OGGV_SLIDER_QUALITY, DLSZ_SIZE_X)
      DLGRESIZE_CONTROL(IDC_OGGV_STATIC_QUALITY, DLSZ_MOVE_X)
      DLGRESIZE_CONTROL(IDC_OGGV_STATIC_BITRATE, DLSZ_MOVE_X)
      DLGRESIZE_CONTROL(IDC_OGGV_STATIC2, DLSZ_MOVE_X)
      DLGRESIZE_CONTROL(IDC_OGGV_BEVEL2, DLSZ_SIZE_X)
   END_DLGRESIZE_MAP()

   BEGIN_MSG_MAP(OggVorbisSettingsPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnButtonOK)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnButtonCancel)
      COMMAND_HANDLER(ID_WIZBACK, BN_CLICKED, OnButtonBack)
      COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE1, BN_CLICKED, OnSetBitrateMode)
      COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE2, BN_CLICKED, OnSetBitrateMode)
      COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE3, BN_CLICKED, OnSetBitrateMode)
      COMMAND_HANDLER(IDC_OGGV_RADIO_BRMODE4, BN_CLICKED, OnSetBitrateMode)
      COMMAND_HANDLER(IDC_OGGV_EDIT_NOM_BITRATE, EN_CHANGE, OnChangeEditNominalBitrate)
      MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
      CHAIN_MSG_MAP(CDialogResize<OggVorbisSettingsPage>)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

   // Handler prototypes:
   // LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   // LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   // LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   /// inits the page
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when page is left with Next button
   LRESULT OnButtonOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when page is left with Cancel button
   LRESULT OnButtonCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when page is left with Back button
   LRESULT OnButtonBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when the bitrate mode changes
   LRESULT OnSetBitrateMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when the text of the "nominal bitrate" edit control changes
   LRESULT OnChangeEditNominalBitrate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when slider is moved
   LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// updates controls when bitrate mode changes
   void UpdateBitrateMode(int pos, bool init = false);

   /// updates quality value
   void UpdateQuality();

   /// user clicked on quick quality spin control
   void OnQuickQualitySpin(WORD wCount, WORD wType);

   /// loads settings data into controls
   void LoadData();

   /// saves settings data from controls
   void SaveData();

private:
   // controls

   CTrackBarCtrl m_sliderQuality;                  ///< quality slider
   CUpDownCtrl m_spinQuickQuality;                 ///< quality spin control
   FixedValueSpinButtonCtrl m_spinBitrateNominal;  ///< nominal bitrate spin button control
   FixedValueSpinButtonCtrl m_spinBitrateMin;      ///< min bitrate spin button control
   FixedValueSpinButtonCtrl m_spinBitrateMax;      ///< max bitrate spin button control
   BevelLine m_bevel1; ///< bevel line
   BevelLine m_bevel2; ///< bevel line

   // model

   /// settings
   UISettings& m_uiSettings;
};

} // namespace UI
