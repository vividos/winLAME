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

*/
/*! \file VariableManager.h

   \brief manager for variables and their properties

   manager for variables, their string names, descriptions and default values
   also, herein are defined the enum values for the variable ID's to use in
   the settings manager

*/
/*! \ingroup settings */
/*! @{ */

// include guard
#pragma once

// forward declaration
struct SettingsVarMap;


//! class for managing settings variables
class VariableManager
{
public:
   // ctor
   VariableManager(){ varmap=NULL; }

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
   const SettingsVarMap *varmap;
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
   VarFirst = 0,
   LameOptNoGap,
   LameHideSettings,
   LameWriteWaveHeader,

   LameSimpleMode,
   LameSimpleEncodeQuality,
   LameSimpleMono,
   LameSimpleQualityOrBitrate,
   LameSimpleBitrate,
   LameSimpleQuality,
   LameSimpleCBR,
   LameSimpleVBRMode,

   OggBitrateMode,
   OggBaseQuality,
   OggVarMinBitrate,
   OggVarMaxBitrate,
   OggVarNominalBitrate,

   WaveRawAudioFile,
   WaveOutputFormat,
   WaveFileFormat,
   WaveWriteWavEx,

   AacBitrate,
   AacBandwidth,
   AacMpegVersion,
   AacObjectType,
   AacAllowMS,
   AacUseTNS,
   AacUseLFEChan,
   AacBRCMethod,
   AacQuality,
   AacAutoBandwidth,

   wlGeneralIsLastFile,

   WmaBitrate,
   WmaQuality,
   WmaBitrateMode,

   VarLast
};


//! manager class for the facilities
class VarMgrFacilities: public VariableManager
{
public:
   VarMgrFacilities();
};


//! manager class for the variables
class VarMgrVariables: public VariableManager
{
public:
   VarMgrVariables();
};


//@}
