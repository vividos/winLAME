//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
/// \file EncoderImpl.cpp
/// \brief contains the actual encoder implementation
//
#include "stdafx.h"
#include "EncoderImpl.hpp"
#include "EncoderErrorHandler.hpp"
#include <fstream>
#include "LameOutputModule.hpp"
#include <sndfile.h>
#include <ulib/thread/LightweightMutex.hpp>

using namespace Encoder;

// static EncoderInterface methods

EncoderInterface* EncoderInterface::CreateEncoder()
{
   return new EncoderImpl;
}

// globals

/// mutex to protect threads from generating the same output filenames
static LightweightMutex s_mutexTempOutputFile;

// EncoderImpl methods

EncoderImpl::EncoderImpl()
   :m_settingsManager(nullptr),
   m_moduleManager(IoCContainer::Current().Resolve<Encoder::ModuleManager>()),
   m_errorHandler(nullptr)
{
}

EncoderImpl::~EncoderImpl()
{
   if (m_workerThread != nullptr)
   {
      try
      {
         m_workerThread->join();
         m_workerThread.reset();
      }
      catch (const std::exception& ex) // NOSONAR
      {
         UNUSED(ex);
         ATLTRACE(_T("Exception while joining worker thread: %hs"), ex.what());
      }
      catch (...)
      {
         ATLTRACE(_T("Unknown exception while joining worker thread"));
      }
   }
}

void EncoderImpl::StartEncode()
{
   if (m_encoderState.m_running)
      return;

   if (m_encoderState.m_paused)
   {
      PauseEncoding();
      return;
   }

   m_encoderState.m_encodingDescription.Empty();

   m_encoderState.m_running = true;
   m_encoderState.m_paused = false;

   // when there's a previous worker thread, it must already have been join()ed
   ATLASSERT(m_workerThread == nullptr || !m_workerThread->joinable());

   m_workerThread.reset(
      new std::thread(
         std::bind(&EncoderImpl::Encode, this)));
}

void EncoderImpl::StopEncode()
{
   // forces encoder to stop
   {
      std::unique_lock<std::recursive_mutex> lock(m_mutex);
      m_encoderState.m_running = false;
      m_encoderState.m_paused = false;
   }

   // wait for thread to finish
   if (m_workerThread != nullptr)
   {
      m_workerThread->join();
      m_workerThread.reset();
   }
}

void EncoderImpl::Encode()
{
   if (!m_encoderState.m_running)
   {
      return;
   }

   std::unique_lock<std::recursive_mutex> lock(m_mutex, std::defer_lock);
   lock.lock();

   m_encoderState.m_percent = 0.f;
   m_encoderState.m_errorCode = 0;
   m_encoderState.m_encodingDescription.Empty();

   bool initOutputModule = false;

   ATLASSERT(m_inputModule == nullptr); // must not be set, or else Encode() was called twice!
   ATLASSERT(m_outputModule == nullptr);

   if (m_inputModule != nullptr || m_outputModule != nullptr)
   {
      // end thread
      m_encoderState.m_running = false;
      m_encoderState.m_paused = false;
      m_encoderState.m_finished = true;
      m_encoderState.m_errorCode = -1;
      return;
   }

   // get input and output modules
   ModuleManagerImpl* modimpl = reinterpret_cast<ModuleManagerImpl*>(&m_moduleManager);
   m_inputModule = std::unique_ptr<InputModule>(modimpl->ChooseInputModule(m_encoderSettings.m_inputFilename));
   m_outputModule = std::unique_ptr<OutputModule>(modimpl->GetOutputModule(m_encoderSettings.m_outputModuleID));

   if (m_inputModule == nullptr ||
      m_outputModule == nullptr)
   {
      // end thread
      m_encoderState.m_running = false;
      m_encoderState.m_paused = false;
      m_encoderState.m_finished = true;
      return;
   }

   // init input module
   TrackInfo trackInfo;

   bool skipFile = false;

   if (!PrepareInputModule(trackInfo))
      skipFile = true;

   CString tempOutputFilename;

   bool skipMoveFile = false;
   if (!skipFile)
   {
      do
      {
         if (!PrepareOutputModule())
         {
            skipFile = true;
            break;
         }

         // use the provided track info
         if (m_encoderSettings.m_useTrackInfo)
            trackInfo = m_encoderSettings.m_trackInfo;

         // generate temporary name, in case the output module doesn't support unicode filenames
         GenerateTempOutFilename(m_encoderSettings.m_outputFilename, tempOutputFilename);

         bool bRet = InitOutputModule(tempOutputFilename, trackInfo);
         initOutputModule = true;

         if (!bRet)
         {
            skipFile = true;
            break;
         }

         FormatEncodingDescription();

      } while (false);
   }

   lock.unlock();

   if (!skipFile && !skipMoveFile)
      skipFile = MainLoop();

   if (!skipFile)
   {
      // write playlist entry, when enabled
      if (m_encoderState.m_running && !m_encoderSettings.m_playlistFilename.IsEmpty())
         WritePlaylistEntry(m_encoderSettings.m_outputFilename);
   }

   // done with modules
   if (m_inputModule != nullptr)
      m_inputModule->DoneInput();

   if (initOutputModule && m_outputModule != nullptr)
      m_outputModule->DoneOutput();

   // delete modules
   m_inputModule.reset();
   m_outputModule.reset();

   // rename when we used a temporary filename
   if (!skipFile)
   {
      if (!tempOutputFilename.IsEmpty() &&
         m_encoderSettings.m_outputFilename != tempOutputFilename)
      {
         if (m_encoderSettings.m_overwriteExisting)
            DeleteFile(m_encoderSettings.m_outputFilename);
         else
            // not overwriting, but "delete after encoding" flag set?
            if (m_encoderSettings.m_deleteInputAfterEncode)
               DeleteFile(m_encoderSettings.m_inputFilename);

         MoveFile(tempOutputFilename, m_encoderSettings.m_outputFilename);
      }
      else
      {
         // delete the old file, if option is set
         // as no temp output filename was generated, assume the names are different
         if (m_encoderSettings.m_deleteInputAfterEncode)
            DeleteFile(m_encoderSettings.m_inputFilename);
      }
   }

   // end thread
   m_encoderState.m_running = false;
   m_encoderState.m_paused = false;
   m_encoderState.m_finished = true;
}

bool EncoderImpl::PrepareInputModule(TrackInfo& trackInfo)
{
   // init new
   m_sampleContainer = SampleContainer();

   int res = m_inputModule->InitInput(m_encoderSettings.m_inputFilename, *m_settingsManager,
      trackInfo, m_sampleContainer);

   m_inputModule->ResolveRealFilename(m_encoderSettings.m_inputFilename);

   // catch errors
   if (m_errorHandler != nullptr && res < 0)
   {
      switch (m_errorHandler->HandleError(m_encoderSettings.m_inputFilename, m_inputModule->GetModuleName(),
         -res, m_inputModule->GetLastError()))
      {
      case EncoderErrorHandler::Continue:
         break;

      case EncoderErrorHandler::SkipFile:
         m_encoderState.m_errorCode = 1;
         return false;

      case EncoderErrorHandler::StopEncode:
         m_encoderState.m_errorCode = -1;
         return false;
      default:
         ATLASSERT(false);
         break;
      }
   }

   return true;
}

bool EncoderImpl::PrepareOutputModule()
{
   // prepare output module
   m_outputModule->PrepareOutput(*m_settingsManager);

   // do output filename
   if (m_encoderSettings.m_outputFilename.IsEmpty())
      m_encoderSettings.m_outputFilename = GetOutputFilename(m_encoderSettings.m_outputFolder, m_encoderSettings.m_inputFilename, *m_outputModule);

   // test if input and output file name is the same file
   if (!CheckSameInputOutputFilenames(m_encoderSettings.m_inputFilename, m_encoderSettings.m_outputFilename, *m_outputModule))
   {
      m_encoderState.m_errorCode = 2;
      return false;
   }

   // check if outputFilename already exists
   if (!m_encoderSettings.m_overwriteExisting)
   {
      // test if file exists
      if (Path(m_encoderSettings.m_outputFilename).FileExists())
      {
         // note: could do a message box here, but we really want the encoding progress
         // without any message boxes waiting.

         // skip this file
         m_encoderState.m_errorCode = 2;
         return false;
      }
   }

   return true;
}

CString EncoderImpl::GetOutputFilename(const CString& outputPath, const CString& inputFilename, OutputModule& outputModule)
{
   CString outputFilename = outputPath;

   CString filenameOnly = Path(inputFilename).FilenameOnly();
   outputFilename += filenameOnly;
   outputFilename += _T(".");
   outputFilename += outputModule.GetOutputExtension();

   return outputFilename;
}

bool EncoderImpl::CheckSameInputOutputFilenames(const CString& inputFilename,
   CString& outputFilename, OutputModule& outputModule)
{
   for (int i = 0; i < 2; i++)
   {
      // first, test if output filename already exists
      if (!Path(outputFilename).FileExists())
         break; // no, so we don't need to test

      // check if the file's short name is the same
      CString shortInputFilename = Path(inputFilename).ShortPathName();
      CString shortOutputFilename = Path(outputFilename).ShortPathName();

      if (0 == shortInputFilename.CompareNoCase(shortOutputFilename))
      {
         if (i == 0)
         {
            // when overwriting original, use same name as input filename
            // test again with another file name
            if (m_encoderSettings.m_overwriteExisting)
            {
               outputFilename = inputFilename;
               break;
            }
            else
            {
               // when not overwriting original, add extra extension
               outputFilename += _T(".");
               outputFilename += m_outputModule->GetOutputExtension();
            }
            continue;
         }
         else
         {
            // they're still the same file, skip this one completely
            return false;
         }
      }
   }

   return true;
}

void EncoderImpl::GenerateTempOutFilename(const CString& originalFilename, CString& tempFilename)
{
   LightweightMutex::LockType lock(s_mutexTempOutputFile);

   tempFilename = originalFilename;

   Path originalPath(originalFilename);
   CString pathName = originalPath.FolderName();
   CString fileName = originalPath.FilenameAndExt();

   // find short name of path
   CString shortPathName = Path(pathName).ShortPathName();

   // convert filename to ansi and back, and remove '?' chars
   fileName = CString(CStringA(fileName));
   fileName.Replace(_T('?'), _T('_'));

   // now add a ".part" suffix
   unsigned int fileIndex = 0;
   do
   {
      tempFilename = Path::Combine(shortPathName, fileName).ToString();
      if (fileIndex == 0)
         tempFilename += _T(".temp");
      else
      {
         tempFilename.AppendFormat(_T(".%u.temp"), fileIndex);
      }

      fileIndex++;
   }
   while (Path(tempFilename).FileExists());

   // create a dummy file so the next thread can't use this filename anymore
   FILE* fd = _tfopen(tempFilename, _T("wb"));
   if (fd != nullptr)
      fclose(fd);
}

bool EncoderImpl::InitOutputModule(const CString& tempOutputFilename, TrackInfo& trackInfo)
{
   // init output module
   int res = m_outputModule->InitOutput(tempOutputFilename, *m_settingsManager,
      trackInfo, m_sampleContainer);

   // catch errors
   if (m_errorHandler != nullptr && res < 0)
   {
      switch (m_errorHandler->HandleError(m_encoderSettings.m_inputFilename, m_outputModule->GetModuleName(),
         -res, m_outputModule->GetLastError()))
      {
      case EncoderErrorHandler::Continue:
         break;
      case EncoderErrorHandler::SkipFile:
         m_encoderState.m_errorCode = 2;
         return false;
      case EncoderErrorHandler::StopEncode:
         m_encoderState.m_errorCode = -2;
         return false;
      default:
         ATLASSERT(false);
         break;
      }
   }

   return true;
}

void EncoderImpl::FormatEncodingDescription()
{
   CString inputDescription = m_inputModule->GetDescription();
   CString outputDescription = m_outputModule->GetDescription();

   CString containerInfo;
#ifdef _DEBUG
   // get sample container description
   containerInfo.Format(
      _T("Sample Container: ")
      _T("Input: %i bit, %i channels, sample rate %i Hz, ")
      _T("Output: %i bit\r\n"),
      m_sampleContainer.GetInputModuleBitsPerSample(),
      m_sampleContainer.GetInputModuleChannels(),
      m_sampleContainer.GetInputModuleSampleRate(),
      m_sampleContainer.GetOutputModuleBitsPerSample());
#endif

   m_encoderState.m_encodingDescription.Format(
      IDS_ENCODER_ENCODE_INFO,
      inputDescription.GetString(),
      containerInfo.GetString(),
      outputDescription.GetString());
}

CString GetLastErrorString()
{
   DWORD dwError = GetLastError();

   LPVOID lpMsgBuf = nullptr;
   ::FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      dwError,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
      reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

   CString errorMessage;
   if (lpMsgBuf)
   {
      errorMessage = reinterpret_cast<LPTSTR>(lpMsgBuf);
      LocalFree(lpMsgBuf);
   }

   errorMessage.TrimRight(_T("\r\n"));

   return errorMessage;
}

bool EncoderImpl::IsLossyInputModule(int inputModuleID)
{
   return
      inputModuleID == ID_IM_MAD ||
      inputModuleID == ID_IM_OGGV ||
      inputModuleID == ID_IM_AAC ||
      inputModuleID == ID_IM_BASS ||
      inputModuleID == ID_IM_SPEEX ||
      inputModuleID == ID_IM_OPUS ||
      inputModuleID == ID_IM_LIBMPG123;
}

bool EncoderImpl::IsLossyOutputModule(int outputModuleID)
{
   return
      outputModuleID == ID_OM_LAME ||
      outputModuleID == ID_OM_OGGV ||
      outputModuleID == ID_OM_AAC ||
      outputModuleID == ID_OM_BASSWMA ||
      outputModuleID == ID_OM_OPUS;
}

bool EncoderImpl::MainLoop()
{
   bool skipFile = false;

   do
   {
      int ret = m_inputModule->DecodeSamples(m_sampleContainer);

      // no more samples?
      if (ret == 0)
         break;

      // catch errors
      if (m_errorHandler != nullptr && ret < 0)
      {
         switch (m_errorHandler->HandleError(m_encoderSettings.m_inputFilename, m_inputModule->GetModuleName(),
            -ret, m_inputModule->GetLastError()))
         {
         case EncoderErrorHandler::Continue:
            break;
         case EncoderErrorHandler::SkipFile:
            m_encoderState.m_errorCode = 3;
            skipFile = true;
            break;
         case EncoderErrorHandler::StopEncode:
            m_encoderState.m_errorCode = -3;
            skipFile = true;
            break;
         default:
            ATLASSERT(false);
            break;
         }
      }

      // get percent done
      m_encoderState.m_percent = m_inputModule->PercentDone();

      // stuff all samples received into output module
      ret = m_outputModule->EncodeSamples(m_sampleContainer);

      // catch errors
      if (m_errorHandler != nullptr && ret < 0)
      {
         switch (m_errorHandler->HandleError(m_encoderSettings.m_inputFilename, m_outputModule->GetModuleName(),
            -ret, m_outputModule->GetLastError()))
         {
         case EncoderErrorHandler::Continue:
            break;
         case EncoderErrorHandler::SkipFile:
            m_encoderState.m_errorCode = 4;
            skipFile = true;
            break;
         case EncoderErrorHandler::StopEncode:
            m_encoderState.m_errorCode = -4;
            skipFile = true;
            break;
         default:
            ATLASSERT(false);
            break;
         }
      }

      // check if we should stop the thread
      if (!m_encoderState.m_running ||
         skipFile)
         break;

      // sleep if we should pause
      while (m_encoderState.m_paused)
         Sleep(50);
   }
   while (true); // outer encoding loop

   return skipFile;
}

void EncoderImpl::WritePlaylistEntry(const CString& outputFilename)
{
   CString playlistPathAndFilename = Path::Combine(m_encoderSettings.m_outputFolder, m_encoderSettings.m_playlistFilename).ToString();

   FILE* fd = _tfopen(playlistPathAndFilename, _T("at"));
   if (fd == nullptr)
      return;

   // get filename, without path
   CString filename = Path(outputFilename).FilenameAndExt();

   // write to playlist file
   _ftprintf(fd, _T("%s\n"), filename.GetString());
   fclose(fd);
}
