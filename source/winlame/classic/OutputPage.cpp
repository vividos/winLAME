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

*/
/// \file OutputPage.cpp
/// \brief contains implementation of the output settings page

// needed includes
#include "stdafx.h"
#include "OutputPage.hpp"
#include "PresetsPage.hpp"
#include "LameSimpleSettingsPage.hpp"
#include "OggVorbisSettingsPage.hpp"
#include "WaveOutputSettingsPage.hpp"
#include "AacSettingsPage.hpp"
#include "WmaOutputSettingsPage.hpp"
#include "OpusSettingsPage.hpp"
#include "EncoderInterface.hpp"
#include "CDRipPage.hpp"
#include "ModuleInterface.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

// OutputPage methods

LRESULT OutputPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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
   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
   int max = moduleManager.GetOutputModuleCount();
   for(int i=0; i<max; i++)
      SendDlgItemMessage(IDC_OUT_COMBO_OUTMODULE, CB_ADDSTRING, 0,
         (LPARAM)(LPCTSTR)moduleManager.GetOutputModuleName(i));

   // insert all possible "shutdown" actions
   UINT ActionStringIDs[] = {
      IDS_OUT_ACTION_EXIT, IDS_OUT_ACTION_SHUTDOWN, IDS_OUT_ACTION_LOGOFF,
      IDS_OUT_ACTION_HIBERNATE, IDS_OUT_ACTION_SUSPEND
   };

   CString action;
   for(int j=0; j<sizeof(ActionStringIDs)/sizeof(ActionStringIDs[0]); j++)
   {
      action.LoadString(ActionStringIDs[j]);
      SendDlgItemMessage(IDC_OUT_COMBO_FINISHED_ACTION, CB_ADDSTRING, 0,
         (LPARAM)(LPCTSTR)action);
   }

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT OutputPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ilIcons.Destroy();

   // store setting values
   UISettings &settings = pui->getUISettings();

   // get control values
   settings.m_defaultSettings.delete_after_encode = BST_CHECKED == SendDlgItemMessage(IDC_OUT_DELAFTER, BM_GETCHECK);
   settings.output_module = SendDlgItemMessage(IDC_OUT_COMBO_OUTMODULE, CB_GETCURSEL);
   settings.out_location_use_input_dir = BST_CHECKED==SendDlgItemMessage(IDC_OUT_USE_INDIR, BM_GETCHECK);

   return 0;
}

void OutputPage::RefreshHistory()
{
   UISettings &settings = pui->getUISettings();

   // reset combobox
   SendDlgItemMessage(IDC_OUT_OUTPATH, CB_RESETCONTENT);

   // set output path as the first in list
   SendDlgItemMessage(IDC_OUT_OUTPATH, CB_ADDSTRING, 0,
      (LPARAM)(LPCTSTR)settings.m_defaultSettings.outputdir);

   // output directory history
   int max=settings.outputhistory.size();
   for(int i=0; i<max; i++)
      SendDlgItemMessage(IDC_OUT_OUTPATH, CB_ADDSTRING, 0,
         (LPARAM)(LPCTSTR)settings.outputhistory[i]);

   // select first
   SendDlgItemMessage(IDC_OUT_OUTPATH, CB_SETCURSEL, 0);
}

void OutputPage::OnEnterPage()
{
   UISettings &settings = pui->getUISettings();

   // initially set output dir when not set
   if (settings.m_defaultSettings.outputdir.IsEmpty())
   {
      LPITEMIDLIST ppidl;
      ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_PERSONAL , &ppidl);
      TCHAR buffer[MAX_PATH];
      ::SHGetPathFromIDList(ppidl, buffer);
      settings.m_defaultSettings.outputdir = buffer;
   }

   // output directory
   RefreshHistory();

   // "delete after encoding" check
   SendDlgItemMessage(IDC_OUT_DELAFTER, BM_SETCHECK,
      settings.m_defaultSettings.delete_after_encode ? BST_CHECKED : BST_UNCHECKED);

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
      settings.m_defaultSettings.overwrite_existing ? BST_CHECKED : BST_UNCHECKED);

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

/// insert pages depending on selected module
void InsertWizardPages(UIinterface *pui,int pos)
{
   // find out output module id
   UISettings& settings = pui->getUISettings();
   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();

   int modid = moduleManager.GetOutputModuleID(settings.output_module);

   switch(modid)
   {
   case ID_OM_LAME:
      //pui->insertWizardPage(pos,new LameBasicSettingsPage);
      pui->insertWizardPage(pos,new LameSimpleSettingsPage);
      break;

   case ID_OM_OGGV:
      pui->insertWizardPage(pos,new OggVorbisSettingsPage);
      break;

   case ID_OM_WAVE:
      pui->insertWizardPage(pos,new WaveOutputSettingsPage);
      break;

   case ID_OM_AAC:
      pui->insertWizardPage(pos,new AacSettingsPage);
      break;

   case ID_OM_BASSWMA:
      pui->insertWizardPage(pos,new WmaOutputSettingsPage);
      break;

   case ID_OM_OPUS:
      pui->insertWizardPage(pos, new OpusSettingsPage);
      break;
      
   default:
      ATLASSERT(false);
      break;
   }
}

bool OutputPage::OnLeavePage()
{
   UISettings& settings = pui->getUISettings();

   // get control values
   GetDlgItemText(IDC_OUT_OUTPATH, settings.m_defaultSettings.outputdir.GetBuffer(MAX_PATH), MAX_PATH);
   settings.m_defaultSettings.outputdir.ReleaseBuffer();

   // "delete after encoding" check
   settings.m_defaultSettings.delete_after_encode =
      BST_CHECKED==SendDlgItemMessage(IDC_OUT_DELAFTER, BM_GETCHECK);

   // output module combo box
   settings.output_module = SendDlgItemMessage(IDC_OUT_COMBO_OUTMODULE, CB_GETCURSEL);

   // "use input folder as output location" check
   settings.out_location_use_input_dir =
      BST_CHECKED==SendDlgItemMessage(IDC_OUT_USE_INDIR, BM_GETCHECK);

   // "overwrite existing" check
   settings.m_defaultSettings.overwrite_existing =
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
      if (settings.m_defaultSettings.outputdir.IsEmpty())
      {
         AppMessageBox(m_hWnd, IDS_OUT_OUTDIR_EMPTY, MB_OK | MB_ICONEXCLAMATION);
         return false;
      }

      // check if we should add a slash
      if (_T("\\") != settings.m_defaultSettings.outputdir.Right(1))
         settings.m_defaultSettings.outputdir += _T('\\');

      // check if output directory should be created
      CString path = settings.m_defaultSettings.outputdir;
      path.TrimRight(_T('\\'));

      struct _stat filestat;
      if (-1==::_tstat(path, &filestat) && !(path.GetLength()==2 && path[1]==':'))
      {
         if (IDNO == AppMessageBox(m_hWnd, IDS_OUT_CREATE, MB_YESNO | MB_ICONQUESTION))
            return false;

         // create the output directory
         if (::_tmkdir(settings.m_defaultSettings.outputdir) != 0)
         {
            AppMessageBox(m_hWnd, IDS_OUT_CREATE_FAILED, MB_OK | MB_ICONEXCLAMATION);
            return false;
         }
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
   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();

   int modid = moduleManager.GetOutputModuleID(settings.output_module);

   // check if presets page should be inserted
   if (settings.preset_avail)
   {
      // check if current facility has only one preset
      PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

      // set facility name, which is looked up by module id
      VarMgrFacilities fac;
      presetManager.setFacility(fac.lookupName(modid));

      // do we have more than the default preset?
      if (presetManager.getPresetCount() > 0)
         pui->insertWizardPage(pos++,new PresetsPage);
   }

   // insert pages depending on selected module
   InsertWizardPages(pui,pos);

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
         pui->insertWizardPage(pos+1,new CDRipPage);
   }

   // delete playlist file
   {
      CString fname = settings.m_defaultSettings.outputdir;
      fname += settings.playlist_filename;

      _tremove(fname);
   }

   // save settings, so that output folder history gets saved
   settings.StoreSettings();

   return true;
}
