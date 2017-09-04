//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file AudioFileInfoManager.cpp
/// \brief Manager to fetch audio file infos
//
#include "stdafx.h"
#include "AudioFileInfoManager.hpp"
#include "ModuleManager.hpp"
#include <boost/ref.hpp>

AudioFileInfoManager::AudioFileInfoManager()
   :m_ioService(1), // one thread max.
   m_upDefaultWork(new boost::asio::io_service::work(m_ioService)),
   m_stopping(false)
{
}

AudioFileInfoManager::~AudioFileInfoManager()
{
   try
   {
      Stop();
   }
   catch (...)
   {
      // ignore errors when stopping
   }

   if (m_upThread != nullptr)
      m_upThread->join();
}

bool AudioFileInfoManager::GetAudioFileInfo(LPCTSTR filename,
   int& lengthInSeconds, int& bitrateInBps, int& sampleFrequencyInHz, CString& errorMessage)
{
   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();

   return moduleManager.GetAudioFileInfo(filename, lengthInSeconds, bitrateInBps, sampleFrequencyInHz, errorMessage);
}

void AudioFileInfoManager::AsyncGetAudioFileInfo(LPCTSTR filename, AudioFileInfoManager::T_fnCallback fnCallback)
{
   ATLASSERT(fnCallback != nullptr);

   if (m_stopping)
      return;

   if (m_upThread == nullptr)
   {
      m_upThread.reset(new std::thread(std::bind(&AudioFileInfoManager::RunThread, boost::ref(m_ioService))));
   }

   m_ioService.post(
      std::bind(&AudioFileInfoManager::WorkerGetAudioFileInfo, std::cref(m_stopping), CString(filename), fnCallback));
}

void AudioFileInfoManager::Stop()
{
   m_stopping = true;
   m_upDefaultWork.reset();
   m_ioService.stop();
}

void AudioFileInfoManager::RunThread(boost::asio::io_service& ioService)
{
   try
   {
      ioService.run();
   }
   catch (boost::system::system_error& error)
   {
      error;
      ATLTRACE(_T("system_error: %hs\n"), error.what());
      ATLASSERT(false);
   }
}

void AudioFileInfoManager::WorkerGetAudioFileInfo(const std::atomic<bool>& stopping,
   const CString& filename, AudioFileInfoManager::T_fnCallback fnCallback)
{
   if (stopping)
      return;

   int lengthInSeconds = 0;
   int bitrateInBps = 0;
   int sampleFrequencyInHz = 0;
   CString errorMessage;

   bool ret = GetAudioFileInfo(filename, lengthInSeconds, bitrateInBps, sampleFrequencyInHz, errorMessage);

   if (stopping)
      return;

   fnCallback(!ret, errorMessage, lengthInSeconds, bitrateInBps, sampleFrequencyInHz);
}
