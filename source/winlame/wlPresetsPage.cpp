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

   $Id: wlPresetsPage.cpp,v 1.26 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlPresetsPage.cpp

   \brief contains implementation of the preset selection page

*/

// needed includes
#include "stdafx.h"
#include "wlPresetsPage.h"
#include "wlEncoderInterface.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// static variable

int wlPresetsPage::lastindex = 1;


// wlPresetsPage methods

LRESULT wlPresetsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_PRE_BEVEL1));

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT wlPresetsPage::OnSelItemChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   CListBox listbox(hWndCtl);
   wlPresetManagerInterface *presetmgr = pui->getUISettings().preset_manager;

   // set new description
   int index = listbox.GetCurSel();
   if (index!=0)
      SetDlgItemText(IDC_PRE_DESC,presetmgr->getPresetDescription(index-1).c_str());
   else
   {
      // set default description
      CString text;
      text.LoadString(IDS_PRE_DESC_DEFAULT);
      SetDlgItemText(IDC_PRE_DESC,text);
   }

   return 0;
}

LRESULT wlPresetsPage::OnLButtonDblClk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
#ifdef _DEBUG // only in debug mode
   CListBox listbox(hWndCtl);
   int index = listbox.GetCurSel();

   // edit doubleclicked item
   if (index!=0)
      pui->getUISettings().preset_manager->editSettingsDialog(index-1);
#endif

   return 0;
}

void wlPresetsPage::OnEnterPage()
{
   CListBox listbox(GetDlgItem(IDC_PRE_LIST_PRESET));
   wlPresetManagerInterface *presetmgr = pui->getUISettings().preset_manager;

   // fill listbox
   CString cszText(MAKEINTRESOURCE(IDS_PRESETS_CUSTOM_SETTINGS));
   listbox.AddString(cszText);

   int max = presetmgr->getPresetCount();
   for(int i=0; i<max; i++)
      listbox.AddString(presetmgr->getPresetName(i).c_str());

   listbox.SetCurSel(lastindex);

   BOOL dummy;
   OnSelItemChanged(0,0,GetDlgItem(IDC_PRE_LIST_PRESET),dummy);

   // delete all pages up to the encoder page
   int pos = pui->getCurrentWizardPage()+1;

      while(pui->getWizardPageID(pos)!=IDD_DLG_ENCODE && pui->getWizardPageID(pos)!=IDD_DLG_CDRIP)
         pui->deleteWizardPage(pos);
}

extern void wlInsertWizardPages(wlUIinterface *pui,int pos);

bool wlPresetsPage::OnLeavePage()
{
   CListBox listbox(GetDlgItem(IDC_PRE_LIST_PRESET));
   lastindex = listbox.GetCurSel();

   // set default values
   wlUISettings &settings = pui->getUISettings();
   settings.preset_manager->setDefaultSettings(settings.settings_manager);

   // set selected preset
   if (lastindex!=0)
      settings.preset_manager->setSettings(lastindex-1,settings.settings_manager);

   // insert config pages depending on output selection
   wlInsertWizardPages(pui,pui->getCurrentWizardPage()+1);

   // hide all pages when hideLameSettings is set to 1 in the preset
   if (1==pui->getUISettings().settings_manager.queryValueInt(wlLameHideSettings))
   {
      int pos = pui->getCurrentWizardPage()+1;

      while(pui->getWizardPageID(pos)!=IDD_DLG_ENCODE && pui->getWizardPageID(pos)!=IDD_DLG_CDRIP)
         pui->deleteWizardPage(pos);
   }

   return true;
}
