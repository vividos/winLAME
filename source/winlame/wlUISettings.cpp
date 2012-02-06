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

   $Id: wlUISettings.cpp,v 1.23 2011/02/15 19:16:14 vividos Exp $

*/
/*! \file wlUISettings.cpp

   \brief contains functions to read and store the general UI settings in the registry

*/

// needed includes
#include "stdafx.h"
#include "wlUIinterface.h"
#include <atlbase.h>
#include <stdio.h>
#include <sys/stat.h>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



// constants

//! registry root path
LPCTSTR wlRegistryRoot = _T("Software\\winLAME");

LPCTSTR wlOutputPath = _T("OutputPath");
LPCTSTR wlInputOutputSameFolder = _T("InputOutputSameFolder");
LPCTSTR wlLastInputPath = _T("LastInputPath");
LPCTSTR wlDeleteAfterEncode = _T("DeleteAfterEncode");
LPCTSTR wlHideAdvancedLAME = _T("HideAdvancedLAME");
LPCTSTR wlOverwriteExisting = _T("OverwriteExisting");
LPCTSTR wlWarnLossyTrans = _T("WarnLossyTranscoding");
LPCTSTR wlActionAfterEncoding = _T("ActionAfterEncoding");
LPCTSTR wlCdripAutostartEncoding = _T("CDExtractAutostartEncoding");
LPCTSTR wlCdripTempFolder = _T("CDExtractTempFolder");
LPCTSTR wlOutputPathHistory = _T("OutputPathHistory%02u");
LPCTSTR wlFreedbServer = _T("FreedbServer");
LPCTSTR wlFreedbUsername = _T("FreedbUsername");
LPCTSTR wlDiscInfosCdplayerIni = _T("StoreDiscInfosInCdplayerIni");
LPCTSTR wlLanguageId = _T("LanguageId");


// wlUISettings methods

wlUISettings::wlUISettings()
:delete_after_encode(false),
   output_module(0),
   out_location_use_input_dir(false),
   preset_avail(false),
   hide_advanced_lame(true),
   thread_prio(0),
   overwrite_existing(true),
   warn_lossy_transcoding(true),
   after_encoding_action(-1),
   create_playlist(false),
   playlist_filename(MAKEINTRESOURCE(IDS_GENERAL_PLAYLIST_FILENAME)),
   cdrip_autostart_encoding(true),
   last_page_was_cdrip_page(false),
   freedb_server(_T("freedb.freedb.org")),
   freedb_username(_T("default")),
   store_disc_infos_cdplayer_ini(true),
   language_id(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))
{
   ::GetTempPath(MAX_PATH, cdrip_temp_folder.GetBuffer(MAX_PATH));
   cdrip_temp_folder.ReleaseBuffer();
}

#pragma warning(push)
#pragma warning(disable: 4996) // 'ATL::CRegKey::QueryValue': CRegKey::QueryValue(TCHAR *value, TCHAR *valueName) has been superseded by CRegKey::QueryStringValue and CRegKey::QueryMultiStringValue

void ReadStringValue(CRegKey& regKey, LPCTSTR pszName, UINT uiMaxLength, CString& cszValue)
{
   std::vector<TCHAR> buffer(uiMaxLength, 0);
   DWORD count = uiMaxLength;
   if (ERROR_SUCCESS==regKey.QueryValue(&buffer[0], pszName, &count))
      cszValue = &buffer[0];
}

void ReadIntValue(CRegKey& regKey, LPCTSTR pszName, int& iValue)
{
   DWORD value = 0;
   if (ERROR_SUCCESS==regKey.QueryValue(value,wlActionAfterEncoding))
      iValue = static_cast<int>(value);
}

void ReadUIntValue(CRegKey& regKey, LPCTSTR pszName, UINT& uiValue)
{
   DWORD value = 0;
   if (ERROR_SUCCESS==regKey.QueryValue(value,wlActionAfterEncoding))
      uiValue = value;
}

void ReadBooleanValue(CRegKey& regKey, LPCTSTR pszName, bool& bValue)
{
   DWORD value;
   if (ERROR_SUCCESS==regKey.QueryValue(value, pszName))
      bValue = value==1;
}
#pragma warning(pop)

void wlUISettings::ReadSettings()
{
   // open root key
   CRegKey regRoot;
   if (ERROR_SUCCESS!=regRoot.Open(HKEY_CURRENT_USER,wlRegistryRoot,KEY_READ))
      return;

   // read output path
   ReadStringValue(regRoot, wlOutputPath, MAX_PATH, outputdir);

   // read last input path
   CString cszLastInputPath;
   ReadStringValue(regRoot, wlLastInputPath, MAX_PATH, cszLastInputPath);
   if (!cszLastInputPath.IsEmpty())
      lastinputpath = cszLastInputPath;

   // read "use input file's folder as output location" value
   ReadBooleanValue(regRoot, wlInputOutputSameFolder, out_location_use_input_dir);

   // read "delete after encode" value
   ReadBooleanValue(regRoot, wlDeleteAfterEncode, delete_after_encode);

   // read "hide advanced" value
   ReadBooleanValue(regRoot, wlHideAdvancedLAME, hide_advanced_lame);

   // read "overwrite existing" value
   ReadBooleanValue(regRoot, wlOverwriteExisting, overwrite_existing);

   // read "warn about lossy transcoding" value
   ReadBooleanValue(regRoot, wlWarnLossyTrans, warn_lossy_transcoding);

   // read "action after encoding" value
   ReadIntValue(regRoot, wlActionAfterEncoding, after_encoding_action);

   // read "autostart after encoding" value
   ReadBooleanValue(regRoot, wlCdripAutostartEncoding, cdrip_autostart_encoding);

   // read "cd extraction temp folder"
   ReadStringValue(regRoot, wlCdripTempFolder, MAX_PATH, cdrip_temp_folder);

   // read "freedb server"
   ReadStringValue(regRoot, wlFreedbServer, MAX_PATH, freedb_server);

   // read "freedb username"
   ReadStringValue(regRoot, wlFreedbUsername, MAX_PATH, freedb_username);

   // read "store disc infos in cdplayer.ini" value
   ReadBooleanValue(regRoot, wlDiscInfosCdplayerIni, store_disc_infos_cdplayer_ini);

   // read "language id" value
   ReadUIntValue(regRoot, wlLanguageId, language_id);

   // read "output path history" entries
   outputhistory.clear();

   CString histkey, histentry;
   for(int i=0; i<10; i++)
   {
      histkey.Format(wlOutputPathHistory, i);

      histentry.Empty();
      ReadStringValue(regRoot, histkey, MAX_PATH, histentry);

      if (!histentry.IsEmpty())
      {
         // remove last slash
         histentry.TrimRight(_T('\\'));

         // check if path exists
         DWORD dwAttr = ::GetFileAttributes(histentry);
         if (INVALID_FILE_ATTRIBUTES != dwAttr && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
            outputhistory.push_back(histentry + _T("\\"));
      }
   }

   regRoot.Close();
}

void wlUISettings::StoreSettings()
{
   // open root key
   CRegKey regRoot;
   if (ERROR_SUCCESS!=regRoot.Open(HKEY_CURRENT_USER,wlRegistryRoot))
   {
      // try to create key
      if (ERROR_SUCCESS!=regRoot.Create(HKEY_CURRENT_USER,wlRegistryRoot))
         return;
   }

#pragma warning(push)
#pragma warning(disable: 4996) // 'ATL::CRegKey::QueryValue': CRegKey::QueryValue(TCHAR *value, TCHAR *valueName) has been superseded by CRegKey::QueryStringValue and CRegKey::QueryMultiStringValue

   // write output path
   regRoot.SetValue(outputdir, wlOutputPath);

   // write last input path
   regRoot.SetValue(lastinputpath.c_str(),wlLastInputPath);

   // write "use input file's folder as output location" value
   DWORD value = out_location_use_input_dir ? 1 : 0;
   regRoot.SetValue(value,wlInputOutputSameFolder);

   // write "delete after encode" value
   value = delete_after_encode ? 1 : 0;
   regRoot.SetValue(value,wlDeleteAfterEncode);

   // write "hide advanced" value
   value = hide_advanced_lame ? 1 : 0;
   regRoot.SetValue(value,wlHideAdvancedLAME);

   // write "overwrite existing" value
   value = overwrite_existing ? 1 : 0;
   regRoot.SetValue(value,wlOverwriteExisting);

   // write "warn about lossy transcoding" value
   value = warn_lossy_transcoding ? 1 : 0;
   regRoot.SetValue(value,wlWarnLossyTrans);

   // write "action after encoding" value
   value = after_encoding_action;
   regRoot.SetValue(value,wlActionAfterEncoding);

   // write "autostart after encoding" value
   value = cdrip_autostart_encoding ? 1 : 0;
   regRoot.SetValue(value,wlCdripAutostartEncoding);

   // write cd extraction temp folder
   regRoot.SetValue(cdrip_temp_folder, wlCdripTempFolder);

   // write freedb server
   regRoot.SetValue(freedb_server, wlFreedbServer);

   // write freedb username
   regRoot.SetValue(freedb_username, wlFreedbUsername);

   // write "store disc infos in cdplayer.ini" value
   value = store_disc_infos_cdplayer_ini ? 1 : 0;
   regRoot.SetValue(value,wlDiscInfosCdplayerIni);

   // write "language id" value
   value = language_id;
   regRoot.SetValue(value, wlLanguageId);

   // store "output path history" entries
   TCHAR buffer[64];
   int i,max = outputhistory.size() > 10 ? 10 : outputhistory.size();
   for(i=0; i<max; i++)
   {
      _sntprintf(buffer, 64, wlOutputPathHistory,i);
      regRoot.SetValue(outputhistory[i], buffer);
   }

   // delete the rest of the entries
   for(i=max; i<10; i++)
   {
      _sntprintf(buffer, 64, wlOutputPathHistory,i);
      regRoot.DeleteValue(buffer);
   }
#pragma warning(pop)

   regRoot.Close();
}
