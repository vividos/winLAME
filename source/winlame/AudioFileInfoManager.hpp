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
/// \file AudioFileInfoManager.hpp
/// \brief Manager to fetch audio file infos
//
#pragma once

#include <thread>
#include <atomic>

/// Manager to fetch audio file infos asynchronously
class AudioFileInfoManager
{
public:
   /// callback function type
   typedef std::function<void(bool error, const CString& errorMessage, int lengthInSeconds, int bitrateInBps, int sampleFrequencyInHz)> T_fnCallback;

   /// ctor
   AudioFileInfoManager();

   /// dtor
   ~AudioFileInfoManager();

   /// returns infos about audio file; returns false when not supported; synchronous version
   static bool GetAudioFileInfo(LPCTSTR filename,
      int& lengthInSeconds, int& bitrateInBps, int& sampleFrequencyInHz, CString& errorMessage);

   /// retrieves info about audio file; asynchronous vesrion
   void AsyncGetAudioFileInfo(LPCTSTR filename, T_fnCallback fnCallback);

   /// stops all further processing; no callbacks are called anymore that are not already active
   void Stop();

private:
   /// thread function
   static void RunThread(boost::asio::io_service& ioService);

   /// worker thread function to get audio file infos and to call callback
   static void WorkerGetAudioFileInfo(std::atomic<bool>& stopping, const CString& filename, AudioFileInfoManager::T_fnCallback fnCallback);

private:
   /// worker thread
   std::unique_ptr<std::thread> m_upThread;

   /// io service
   boost::asio::io_service m_ioService;

   /// default work for io service
   std::unique_ptr<boost::asio::io_service::work> m_upDefaultWork;

   /// indicates when manager is currently stopping
   std::atomic<bool> m_stopping;
};
