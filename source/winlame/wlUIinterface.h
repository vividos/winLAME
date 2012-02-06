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

   $Id: wlUIinterface.h,v 1.30 2011/01/21 17:50:26 vividos Exp $

*/
/*! \file wlUIinterface.h

   \brief wlUIinterface is an interface class for interacting with the UI of winLAME

*/
/*! \defgroup userinterface User Interface

   contains all classes that have to do with the user interface

*/
/*! \ingroup userinterface */
/*! @{ */

// prevent multiple including
#ifndef __wluiinterface_h_
#define __wluiinterface_h_

// needed includes
#include "wlSettingsManager.h"
#include "wlPresetManagerInterface.h"
#include "wlEncoderInterface.h"


// forward declarations
class wlPageBase;
class CLanguageResourceManager;

//! list of filenames
typedef std::vector<wlEncoderJob> wlEncoderJobList;


// classes

//! general UI settings
struct wlUISettings
{
   //! ctor
   wlUISettings();

   //! reads settings from the registry
   void ReadSettings();

   //! stores settings in the registry
   void StoreSettings();

   //! list of filenames to encode
   wlEncoderJobList encoderjoblist;

   //! output directory
   CString outputdir;

   //! output directory history list
   std::vector<CString> outputhistory;

   //! last input files folder
   std::tstring lastinputpath;

   //! indicates if source files should be deleted after encoding
   bool delete_after_encode;

   //! indicates if existing files will be overwritten
   bool overwrite_existing;

   //! warn about lossy transcoding
   bool warn_lossy_transcoding;

   //! indicates if an output playlist should be created
   bool create_playlist;

   //! playlist filename
   CString playlist_filename;

   //! action to perform after all files were encoded
   int after_encoding_action;

   //! indicates if advanced lame settings should be hidden
   bool hide_advanced_lame;

   //! last selected output module id
   int output_module;

   //! use input dir as output location
   bool out_location_use_input_dir;

   //! indicates if presets are available
   bool preset_avail;

   //! current filename of presets.xml file
   CString presets_filename;

   //! current encoder thread priority
   int thread_prio;

   //! autostart encoding after cd ripping?
   bool cdrip_autostart_encoding;

   //! indicates that last page was cdrip page
   bool last_page_was_cdrip_page;

   //! temporary folder for cd ripping
   CString cdrip_temp_folder;

   //! freedb servername
   CString freedb_server;

   //! freedb username
   CString freedb_username;

   //! indicates if disc infos retrieved by freedb should be stored in cdplayer.ini
   bool store_disc_infos_cdplayer_ini;

   //! language id
   UINT language_id;

   //! settings manager
   wlSettingsManager settings_manager;

   //! preset manager
   wlPresetManagerInterface *preset_manager;

   //! module manager
   wlModuleManager *module_manager;
};


//! ui interface
class wlUIinterface
{
public:
   //! ctor
   wlUIinterface(){}

   //! returns the currently displayed wizard page
   virtual int getCurrentWizardPage()=0;

   //! returns wizard page count
   virtual int getWizardPageCount()=0;

   //! inserts a wizard page at a given position
   virtual void insertWizardPage(int at, wlPageBase *page)=0;

   //! returns the wizard page dialog ID
   virtual int getWizardPageID(int at)=0;

   //! deletes a wizard page from the given position
   virtual void deleteWizardPage(int at)=0;

   //! sets a new current page
   virtual void setCurrentPage(int page)=0;

   //! locks or unlocks the wizard back and next buttons
   virtual void lockWizardButtons(bool lock, bool only_lock_next=false)=0;

   //! returns user interface settings struct
   virtual wlUISettings& getUISettings()=0;

   //! returns language resource manager
   virtual CLanguageResourceManager& GetLanguageResourceManager() = 0;
};


//@}

#endif
