//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
#include "stdafx.h"
#include "EncoderTask.hpp"

using Encoder::EncoderTask;
using Encoder::EncoderTaskSettings;

EncoderTask::EncoderTask(unsigned int dependentTaskId, const EncoderTaskSettings& settings)
   :Task(dependentTaskId),
   m_settings(settings),
   m_stopped(false)
{
   EncoderImpl::SetEncoderSettings(m_settings);

   EncoderImpl::SetSettingsManager(&m_settings.m_settingsManager);
}

CString EncoderTask::GenerateOutputFilename(const CString& inputTitle)
{
   if (EncoderImpl::GetEncoderSettings().m_outputFilename.IsEmpty())
   {
      Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
      Encoder::ModuleManagerImpl& modImpl = reinterpret_cast<Encoder::ModuleManagerImpl&>(moduleManager);

      std::unique_ptr<Encoder::OutputModule> outputModule(modImpl.GetOutputModule(m_settings.m_outputModuleID));
      ATLASSERT(outputModule != nullptr);

      outputModule->PrepareOutput(m_settings.m_settingsManager);

      EncoderImpl::GetEncoderSettings().m_outputFilename =
         EncoderImpl::GetOutputFilenameByInputTitle(m_settings.m_outputFolder, inputTitle, *outputModule.get());
   }

   return EncoderImpl::GetEncoderSettings().m_outputFilename;
}

TaskInfo EncoderTask::GetTaskInfo()
{
   TaskInfo info(Id(), TaskInfo::taskEncoding);

   EncoderState encoderState = EncoderImpl::GetEncoderState();

   info.Name(m_settings.m_title);

   info.Description(encoderState.m_encodingDescription);

   info.Status(
      encoderState.m_finished || m_stopped ? TaskInfo::statusCompleted :
      EncoderImpl::GetEncoderState().m_errorCode != 0 ? TaskInfo::statusError :
      encoderState.m_running ? TaskInfo::statusRunning :
      TaskInfo::statusWaiting);

   info.Progress(static_cast<unsigned int>(encoderState.m_percent));

   return info;
}

void EncoderTask::Run()
{
   if (m_stopped)
      return;

   m_encoderState.m_running = true;
   m_stopped = false;

   EncoderImpl::Encode();

   CheckErrors();
}

void EncoderTask::Stop()
{
   m_stopped = true;
   EncoderImpl::StopEncode();
}

void EncoderTask::CheckErrors()
{
   auto allErrors = EncoderImpl::GetAllErrorInfos();
   if (allErrors.empty())
      return;

   CString errorText;
   bool isFirst = true;

   std::for_each(allErrors.begin(), allErrors.end(), [&](const ErrorInfo& info)
   {
      if (isFirst)
      {
         errorText.Format(IDS_ENCODER_ERROR_ERRORINFO_FILENAME_S,
            info.m_inputFilename.GetString());

         isFirst = false;
      }

      errorText.Append(_T("\r\n"));

      errorText.AppendFormat(IDS_ENCODER_ERROR_ERRORINFO_SSI,
         info.m_moduleName.GetString(),
         info.m_errorMessage.GetString(),
         info.m_errorNumber);
   });

   SetTaskError(errorText);
}
