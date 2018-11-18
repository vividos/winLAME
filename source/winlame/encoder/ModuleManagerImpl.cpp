//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file ModuleManagerImpl.cpp
/// \brief contains the module manager implementation
//
#include "stdafx.h"
#include <fstream>
#include <algorithm>
#include "ModuleManagerImpl.hpp"
#include "LameOutputModule.hpp"
#include "OggVorbisOutputModule.hpp"
#include "SndFileOutputModule.hpp"
#include "SndFileInputModule.hpp"
#include "OggVorbisInputModule.hpp"
#include "AacInputModule.hpp"
#include "AacOutputModule.hpp"
#include "FlacInputModule.hpp"
#include "BassInputModule.hpp"
#include "BassWmaOutputModule.hpp"
#include "MonkeysAudioInputModule.hpp"
#include "SpeexInputModule.hpp"
#include "OpusInputModule.hpp"
#include "OpusOutputModule.hpp"
#include "LibMpg123InputModule.hpp"
#include "resource.h"

using namespace Encoder;

// global functions

/// max number of input modules GetNewInputModule can return
const int c_maxInputModule = 9;

/// returns a new input module by index
InputModule* GetNewInputModule(int index)
{
   InputModule* inputModule = nullptr;
   switch (index)
   {
   case 0:
      inputModule = new LibMpg123InputModule;
      break;
   case 1:
      inputModule = new OpusInputModule;
      break;
   case 2:
      inputModule = new OggVorbisInputModule;
      break;
   case 3:
      inputModule = new AacInputModule;
      break;
   case 4:
      inputModule = new MonkeysAudioInputModule;
      break;
   case 5:
      inputModule = new FlacInputModule;
      break;
   case 6:
      inputModule = new BassInputModule;
      break;
   case 7:
      inputModule = new SpeexInputModule;
      break;
   case 8:
      inputModule = new SndFileInputModule;
      break;
   default:
      ATLASSERT(false);
      break;
   }

   return inputModule;
}

/// max number of output modules GetNewOutputModule can return
const int c_maxOutputModule = 6;

/// returns a new output module by index
OutputModule* GetNewOutputModule(int index)
{
   OutputModule* outputModule = NULL;
   switch (index)
   {
   case 0:
      outputModule = new LameOutputModule;
      break;
   case 1:
      outputModule = new OpusOutputModule;
      break;
   case 2:
      outputModule = new OggVorbisOutputModule;
      break;
   case 3:
      outputModule = new SndFileOutputModule;
      break;
   case 4:
      outputModule = new BassWmaOutputModule;
      break;
   case 5:
      outputModule = new AacOutputModule;
      break;
   default:
      ATLASSERT(false);
      break;
   }

   return outputModule;
}

void ModuleManagerImpl::GetFilterString(CString& filterstring) const
{
   // get all filter strings
   CString filter;
   int max = GetInputModuleCount();
   for (int i = 0; i < max; i++)
      filter += GetInputModuleFilterString(i);

   // add an "all supported audio files" option
   CString allFilter;
   {
      CString temp(filter);
      int pos;

      do
      {
         // search for second string delimited by a | char
         pos = temp.Find(_T('|'));
         if (pos != -1)
         {
            temp.Delete(0, pos + 1);
            pos = temp.Find(_T('|'));
            if (pos == -1)
               break;

            // append the filter value
            allFilter += temp.Left(pos);
            allFilter += _T(';');
            temp.Delete(0, pos + 1);
         }
      } while (pos != -1);

      // remove last semicolon
      allFilter.TrimRight(_T(';'));
   }

   // combine filter lists
   filterstring.LoadString(IDS_FILTER_ALL_SUPPORTED);
   filterstring.AppendFormat(_T("|%s|%s"), allFilter.GetString(), filter.GetString());
}

bool ModuleManagerImpl::GetAudioFileInfo(LPCTSTR filename,
   int& lengthInSeconds, int& bitrateInBps, int& samplerateInHz, CString& errorMessage)
{
   // get appropriate input module
   InputModule* inputModule = ChooseInputModule(filename);
   if (inputModule == nullptr)
   {
      errorMessage.LoadString(IDS_ENCODER_MISSING_INPUT_MOD);
      return false;
   }

   // get infos
   Encoder::TrackInfo trackInfo;
   SampleContainer samples;
   SettingsManager dummy;
   int ret = inputModule->InitInput(filename, dummy, trackInfo, samples);

   if (ret >= 0)
   {
      int dummy2;
      inputModule->GetInfo(dummy2, bitrateInBps, lengthInSeconds, samplerateInHz);
   }
   else
      errorMessage = inputModule->GetLastError();

   inputModule->DoneInput();

   delete inputModule;

   return ret >= 0;
}

ModuleManagerImpl::ModuleManagerImpl()
{
   // check which output modules are available
   for (int i = 0; i < c_maxOutputModule; i++)
   {
      OutputModule* outputModule = GetNewOutputModule(i);

      if (outputModule != nullptr)
      {
         if (outputModule->IsAvailable())
         {
            m_outputModules.push_back(outputModule);
            m_mapOutputModuleIdToModuleIndex.insert(
               std::make_pair(outputModule->GetModuleID(), i));
         }
         else
            delete outputModule;
      }
   }

   // check which input modules are available
   for (int i = 0; i < c_maxInputModule; i++)
   {
      InputModule* inputModule = GetNewInputModule(i);

      if (inputModule != nullptr)
      {
         if (inputModule->IsAvailable())
            m_inputModules.push_back(inputModule);
         else
            delete inputModule;
      }
   }
}

ModuleManagerImpl::~ModuleManagerImpl()
{
   // delete all output modules
   int max = m_outputModules.size();
   for (int i = 0; i < max; i++)
      delete m_outputModules[i];

   // delete all input modules
   max = m_inputModules.size();
   for (int i = 0; i < max; i++)
      delete m_inputModules[i];
}

InputModule* ModuleManagerImpl::ChooseInputModule(LPCTSTR filename)
{
   InputModule* inputModule = nullptr;

   // get file extension
   std::tstring extension(filename);
   std::tstring::size_type pos = extension.find_last_of('.');
   if (pos == std::tstring::npos)
      return nullptr; // no extension
   extension.erase(0, pos);

   CString lowerExtension(extension.c_str());
   lowerExtension.MakeLower();
   extension = lowerExtension.GetString();

   // search all filter strings for file extension
   int max = GetInputModuleCount();
   for (int i = 0; i < max && inputModule == nullptr; i++)
   {
      CString lowerFilter = GetInputModuleFilterString(i);
      lowerFilter.MakeLower();
      std::tstring filter = lowerFilter.GetString();

      do
      {
         // search for second string delimited by a | char
         pos = filter.find_first_of('|');
         if (pos != std::tstring::npos)
         {
            filter.erase(0, pos + 1);
            pos = filter.find_first_of('|');
            if (pos == std::tstring::npos)
               break;

            // check if the extension is in the filter wildcard pattern
            std::tstring temp(filter.c_str(), pos);
            std::tstring::size_type pos2 = temp.find(extension.c_str());

            if (pos2 != std::tstring::npos)
            {
               // clone input module
               inputModule = m_inputModules[i]->CloneModule();
               break;
            }
         }
      } while (pos != std::tstring::npos);
   }

   return inputModule;
}

OutputModule *ModuleManagerImpl::GetOutputModule(int m_moduleId)
{
   // search output module per id
   OutputModule *outputModule = nullptr;

   int max = GetOutputModuleCount();
   for (int i = 0; i < max; i++)
      if (GetOutputModuleID(i) == m_moduleId)
      {
         std::map<int, int>::iterator pos =
            m_mapOutputModuleIdToModuleIndex.find(m_moduleId);

         if (pos == m_mapOutputModuleIdToModuleIndex.end())
            continue; // should not happen, normally

         outputModule = GetNewOutputModule(pos->second);
         break;
      }

   return outputModule;
}

void ModuleManagerImpl::GetModuleVersionString(CString& version,
   int m_moduleId, int special)
{
   int max, i;

   // search all input modules for module ID
   max = GetInputModuleCount();
   for (i = 0; i < max; i++)
      if (GetInputModuleID(i) == m_moduleId)
      {
         m_inputModules[i]->GetVersionString(version, special);
         return;
      }

   // and now all output modules
   max = GetOutputModuleCount();
   for (i = 0; i < max; i++)
      if (GetOutputModuleID(i) == m_moduleId)
      {
         m_outputModules[i]->GetVersionString(version, special);
         return;
      }

   version.Empty();
}

CString Encoder::GetAnsiCompatFilename(LPCTSTR filename)
{
   return Path(filename).ShortPathName();
}
