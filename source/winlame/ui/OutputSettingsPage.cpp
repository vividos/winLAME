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
/// \file OutputSettingsPage.cpp
/// \brief Output settings page
//
#include "StdAfx.h"
#include "OutputSettingsPage.hpp"
#include "WizardPageHost.hpp"
#include "IoCContainer.hpp"
#include "UISettings.h"
#include "InputFilesPage.hpp"
#include "InputCDPage.hpp"
#include "PresetSelectionPage.hpp"
#include "LAMESettingsPage.hpp"
#include "OggVorbisSettingsPage.hpp"
#include "LibsndfileSettingsPage.hpp"
#include "AACSettingsPage.hpp"
#include "WMASettingsPage.hpp"
#include "OpusSettingsPage.hpp"
#include "ModuleInterface.h"
#include "CommonStuff.h"

using namespace UI;

OutputSettingsPage::OutputSettingsPage(WizardPageHost& pageHost) throw()
:WizardPage(pageHost, IDD_PAGE_OUTPUT_SETTINGS, WizardPage::typeCancelBackNext),
m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
{
}

LRESULT OutputSettingsPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   m_ilIcons.Create(MAKEINTRESOURCE(IDB_BITMAP_BTNICONS), 16, 0, RGB(192, 192, 192));

   // set icons on buttons
   m_btnSelectPath.SetIcon(m_ilIcons.ExtractIcon(0));

   SetupOutputModulesList();

   LoadData();

   RefreshHistory();

   return 1;
}

LRESULT OutputSettingsPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   // store setting values
   if (!SaveData(false))
      return 1; // don't leave dialog

   SetWizardPage();

   return 0;
}

LRESULT OutputSettingsPage::OnButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData(true);

   return 0;
}

LRESULT OutputSettingsPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   SaveData(true);

   if (m_uiSettings.m_bFromInputFilesPage)
   {
      std::vector<CString> vecInputFiles;

      std::for_each(m_uiSettings.encoderjoblist.begin(), m_uiSettings.encoderjoblist.end(), [&](const EncoderJob& job)
      {
         vecInputFiles.push_back(job.InputFilename());
      });

      m_uiSettings.encoderjoblist.clear();

      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new InputFilesPage(m_pageHost, vecInputFiles)));
   }
   else
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new InputCDPage(m_pageHost)));

   return 0;
}

void OutputSettingsPage::LoadData()
{
   // initially set output dir when not set
   if (m_uiSettings.m_defaultSettings.outputdir.IsEmpty())
   {
      LPITEMIDLIST ppidl;
      ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_PERSONAL, &ppidl);
      TCHAR buffer[MAX_PATH];
      ::SHGetPathFromIDList(ppidl, buffer);
      m_uiSettings.m_defaultSettings.outputdir = buffer;
   }

   // "delete after encoding" check
   m_checkDeleteAfter.SetCheck(m_uiSettings.m_defaultSettings.delete_after_encode ? BST_CHECKED : BST_UNCHECKED);

   // output module combo box
   m_cbOutputModule.SetCurSel(m_uiSettings.output_module);

   // "use input folder as output location" check
   m_checkUseInputDir.SetCheck(m_uiSettings.out_location_use_input_dir ? BST_CHECKED : BST_UNCHECKED);

   // update edit field state
   BOOL dummy;
   OnCheckUseInputFolder(0, 0, 0, dummy);

   // "overwrite existing" check
   m_checkOverwrite.SetCheck(m_uiSettings.m_defaultSettings.overwrite_existing ? BST_CHECKED : BST_UNCHECKED);

   // "create output playlist" check
   m_checkCreatePlaylist.SetCheck(m_uiSettings.create_playlist ? BST_CHECKED : BST_UNCHECKED);

   // playlist filename
   m_ecPlaylistName.SetWindowText(m_uiSettings.playlist_filename);
   m_ecPlaylistName.EnableWindow(m_uiSettings.create_playlist ? TRUE : FALSE);
}

bool OutputSettingsPage::SaveData(bool bSilent)
{
   // get control values
   m_cbOutputPath.GetWindowText(m_uiSettings.m_defaultSettings.outputdir);

   // "delete after encoding" check
   m_uiSettings.m_defaultSettings.delete_after_encode =
      BST_CHECKED == m_checkDeleteAfter.GetCheck();

   // output module combo box
   m_uiSettings.output_module = m_cbOutputModule.GetCurSel();

   // "use input folder as output location" check
   m_uiSettings.out_location_use_input_dir =
      BST_CHECKED == m_checkUseInputDir.GetCheck();

   // "overwrite existing" check
   m_uiSettings.m_defaultSettings.overwrite_existing =
      BST_CHECKED == m_checkOverwrite.GetCheck();

   // "create output playlist" check
   m_uiSettings.create_playlist = BST_CHECKED == m_checkCreatePlaylist.GetCheck();

   // playlist filename
   m_ecPlaylistName.GetWindowText(m_uiSettings.playlist_filename);

   // do we have to check the output location string?
   if (!m_uiSettings.out_location_use_input_dir && !bSilent)
   {
      // check for empty outputdir string
      if (m_uiSettings.m_defaultSettings.outputdir.IsEmpty())
      {
         AppMessageBox(m_hWnd, IDS_OUT_OUTDIR_EMPTY, MB_OK | MB_ICONEXCLAMATION);
         return false;
      }

      Path::AddEndingBackslash(m_uiSettings.m_defaultSettings.outputdir);

      // check if output directory should be created
      CString path = m_uiSettings.m_defaultSettings.outputdir;
      path.TrimRight(_T('\\'));

      struct _stat filestat;
      if (-1 == ::_tstat(path, &filestat) && !(path.GetLength() == 2 && path[1] == ':'))
      {
         if (IDNO == AppMessageBox(m_hWnd, IDS_OUT_CREATE, MB_YESNO | MB_ICONQUESTION))
            return false;

         // create the output directory
         ::_tmkdir(m_uiSettings.m_defaultSettings.outputdir);
      }
   }

   return true;
}

void OutputSettingsPage::SetWizardPage()
{
   // find out output module id
   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();

   int modid = moduleManager.getOutputModuleID(m_uiSettings.output_module);

   // check if presets page should be inserted
   if (m_uiSettings.preset_avail)
   {
      // check if current facility has only one preset
      PresetManagerInterface& presetManager = IoCContainer::Current().Resolve<PresetManagerInterface>();

      // set facility name, which is looked up by module id
      VarMgrFacilities fac;
      presetManager.setFacility(fac.lookupName(modid));

      // do we have more than the default preset?
      if (presetManager.getPresetCount() > 0)
      {
         m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
         return;
      }
   }

   SetWizardPageByOutputModule(m_pageHost, modid);
}

void OutputSettingsPage::SetWizardPageByOutputModule(WizardPageHost& pageHost, int modid)
{
   switch (modid)
   {
   case ID_OM_LAME:
      pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new LAMESettingsPage(pageHost)));
      break;

   case ID_OM_OGGV:
      pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new UI::OggVorbisSettingsPage(pageHost)));
      break;

   case ID_OM_WAVE:
      pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new LibsndfileSettingsPage(pageHost)));
      break;

   case ID_OM_AAC:
      pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new AACSettingsPage(pageHost)));
      break;

   case ID_OM_BASSWMA:
      pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new WMASettingsPage(pageHost)));
      break;

   case ID_OM_OPUS:
      pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new OpusSettingsPage(pageHost)));
      break;

   default:
      ATLASSERT(false);
      break;
   }
}

LRESULT OutputSettingsPage::OnButtonSelectOutputPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   CString path = m_uiSettings.m_defaultSettings.outputdir;

   // lets user select a path
   if (BrowseForFolder(m_hWnd, path))
   {
      // move history entries
      m_uiSettings.outputhistory.insert(m_uiSettings.outputhistory.begin(),
         m_uiSettings.m_defaultSettings.outputdir);
      m_uiSettings.m_defaultSettings.outputdir = path;

      // update combobox
      RefreshHistory();
   }

   return 0;
}

LRESULT OutputSettingsPage::OnCheckCreatePlaylist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   BOOL check = m_checkCreatePlaylist.GetCheck() == BST_CHECKED ? TRUE : FALSE;
   m_ecPlaylistName.EnableWindow(check);

   return 0;
}

LRESULT OutputSettingsPage::OnCheckUseInputFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   BOOL check = m_checkUseInputDir.GetCheck() == BST_CHECKED ? FALSE : TRUE;
   m_cbOutputPath.EnableWindow(check);
   m_btnSelectPath.EnableWindow(check);

   return 0;
}

LRESULT OutputSettingsPage::OnOutPathSelEndOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   // on selection, remove selected item from history
   int sel = m_cbOutputPath.GetCurSel();
   if (sel > 0 && unsigned(sel) <= m_uiSettings.outputhistory.size())
   {
      m_uiSettings.outputhistory.insert(m_uiSettings.outputhistory.begin(), m_uiSettings.m_defaultSettings.outputdir);
      m_uiSettings.m_defaultSettings.outputdir = m_uiSettings.outputhistory[sel];
      m_uiSettings.outputhistory.erase(m_uiSettings.outputhistory.begin()+sel);
   }

   RefreshHistory();
   return 0;
}

void OutputSettingsPage::SetupOutputModulesList()
{
   // query module names from encoder interface
   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
   int max = moduleManager.getOutputModuleCount();
   for (int i = 0; i < max; i++)
      m_cbOutputModule.AddString(moduleManager.getOutputModuleName(i));
}

void OutputSettingsPage::RefreshHistory()
{
   m_cbOutputPath.ResetContent();

   // set output path as the first in list
   m_cbOutputPath.AddString(m_uiSettings.m_defaultSettings.outputdir);

   // output directory history
   int max = m_uiSettings.outputhistory.size();
   for (int i = 0; i<max; i++)
      m_cbOutputPath.AddString(m_uiSettings.outputhistory[i]);

   m_cbOutputPath.SetCurSel(0);
}
