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

   $Id: wlModuleManagerImpl.cpp,v 1.29 2009/12/17 18:49:51 vividos Exp $

*/
/*! \file wlModuleManagerImpl.cpp

   \brief contains the module manager implementation

*/

// needed includes
#include "stdafx.h"
#include <fstream>
#include <algorithm>
#include "wlModuleManagerImpl.h"
#include "wlLameOutputModule.h"
#include "wlOggVorbisOutputModule.h"
#include "wlWaveOutputModule.h"
#include "wlSndFileInputModule.h"
#include "wlMadMpegInputModule.h"
#include "wlOggVorbisInputModule.h"
#include "wlAacInputModule.h"
#include "wlAacOutputModule.h"
#include "wlWinampPluginInputModule.h"
#include "wlFlacInputModule.h"
#include "wlBassInputModule.h"
#include "wlBassWmaOutputModule.h"
#include "wlMonkeysAudioInputModule.h"
#include "wlCDReadoutModule.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



// global functions

//! max number of input modules wlGetNewInputModule can return
const int wlMaxInputModule = 8;

//! returns a new input module by index
wlInputModule *wlGetNewInputModule(int index)
{
   wlInputModule *inmod = NULL;
   switch(index)
   {
   case 0:
      inmod = new wlSndFileInputModule;
      break;
   case 1:
      inmod = new wlMadMpegInputModule;
      break;
   case 2:
      inmod = new wlOggVorbisInputModule;
      break;
   case 3:
      inmod = new wlAacInputModule;
      break;
   case 4:
      inmod = new wlMonkeysAudioInputModule;
      break;
   case 5:
      inmod = new wlFlacInputModule;
      break;
   case 6:
      inmod = new wlBassInputModule;
      break;
   case 7:
      inmod = new wlCDReadoutModule;
   }
   return inmod;
}

//! max number of output modules wlGetNewOutputModule can return
const int wlMaxOutputModule = 5;

//! returns a new output module by index
wlOutputModule *wlGetNewOutputModule(int index)
{
   wlOutputModule *outmod = NULL;
   switch(index)
   {
   case 0:
      outmod = new wlLameOutputModule;
      break;
   case 1:
      outmod = new wlOggVorbisOutputModule;
      break;
   case 2:
      outmod = new wlWaveOutputModule;
      break;
   case 3:
      outmod = new wlBassWmaOutputModule;
      break;
   case 4:
      outmod = new wlAacOutputModule;
      break;
   }
   return outmod;
}


// static wlModuleManager methods

wlModuleManager* wlModuleManager::getNewModuleManager()
{
   return new wlModuleManagerImpl;
}


// wlModuleManagerImpl methods

void wlModuleManagerImpl::getFilterString(CString& filterstring)
{
   // get all filter strings
   CString filter;
   int max = getInputModuleCount();
   for(int i=0; i<max; i++)
      filter += getInputModuleFilterString(i);

   // add an "all supported audio files" option
   CString allfilter;
   {
      CString temp(filter);
      int pos;

      do
      {
         // search for second string delimited by a | char
         pos = temp.Find(_T('|'));
         if (pos != -1)
         {
            temp.Delete(0, pos+1);
            pos = temp.Find(_T('|'));
            if (pos == -1)
               break;

            // append the filter value
            allfilter += temp.Left(pos);
            allfilter += _T(';');
            temp.Delete(0, pos+1);
         }
      }
      while(pos != -1);

      // remove last semicolon
      allfilter.TrimRight(_T(';'));
   }

   // combine filter lists
   filterstring.LoadString(IDS_FILTER_ALL_SUPPORTED);
   filterstring += _T('|');
   filterstring += allfilter;
   filterstring += _T('|');
   filterstring += filter;
}

bool wlModuleManagerImpl::getAudioFileInfo(LPCTSTR filename,
   int& length, int& bitrate, int& samplefreq, CString& errormsg)
{
   // get appropriate input module
   wlInputModule *inmod = chooseInputModule(filename);
   if (inmod==NULL)
   {
      errormsg.LoadString(IDS_ENCODER_MISSING_INPUT_MOD);
      return false;
   }

   // get infos
   wlSettingsManager dummy;
   int dummy2;

   wlTrackInfo trackinfo;
   wlSampleContainer sampcont;
   int ret = inmod->initInput(filename,dummy,trackinfo,sampcont);
   if (ret>=0)
      inmod->getInfo(dummy2,bitrate,length,samplefreq);
   else
      errormsg = inmod->getLastError();

   inmod->doneInput();

   // delete module
   delete inmod;

   return ret>=0;
}

wlModuleManagerImpl::wlModuleManagerImpl()
{
   // check which output modules are available
   int i;
   wlOutputModule *outmod;
   for(i=0; i<wlMaxOutputModule; i++)
   {
      outmod = wlGetNewOutputModule(i);

      if (outmod!=NULL)
      {
         if (outmod->isAvailable())
         {
            out_modules.push_back(outmod);
            out_id_to_modidx.insert(
               std::make_pair<int,int>(outmod->getModuleID(),i));
         }
         else
            delete outmod;
      }
   }

   // check which input modules are available
   wlInputModule *inmod;
   for(i=0; i<wlMaxInputModule; i++)
   {
      inmod = wlGetNewInputModule(i);

      if (inmod!=NULL)
      {
         if (inmod->isAvailable())
            in_modules.push_back(inmod);
         else
            delete inmod;
      }
   }

   // add extra input modules
   addWinampModules();
}

wlModuleManagerImpl::~wlModuleManagerImpl()
{
   // delete all output modules
   int i,max = out_modules.size();
   for(i=0; i<max; i++)
      delete out_modules[i];

   // delete all input modules
   max = in_modules.size();
   for(i=0; i<max; i++)
      delete in_modules[i];
}

wlInputModule *wlModuleManagerImpl::chooseInputModule(LPCTSTR filename)
{
   wlInputModule *inmod = NULL;

   // shortcut: when starting with cdrip://, return wlCDReadoutModule
   extern LPCTSTR g_pszCDRipPrefix;
   if (filename==_tcsstr(filename, g_pszCDRipPrefix))
   {
      return new wlCDReadoutModule;
   }

   // get file extension
   std::tstring extension(filename);
   std::tstring::size_type pos = extension.find_last_of('.');
   if (pos==std::tstring::npos)
      return NULL; // no extension
   extension.erase(0,pos);
   std::transform(extension.begin(),extension.end(),extension.begin(),::tolower);

   // search all filter strings for file extension
   int max = getInputModuleCount();
   for(int i=0; i<max && inmod==NULL; i++)
   {
      std::tstring filter(getInputModuleFilterString(i));
      std::transform(filter.begin(),filter.end(),filter.begin(),::tolower);

      do
      {
         // search for second string delimited by a | char
         pos = filter.find_first_of('|');
         if (pos!=std::tstring::npos)
         {
            filter.erase(0,pos+1);
            pos = filter.find_first_of('|');
            if (pos==std::tstring::npos)
               break;

            // check if the extension is in the filter wildcard pattern
            std::tstring temp(filter.c_str(),pos);
            std::tstring::size_type pos2 = temp.find(extension.c_str());

            if (pos2!=std::tstring::npos)
            {
               // clone input module
               inmod = in_modules[i]->cloneModule();
               break;
            }
         }
      }
      while(pos!=std::tstring::npos);
   }

   return inmod;
}

wlOutputModule *wlModuleManagerImpl::getOutputModule(int module_id)
{
   // search output module per id
   wlOutputModule *outmod = NULL;

   int max = getOutputModuleCount();
   for(int i=0; i<max; i++)
   if (getOutputModuleID(i)==module_id)
   {
      std::map<int,int>::iterator pos =
         out_id_to_modidx.find(module_id);

      if (pos == out_id_to_modidx.end())
         continue; // should not happen, normally

      outmod = wlGetNewOutputModule(pos->second);
      break;
   }

   return outmod;
}

void wlModuleManagerImpl::getModuleVersionString(CString& version,
   int module_id, int special)
{
   int max,i;

   // search all input modules for module ID
   max = getInputModuleCount();
   for(i=0; i<max; i++)
   if (getInputModuleID(i)==module_id)
   {
      in_modules[i]->getVersionString(version,special);
      return;
   }

   // and now all output modules
   max = getOutputModuleCount();
   for(i=0; i<max; i++)
   if (getOutputModuleID(i)==module_id)
   {
      out_modules[i]->getVersionString(version,special);
      return;
   }

   version.Empty();
}

void wlModuleManagerImpl::addWinampModules()
{
   // do search path
   CString winamp_path(_T("./"));
   CString searchpath(winamp_path);
   searchpath += _T("in_*.dll");

   // start search for input plugins
   WIN32_FIND_DATA fds;
   HANDLE hnd = NULL;

   if (INVALID_HANDLE_VALUE != (hnd = ::FindFirstFile(searchpath, &fds)))
   {
      do
      {
         CString dllname(winamp_path);
         dllname += fds.cFileName;

         // try to load module
         HMODULE dll = ::LoadLibrary(dllname);
         if (dll!=NULL)
         {
            wlWinampPluginInputModule *mod = new wlWinampPluginInputModule;
            mod->setDLLModuleHandle(dll);

            // module available?
            if (mod->isAvailable())
            {
               // then remember it
               in_modules.push_back(mod);

               ATLTRACE(_T("winLAME: using Winamp module %s: %s\n"), fds.cFileName, mod->getModuleName());
            }
            else
            {
               // discard module
               delete mod;
               mod = NULL;
               ::FreeLibrary(dll);
            }
         }
      }
      while (TRUE==::FindNextFile(hnd,&fds));

      ::FindClose(hnd);
   }
}

CString wlGetAnsiCompatFilename(LPCTSTR pszFilename)
{
   CString cszFilename;
   DWORD dwRet = GetShortPathName(pszFilename, cszFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszFilename.ReleaseBuffer();

   return dwRet == 0 ? CString(pszFilename) : cszFilename;
}
