/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2007 Michael Fink
   Copyright (c) 2004 DeXT

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

   $Id: WaveOutputModule.h,v 1.17 2010/01/08 19:58:43 vividos Exp $

*/
/*! \file WaveOutputModule.h

   \brief contains the wave output module definition

*/
/*! \ingroup encoder */
/*! @{ */

// prevent multiple including
#ifndef __wlwaveoutputmodule_h_
#define __wlwaveoutputmodule_h_

// needed includes
#include "ModuleInterface.h"
#include "sndfile.h"


//! wave output module

class WaveOutputModule: public OutputModule
{
public:
   // ctor
   WaveOutputModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("Wave Output"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns the extension the output module produces
   virtual LPCTSTR getOutputExtension();

   // lets the output module fetch some settings, right after module creation
   void prepareOutput(SettingsManager &mgr);

   // initializes the output module
   virtual int initOutput(LPCTSTR outfilename, SettingsManager &mgr,
      const TrackInfo& trackinfo, SampleContainer &samplecont);

   // encodes samples from the sample container
   virtual int encodeSamples(SampleContainer &samples);

   // cleans up the output module
   virtual void doneOutput();

protected:
   //! file handle
   SNDFILE *sndfile;

   //! soundfile info
   SF_INFO sfinfo;

   //! last error occured
   CString lasterror;

   //! indicates if a raw file is to be written
   bool rawfile;

   //! write waveformatextensible header
   bool wavex;

   //! output format
   int outfmt;

   //! file format
   int filefmt;
};


//@}

#endif
