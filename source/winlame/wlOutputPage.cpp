/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink
   Copyright (c) 2004 DeXT

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

   $Id: wlOutputPage.cpp,v 1.35 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlOutputPage.cpp

   \brief contains implementation of the output settings page

*/

// needed includes
#include "stdafx.h"
#include "wlOutputPage.h"
#include "wlPresetsPage.h"
#include "wlLameSimpleSettingsPage.h"
#include "wlOggVorbisSettingsPage.h"
#include "wlWaveOutputSettingsPage.h"
#include "wlAacSettingsPage.h"
#include "wlWmaOutputSettingsPage.h"
#include "wlEncoderInterface.h"
#include "wlCDRipPage.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// wlOutputPage methods

LRESULT wlOutputPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_OUT_BEVEL1));
   bevel2.SubclassWindow(GetDlgItem(IDC_OUT_BEVEL2));
   bevel3.SubclassWindow(GetDlgItem(IDC_OUT_BEVEL3));

   // create the image list
   ilIcons.Create(MAKEINTRESOURCE(IDB_BITMAP_BTNICONS),16,0,RGB(192,192,192));

   // set icons on buttons
   SendDlgItemMessage(IDC_OUT_SELECTPATH, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(0) );

   // query module names from encoder interface
   wlModuleManager *modmgr = pui->getUISettings().module_manager;
   int max = modmgr->getOutputModuleCount();
   for(int i=0; i<max; i++)
      SendDlgItemMessage(IDC_OUT_COMBO_OUTMODULE, CB_ADDSTRING, 0,
         (LPARAM)(LPCTSTR)modmgr->getOutputModuleName(i));

   // insert all possible "shutdown" actions
   UINT wlActionStringIDs[] = {
      IDS_OUT_ACTION_EXIT, IDS_OUT_ACTION_SHUTDOWN, IDS_OUT_ACTION_LOGOFF,
      IDS_OUT_ACTION_HIBERNATE, IDS_OUT_ACTION_SUSPEND
   };

   CString action;
   for(int j=0; j<sizeof(wlActionStringIDs)/sizeof(wlActionStringIDs[0]); j++)
   {
      action.LoadString(wlActionStringIDs[j]);
      SendDlgItemMessage(IDC_OUT_COMBO_FINISHED_ACTION, CB_ADDSTRING, 0,
         (LPARAM)(LPCTSTR)action);
   }

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT wlOutputPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ilIcons.Destroy();

   // store setting values
   wlUISettings &settings = pui->getUISettings();

   // get control values
   settings.delete_after_encode = BST_CHECKED==SendDlgItemMessage(IDC_OUT_DELAFTER, BM_GETCHECK);
   settings.output_module = SendDlgItemMessage(IDC_OUT_COMBO_OUTMODULE, CB_GETCURSEL);
   settings.out_location_use_input_dir = BST_CHECKED==SendDlgItemMessage(IDC_OUT_USE_INDIR, BM_GETCHECK);

   return 0;
}

void wlOutputPage::RefreshHistory()
{
   wlUISettings &settings = pui->getUISettings();

   // reset combobox
   SendDlgItemMessage(IDC_OUT_OUTPATH, CB_RESETCONTENT);

   // set output path as the first in list
   SendDlgItemMessage(IDC_OUT_OUTPATH, CB_ADDSTRING, 0,
      (LPARAM)(LPCTSTR)settings.outputdir);

   // output directory history
   int max=settings.outputhistory.size();
   for(int i=0; i<max; i++)
      SendDlgItemMessage(IDC_OUT_OUTPATH, CB_ADDSTRING, 0,
         (LPARAM)(LPCTSTR)settings.outputhistory[i]);

   // select first
   SendDlgItemMessage(IDC_OUT_OUTPATH, CB_SETCURSEL, 0);
}

void wlOutputPage::OnEnterPage()
{
   wlUISettings &settings = pui->getUISettings();

   // initially set output dir when not set
   if (settings.outputdir.IsEmpty())
   {
      LPITEMIDLIST ppidl;
      ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_PERSONAL , &ppidl);
      TCHAR buffer[MAX_PATH];
      ::SHGetPathFromIDList(ppidl, buffer);
      settings.outputdir = buffer;
   }

   // output directory
   RefreshHistory();

   // "delete after encoding" check
   SendDlgItemMessage(IDC_OUT_DELAFTER, BM_SETCHECK,
      settings.delete_after_encode ? BST_CHECKED : BST_UNCHECKED);

   // output module combo box
   SendDlgItemMessage(IDC_OUT_COMBO_OUTMODULE, CB_SETCURSEL, settings.output_module);

   // "use input folder as output location" check
   SendDlgItemMessage(IDC_OUT_USE_INDIR, BM_SETCHECK,
      settings.out_location_use_input_dir ? BST_CHECKED : BST_UNCHECKED);

   // update edit field state
   BOOL dummy;
   OnCheckUseInputFolder(0,0,0,dummy);

   // "overwrite existing" check
   SendDlgItemMessage(IDC_OUT_CHECK_OVERWRITE, BM_SETCHECK,
      settings.overwrite_existing ? BST_CHECKED : BST_UNCHECKED);

   // "warn about lossy transcoding" check
   SendDlgItemMessage(IDC_OUT_CHECK_WARN, BM_SETCHECK,
      settings.warn_lossy_transcoding ? BST_CHECKED : BST_UNCHECKED);

   // "create output playlist" check
   SendDlgItemMessage(IDC_OUT_CREATEPLAYLIST, BM_SETCHECK,
      settings.create_playlist ? BST_CHECKED : BST_UNCHECKED);

   // playlist filename
   SetDlgItemText(IDC_OUT_PLAYLISTNAME,settings.playlist_filename);
   ::EnableWindow(GetDlgItem(IDC_OUT_PLAYLISTNAME),
      settings.create_playlist ? TRUE: FALSE);

   // "after encoding" check
   int value = settings.after_encoding_action;
   SendDlgItemMessage(IDC_OUT_CHECK_FINISHED_ACTION, BM_SETCHECK,
      value > 0 ? BST_CHECKED : BST_UNCHECKED);

   ::EnableWindow(GetDlgItem(IDC_OUT_COMBO_FINISHED_ACTION),
      value > 0 ? TRUE: FALSE);

   // combo box
   SendDlgItemMessage(IDC_OUT_COMBO_FINISHED_ACTION, CB_SETCURSEL, abs(value)-1);

   // delete all pages up to the encoder page
   int pos = pui->getCurrentWizardPage()+1;

      while(pui->getWizardPageID(pos)!=IDD_DLG_ENCODE && pui->getWizardPageID(pos)!=IDD_DLG_CDRIP)
         pui->deleteWizardPage(pos);
}

//! insert pages depending on selected module
void wlInsertWizardPages(wlUIinterface *pui,int pos)
{
   // find out output module id
   wlUISettings &settings = pui->getUISettings();
   int modid = settings.module_manager->getOutputModuleID(settings.output_module);

   switch(modid)
   {
   case ID_OM_LAME:
      //pui->insertWizardPage(pos,new wlLameBasicSettingsPage);
      pui->insertWizardPage(pos,new wlLameSimpleSettingsPage);
      break;

   case ID_OM_OGGV:
      pui->insertWizardPage(pos,new wlOggVorbisSettingsPage);
      break;

   case ID_OM_WAVE:
      pui->insertWizardPage(pos,new wlWaveOutputSettingsPage);
      break;

   case ID_OM_AAC:
      pui->insertWizardPage(pos,new wlAacSettingsPage);
      break;

   case ID_OM_BASSWMA:
      pui->insertWizardPage(pos,new wlWmaOutputSettingsPage);
      break;
   }
}

bool wlOutputPage::OnLeavePage()
{
   wlUISettings& settings = pui->getUISettings();

   // get control values
   GetDlgItemText(IDC_OUT_OUTPATH, settings.outputdir.GetBuffer(MAX_PATH), MAX_PATH);
   settings.outputdir.ReleaseBuffer();

   // "delete after encoding" check
   settings.delete_after_encode =
      BST_CHECKED==SendDlgItemMessage(IDC_OUT_DELAFTER, BM_GETCHECK);

   // output module combo box
   settings.output_module = SendDlgItemMessage(IDC_OUT_COMBO_OUTMODULE, CB_GETCURSEL);

   // "use input folder as output location" check
   settings.out_location_use_input_dir =
      BST_CHECKED==SendDlgItemMessage(IDC_OUT_USE_INDIR, BM_GETCHECK);

   // "overwrite existing" check
   settings.overwrite_existing =
      BST_CHECKED==SendDlgItemMessage(IDC_OUT_CHECK_OVERWRITE, BM_GETCHECK);

   // "warn about lossy transcoding" check
   settings.warn_lossy_transcoding =
      BST_CHECKED==SendDlgItemMessage(IDC_OUT_CHECK_WARN, BM_GETCHECK);

   // "create output playlist" check
   settings.create_playlist =
      BST_CHECKED==SendDlgItemMessage(IDC_OUT_CREATEPLAYLIST, BM_GETCHECK);

   // playlist filename
   GetDlgItemText(IDC_OUT_PLAYLISTNAME, settings.playlist_filename.GetBuffer(MAX_PATH), MAX_PATH);
   settings.playlist_filename.ReleaseBuffer();

   // get "after encoding action" combo box
   int check = SendDlgItemMessage(IDC_OUT_CHECK_FINISHED_ACTION, BM_GETCHECK);

   // update action value
   settings.after_encoding_action = (check==BST_CHECKED ? 1 : -1) *
      (SendDlgItemMessage(IDC_OUT_COMBO_FINISHED_ACTION, CB_GETCURSEL) + 1);

   // do we have to check the output location string?
   if (!settings.out_location_use_input_dir)
   {
      // check for empty outputdir string
      if (settings.outputdir.IsEmpty())
      {
         wlMessageBox(m_hWnd, IDS_OUT_OUTDIR_EMPTY, MB_OK | MB_ICONEXCLAMATION);
         return false;
      }

      // check if we should add a slash
      if (_T("\\") != settings.outputdir.Right(1))
         settings.outputdir += _T('\\');

      // check if output directory should be created
      CString path = settings.outputdir;
      path.TrimRight(_T('\\'));

      USES_CONVERSION;
      struct _stat filestat;
      if (-1==::_tstat(path, &filestat) && !(path.GetLength()==2 && path[1]==':'))
      {
         if (IDNO == wlMessageBox(m_hWnd, IDS_OUT_CREATE, MB_YESNO | MB_ICONQUESTION))
            return false;

         // create the output directory
         ::_tmkdir(settings.outputdir);
      }
   }

   // insert config pages depending on output selection
   int pos = pui->getCurrentWizardPage()+1;

   // delete all next pages up to the encode page
   {
      int nMax = pui->getWizardPageCount()-1;
      for(int n=pos; n<nMax; n++)
         pui->deleteWizardPage(n);
   }

   // find out output module id
   int modid = settings.module_manager->getOutputModuleID(settings.output_module);

   // check if presets page should be inserted
   if (settings.preset_avail)
   {
      // check if current facility has only one preset

      // set facility name, which is looked up by module id
      wlVarMgrFacilities fac;
      settings.preset_manager->setFacility(fac.lookupName(modid));

      // do we have more than the default preset?
      if (settings.preset_manager->getPresetCount() > 0)
         pui->insertWizardPage(pos++,new wlPresetsPage);
   }

   // insert pages depending on selected module
   wlInsertWizardPages(pui,pos);

   // insert cd rip page when a cdrip-filename is in the file list
   {
      extern LPCTSTR g_pszCDRipPrefix;

      bool bCdRipNeeded = false;
      unsigned int nMax = settings.encoderjoblist.size();
      for(unsigned int n=0; n<nMax; n++)
      if (0 == settings.encoderjoblist[n].InputFilename().Find(g_pszCDRipPrefix))
      {
         bCdRipNeeded = true;
         break;
      }

      if (bCdRipNeeded)
         pui->insertWizardPage(pos+1,new wlCDRipPage);
   }

   // delete playlist file
   {
      CString fname = settings.outputdir;
      fname += settings.playlist_filename;

      _tremove(fname);
   }

   return true;
}
