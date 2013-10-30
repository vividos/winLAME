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
   // TODO
   EncoderImpl::setInputFilename(m_settings.m_cszInputFilename);
   //EncoderImpl::setOutputPath(settings.
   EncoderImpl::setSettingsManager(&m_settings.m_settingsManager);
   EncoderImpl::setModuleManager(m_settings.m_pModuleManager);
   EncoderImpl::setOutputModule(m_settings.m_iOutputModuleId);
   EncoderImpl::setErrorHandler(&m_errorHandler);
   //EncoderImpl::setOverwriteFiles(settings.
   //EncoderImpl::setDeleteAfterEncode(settings.
   EncoderImpl::setWarnLossy(false);
   //EncoderImpl::setOutputPlaylistFilename
}

TaskInfo EncoderTask::GetTaskInfo()
{
   TaskInfo info;
   // TODO set name, desc, etc.
   float fPercent = EncoderImpl::queryPercentDone();
   info.Progress(static_cast<int>(fPercent));

   // TODO EncoderImpl::getEncodingDescription();

   return info;
}

void EncoderTask::Run()
{
   EncoderImpl::encode();
}

void EncoderTask::Stop()
{
   EncoderImpl::stopEncode();
}
