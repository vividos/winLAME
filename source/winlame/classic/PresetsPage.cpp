/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2006 Michael Fink

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
/// \file PresetsPage.cpp
/// \brief contains implementation of the preset selection page

// needed includes
#include "stdafx.h"
#include "PresetsPage.h"
#include "EncoderInterface.h"

// static variable

int PresetsPage::lastindex = 1;


// PresetsPage methods

LRESULT PresetsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_PRE_BEVEL1));

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT PresetsPage::OnSelItemChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   CListBox listbox(hWndCtl);

   // set new description
   int index = listbox.GetCurSel();
   if (index == LB_ERR)
      return 0;

   if (index > 0)
      SetDlgItemText(IDC_PRE_DESC, m_presetManager.getPresetDescription(index-1).c_str());
   else
   {
      // set default description
      CString text;
      text.LoadString(IDS_PRE_DESC_DEFAULT);
      SetDlgItemText(IDC_PRE_DESC,text);
   }

   return 0;
}

LRESULT PresetsPage::OnLButtonDblClk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
#ifdef _DEBUG // only in debug mode
   CListBox listbox(hWndCtl);
   int index = listbox.GetCurSel();

   // edit doubleclicked item
   if (index!=0)
      m_presetManager.showPropertyDialog(index-1);
#endif

   return 0;
}

void PresetsPage::OnEnterPage()
{
   CListBox listbox(GetDlgItem(IDC_PRE_LIST_PRESET));

   // fill listbox
   CString cszText(MAKEINTRESOURCE(IDS_PRESETS_CUSTOM_SETTINGS));
   listbox.AddString(cszText);

   size_t max = m_presetManager.getPresetCount();
   for(size_t i=0; i<max; i++)
      listbox.AddString(m_presetManager.getPresetName(i).c_str());

   listbox.SetCurSel(lastindex);

   BOOL dummy;
   OnSelItemChanged(0,0,GetDlgItem(IDC_PRE_LIST_PRESET),dummy);

   // delete all pages up to the encoder page
   int pos = pui->getCurrentWizardPage()+1;

      while(pui->getWizardPageID(pos)!=IDD_DLG_ENCODE && pui->getWizardPageID(pos)!=IDD_DLG_CDRIP)
         pui->deleteWizardPage(pos);
}

extern void InsertWizardPages(UIinterface *pui,int pos);

bool PresetsPage::OnLeavePage()
{
   CListBox listbox(GetDlgItem(IDC_PRE_LIST_PRESET));
   lastindex = listbox.GetCurSel();

   // set default values
   UISettings &settings = pui->getUISettings();
   m_presetManager.setDefaultSettings(settings.settings_manager);

   // set selected preset
   if (lastindex!=0)
      m_presetManager.setSettings(lastindex-1,settings.settings_manager);

   // insert config pages depending on output selection
   InsertWizardPages(pui,pui->getCurrentWizardPage()+1);

   // hide all pages when hideLameSettings is set to 1 in the preset
   if (1==pui->getUISettings().settings_manager.queryValueInt(LameHideSettings))
   {
      int pos = pui->getCurrentWizardPage()+1;

      while(pui->getWizardPageID(pos)!=IDD_DLG_ENCODE && pui->getWizardPageID(pos)!=IDD_DLG_CDRIP)
         pui->deleteWizardPage(pos);
   }

   return true;
}
