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

   $Id: wlModuleManagerImpl.h,v 1.16 2007/02/23 17:26:37 vividos Exp $

*/
/*! \file wlModuleManagerImpl.h

   \brief contains the module manager implementation class definition

*/
/*! \ingroup encoder */
/*! @{ */

// prevent multiple including
#ifndef __wlmodulemanagerimpl_h_
#define __wlmodulemanagerimpl_h_

// needed includes
#include <map>
#include "wlEncoderInterface.h"
#include "wlModuleInterface.h"


//! module manager implementation class

class wlModuleManagerImpl: public wlModuleManager
{
public:
   //! ctor
   wlModuleManagerImpl();
   //! dtor
   ~wlModuleManagerImpl();

   // info functions

   //! returns currently available filter string for open file dialog
   virtual void getFilterString(CString& filterstring);

   //! returns infos about audio file; returns false when not supported
   virtual bool getAudioFileInfo(LPCTSTR filename,
      int &length, int &bitrate, int &samplefreq, CString& errormsg);

   // input module

   //! returns the number of available input modules
   virtual int getInputModuleCount()
   {
      return in_modules.size();
   }

   //! returns the name of the input module
   virtual CString getInputModuleName(int index)
   {
      return in_modules[index]->getModuleName();
   }

   //! returns the input module ID
   virtual int getInputModuleID(int index)
   {
      return in_modules[index]->getModuleID();
   }

   //! returns the input module filter string
   virtual CString getInputModuleFilterString(int index)
   {
      return in_modules[index]->getFilterString();
   }

   //! returns input module instance
   virtual wlInputModule* getInputModuleInstance(int index)
   {
      return in_modules[index];
   }

   /*! chooses an input module suitable for opening file with given filename;
       pointer has to be deleted! */
   wlInputModule *chooseInputModule(LPCTSTR filename);

   // output module

   //! returns the number of available output modules
   virtual int getOutputModuleCount()
   {
      return out_modules.size();
   }

   //! returns the name of an output module
   virtual CString getOutputModuleName(int index)
   {
      return out_modules[index]->getModuleName();
   }

   //! returns the output module ID
   virtual int getOutputModuleID(int index)
   {
      return out_modules[index]->getModuleID();
   }

   //! retrieves a module version string
   virtual void getModuleVersionString(CString& version, int module_id, int special=0);

   //! returns output module with given module id; pointer has to be deleted!
   wlOutputModule *getOutputModule(int module_id);

protected:
   //! adds winamp input modules to in_modules list
   void addWinampModules();

protected:
   //! all available output modules
   std::vector<wlOutputModule*> out_modules;

   //! all available input modules
   std::vector<wlInputModule*> in_modules;

   //! output module ID to index mapping
   std::map<int,int> out_id_to_modidx;
};


//@}

#endif
