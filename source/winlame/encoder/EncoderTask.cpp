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
/// \file EncoderTask.cpp
/// \brief encoder task class
//
#include "StdAfx.h"
#include "EncoderTask.hpp"

using Encoder::EncoderTask;
using Encoder::EncoderTaskSettings;

EncoderTask::EncoderTask(unsigned int dependentTaskId, const EncoderTaskSettings& settings)
:Task(dependentTaskId),
m_settings(settings),
m_stopped(false)
{
   EncoderImpl::setInputFilename(m_settings.m_cszInputFilename);
   EncoderImpl::setOutputPath(m_settings.m_cszOutputPath);
   EncoderImpl::setSettingsManager(&m_settings.m_settingsManager);
   EncoderImpl::setModuleManager(m_settings.m_pModuleManager);
   EncoderImpl::setOutputModule(m_settings.m_iOutputModuleId);
   EncoderImpl::setErrorHandler(&m_errorHandler);
   EncoderImpl::setOverwriteFiles(m_settings.m_bOverwriteFiles);
   EncoderImpl::setDeleteAfterEncode(m_settings.m_bDeleteAfterEncode);
   EncoderImpl::setWarnLossy(false);

   if (m_settings.m_useTrackInfo)
      EncoderImpl::setTrackInfo(m_settings.m_trackInfo);
}

CString EncoderTask::GenerateOutputFilename(const CString& inputFilename)
{
   if (m_precalculatedOutputFilename.IsEmpty())
   {
      Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
      Encoder::ModuleManagerImpl& modImpl = reinterpret_cast<Encoder::ModuleManagerImpl&>(moduleManager);

      std::unique_ptr<Encoder::OutputModule> outputModule(modImpl.GetOutputModule(m_settings.m_iOutputModuleId));
      ATLASSERT(outputModule != nullptr);

      outputModule->PrepareOutput(m_settings.m_settingsManager);

      m_precalculatedOutputFilename = EncoderImpl::GetOutputFilename(m_settings.m_cszOutputPath, inputFilename, *outputModule.get());
   }

   return m_precalculatedOutputFilename;
}

TaskInfo EncoderTask::GetTaskInfo()
{
   TaskInfo info(Id(), TaskInfo::taskEncoding);

   info.Name(m_settings.m_cszTitle);

   info.Description(EncoderImpl::getEncodingDescription());

   info.Status(
      finished || m_stopped ? TaskInfo::statusCompleted :
      error != 0 ? TaskInfo::statusError :
      running ? TaskInfo::statusRunning :
      TaskInfo::statusWaiting);

   float PercentDone = EncoderImpl::queryPercentDone();
   info.Progress(static_cast<unsigned int>(PercentDone));

   return info;
}

void EncoderTask::Run()
{
   if (m_stopped)
      return;

   running = true;
   m_stopped = false;

   EncoderImpl::encode();

   if (!m_errorHandler.AllErrors().empty())
   {
      AddErrorText();
   }
}

void EncoderTask::Stop()
{
   m_stopped = true;
   EncoderImpl::stopEncode();
}

void EncoderTask::AddErrorText()
{
   CString errorText;

   bool isFirst = true;

   std::for_each(m_errorHandler.AllErrors().begin(), m_errorHandler.AllErrors().end(),
      [&](const AlwaysSkipErrorHandler::ErrorInfo& info)
   {
      if (isFirst)
      {
         errorText.Format(IDS_ENCODER_ERROR_ERRORINFO_FILENAME_S,
            info.m_cszInputFilename);

         isFirst = false;
      }

      errorText.Append(_T("\r\n"));

      errorText.AppendFormat(IDS_ENCODER_ERROR_ERRORINFO_SSI,
         info.m_cszModuleName,
         info.m_cszErrorMessage,
         info.m_iErrorNumber);
   });

   SetTaskError(errorText);
}

