//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2007 Michael Fink
// Copyright (c) 2004 DeXT
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
/// \file UIinterface.hpp
/// \brief UIinterface is an interface class for interacting with the UI of winLAME
//
#pragma once

// needed includes
#include "SettingsManager.hpp"
#include "UISettings.hpp"

// forward declarations
class PageBase;
class LanguageResourceManager;

/// ui interface
class UIinterface
{
public:
   /// ctor
   UIinterface(){}

   /// returns the currently displayed wizard page
   virtual int getCurrentWizardPage()=0;

   /// returns wizard page count
   virtual int getWizardPageCount()=0;

   /// inserts a wizard page at a given position
   virtual void insertWizardPage(int at, PageBase *page)=0;

   /// returns the wizard page dialog ID
   virtual int getWizardPageID(int at)=0;

   /// deletes a wizard page from the given position
   virtual void deleteWizardPage(int at)=0;

   /// sets a new current page
   virtual void setCurrentPage(int page)=0;

   /// locks or unlocks the wizard back and next buttons
   virtual void lockWizardButtons(bool lock, bool only_lock_next=false)=0;

   /// returns user interface settings struct
   virtual UISettings& getUISettings()=0;

   /// returns language resource manager
   virtual LanguageResourceManager& GetLanguageResourceManager() = 0;
};
