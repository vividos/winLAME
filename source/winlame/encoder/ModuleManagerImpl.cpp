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
/// \file ModuleManagerImpl.cpp
/// \brief contains the module manager implementation

// needed includes
#include "stdafx.h"
#include <fstream>
#include <algorithm>
#include "ModuleManagerImpl.h"
#include "LameOutputModule.h"
#include "OggVorbisOutputModule.h"
#include "WaveOutputModule.h"
#include "SndFileInputModule.h"
#include "MadMpegInputModule.h"
#include "OggVorbisInputModule.h"
#include "AacInputModule.h"
#include "AacOutputModule.h"
#include "FlacInputModule.h"
#include "BassInputModule.h"
#include "BassWmaOutputModule.h"
#include "MonkeysAudioInputModule.h"
#include "CDReadoutModule.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



// global functions

/// max number of input modules GetNewInputModule can return
const int MaxInputModule = 8;

/// returns a new input module by index
InputModule *GetNewInputModule(int index)
{
   InputModule *inmod = NULL;
   switch(index)
   {
   case 0:
      inmod = new SndFileInputModule;
      break;
   case 1:
      inmod = new MadMpegInputModule;
      break;
   case 2:
      inmod = new OggVorbisInputModule;
      break;
   case 3:
      inmod = new AacInputModule;
      break;
   case 4:
      inmod = new MonkeysAudioInputModule;
      break;
   case 5:
      inmod = new FlacInputModule;
      break;
   case 6:
      inmod = new BassInputModule;
      break;
   case 7:
      inmod = new CDReadoutModule;
   }
   return inmod;
}

/// max number of output modules GetNewOutputModule can return
const int c_iMaxOutputModule = 5;

/// returns a new output module by index
OutputModule *GetNewOutputModule(int index)
{
   OutputModule *outmod = NULL;
   switch(index)
   {
   case 0:
      outmod = new LameOutputModule;
      break;
   case 1:
      outmod = new OggVorbisOutputModule;
      break;
   case 2:
      outmod = new WaveOutputModule;
      break;
   case 3:
      outmod = new BassWmaOutputModule;
      break;
   case 4:
      outmod = new AacOutputModule;
      break;
   }
   return outmod;
}


// static ModuleManager methods

ModuleManager* ModuleManager::getNewModuleManager()
{
   return new ModuleManagerImpl;
}


// ModuleManagerImpl methods

void ModuleManagerImpl::getFilterString(CString& filterstring)
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

bool ModuleManagerImpl::getAudioFileInfo(LPCTSTR filename,
   int& length, int& bitrate, int& samplefreq, CString& errormsg)
{
   // get appropriate input module
   InputModule *inmod = chooseInputModule(filename);
   if (inmod==NULL)
   {
      errormsg.LoadString(IDS_ENCODER_MISSING_INPUT_MOD);
      return false;
   }

   // get infos
   SettingsManager dummy;
   int dummy2;

   TrackInfo trackinfo;
   SampleContainer sampcont;
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

ModuleManagerImpl::ModuleManagerImpl()
{
   // check which output modules are available
   int i;
   OutputModule *outmod;
   for(i=0; i<c_iMaxOutputModule; i++)
   {
      outmod = GetNewOutputModule(i);

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
   InputModule *inmod;
   for(i=0; i<MaxInputModule; i++)
   {
      inmod = GetNewInputModule(i);

      if (inmod!=NULL)
      {
         if (inmod->isAvailable())
            in_modules.push_back(inmod);
         else
            delete inmod;
      }
   }
}

ModuleManagerImpl::~ModuleManagerImpl()
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

InputModule *ModuleManagerImpl::chooseInputModule(LPCTSTR filename)
{
   InputModule *inmod = NULL;

   // shortcut: when starting with cdrip://, return CDReadoutModule
   extern LPCTSTR g_pszCDRipPrefix;
   if (filename==_tcsstr(filename, g_pszCDRipPrefix))
   {
      return new CDReadoutModule;
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

OutputModule *ModuleManagerImpl::getOutputModule(int module_id)
{
   // search output module per id
   OutputModule *outmod = NULL;

   int max = getOutputModuleCount();
   for(int i=0; i<max; i++)
   if (getOutputModuleID(i)==module_id)
   {
      std::map<int,int>::iterator pos =
         out_id_to_modidx.find(module_id);

      if (pos == out_id_to_modidx.end())
         continue; // should not happen, normally

      outmod = GetNewOutputModule(pos->second);
      break;
   }

   return outmod;
}

void ModuleManagerImpl::getModuleVersionString(CString& version,
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

CString GetAnsiCompatFilename(LPCTSTR pszFilename)
{
   CString cszFilename;
   DWORD dwRet = GetShortPathName(pszFilename, cszFilename.GetBuffer(MAX_PATH), MAX_PATH);
   cszFilename.ReleaseBuffer();

   return dwRet == 0 ? CString(pszFilename) : cszFilename;
}
