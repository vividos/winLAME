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

   $Id: wlWinampPluginInputModule.cpp,v 1.25 2009/12/17 18:49:51 vividos Exp $

*/
/*! \file wlWinampPluginInputModule.cpp

   \brief contains the implementation of the winamp plugin input module

*/

// needed includes
#include "stdafx.h"
#include "resource.h"
#include <algorithm>
#include "wlWinampPluginInputModule.h"
#include <sys/stat.h>
#include <stdio.h>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// wlWinampPluginInputModule methods

wlWinampPluginInputModule::wlWinampPluginInputModule()
{
   module_id = ID_IM_WINAMP;
   free_mod = true;
}

wlWinampPluginInputModule::~wlWinampPluginInputModule()
{
   if (free_mod)
      ::FreeModule(mod);
}

wlInputModule *wlWinampPluginInputModule::cloneModule()
{
   wlWinampPluginInputModule *inmod2 = new wlWinampPluginInputModule;
   inmod2->setDLLModuleHandle(mod);
   inmod2->free_mod = false;
   return inmod2;
}

void wlWinampPluginInputModule::setDLLModuleHandle(HMODULE themod)
{
   mod = themod;

   typedef In_Module *(*winampGetInModule2func)();

   // try to get function
   FARPROC func = ::GetProcAddress(mod,"winampGetInModule2");
   if (func==NULL)
      inmod = NULL;
   else
   {
      // retrieve input module
      winampGetInModule2func winampGetInModule2;
      winampGetInModule2 = (winampGetInModule2func)func;
      inmod = winampGetInModule2();
      inmod->hMainWindow = NULL;
      inmod->hDllInstance = mod;
      inmod->Init();
      started = false;

      // get module name
      USES_CONVERSION;
      module_name.Format(_T("Winamp Input Module \"%hs\""), inmod->description);

      // build filter string
      char* ext = inmod->FileExtensions;
      while(*ext!=0)
      {
         // semicolon-separated extensions
         USES_CONVERSION;
         CString ext_str(A2CT(ext));

         // search for next 0 char
         while(*ext++!=0);

         // append description
         filterstring += A2CT(ext);
         filterstring += TCHAR('|');

         // search for next 0 char
         while(*ext++!=0);

         // build filter string
         ext_str.Insert(0, _T("*."));
         int iPos = 0;
         while(-1 != (iPos = ext_str.Find(';', iPos)) )
            ext_str.Insert(++iPos, _T("*."));

         filterstring += ext_str;
         filterstring += _T('|');
      }
   }
}

void wlWinampPluginInputModule::configureModule()
{
   inmod->Config(NULL/*::GetActiveWindow()*/);
}

bool wlWinampPluginInputModule::isAvailable()
{
   return (inmod!=NULL && inmod->UsesOutputPlug!=0);
}

namespace
{

// mutex to lock buffer access
HANDLE mutex;

// mutex functions

void mutex_create()
{
   mutex = ::CreateMutex(NULL, FALSE, NULL);
}

void mutex_delete()
{
   ::CloseHandle(mutex);
}

bool mutex_lock()
{
   DWORD ret = WaitForSingleObject(mutex,INFINITE);
   return ret == WAIT_OBJECT_0;
}

bool mutex_try_lock()
{
   DWORD ret = WaitForSingleObject(mutex,0);
   return ret == WAIT_OBJECT_0;
}

void mutex_unlock()
{
   ::ReleaseMutex(mutex);
}


// input file properties
int g_samplerate;
int g_numchannels;
int g_bitspersamp;

// sample buffer
const int buffer_size = 8192;
int buf_fill;
short buffer_samples[8192];


// dummy input module functions
void in_setinfo(int,int,int,int){}
void in_savsainit(int,int){}
void in_vsasetinfo(int,int){}
int in_dsp_isactive(){ return 0; }
void in_savsadeinit(){}
int in_sagetmode(){ return 1; }
int in_vsagetmode(int *a, int *b){ *a = *b = 0; return 0; }
void in_saadd(void *,int,int){}
void in_saaddpcmdata(void *PCMData, int nch, int bps, int timestamp){}
void in_vsaaddpcmdata(void *PCMData, int nch, int bps, int timestamp){}
int dsp_dosamples(short int *samples, int numsamples, int bps, int nch, int srate){ return numsamples; }
void in_vsaadd(void *data, int timestamp){}

// dummy output module functions
void out_config(HWND){}
void out_about(HWND){}
void out_init(){}
void out_quit(){}
void out_close(){}
int out_pause(int p){ return 0; }
void out_setVolume(int vol){}
void out_setPan(int pan){}
void out_flush(int t){}
int out_getOutputTime(){ return 0; }
int out_getWrittenTime(){ return 0; }


// called when opening output module
int out_open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
   mutex_lock();

   g_samplerate = samplerate;
   g_numchannels = numchannels;
   g_bitspersamp = bitspersamp;

   mutex_unlock();

   return 0;
}

// called to write output samples
int out_write(char *buf, int len)
{
   // lock mutex
   bool loop = false;
   do
   {
      mutex_lock();

      if (buf_fill<0)
         break;

      if (buf_fill!=0)
      {
         // do another loop
         loop = true;
      }
      else
      {
         // copy samples to sample buffer
         memcpy(buffer_samples,buf,len);
         buf_fill = len/g_numchannels/2;//(2*g_numchannels);
      }

      mutex_unlock();

      if (loop)
         Sleep(0);

   } while(loop);

   return 0;
}

// called to determine how many bytes can be written
int out_canWrite()
{
   int numbytes = 0;

   mutex_lock();

   if (buf_fill==0)
      numbytes = buffer_size;

   mutex_unlock();

   return numbytes;
}

// called when input module stops
int out_isPlaying()
{
   mutex_lock();

   buf_fill = -1;

   mutex_unlock();

   return 0; // 0 == not playing anymore
}

} // end unnamed namespace


int wlWinampPluginInputModule::initInput(LPCTSTR infilename,
   wlSettingsManager &mgr, wlTrackInfo &trackinfo,
   wlSampleContainer &samplecont)
{
   if (inmod==NULL)
   {
      lasterror = _T("incorrect input module handle!");
      return -1;
   }

   // setup input module
   inmod->outMod = &outmod;
   inmod->SetInfo = in_setinfo;
   inmod->SAVSAInit = in_savsainit;
   inmod->VSASetInfo = in_vsasetinfo;
   inmod->dsp_isactive = in_dsp_isactive;
   inmod->SAVSADeInit = in_savsadeinit;
   inmod->SAGetMode = in_sagetmode;
   inmod->VSAGetMode = in_vsagetmode;
   inmod->SAAdd = in_saadd;
   inmod->SAAddPCMData = in_saaddpcmdata;
   inmod->VSAAddPCMData = in_vsaaddpcmdata;
   inmod->dsp_dosamples = dsp_dosamples;
   inmod->VSAAdd = in_vsaadd;

   // setup output module
   outmod.version = OUT_VER;
   outmod.description = "fake winLAME output module";
   outmod.id = 65536;
   outmod.hMainWindow = inmod->hMainWindow;
   outmod.hDllInstance = NULL;
   outmod.Config = out_config;
   outmod.About = out_about;
   outmod.Init = out_init;
   outmod.Quit = out_quit;
   outmod.Open = out_open;
   outmod.Close = out_close;
   outmod.Write = out_write;
   outmod.CanWrite = out_canWrite;
   outmod.IsPlaying = out_isPlaying;
   outmod.Pause = out_pause;
   outmod.SetVolume = out_setVolume;
   outmod.SetPan = out_setPan;
   outmod.Flush = out_flush;
   outmod.GetOutputTime = out_getOutputTime;
   outmod.GetWrittenTime = out_getWrittenTime;

   // remember filename
   in_filename = infilename;

   g_samplerate = -1;
   g_numchannels = -1;
   g_bitspersamp = -1;
   buf_fill = -1; // disable buffer fill

   // create mutex to lock buffer access
   mutex_create();

   // try to retrieve sample rate and channel numbers
   int length_in_ms=0;
   {
      USES_CONVERSION;
      inmod->Play(const_cast<char*>(T2CA(wlGetAnsiCompatFilename(infilename))));

      bool loop = true;
      do
      {
         mutex_lock();

         if (g_samplerate==-1)
            Sleep(0);
         else
            loop = false;

         mutex_unlock();
      }
      while(loop);

      length_in_ms = inmod->GetLength();

      inmod->Stop();
   }

   buf_fill = 0; // enable buffer fill

   numsamples = (unsigned __int64)((length_in_ms/1000.0)*double(g_samplerate));
   samplecount = 0;

   // setup input traits
   samplecont.setInputModuleTraits(16,wlSamplesInterleaved,
      g_samplerate, g_numchannels);

   return 0;
}

void wlWinampPluginInputModule::getDescription(CString& desc)
{
   desc.Format(_T("%s, %u Hz, %u channels, %u bits per sample"),
      module_name, g_samplerate, g_numchannels, g_bitspersamp);
}

CString wlWinampPluginInputModule::getFilterString()
{
   return filterstring;
}

void wlWinampPluginInputModule::getInfo(int &ch, int &bitrate, int &length, int &srate)
{
   ch = g_numchannels;
   srate = g_samplerate;
   length = int(numsamples/g_samplerate);

   // determine bitrate
   {
      USES_CONVERSION;
      struct _stat statbuf;
      _tstat(in_filename, &statbuf);

      bitrate = int(double(statbuf.st_size*8)/double(__int64(numsamples))*double(g_samplerate));
   }
}

int wlWinampPluginInputModule::decodeSamples(wlSampleContainer &samples)
{
   if (!started)
   {
      // lock mutex
      mutex_lock();

      USES_CONVERSION;
      inmod->Play(const_cast<char*>(T2CA(wlGetAnsiCompatFilename(in_filename))));
      started = true;

      // release mutex
      mutex_unlock();
   }

   int res = 0;

   bool loop = true;
   do
   {
      // lock mutex
      mutex_lock();

      // now we have exclusive access to the buffer

      if (buf_fill<0)
      {
         // stop processing
         loop = false;
         res = 0;
      }
      else
      if (buf_fill != 0)
      {
         loop = false;

         // put samples in container
         samples.putSamplesInterleaved(buffer_samples,buf_fill);

         res = buf_fill;
         buf_fill = 0;
      }

      // release mutex
      mutex_unlock();

   } while(loop);

   samplecount += res;

   return res;
}

void wlWinampPluginInputModule::doneInput()
{
   // stop input module thread
   if (inmod!=NULL && started)
   {
      inmod->Stop();
      inmod->Quit();
      inmod->outMod = NULL;
   }

   // wait for mutex
   mutex_lock();
   mutex_unlock();

   // delete mutex
   mutex_delete();
}
