/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2005-2007 Michael Fink

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
/*! \file wlCdReadoutModule.h

   \brief contains the Bass input module definition

*/
/*! \ingroup encoder */
/*! @{ */

// include guard
#pragma once

// needed includes
#include "ModuleInterface.h"
#include "SndFileInputModule.h"

//! CD readout input module

class CDReadoutModule: public SndFileInputModule
{
public:
   // ctor
   CDReadoutModule();
   virtual ~CDReadoutModule(){}

   //! clones input module
   virtual InputModule *cloneModule();

   // returns the module name
   virtual CString getModuleName(){ return _T("CD Audio Extraction"); }

   // returns description of current file
   virtual void getDescription(CString& desc);

   // returns filter string
   virtual CString getFilterString();

   // resolves possibly encoded filenames
   virtual void resolveRealFilename(CString& filename);

   // initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager &mgr,
      TrackInfo &trackinfo, SampleContainer &samples);

   // called when done with decoding
   virtual void doneInput(bool fCompletedTrack);

protected:
   /// track index
   unsigned int m_nTrackIndex;
};


//@}
