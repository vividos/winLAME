/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2012 Michael Fink

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
/// \file ModuleManager.h
/// \brief Module Manager interface

/// \ingroup encoder
/// @{

// include guard
#pragma once

// forward references
class InputModule;

/// module manager interface
class ModuleManager
{
public:
   /// dtor
   virtual ~ModuleManager() throw() {}

   /// returns new encoder object; destroy with delete operator
   static ModuleManager* getNewModuleManager();

   // info functions

   /// returns currently available filter string for open file dialog
   virtual void getFilterString(CString& filterstring)=0;

   /// returns infos about audio file; returns false when not supported
   virtual bool getAudioFileInfo(LPCTSTR filename,
      int& length, int& bitrate, int& samplefreq, CString& errormsg)=0;

   // module functions

   /// returns the number of available input modules
   virtual int getInputModuleCount()=0;

   /// returns the name of the input module
   virtual CString getInputModuleName(int index)=0;

   /// returns the input module ID
   virtual int getInputModuleID(int index)=0;

   /// returns the input module filter string
   virtual CString getInputModuleFilterString(int index)=0;

   /// returns the input module filter string
   virtual InputModule* getInputModuleInstance(int index)=0;

   /// returns the number of available output modules
   virtual int getOutputModuleCount()=0;

   /// returns the name of the output module
   virtual CString getOutputModuleName(int index)=0;

   /// returns the output module ID
   virtual int getOutputModuleID(int index)=0;

   /// retrieves a module version string
   virtual void getModuleVersionString(CString& version, int module_id, int special=0)=0;
};

/// @}
