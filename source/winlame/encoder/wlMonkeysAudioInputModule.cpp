/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2004 Kjetil Haga
   Copyright (c) 2004 DeXT
   Copyright (c) 2007 Michael Fink
*/
/*!
*   \author   Kjetil Haga (programmer.khaga@start.no)
*/

#include "stdafx.h"
#include "resource.h"
#include <stdio.h>
#include "wlMonkeysAudioInputModule.h"
#include <cassert>
#include "wlId3v1Tag.h"

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



// static references
monkey::MAC_DLL wlMonkeysAudioInputModule::dll;


namespace monkey
{

   // buffer size for monkey's audio module
   const int MAC_BUFFER_SIZE = 8192;


   // checks the interface version
   int VersionCheckInterface(HMODULE hMACDll)
   {
      assert(hMACDll);
      
      int nRetVal = -1;
      proc_GetInterfaceCompatibility GetInterfaceCompatibility = 
         (proc_GetInterfaceCompatibility) GetProcAddress(hMACDll, "GetInterfaceCompatibility");
      if (GetInterfaceCompatibility)
      {
         nRetVal = GetInterfaceCompatibility(MAC_VERSION_NUMBER, TRUE, NULL);
      }

      return nRetVal;
   }


   // encodes an error string based on macdll error code
   void EncodeMonkeyErrorString(int errorCode, CString& desc)
   {
      desc = _T("APE error (");

      switch(errorCode)
      {
      case ERROR_IO_READ:
         desc += _T("I/O read error");
         break;
      case ERROR_IO_WRITE:
         desc += _T("I/O write error");
         break;
      case ERROR_INVALID_INPUT_FILE:
         desc += _T("invalid input file");
         break;
      case ERROR_INVALID_OUTPUT_FILE:
         desc += _T("invalid output file");
         break;
      case ERROR_INPUT_FILE_TOO_LARGE:
         desc += _T("input file file too large");
         break;
      case ERROR_INPUT_FILE_UNSUPPORTED_BIT_DEPTH:
         desc += _T("input file unsupported bit depth");
         break;
      case ERROR_INPUT_FILE_UNSUPPORTED_SAMPLE_RATE:
         desc += _T("input file unsupported sample rate");
         break;
      case ERROR_INPUT_FILE_UNSUPPORTED_CHANNEL_COUNT:
         desc += _T("input file unsupported channel count");
         break;
      case ERROR_INPUT_FILE_TOO_SMALL:
         desc += _T("input file too small");
         break;
      case ERROR_INVALID_CHECKSUM:
         desc += _T("invalid checksum");
         break;
      case ERROR_DECOMPRESSING_FRAME:
         desc += _T("decompressing frame");
         break;
      case ERROR_INITIALIZING_UNMAC:
         desc += _T("initializing unmac");
         break;
      case ERROR_INVALID_FUNCTION_PARAMETER:
         desc += _T("invalid function parameter");
         break;
      case ERROR_UNSUPPORTED_FILE_TYPE:
         desc += _T("unsupported file type");
         break;
      case ERROR_INSUFFICIENT_MEMORY:
         desc += _T("insufficient memory");
         break;
      case ERROR_LOADINGAPE_DLL:
         desc += _T("loading MAC.dll");
         break;
      case ERROR_LOADINGAPE_INFO_DLL:
         desc += _T("loading MACinfo.dll");
         break;
      case ERROR_LOADING_UNMAC_DLL:
         desc += _T("loading UnMAC.dll");
         break;
      case ERROR_USER_STOPPED_PROCESSING:
         desc += _T("user stopped processing");
         break;
      case ERROR_SKIPPED:
         desc += _T("skipped...");
         break;
      case ERROR_BAD_PARAMETER:
         desc += _T("bad parameter");
         break;
      case ERROR_APE_COMPRESS_TOO_MUCH_DATA:
         desc += _T("APE compress too much data");
         break;
      default:
         desc += _T("undefined error");
         break;
      }

      CString tmp;
      tmp.Format(_T("; code: %d)"), errorCode);

      desc += tmp;
   }
}




wlMonkeysAudioInputModule::wlMonkeysAudioInputModule() : macfile(0), samplecount(0), totalsamples(0)
{

}

wlMonkeysAudioInputModule::~wlMonkeysAudioInputModule()
{

}


wlInputModule *wlMonkeysAudioInputModule::cloneModule()
{
   return new wlMonkeysAudioInputModule;
}


bool wlMonkeysAudioInputModule::isAvailable()
{
   using namespace monkey;
   
   while(!dll.module)
   {
      // load dll
      dll.module = ::LoadLibraryA("MACDll.dll");

      if(!dll.module) break;

      // check interface version
      if (VersionCheckInterface(dll.module) != 0)
      {
         ::FreeLibrary(dll.module);
         break;
      }

      // load the functions
      dll.Create      = (proc_APEDecompress_Create) GetProcAddress   (dll.module, "c_APEDecompress_Create");
      dll.Destroy      = (proc_APEDecompress_Destroy) GetProcAddress   (dll.module, "c_APEDecompress_Destroy");
      dll.GetData      = (proc_APEDecompress_GetData) GetProcAddress   (dll.module, "c_APEDecompress_GetData");
      dll.Seek      = (proc_APEDecompress_Seek) GetProcAddress      (dll.module, "c_APEDecompress_Seek");
      dll.GetInfo      = (proc_APEDecompress_GetInfo) GetProcAddress   (dll.module, "c_APEDecompress_GetInfo");
      dll.GetID3Tag   = (proc_APEGetID3Tag) GetProcAddress         (dll.module, "GetID3Tag");

      // error check (unload dll on failure)
      if ((dll.Create      == 0) ||
         (dll.Destroy   == 0) ||
         (dll.GetData   == 0) ||
         (dll.Seek      == 0) ||
         (dll.GetInfo   == 0  ))
      {
         ::FreeLibrary(dll.module);
         dll.module = 0;
      }

      break;
   }

   return dll.module ? true : false;
}


void wlMonkeysAudioInputModule::getDescription(CString& desc)
{
   ATLASSERT(dll.module);

   // sanity check .. ;)
   if(!macfile)
   {
      return;
   }
   
   // get fileinfo
   int samplerate = dll.GetInfo(macfile, monkey::APE_INFO_SAMPLE_RATE, 0, 0);
   int channels = dll.GetInfo(macfile, monkey::APE_INFO_CHANNELS, 0, 0);
   int level = dll.GetInfo(macfile, monkey::APE_INFO_COMPRESSION_LEVEL, 0, 0);

   // create compression level string
   LPCTSTR level_str;
   switch(level)
   {
   case COMPRESSION_LEVEL_FAST:
      level_str = _T("Fast compression");
      break;
   case COMPRESSION_LEVEL_NORMAL:
      level_str = _T("Normal compression");
      break;
   case COMPRESSION_LEVEL_HIGH:
      level_str = _T("High compression");
      break;
   case COMPRESSION_LEVEL_EXTRA_HIGH:
      level_str = _T("Extra high compression");
      break;
   case COMPRESSION_LEVEL_INSANE:
      level_str = _T("Insane compression");
      break;
   default:
      level_str = _T("Unknown compression level.");
      break;
   }

   // encode desc string
   desc.Format(IDS_FORMAT_INFO_MONKEYS_AUDIO,
      level_str, samplerate, channels);
}

void wlMonkeysAudioInputModule::getVersionString(CString& version, int special)
{
   version = MAC_VERSION_STRING;
}


CString wlMonkeysAudioInputModule::getFilterString()
{
   CString cszFilter;
   cszFilter.LoadString(IDS_FILTER_MONKEYS_AUDIO_INPUT);
   return cszFilter;
}


int wlMonkeysAudioInputModule::initInput(LPCTSTR infilename,
                               wlSettingsManager &mgr, wlTrackInfo &trackinfo,
                               wlSampleContainer &samplecont)
{
   assert(dll.module);

   // close old file if still open
   if(macfile)
   {
      dll.Destroy(macfile);
      macfile = 0;
   }
   
   //internal retval used by macdll
   int retval = 0;
   
   // resets the sample count
   samplecount = 0;

   // get id3 tag
   getId3Tag(infilename, trackinfo);
   
   // opens the file for reading
   USES_CONVERSION;
   macfile = dll.Create(T2CA(wlGetAnsiCompatFilename(infilename)), &retval);
   if(NULL == macfile)
   {
      monkey::EncodeMonkeyErrorString(retval, lasterror);
      return -1;
   }
   
   // get some fileinfo
   int samplerate = dll.GetInfo(macfile, monkey::APE_INFO_SAMPLE_RATE, 0, 0);
   int channels = dll.GetInfo(macfile, monkey::APE_INFO_CHANNELS, 0, 0);
   int bps = dll.GetInfo(macfile, monkey::APE_INFO_BITS_PER_SAMPLE, 0, 0);

   // set total samples in file (for stats update)
   totalsamples = dll.GetInfo(macfile, monkey::APE_DECOMPRESS_TOTAL_BLOCKS, 0, 0);
   
   // set up input traits
   samplecont.setInputModuleTraits(bps, wlSamplesInterleaved, samplerate, channels);
   
   return 0;
}


void wlMonkeysAudioInputModule::getInfo(int &channels, int &bitrate, int &length, int &samplerate)
{
   assert(dll.module);
   
   if (macfile!=0)
   {
      // bits per sample
      int bits_per_sample = dll.GetInfo(macfile, monkey::APE_INFO_BITS_PER_SAMPLE, 0, 0);
      
      // number of channels
      channels = dll.GetInfo(macfile, monkey::APE_INFO_CHANNELS, 0, 0);
      
      // samplerate
      samplerate = dll.GetInfo(macfile, monkey::APE_INFO_SAMPLE_RATE, 0, 0);

      // length (seconds)
      length = dll.GetInfo(macfile, monkey::APE_DECOMPRESS_LENGTH_MS, 0, 0) / 1000;
            
      // bitrate
      bitrate = samplerate * bits_per_sample;
   }
}


int wlMonkeysAudioInputModule::decodeSamples(wlSampleContainer &samples)
{
   assert(dll.module && macfile);

   unsigned char buffer[monkey::MAC_BUFFER_SIZE];
   int blocks_retrieved = 0;
   
   // get data from file
   int blockalign = dll.GetInfo(macfile, monkey::APE_INFO_BLOCK_ALIGN, 0, 0);
   int retval = dll.GetData(macfile, reinterpret_cast<char*>(buffer), monkey::MAC_BUFFER_SIZE / blockalign, &blocks_retrieved);

   // success?
   if(0 != retval)
   {
      monkey::EncodeMonkeyErrorString(retval, lasterror);
      return (retval * -1);
   }

   // if we are dealing with 8-bit samples we have to convert them to signed samples
   // (8-bit samples from monkey's audio are unsigned)
   int bits_per_sample = dll.GetInfo(macfile, monkey::APE_INFO_BITS_PER_SAMPLE, 0, 0);
   if(8 == bits_per_sample)
   {
      for(int i=0; i<monkey::MAC_BUFFER_SIZE; ++i)
      {
         buffer[i] = ((buffer[i] & 0x80) ^ 0x80) | (buffer[i] & 0x7f);   //invert most significant bit
      }
   }

   // update stats
   samplecount += blocks_retrieved;

   // put samples in container
   samples.putSamplesInterleaved(buffer, blocks_retrieved);
   
   // return samples retrieved
   return blocks_retrieved;
}


void wlMonkeysAudioInputModule::doneInput()
{
   // close file if open
   if(macfile)
   {
      dll.Destroy(macfile);
      macfile = 0;
   }
}

bool wlMonkeysAudioInputModule::getId3Tag(LPCTSTR filepath, wlTrackInfo& trackinfo)
{
   assert(dll.module);

   wlId3v1Tag id3tag;
   monkey::ID3_TAG *info = (monkey::ID3_TAG *)id3tag.getData();
   
   USES_CONVERSION;
   if(ERROR_SUCCESS != dll.GetID3Tag(T2CA(wlGetAnsiCompatFilename(filepath)), info) )
      return false;

   id3tag.toTrackInfo(trackinfo);

   return true;
}
