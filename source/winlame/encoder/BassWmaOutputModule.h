/*
   winLAME - a frontend for the LAME encoding engine
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

   $Id: BassWmaOutputModule.h,v 1.5 2010/01/08 19:58:43 vividos Exp $

*/
/*! \file BassWmaOutputModule.h

   \brief contains the basswma output module definition

*/
/*! \ingroup encoder */
/*! @{ */

// prevent multiple including
#ifndef __wlbasswmaoutputmodule_h_
#define __wlbasswmaoutputmodule_h_

// needed includes
#include "ModuleInterface.h"
#include "bass.h"
#include "basswma.h"


//! basswma output module

class BassWmaOutputModule: public OutputModule
{
public:
   // ctor
   BassWmaOutputModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("Windows Media Audio Encoder"); }

   // returns the last error
   virtual CString getLastError(){ return lasterror; }

   // returns if the module is available
   virtual bool isAvailable();

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns the extension the output module produces
   virtual LPCTSTR getOutputExtension(){ return _T("wma"); }

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
   //! last error occured
   CString lasterror;

   HWMENCODE handle; ///< encoder handle
   int samplerate;   ///< sample rate
   int channels;  ///< number of channels
   int bitrate;   ///< bitrate
   int brmode;    ///< bitrate mode
};


//@}

#endif
