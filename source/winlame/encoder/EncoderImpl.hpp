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
/// \file EncoderImpl.hpp
/// \brief contains the encoder implementation class definition
//
#pragma once

#include <string>
#include "EncoderInterface.hpp"
#include "ModuleInterface.hpp"
#include "ModuleManagerImpl.hpp"
#include <thread>
#include <mutex>
#include "EncoderState.hpp"
#include "EncoderSettings.hpp"

namespace Encoder
{
   /// encoder implementation class
   class EncoderImpl : public EncoderInterface
   {
   public:
      /// ctor
      EncoderImpl();

      /// dtor
      virtual ~EncoderImpl();

      /// sets encoder settings
      virtual void SetEncoderSettings(const EncoderSettings& encoderSettings) override
      {
         m_encoderSettings = encoderSettings;
      }

      /// returns encoder state
      virtual EncoderState GetEncoderState() const override
      {
         std::unique_lock<std::recursive_mutex> lock(m_mutex);

         return m_encoderState;
      }

      /// returns list of all error infos occured (so far)
      virtual std::vector<ErrorInfo> GetAllErrorInfos() const override
      {
         std::unique_lock<std::recursive_mutex> lock(m_mutex);

         return m_allErrorsList;
      }

      /// sets the settings manager to use
      virtual void SetSettingsManager(SettingsManager* settingsManager) override
      {
         m_settingsManager = settingsManager;
      }

      /// starts encoding thread; returns immediately
      virtual void StartEncode() override;

      /// pauses encoding
      virtual void PauseEncoding() override
      {
         std::unique_lock<std::recursive_mutex> lock(m_mutex);
         m_encoderState.m_paused = !m_encoderState.m_paused;
      }

      /// stops encoding
      virtual void StopEncode() override;

      /// creates output filename from input filename
      static CString GetOutputFilename(const CString& outputPath, const CString& inputFilename, OutputModule& outputModule);

      /// returns if input module with given id is lossy
      static bool IsLossyInputModule(int inputModuleId);

      /// returns if output module with given id is lossy
      static bool IsLossyOutputModule(int outputModuleID);

   protected:
      /// encodes using encoder settings
      void Encode();

      /// prepares input module for work
      bool PrepareInputModule(TrackInfo& trackInfo);

      /// prepares output module for work; step 1 of 2; see InitOutputModule()
      bool PrepareOutputModule();

      /// checks if the input and output filenames are the same and modifies output filename
      bool CheckSameInputOutputFilenames(const CString& inputFilename,
         CString& outputFilename, OutputModule& outputModule);

      /// generates temporary output filename
      void GenerateTempOutFilename(const CString& originalFilename, CString& tempFilename);

      /// inits output module; step 2 of 2; see PrepareOutputModule()
      bool InitOutputModule(const CString& tempOutputFilename, TrackInfo& trackInfo);

      /// formats encoding description
      void FormatEncodingDescription();

      /// main encoding loop; returns if file should be skipped
      bool MainLoop();

      /// writes playlist entry
      void WritePlaylistEntry(const CString& outputFilename);

      /// error handler function
      void HandleError(LPCTSTR inputFilename, LPCTSTR moduleName, int errorNumber, LPCTSTR errorMessage);

      /// returns encoder settings; const version
      const EncoderSettings& GetEncoderSettings() const { return m_encoderSettings; }

      /// returns encoder settings; non-const version
      EncoderSettings& GetEncoderSettings() { return m_encoderSettings; }

   private:
      friend class EncoderTask; // needed to set m_encoderState

      /// encoder settings
      EncoderSettings m_encoderSettings;

      /// the settings manager to use
      SettingsManager* m_settingsManager;

      /// module manager
      ModuleManager& m_moduleManager;

      /// list of all errors
      std::vector<ErrorInfo> m_allErrorsList;

      /// current encoder state
      EncoderState m_encoderState;

      /// input module
      std::unique_ptr<InputModule> m_inputModule;

      /// input module
      std::unique_ptr<OutputModule> m_outputModule;

      /// sample container
      SampleContainer m_sampleContainer;

      /// mutex to protect encoder state
      mutable std::recursive_mutex m_mutex;

      /// encoder worker thread
      std::unique_ptr<std::thread> m_workerThread;
   };

} // namespace Encoder
