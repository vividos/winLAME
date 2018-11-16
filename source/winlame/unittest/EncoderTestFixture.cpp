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
/// \file EncoderTestFixture.cpp
/// \brief Test fixture for testing Encoder classes

#include "stdafx.h"
#include "EncoderTestFixture.hpp"
#include <ulib/IoCContainer.hpp>
#include "LameNogapInstanceManager.hpp"
#include "ModuleManager.hpp"
#include "ModuleManagerImpl.hpp"
#include <ulib/win32/ResourceData.hpp>
#include "EncoderImpl.hpp"

using unittest::EncoderTestFixture;

/// instance of static LAME NoGap instance manager
std::shared_ptr<Encoder::LameNogapInstanceManager> EncoderTestFixture::m_spLameNogapInstanceManager;

/// instance of static module manager
std::shared_ptr<Encoder::ModuleManager> EncoderTestFixture::m_spModuleManager;

void EncoderTestFixture::SetUp()
{
   // register objects in IoC container
   IoCContainer& ioc = IoCContainer::Current();

   if (m_spLameNogapInstanceManager == nullptr)
   {
      m_spLameNogapInstanceManager.reset(new Encoder::LameNogapInstanceManager);
      ioc.Register<Encoder::LameNogapInstanceManager>(boost::ref(*m_spLameNogapInstanceManager.get()));
   }

   if (m_spModuleManager == nullptr)
   {
      m_spModuleManager.reset(new Encoder::ModuleManagerImpl);
      ioc.Register<Encoder::ModuleManager>(boost::ref(*m_spModuleManager.get()));
   }
}

void EncoderTestFixture::ExtractFromResource(UINT resourceId, LPCTSTR filename)
{
   HINSTANCE hInstance = g_hDllInstance;
   Win32::ResourceData data(MAKEINTRESOURCE(resourceId), _T("\"RT_RCDATA\""), hInstance);

   data.AsFile(filename);
}

void EncoderTestFixture::StartEncodeAndWaitForFinish(Encoder::EncoderImpl& encoder)
{
   encoder.StartEncode();

   // wait for encoding to finish
   bool isRunning = false;
   do
   {
      Encoder::EncoderState state = encoder.GetEncoderState();
      isRunning = state.m_running;
      Sleep(10);

   } while (isRunning);

   encoder.StopEncode();
}

void EncoderTestFixture::GetAudioFileInfos(LPCTSTR filename, int& numChannels, int& bitrateInBps, int& lengthInSeconds, int& samplerateInHz)
{
   Encoder::ModuleManagerImpl moduleManager;

   std::unique_ptr<Encoder::InputModule> inputModule(moduleManager.ChooseInputModule(filename));
   if (inputModule == nullptr)
      throw std::runtime_error("couldn't find input module for filename");

   Encoder::TrackInfo trackInfo;
   Encoder::SampleContainer samples;
   SettingsManager dummy;
   inputModule->InitInput(filename, dummy, trackInfo, samples);

   inputModule->GetInfo(numChannels, bitrateInBps, lengthInSeconds, samplerateInHz);

   inputModule->DoneInput();
}

Encoder::TrackInfo EncoderTestFixture::GetTrackInfo(LPCTSTR filename)
{
   Encoder::ModuleManagerImpl moduleManager;

   std::unique_ptr<Encoder::InputModule> inputModule(moduleManager.ChooseInputModule(filename));
   if (inputModule == nullptr)
      throw std::runtime_error("couldn't find input module for filename");

   Encoder::TrackInfo trackInfo;
   Encoder::SampleContainer samples;
   SettingsManager dummy;
   inputModule->InitInput(filename, dummy, trackInfo, samples);

   inputModule->DoneInput();

   return trackInfo;
}
