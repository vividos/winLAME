/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink

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
/// \file ModuleInterface.h
/// \brief contains the interface definition for input and output modules
/// \ingroup encoder
/// @{

// include guard
#pragma once

// needed includes
#include <string>
#include "SettingsManager.h"
#include "TrackInfo.h"
#include "SampleContainer.h"


// input and output module ids

#define ID_OM_LAME                      1
#define ID_OM_OGGV                      2
#define ID_OM_WAVE                      3
#define ID_OM_AAC                       4
#define ID_IM_SNDFILE                   5
#define ID_IM_MAD                       6
#define ID_IM_OGGV                      7
#define ID_IM_AAC                       8
#define ID_IM_FLAC                      10
#define ID_IM_BASS                      11
#define ID_OM_BASSWMA                   12
#define ID_IM_CDRIP                     13
#define ID_IM_SPEEX                     14
#define ID_IM_OPUS                      15


/// module base class

class ModuleBase
{
public:
   /// dtor
   virtual ~ModuleBase(){ module_id = 0; }

   /// returns the module name
   virtual CString getModuleName()=0;

   /// returns the last error
   virtual CString getLastError()=0;

   /// returns if the module is available
   virtual bool isAvailable()=0;

   /// returns module id
   int getModuleID(){ return module_id; }

   /// returns description of current file
   virtual void getDescription(CString& desc){ desc.Empty(); }

   /// returns version string; value in special may denote special type of string
   virtual void getVersionString(CString& version, int special = 0){ version.Empty(); special; }

   /// resolves possibly encoded filenames
   virtual void resolveRealFilename(CString& filename){ filename; }

protected:
   /// module id
   int module_id;

   /// number of channels
   int channels;

   /// sample rate
   int samplerate;
};


/// input module base class

class InputModule: public ModuleBase
{
public:
   /// dtor
   virtual ~InputModule(){}

   /// clones input module
   virtual InputModule *cloneModule()=0;

   /// returns filter string
   virtual CString getFilterString()=0;

   /// initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager &mgr,
      TrackInfo &trackinfo, SampleContainer &samples)=0;

   /// returns info about the input file
   virtual void getInfo(int &channels, int &bitrate, int &length, int &samplerate)=0;

   /// \brief decodes samples and stores them in the sample container
   /// \details returns number of samples decoded, or 0 if finished
   /// a negative value indicates an error
   virtual int decodeSamples(SampleContainer &samples)=0;

   /// returns the number of percent done
   virtual float percentDone(){ return 0.f; }

   /// called when done with decoding
   virtual void doneInput(){}

   /// called when done with decoding
   virtual void doneInput(bool /*fCompletedTrack*/){ doneInput(); }
};


/// output module base class

class OutputModule: public ModuleBase
{
public:
   /// dtor
   virtual ~OutputModule(){}

   /// returns the extension the output module produces (e.g. "mp3")
   virtual LPCTSTR getOutputExtension()=0;

   /// lets the output module fetch some settings, right after module creation
   virtual void prepareOutput(SettingsManager &mgr){ mgr; }

   /// initializes the output module
   virtual int initOutput(LPCTSTR outfilename, SettingsManager &mgr,
      const TrackInfo& trackinfo, SampleContainer &samplecont)=0;

   /// \brief encodes samples from the sample container
   /// \details it is required that all samples from the container will be used up;
   /// returns 0 if all was ok, or a negative value on error
   virtual int encodeSamples(SampleContainer &samples)=0;

   /// cleans up the output module
   virtual void doneOutput(){}
};


/// returns a filename compatible for ansi APIs such as fopen()
CString GetAnsiCompatFilename(LPCTSTR pszFilename);

/// @}
