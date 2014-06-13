/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink

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
/// \file UISettings.h
/// \brief UI settings class

/// \ingroup userinterface
/// @{

// include guard
#pragma once

// needed includes
#include "SettingsManager.h"
#include "PresetManagerInterface.h"
#include "EncoderInterface.h"
//#include "ModuleManager.h"

class ModuleManager;

/// list of filenames
typedef std::vector<EncoderJob> EncoderJobList;

/// \brief settings for encoding a file
/// \details encoding settings may differ between two files, so this struct
/// collects all those settings.
struct EncodingSettings
{
   /// ctor
   EncodingSettings();

   /// output directory
   CString outputdir;

   /// indicates if source files should be deleted after encoding
   bool delete_after_encode;

   /// indicates if existing files will be overwritten
   bool overwrite_existing;
};

/// general UI settings
struct UISettings
{
   /// ctor
   UISettings();

   /// reads settings from the registry
   void ReadSettings();

   /// stores settings in the registry
   void StoreSettings();

   /// list of encoder jobs
   EncoderJobList encoderjoblist;

   /// default encoding settings
   EncodingSettings m_defaultSettings;

   /// output directory history list
   std::vector<CString> outputhistory;

   /// last input files folder
   CString lastinputpath;

   /// warn about lossy transcoding
   bool warn_lossy_transcoding;

   /// indicates if an output playlist should be created
   bool create_playlist;

   /// playlist filename
   CString playlist_filename;

   /// action to perform after all files were encoded
   int after_encoding_action;

   /// last selected output module id
   int output_module;

   /// use input dir as output location
   bool out_location_use_input_dir;

   /// indicates if presets are available
   bool preset_avail;

   /// current filename of presets.xml file
   CString presets_filename;

   /// autostart encoding after cd ripping?
   bool cdrip_autostart_encoding;

   /// indicates that last page was cdrip page
   bool last_page_was_cdrip_page;

   /// temporary folder for cd ripping
   CString cdrip_temp_folder;

   /// freedb servername
   CString freedb_server;

   /// freedb username
   CString freedb_username;

   /// indicates if disc infos retrieved by freedb should be stored in cdplayer.ini
   bool store_disc_infos_cdplayer_ini;

   /// language id
   UINT language_id;

   /// settings manager
   SettingsManager settings_manager;
};

/// @}
