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

EncoderTask::EncoderTask(const EncoderTaskSettings& settings)
:m_settings(settings)
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

   // TODO completed?
   info.Status(running ? TaskInfo::statusRunning : TaskInfo::statusWaiting);

   // TODO set name, desc, etc.
   float fPercent = EncoderImpl::queryPercentDone();
   info.Progress(static_cast<int>(fPercent));

   // TODO EncoderImpl::getEncodingDescription();

   return info;
}

void EncoderTask::Run()
{
   running = true;

   EncoderImpl::encode();
}

void EncoderTask::Stop()
{
   EncoderImpl::stopEncode();
}
