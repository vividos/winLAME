//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file EncoderInterface.hpp
/// \brief EncoderInterface is an interface for the encoder class
/// \details
/// EncoderInterface is an interface for the encoder class and lets the
/// user of the class set the encoding options, and lets the user start,
/// pause and stop the encoding process, as well as querying some infos
///
/// ModuleManager is an interface to the class that manages the used
/// and available input and output modules; it is assumed that the
/// availability of the modules don't change durin a program run
//
#pragma once

#include <string>
#include "resource.h"
#include "TrackInfo.hpp"
#include "EncoderState.hpp"

class SettingsManager;
class ModuleManager;

/// contains all classes and functions that have to do with encoding
namespace Encoder
{
   struct EncoderSettings;

   /// encoder job
   class EncoderJob
   {
   public:
      /// ctor
      explicit EncoderJob(const CString& inputFilename)
         :m_inputFilename(inputFilename)
      {
      }

      // getter

      /// returns input filename
      CString InputFilename() const { return m_inputFilename; }

      /// returns output filename
      CString OutputFilename() const { return m_outputFilename; }

      /// returns track info; const version
      const TrackInfo& GetTrackInfo() const { return m_trackInfo; }

      /// returns track info
      TrackInfo& GetTrackInfo() { return m_trackInfo; }

      // setter

      /// sets output filename
      void OutputFilename(const CString& outputFilename) { m_outputFilename = outputFilename; }

   private:
      CString m_inputFilename;   ///< input filename
      CString m_outputFilename;  ///< output filename
      TrackInfo m_trackInfo;     ///< track info
   };

   /// error info
   struct ErrorInfo
   {
      /// ctor
      ErrorInfo()
         :m_errorNumber(0)
      {
      }

      CString m_inputFilename;   ///< input filename
      CString m_moduleName;      ///< module name
      int m_errorNumber;         ///< error number
      CString m_errorMessage;    ///< error message
   };

   /// encoder interface
   class EncoderInterface
   {
   public:
      /// returns new encoder object; destroy with delete operator
      static EncoderInterface* CreateEncoder();

      // encoder functions

      /// sets new encoder settings
      virtual void SetEncoderSettings(const EncoderSettings& encoderSettings) = 0;

      /// returns encoder state
      virtual EncoderState GetEncoderState() const = 0;

      /// returns list of all error infos occured (so far)
      virtual std::vector<ErrorInfo> GetAllErrorInfos() const = 0;

      /// sets the settings manager to use
      virtual void SetSettingsManager(SettingsManager* settingsManager) = 0;

      /// starts encoding thread; returns immediately
      virtual void StartEncode() = 0;

      /// pauses encoding
      virtual void PauseEncoding() = 0;

      /// stops encoding
      virtual void StopEncode() = 0;

      /// dtor
      virtual ~EncoderInterface() {}

      // helper methods

      /// returns if the encoder thread is running
      bool IsRunning() const
      {
         return GetEncoderState().m_running;
      }

      /// returns if the encoder is currently paused
      bool IsPaused() const
      {
         return GetEncoderState().m_paused;
      }

   protected:
      /// ctor
      EncoderInterface() {}
   };

} // namespace Encoder
