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
/// \file VariableManager.cpp
/// \brief contains the variable mapping lists and lookup functions

// needed includes
#include "stdafx.h"
#include "VariableManager.h"
#include "ModuleInterface.h"
#include "resource.h"

/// variable mapping struct
struct SettingsVarMap
{
   unsigned short id;   ///< settings id
   LPCTSTR name;        ///< name
   LPCTSTR desc;        ///< description
   int defvalue;        ///< default value
};


// VariableManager methods

int VariableManager::lookupID(LPCTSTR name)
{
   const SettingsVarMap *varmap = m_varmap;

   int index = -1;
   while(varmap->name!=NULL)
   {
      if (_tcscmp(name,varmap->name)==0)
      {
         index = varmap->id;
         break;
      }
      varmap++;
   }
   return index;
}

LPCTSTR VariableManager::lookupName(int varID)
{
   const SettingsVarMap *varmap = m_varmap;

   LPCTSTR name = _T("");
   while(varmap->name!=NULL)
   {
      if (varID==varmap->id)
      {
         name = varmap->name;
         break;
      }
      varmap++;
   }
   return name;
}

LPCTSTR VariableManager::lookupDescription(int varID)
{
   const SettingsVarMap *varmap = m_varmap;

   LPCTSTR desc = _T("");
   while(varmap->name!=NULL)
   {
      if (varID==varmap->id)
      {
         desc = varmap->desc;
         break;
      }
      varmap++;
   }
   return desc;
}

int VariableManager::lookupDefaultValue(int varID)
{
   const SettingsVarMap *varmap = m_varmap;

   int defval=-1;
   while(varmap->name!=NULL)
   {
      if (varID==varmap->id)
      {
         defval = varmap->defvalue;
         break;
      }
      varmap++;
   }
   return defval;
}


// macros for building a varmap

/// defines settings var map
#define WL_VARMAP_START(x)       const SettingsVarMap x[] = {
/// defines settings variable entry
#define WL_VARMAP_ENTRY1(x,y,z)  { x, y, z, 0 },
/// defines settings variable entry with default value
#define WL_VARMAP_ENTRY(x,y,z,w) { x, y, z, w },
/// ends settings var map
#define WL_VARMAP_END()          { 0, NULL, NULL } };


// facility varmap

WL_VARMAP_START(varMapFacility)
   WL_VARMAP_ENTRY1(FacilityLAME,     _T("lame"),     _T("LAME"))
   WL_VARMAP_ENTRY1(FacilityOggVorbis,_T("oggvorbis"),_T("Ogg Vorbis"))
   WL_VARMAP_ENTRY1(FacilityWave,     _T("wave"),     _T("Wave"))
   WL_VARMAP_ENTRY1(FacilityAAC,      _T("aac"),      _T("AAC"))
   WL_VARMAP_ENTRY1(FacilityWma,      _T("wma"),      _T("WMA"))
WL_VARMAP_END()


// facility to dialog id map

WL_VARMAP_START(varMapFacilityToModule)
   WL_VARMAP_ENTRY1(ID_OM_LAME,        _T("lame"),     _T("LAME"))
   WL_VARMAP_ENTRY1(ID_OM_OGGV,        _T("oggvorbis"),_T("Ogg Vorbis"))
   WL_VARMAP_ENTRY1(ID_OM_WAVE,        _T("wave"),     _T("Wave"))
   WL_VARMAP_ENTRY1(ID_OM_AAC,         _T("aac"),      _T("AAC"))
   WL_VARMAP_ENTRY1(ID_OM_BASSWMA,     _T("wma"),      _T("WMA"))
WL_VARMAP_END()


// variable varmap

WL_VARMAP_START(varMapVariables)
   // persistent encoder variables
   WL_VARMAP_ENTRY(LameOptNoGap,              _T("lameNoGap"),          _T("nogap Encoding"), 0)
   WL_VARMAP_ENTRY(LameWriteWaveHeader,       _T("lameWriteWaveHeader"),_T("write Wave Header"), 0)
   WL_VARMAP_ENTRY(LameHideSettings,          _T("lameHideSettings"), _T("hide LAME Settings Page"), 0)

   WL_VARMAP_ENTRY(LameSimpleEncodeQuality,   _T("lameEncodeQuality"),    _T("LAME encode quality"), 1)
   WL_VARMAP_ENTRY(LameSimpleMono,            _T("lameMono"),             _T("LAME mono"), 0)
   WL_VARMAP_ENTRY(LameSimpleQualityOrBitrate,_T("lameQualityOrBitrate"), _T("LAME bitrate/quality selection"), 0)
   WL_VARMAP_ENTRY(LameSimpleBitrate,         _T("lameBitrate"), _T("LAME CBR/ABR bitrate"), 192)
   WL_VARMAP_ENTRY(LameSimpleQuality,         _T("lameQuality"), _T("LAME quality"), 4)
   WL_VARMAP_ENTRY(LameSimpleCBR,             _T("lameCBR"),     _T("LAME CBR option"), 0)
   WL_VARMAP_ENTRY(LameSimpleVBRMode,         _T("lameVBRMode"), _T("LAME VBR mode"), 0)

   WL_VARMAP_ENTRY(OggBitrateMode,   _T("oggBitrateMode"),    _T("Bitrate Mode"),   0)
   WL_VARMAP_ENTRY(OggBaseQuality,   _T("oggBaseQuality"),    _T("Base Quality"),   400)
   WL_VARMAP_ENTRY(OggVarMinBitrate, _T("oggVarMinBitrate"),  _T("min. Bitrate"),   64)
   WL_VARMAP_ENTRY(OggVarMaxBitrate, _T("oggVarMaxBitrate"),  _T("max. Bitrate"),   256)
   WL_VARMAP_ENTRY(OggVarNominalBitrate,   _T("oggVarNominal"),  _T("nominal Bitrate"),128)

   WL_VARMAP_ENTRY(AacBitrate,          _T("aacBitrate"),        _T("Bitrate"),           128)
   WL_VARMAP_ENTRY(AacBandwidth,        _T("aacBandwidth"),      _T("Bandwidth"),         16000)
   WL_VARMAP_ENTRY(AacMpegVersion,      _T("aacMpegVer"),        _T("MPEG Version"),      2)
   WL_VARMAP_ENTRY(AacObjectType,       _T("aacObjectType"),     _T("AAC Object Type"),   1)
   WL_VARMAP_ENTRY(AacAllowMS,          _T("aacAllowMS"),        _T("allow Mid/Side"),    1)
   WL_VARMAP_ENTRY(AacUseTNS,           _T("aacUseTNS"),         _T("use TNS"),           1)
   WL_VARMAP_ENTRY(AacUseLFEChan,       _T("aacUseLFE"),         _T("use LFE channel"),   1)
   WL_VARMAP_ENTRY(AacBRCMethod,        _T("aacBRCmethod"),      _T("Bitrate Control Method"), 0)
   WL_VARMAP_ENTRY(AacQuality,          _T("aacQuality"),        _T("Quantizer Quality"), 100)
   WL_VARMAP_ENTRY(AacAutoBandwidth,    _T("aacAutoBandwidth"),  _T("set automatic bandwidth"), 1)

   WL_VARMAP_ENTRY(WmaBitrate,          _T("wmaBitrate"),     _T("WMA Output Bitrate"),64)
   WL_VARMAP_ENTRY(WmaQuality,          _T("wmaQuality"),     _T("WMA VBR Quality"),   50)
   WL_VARMAP_ENTRY(WmaBitrateMode,      _T("wmaBitrateMode"), _T("Bitrate Control Mode"),0)

   WL_VARMAP_ENTRY(WaveRawAudioFile,    _T("waveRaw"),        _T("Write RAW file"),0)
   WL_VARMAP_ENTRY(WaveOutputFormat,    _T("waveOutFmt"),     _T("Output Format"),0)
   WL_VARMAP_ENTRY(WaveFileFormat,      _T("waveFileFmt"),    _T("File Format"),0)
   WL_VARMAP_ENTRY(WaveWriteWavEx,      _T("waveWavEx"),      _T("Write WavEx Header"),0)

   WL_VARMAP_ENTRY(GeneralIsLastFile,   _T("isLastFile"),     _T("is last file"),      0)
WL_VARMAP_END()


// VariableManager derived ctors

VarMgrFacilities::VarMgrFacilities()
{
   m_varmap = varMapFacility;
}

VarMgrVariables::VarMgrVariables()
{
   m_varmap = varMapVariables;
}


// global methods

/// looks up facility name by module id
LPCTSTR LookupNamePerModuleID(int modID)
{
   VarMgrFacilities fac;
   return fac.lookupName(modID);
}
