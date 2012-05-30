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
/// \file WinampPluginInputModule.h
/// \brief contains the winamp plugin input module definition
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include "winamp/in2.h"


/// winamp plugin input module

class WinampPluginInputModule: public InputModule
{
public:
   // ctor
   WinampPluginInputModule();
   /// dtor
   virtual ~WinampPluginInputModule();

   /// clones input module
   virtual InputModule *cloneModule();

   /// sets module handle
   void setDLLModuleHandle(HMODULE mod);

   /// returns true when input module can be configured
   virtual bool canConfigure(){ return inmod != NULL; }

   /// called to configure module
   virtual void configureModule();

   // returns the module name
   virtual CString getModuleName(){ return module_name; }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns filter string
   virtual CString getFilterString();

   // initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager &mgr,
      TrackInfo &trackinfo, SampleContainer &samples);

   // returns info about the input file
   virtual void getInfo(int &channels, int &bitrate, int &length, int &samplerate);

   // decodes samples and stores them in the sample container
   virtual int decodeSamples(SampleContainer &samples);

   // returns the number of percent done
   virtual float percentDone()
   {
      return float(__int64(samplecount))/float(__int64(numsamples))*100.f;
   }

   // called when done with decoding
   virtual void doneInput();

protected:
   /// scans the installed input modules
   void scanModules(bool init=true, LPCTSTR filename=NULL);

protected:
   /// last error occured
   CString lasterror;

   /// filter string
   CString filterstring;

   /// winamp module name
   CString module_name;

   /// input filename
   CString in_filename;

   /// current input module
   In_Module *inmod;

   /// dummy output module
   Out_Module outmod;

   /// number of samples in file
   unsigned __int64 numsamples;

   /// number of samples already decoded
   unsigned __int64 samplecount;

   /// dll module handle
   HMODULE mod;

   /// determines if mod should be freed
   bool free_mod;

   /// indicates if plugin was started
   bool started;
};


/// @}
