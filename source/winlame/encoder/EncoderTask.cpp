/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2012 Michael Fink

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
/// \file EncoderTask.cpp
/// \brief encoder task class

// includes
#include "StdAfx.h"
#include "EncoderTask.h"

EncoderTask::EncoderTask(unsigned int dependentTaskId, const EncoderTaskSettings& settings)
:Task(dependentTaskId),
m_settings(settings)
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
}

TaskInfo EncoderTask::GetTaskInfo()
{
   TaskInfo info(Id(), TaskInfo::taskEncoding);

   info.Name(Path(m_settings.m_cszInputFilename).FilenameAndExt());

   info.Description(EncoderImpl::getEncodingDescription());

   info.Status(
      finished ? TaskInfo::statusCompleted :
      error != 0 ? TaskInfo::statusError :
      running ? TaskInfo::statusRunning :
      TaskInfo::statusWaiting);

   float percentDone = EncoderImpl::queryPercentDone();
   info.Progress(static_cast<unsigned int>(percentDone));

   return info;
}

void EncoderTask::Run()
{
   running = true;

   EncoderImpl::encode();

   if (!m_errorHandler.AllErrors().empty())
   {
      AddErrorText();
   }
}

void EncoderTask::Stop()
{
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

