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
/// \file LibsndfileSettingsPage.cpp
/// \brief Libsndfile settings page
//
#include "StdAfx.h"
#include "LibsndfileSettingsPage.hpp"
#include "WizardPageHost.hpp"
#include "IoCContainer.hpp"
#include "UISettings.h"
#include "OutputSettingsPage.hpp"
#include "PresetSelectionPage.hpp"
#include "FinishPage.hpp"

using namespace UI;

LRESULT LibsndfileSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   // fill output format list
   m_cbOutputFormat.AddString(_T("16 bit PCM"));
   m_cbOutputFormat.AddString(_T("24 bit PCM"));
   m_cbOutputFormat.AddString(_T("32 bit PCM"));
   m_cbOutputFormat.AddString(_T("32 bit Float"));

   // fill file format list
   m_cbFileFormat.AddString(_T("Microsoft WAV"));
   m_cbFileFormat.AddString(_T("Apple AIFF"));
   m_cbFileFormat.AddString(_T("SoundForge W64"));

   LoadData();

   return 1;
}

LRESULT LibsndfileSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new FinishPage(m_pageHost)));

   return 0;
}

LRESULT LibsndfileSettingsPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   return 0;
}

LRESULT LibsndfileSettingsPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData();

   PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

   if (m_uiSettings.preset_avail && presetManager.getPresetCount() > 0)
      m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   else
      m_pageHost.SetWizardPage(boost::shared_ptr<WizardPage>(new OutputSettingsPage(m_pageHost)));

   return 0;
}

void LibsndfileSettingsPage::LoadData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // set raw audio check
   m_checkRawAudio.SetCheck(mgr.queryValueInt(WaveRawAudioFile) == 1 ? BST_CHECKED : BST_UNCHECKED);

   // set writewavex check
   m_checkWavex.SetCheck(mgr.queryValueInt(WaveWriteWavEx) == 1 ? BST_CHECKED : BST_UNCHECKED);

   // set output format
   m_cbOutputFormat.SetCurSel(mgr.queryValueInt(WaveOutputFormat));
   m_cbFileFormat.SetCurSel(mgr.queryValueInt(WaveFileFormat));
}

void LibsndfileSettingsPage::SaveData()
{
   SettingsManager& mgr = m_uiSettings.settings_manager;

   // get raw audio check
   mgr.setValue(WaveRawAudioFile, BST_CHECKED == m_checkRawAudio.GetCheck() ? 1 : 0);

   // get writewavex check
   mgr.setValue(WaveWriteWavEx, BST_CHECKED == m_checkWavex.GetCheck() ? 1 : 0);

   // get output format
   int value = m_cbOutputFormat.GetCurSel();
   mgr.setValue(WaveOutputFormat, value);

   // get file format
   value = m_cbFileFormat.GetCurSel();
   mgr.setValue(WaveFileFormat, value);
}
