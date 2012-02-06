/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2005 Michael Fink
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

   $Id: wlVariableManager.h,v 1.18 2005/11/02 20:49:42 vividos Exp $

*/
/*! \file wlVariableManager.h

   \brief manager for variables and their properties

   manager for variables, their string names, descriptions and default values
   also, herein are defined the enum values for the variable ID's to use in
   the settings manager

*/
/*! \ingroup settings */
/*! @{ */

// prevent multiple including
#ifndef wlvariablemanager_h_
#define wlvariablemanager_h_


// forward declaration
struct wlSettingsVarMap;


//! class for managing settings variables
class wlVariableManager
{
public:
   // ctor
   wlVariableManager(){ varmap=NULL; }

   //! looks up variable ID per name
   int lookupID(LPCTSTR name);

   //! looks up variable name per ID
   LPCTSTR lookupName(int varID);

   //! looks up variable description per ID
   LPCTSTR lookupDescription(int varID);

   //! looks up default variable value per ID
   int lookupDefaultValue(int varID);

protected:
   //! variable map
   const wlSettingsVarMap *varmap;
};


//! facility ID's

enum
{
   wlFacilityLAME = 1,
   wlFacilityOggVorbis,
   wlFacilityWave,
   wlFacilityAAC,
   wlFacilityWma
};


//! variable ID's

enum
{
   wlVarFirst = 0,
   wlLameOptNoGap,
   wlLameHideSettings,
   wlLameWriteWaveHeader,

   wlLameSimpleMode,
   wlLameSimpleEncodeQuality,
   wlLameSimpleMono,
   wlLameSimpleQualityOrBitrate,
   wlLameSimpleBitrate,
   wlLameSimpleQuality,
   wlLameSimpleCBR,
   wlLameSimpleVBRMode,

   wlOggBitrateMode,
   wlOggBaseQuality,
   wlOggVarMinBitrate,
   wlOggVarMaxBitrate,
   wlOggVarNominalBitrate,

   wlWaveRawAudioFile,
   wlWaveOutputFormat,
   wlWaveFileFormat,
   wlWaveWriteWavEx,

   wlAacBitrate,
   wlAacBandwidth,
   wlAacMpegVersion,
   wlAacObjectType,
   wlAacAllowMS,
   wlAacUseTNS,
   wlAacUseLFEChan,
   wlAacBRCMethod,
   wlAacQuality,
   wlAacAutoBandwidth,

   wlGeneralIsLastFile,

   wlWmaBitrate,
   wlWmaQuality,
   wlWmaBitrateMode,

   wlVarLast
};


//! manager class for the facilities
class wlVarMgrFacilities: public wlVariableManager
{
public:
   wlVarMgrFacilities();
};


//! manager class for the variables
class wlVarMgrVariables: public wlVariableManager
{
public:
   wlVarMgrVariables();
};


//@}

#endif
