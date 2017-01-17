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
/// \file EncoderTask.hpp
/// \brief encoder task class
//
#pragma once

#include "Task.hpp"
#include "TrackInfo.hpp"
#include "SettingsManager.hpp"
#include "EncoderImpl.hpp"
#include "AlwaysSkipErrorHandler.hpp"

namespace Encoder
{
   /// settings for EncoderTask
   struct EncoderTaskSettings : public EncoderSettings
   {
      /// ctor
      EncoderTaskSettings()
      {
         m_warnLossyTranscoding = false;
      }

      /// title
      CString m_title;

      /// the settings manager to use
      SettingsManager m_settingsManager;
   };

   /// encoder task
   class EncoderTask :
      public Task,
      private EncoderImpl
   {
   public:
      /// ctor
      EncoderTask(unsigned int dependentTaskId, const EncoderTaskSettings& settings);
      /// dtor
      virtual ~EncoderTask() throw() {}

      /// returns current task info; must return immediately
      virtual TaskInfo GetTaskInfo();

      /// runs task; may take longer
      virtual void Run();

      /// task should be aborted, e.g. when program is closed
      virtual void Stop();

      /// output filename for this task
      const CString& OutputFilename() const throw() { return EncoderImpl::m_encoderSettings.m_outputFilename; }

      /// generates output filename for this task
      CString GenerateOutputFilename(const CString& inputFilename);

   private:
      /// adds error texts from error handler to task result
      void AddErrorText();

   private:
      /// encoder task settings
      EncoderTaskSettings m_settings;

      /// error handler
      AlwaysSkipErrorHandler m_errorHandler;

      /// output filename
      CString m_outputFilename;

      /// indicates if encoder thread has stopped
      std::atomic<bool> m_stopped;
   };

} // namespace Encoder
