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
/// \file UISettings.cpp
/// \brief contains functions to read and store the general UI settings in the registry

// needed includes
#include "stdafx.h"
#include "UIinterface.h"
#include <stdio.h>
#include <sys/stat.h>

// constants

/// registry root path
LPCTSTR g_pszRegistryRoot = _T("Software\\winLAME");

LPCTSTR g_pszOutputPath = _T("OutputPath");
LPCTSTR g_pszOutputModule = _T("OutputModule");
LPCTSTR g_pszInputOutputSameFolder = _T("InputOutputSameFolder");
LPCTSTR g_pszLastInputPath = _T("LastInputPath");
LPCTSTR g_pszDeleteAfterEncode = _T("DeleteAfterEncode");
LPCTSTR g_pszHideAdvancedLAME = _T("HideAdvancedLAME");
LPCTSTR g_pszOverwriteExisting = _T("OverwriteExisting");
LPCTSTR g_pszWarnLossyTrans = _T("WarnLossyTranscoding");
LPCTSTR g_pszActionAfterEncoding = _T("ActionAfterEncoding");
LPCTSTR g_pszLastSelectedPresetIndex = _T("LastSelectedPresetIndex");
LPCTSTR g_pszCdripAutostartEncoding = _T("CDExtractAutostartEncoding");
LPCTSTR g_pszCdripTempFolder = _T("CDExtractTempFolder");
LPCTSTR g_pszOutputPathHistory = _T("OutputPathHistory%02u");
LPCTSTR g_pszFreedbServer = _T("FreedbServer");
LPCTSTR g_pszFreedbUsername = _T("FreedbUsername");
LPCTSTR g_pszDiscInfosCdplayerIni = _T("StoreDiscInfosInCdplayerIni");
LPCTSTR g_pszLanguageId = _T("LanguageId");
LPCTSTR g_pszAppMode = _T("AppMode");


// EncodingSettings methods

EncodingSettings::EncodingSettings()
:delete_after_encode(false),
overwrite_existing(true)
{
}


// UISettings methods

UISettings::UISettings()
:output_module(0),
   out_location_use_input_dir(false),
   preset_avail(false),
   m_bFromInputFilesPage(true),
   warn_lossy_transcoding(true),
   after_encoding_action(-1),
   create_playlist(false),
   playlist_filename(MAKEINTRESOURCE(IDS_GENERAL_PLAYLIST_FILENAME)),
   cdrip_autostart_encoding(true),
   m_iLastSelectedPresetIndex(0),
   last_page_was_cdrip_page(false),
   freedb_server(_T("freedb.freedb.org")),
   freedb_username(_T("default")),
   store_disc_infos_cdplayer_ini(true),
   language_id(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT)),
   m_appMode(modernMode)
{
   ::GetTempPath(MAX_PATH, cdrip_temp_folder.GetBuffer(MAX_PATH));
   cdrip_temp_folder.ReleaseBuffer();
}

#pragma warning(push)
#pragma warning(disable: 4996) // 'ATL::CRegKey::QueryValue': CRegKey::QueryValue(TCHAR *value, TCHAR *valueName) has been superseded by CRegKey::QueryStringValue and CRegKey::QueryMultiStringValue

/// reads string value from registry
void ReadStringValue(CRegKey& regKey, LPCTSTR pszName, UINT uiMaxLength, CString& cszValue)
{
   std::vector<TCHAR> buffer(uiMaxLength, 0);
   DWORD count = uiMaxLength;
   if (ERROR_SUCCESS==regKey.QueryValue(&buffer[0], pszName, &count))
      cszValue = &buffer[0];
}

/// reads int value from registry
void ReadIntValue(CRegKey& regKey, LPCTSTR pszName, int& iValue)
{
   DWORD value = 0;
   if (ERROR_SUCCESS==regKey.QueryValue(value, pszName))
      iValue = static_cast<int>(value);
}

/// reads unsigned int value from registry
void ReadUIntValue(CRegKey& regKey, LPCTSTR pszName, UINT& uiValue)
{
   DWORD value = 0;
   if (ERROR_SUCCESS==regKey.QueryValue(value, pszName))
      uiValue = value;
}

/// reads boolean value from registry
void ReadBooleanValue(CRegKey& regKey, LPCTSTR pszName, bool& bValue)
{
   DWORD value;
   if (ERROR_SUCCESS==regKey.QueryValue(value, pszName))
      bValue = value==1;
}
#pragma warning(pop)

void UISettings::ReadSettings()
{
   // open root key
   CRegKey regRoot;
   if (ERROR_SUCCESS!=regRoot.Open(HKEY_CURRENT_USER, g_pszRegistryRoot, KEY_READ))
      return;

   // read output path
   ReadStringValue(regRoot, g_pszOutputPath, MAX_PATH, m_defaultSettings.outputdir);

   // read last input path
   CString cszLastInputPath;
   ReadStringValue(regRoot, g_pszLastInputPath, MAX_PATH, cszLastInputPath);
   if (!cszLastInputPath.IsEmpty())
      lastinputpath = cszLastInputPath;

   // read "output module" value
   ReadIntValue(regRoot, g_pszOutputModule, output_module);

   // read "use input file's folder as output location" value
   ReadBooleanValue(regRoot, g_pszInputOutputSameFolder, out_location_use_input_dir);

   // read "delete after encode" value
   ReadBooleanValue(regRoot, g_pszDeleteAfterEncode, m_defaultSettings.delete_after_encode);

   // read "overwrite existing" value
   ReadBooleanValue(regRoot, g_pszOverwriteExisting, m_defaultSettings.overwrite_existing);

   // read "warn about lossy transcoding" value
   ReadBooleanValue(regRoot, g_pszWarnLossyTrans, warn_lossy_transcoding);

   // read "action after encoding" value
   ReadIntValue(regRoot, g_pszActionAfterEncoding, after_encoding_action);

   // read last selected preset index
   ReadIntValue(regRoot, g_pszLastSelectedPresetIndex, m_iLastSelectedPresetIndex);

   // read "autostart after encoding" value
   ReadBooleanValue(regRoot, g_pszCdripAutostartEncoding, cdrip_autostart_encoding);

   // read "cd extraction temp folder"
   ReadStringValue(regRoot, g_pszCdripTempFolder, MAX_PATH, cdrip_temp_folder);

   // read "freedb server"
   ReadStringValue(regRoot, g_pszFreedbServer, MAX_PATH, freedb_server);

   // read "freedb username"
   ReadStringValue(regRoot, g_pszFreedbUsername, MAX_PATH, freedb_username);

   // read "store disc infos in cdplayer.ini" value
   ReadBooleanValue(regRoot, g_pszDiscInfosCdplayerIni, store_disc_infos_cdplayer_ini);

   // read "language id" value
   ReadUIntValue(regRoot, g_pszLanguageId, language_id);

   // read "app mode" value
   UINT appMode = 1;
   ReadUIntValue(regRoot, g_pszAppMode, appMode);
   m_appMode = static_cast<ApplicationMode>(appMode);

   if (m_appMode != classicMode &&
      m_appMode != modernMode)
   {
      m_appMode = modernMode;
   }

   // read "output path history" entries
   outputhistory.clear();

   CString histkey, histentry;
   for(int i=0; i<10; i++)
   {
      histkey.Format(g_pszOutputPathHistory, i);

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

void UISettings::StoreSettings()
{
   // open root key
   CRegKey regRoot;
   if (ERROR_SUCCESS!=regRoot.Open(HKEY_CURRENT_USER,g_pszRegistryRoot))
   {
      // try to create key
      if (ERROR_SUCCESS!=regRoot.Create(HKEY_CURRENT_USER,g_pszRegistryRoot))
         return;
   }

#pragma warning(push)
#pragma warning(disable: 4996) // 'ATL::CRegKey::QueryValue': CRegKey::QueryValue(TCHAR *value, TCHAR *valueName) has been superseded by CRegKey::QueryStringValue and CRegKey::QueryMultiStringValue

   // write output path
   regRoot.SetValue(m_defaultSettings.outputdir, g_pszOutputPath);

   // write last input path
   regRoot.SetValue(lastinputpath, g_pszLastInputPath);

   // write "output module" value
   regRoot.SetValue(output_module, g_pszOutputModule);

   // write "use input file's folder as output location" value
   DWORD value = out_location_use_input_dir ? 1 : 0;
   regRoot.SetValue(value, g_pszInputOutputSameFolder);

   // write "delete after encode" value
   value = m_defaultSettings.delete_after_encode ? 1 : 0;
   regRoot.SetValue(value, g_pszDeleteAfterEncode);

   // write "overwrite existing" value
   value = m_defaultSettings.overwrite_existing ? 1 : 0;
   regRoot.SetValue(value, g_pszOverwriteExisting);

   // write "warn about lossy transcoding" value
   value = warn_lossy_transcoding ? 1 : 0;
   regRoot.SetValue(value, g_pszWarnLossyTrans);

   // write "action after encoding" value
   value = after_encoding_action;
   regRoot.SetValue(value, g_pszActionAfterEncoding);

   // write "autostart after encoding" value
   value = cdrip_autostart_encoding ? 1 : 0;
   regRoot.SetValue(value, g_pszCdripAutostartEncoding);

   // write last selected preset index
   regRoot.SetValue(m_iLastSelectedPresetIndex, g_pszLastSelectedPresetIndex);

   // write cd extraction temp folder
   regRoot.SetValue(cdrip_temp_folder, g_pszCdripTempFolder);

   // write freedb server
   regRoot.SetValue(freedb_server, g_pszFreedbServer);

   // write freedb username
   regRoot.SetValue(freedb_username, g_pszFreedbUsername);

   // write "store disc infos in cdplayer.ini" value
   value = store_disc_infos_cdplayer_ini ? 1 : 0;
   regRoot.SetValue(value, g_pszDiscInfosCdplayerIni);

   // write "language id" value
   value = language_id;
   regRoot.SetValue(value, g_pszLanguageId);

   // write "app mode" value
   value = (DWORD)m_appMode;
   regRoot.SetValue(value, g_pszAppMode);

   // store "output path history" entries
   TCHAR buffer[64];
   int i,max = outputhistory.size() > 10 ? 10 : outputhistory.size();
   for(i=0; i<max; i++)
   {
      _sntprintf(buffer, 64, g_pszOutputPathHistory, i);
      regRoot.SetValue(outputhistory[i], buffer);
   }

   // delete the rest of the entries
   for(i=max; i<10; i++)
   {
      _sntprintf(buffer, 64, g_pszOutputPathHistory, i);
      regRoot.DeleteValue(buffer);
   }
#pragma warning(pop)

   regRoot.Close();
}
