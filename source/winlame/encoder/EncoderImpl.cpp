/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2011 Michael Fink
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
/// \file EncoderImpl.cpp
/// \brief contains the actual encoder implementation

// needed includes
#include "stdafx.h"
#include "EncoderImpl.h"
#include <fstream>
#include "LameOutputModule.h"
#include <sndfile.h>

// static EncoderInterface methods

EncoderInterface* EncoderInterface::getNewEncoder()
{
   return new EncoderImpl;
}

// globals

static bool warned_about_lossy = false;


// EncoderImpl methods

void EncoderImpl::encode()
{
   lockAccess();

   percent = 0.f;
   error=0;
   bool init_outmod = false;

   // empty description string
   desc.Empty();

   // get input and output modules
   ModuleManagerImpl* modimpl = reinterpret_cast<ModuleManagerImpl*>(mod_manager);
   InputModule* inmod = modimpl->chooseInputModule(infilename);
   OutputModule* outmod = modimpl->getOutputModule(out_module_id);

   bool fCompletedTrack = false;

   // init input module
   TrackInfo trackinfo;
   SampleContainer sample_container;

   int res = inmod->initInput(infilename, *settings_mgr,
      trackinfo,sample_container);

   inmod->resolveRealFilename(infilename);

   // earliest place to not disturb "goto" scope
   // TODO remove goto
   CString outfilename;
   CString temp_outfilename;

   // catch errors
   if (handler!=NULL && res<0)
   {
      switch(handler->handleError(infilename, inmod->getModuleName(),
         -res, inmod->getLastError()))
      {
      case EncoderErrorHandler::Continue:
         break;
      case EncoderErrorHandler::SkipFile:
         error=1;
         goto skipfile;
         break;
      case EncoderErrorHandler::StopEncode:
         error=-1;
         goto skipfile;
         break;
      }
   }

   // prepare output module
   outmod->prepareOutput(*settings_mgr);

   // do output filename
   outfilename = GetOutputFilename(infilename, *outmod);

   // ugly hack: when input module is cd extraction, remove guid from output filename
   if (inmod->getModuleID() == ID_IM_CDRIP)
   {
      int iPos1 = outfilename.ReverseFind(_T('{'));
      int iPos2 = outfilename.ReverseFind(_T('}'));

      outfilename.Delete(iPos1, iPos2-iPos1+1);
   }

   // test if input and output file name is the same file
   if (!CheckSameInputOutputFilenames(infilename, outfilename, *outmod))
   {
      error=2;
      unlockAccess();
      goto skipfile;
   }

   // check if outfilename already exists
   if (!overwrite)
   {
      // test if file exists
      if (::GetFileAttributes(outfilename) != 0xFFFFFFFF)
      {
         // note: could do a message box here, but we really want the encoding progress
         // without any message boxes waiting.
//         if (AppMessageBox(GetActiveWindow(), _T("The output file already exists. Do you want to overwrite it?"), MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
//         {
            // skip this file
            error=2;
            unlockAccess();
            goto skipfile;
//         }
      }
   }

   // check if we only need copying a cd ripped file to another filename
   if (CheckCDExtractDirectCopy(*inmod, *outmod, *settings_mgr))
   {
      inmod->doneInput(false);
      delete inmod;
      inmod = NULL;

      BOOL fRet = MoveFile(infilename, outfilename);
      if (fRet == FALSE)
      {
         extern CString GetLastErrorString();
         DWORD dwError = GetLastError();
         CString cszErrorMessage = GetLastErrorString();

         CString cszCaption;
         cszCaption.LoadString(IDS_CDRIP_ERROR_CAPTION);
         EncoderErrorHandler::ErrorAction action = handler->handleError(outfilename, cszCaption, dwError, cszErrorMessage, true);
         switch (action)
         {
         case EncoderErrorHandler::Continue:
            break;
         case EncoderErrorHandler::StopEncode:
            error=-2;
            goto skipfile;
            break;
         }

      }
      unlockAccess();
      goto skip_movefile;
   }

   // generate temporary name, in case the output module doesn't support unicode filenames
   GenerateTempOutFilename(outfilename, temp_outfilename);

   // init output module
   res = outmod->initOutput(temp_outfilename,*settings_mgr,
      trackinfo,sample_container);

   init_outmod = true;

   // catch errors
   if (handler!=NULL && res<0)
   {
      switch(handler->handleError(infilename, outmod->getModuleName(),
         -res, outmod->getLastError()))
      {
      case EncoderErrorHandler::Continue:
         break;
      case EncoderErrorHandler::SkipFile:
         error=2;
         unlockAccess();
         goto skipfile;
         break;
      case EncoderErrorHandler::StopEncode:
         error=-2;
         unlockAccess();
         goto skipfile;
         break;
      }
   }

   FormatEncodingDescription(*inmod, *outmod, sample_container);

   unlockAccess();

   // check if transcoding
   // TODO move this to the wizard pages; warning about lossy conversion shouldn't be
   // done while actually encoding.
   if (!CheckWarnTranscoding(*inmod, *outmod))
   {
      // stop encoding
      error=-2;
      goto skipfile;
   }

   bool bSkipFile = false;
   MainLoop(*inmod, *outmod, sample_container, bSkipFile);
   if (bSkipFile)
      goto skipfile;

   // jump target when skipping main encoding loop for MoveFile() optimization
skip_movefile:

   // write playlist entry, when enabled
   if (running && !playlist_filename.IsEmpty())
      WritePlaylistEntry(outfilename);

   if (running)
      fCompletedTrack = true;

skipfile:
   // done with modules
   if (inmod)
      inmod->doneInput(fCompletedTrack);
   if (init_outmod)
      outmod->doneOutput();

   // delete modules
   delete inmod;
   delete outmod;

   // rename when we used a temporary filename
   if (!temp_outfilename.IsEmpty() && outfilename != temp_outfilename)
   {
      if (overwrite)
         DeleteFile(outfilename);
      else
      // not overwriting, but "delete after encoding" flag set?
      if (delete_after_encode)
         DeleteFile(infilename);

      MoveFile(temp_outfilename, outfilename);
   }
   else
   {
      // delete the old file, if option is set
      // as no temp output filename was generated, assume the names are different
      if (delete_after_encode)
         DeleteFile(infilename);
   }

   // end thread
   running = false;
   paused = false;
}

CString EncoderImpl::GetOutputFilename(const CString& cszInputFilename, OutputModule& outputModule)
{
   CString cszOutputFilename = outpathname;
   int iPos = cszInputFilename.ReverseFind(_T('\\'));
   int iPos2 = cszInputFilename.ReverseFind(_T('.'));

   if (iPos != -1 && iPos2 != -1)
      cszOutputFilename += cszInputFilename.Mid(iPos+1, iPos2-iPos);

   cszOutputFilename += outputModule.getOutputExtension();

   return cszOutputFilename;
}

bool EncoderImpl::CheckSameInputOutputFilenames(const CString& cszInputFilename,
   CString& cszOutputFilename, OutputModule& outputModule)
{
   for (int i=0; i<2; i++)
   {
      // first, test if output filename already exists
      if (::GetFileAttributes(cszOutputFilename) == 0xFFFFFFFF)
         break; // no, so we don't need to test

      // check if the file's short name is the same
      TCHAR buffer1[MAX_PATH], buffer2[MAX_PATH];
      GetShortPathName(cszInputFilename, buffer1, MAX_PATH);
      GetShortPathName(cszOutputFilename, buffer2, MAX_PATH);

      if (0 == _tcsicmp(buffer1, buffer2))
      {
         if (i == 0)
         {
            // when overwriting original, use same name as input filename
            // test again with another file name
            if (overwrite)
            {
               cszOutputFilename = cszInputFilename;
               break;
            }
            else
            {
               // when not overwriting original, add extra extension
               cszOutputFilename += _T(".");
               cszOutputFilename += outputModule.getOutputExtension();
            }
            continue;
         }
         else
         {
            // they're still the same file, skip this one completely
            return false;
         }
      }
   }

   return true;
}

bool EncoderImpl::CheckCDExtractDirectCopy(InputModule& inputModule, OutputModule& outputModule,
   SettingsManager& settingsManager)
{
   bool bFound = false;

   unsigned int in_id = inputModule.getModuleID();
   unsigned int out_id = outputModule.getModuleID();

   if (in_id == ID_IM_CDRIP && out_id == ID_OM_WAVE)
   {
      extern const int SndFileOutputFormat[];

      // we have the right input and output modules; now check if output
      // parameters are 44100 Hz, 16 bit, stereo
      if (settingsManager.queryValueInt(WaveRawAudioFile)==0 &&
          settingsManager.queryValueInt(WaveWriteWavEx)==0 &&
          SndFileOutputFormat[settingsManager.queryValueInt(WaveOutputFormat)] == SF_FORMAT_PCM_16 &&
          settingsManager.queryValueInt(WaveFileFormat) == 0/*FILE_WAV*/)
      {
         return true;
      }
   }

   return false;
}

void EncoderImpl::GenerateTempOutFilename(const CString& cszOriginalFilename, CString& cszTempFilename)
{
   cszTempFilename = cszOriginalFilename;

   CString cszPath, cszFilename;

   // split path from filename
   int iPos = cszOriginalFilename.ReverseFind(_T('\\'));
   if (iPos != -1)
   {
      cszPath = cszOriginalFilename.Left(iPos+1);
      cszFilename = cszOriginalFilename.Mid(iPos+1);
   }
   else
      cszFilename = cszOriginalFilename;

   // find short name of path
   CString cszShortPath;
   ::GetShortPathName(cszPath, cszShortPath.GetBuffer(MAX_PATH), MAX_PATH);
   cszShortPath.ReleaseBuffer();

   // convert filename to ansi and back, and remove '?' chars
   USES_CONVERSION;
   cszFilename = CString(T2CA(cszFilename));
   cszFilename.Replace(_T('?'), _T('_'));

   // now add a ".part"
   unsigned int uiIndex = 0;
   do
   {
      cszTempFilename = cszShortPath + cszFilename;
      if (uiIndex == 0)
         cszTempFilename += _T(".temp");
      else
      {
         CString cszCount;
         cszCount.Format(_T(".%u.temp"), uiIndex);
         cszTempFilename += cszCount;
      }

      uiIndex++;
   }
   while (::GetFileAttributes(cszTempFilename) != INVALID_FILE_ATTRIBUTES);
}

void EncoderImpl::FormatEncodingDescription(InputModule& inputModule, OutputModule& outputModule,
                                            SampleContainer& sampleContainer)
{
   CString cszInputDesc, cszContainerInfo, cszOutputDesc;
   inputModule.getDescription(cszInputDesc);
   outputModule.getDescription(cszOutputDesc);

#ifdef _DEBUG
   // get sample container description
   cszContainerInfo.Format(
      _T("Sample Container: ")
      _T("Input: %u bit, %u channels, sample rate %u Hz, ")
      _T("Output: %u bit\r\n")
      ,
      sampleContainer.getInputModuleBitsPerSample(),
      sampleContainer.getInputModuleChannels(),
      sampleContainer.getInputModuleSampleRate(),
      sampleContainer.getOutputModuleBitsPerSample());
#endif

   CString cszText;
   desc.Format(IDS_ENCODER_ENCODE_INFO, cszInputDesc, cszContainerInfo, cszOutputDesc);
}

CString GetLastErrorString()
{
   DWORD dwError = GetLastError();

   LPVOID lpMsgBuf = NULL;
   ::FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dwError,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
      reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, NULL);

   CString cszErrorMessage;
   if (lpMsgBuf)
   {
      cszErrorMessage = reinterpret_cast<LPTSTR>(lpMsgBuf);
      LocalFree(lpMsgBuf);
   }

   cszErrorMessage.TrimRight(_T("\r\n"));

   return cszErrorMessage;
}

bool EncoderImpl::CheckWarnTranscoding(InputModule& inputModule, OutputModule& outputModule)
{
   unsigned int in_id = inputModule.getModuleID();
   unsigned int out_id = outputModule.getModuleID();

   bool in_lossy = in_id == ID_IM_MAD ||
      in_id == ID_IM_MAD ||
      in_id == ID_IM_OGGV ||
      in_id == ID_IM_AAC ||
      in_id == ID_IM_BASS;

   bool out_lossy = out_id == ID_OM_LAME ||
      out_id == ID_OM_OGGV ||
      out_id == ID_OM_AAC ||
      out_id == ID_OM_BASSWMA;

   if (in_lossy && out_lossy && warn_lossy && !warned_about_lossy)
   {
      // warn user about transcoding
      extern bool WarnAboutTranscode();

      if (!WarnAboutTranscode())
      {
         return false;

      }
      warned_about_lossy = true;
   }

   return true;
}

void EncoderImpl::MainLoop(InputModule& inputModule, OutputModule& outputModule,
   SampleContainer& sampleContainer, bool& bSkipFile)
{
   int ret = 0;

   do
   {
      // call input module
      ret = inputModule.decodeSamples(sampleContainer);

      // no more samples?
      if (ret==0) break;

      // catch errors
      if (handler!=NULL && ret<0)
      {
         switch(handler->handleError(infilename, inputModule.getModuleName(),
            -ret, inputModule.getLastError()))
         {
         case EncoderErrorHandler::Continue:
            break;
         case EncoderErrorHandler::SkipFile:
            error=3;
            bSkipFile = true;
            break;
         case EncoderErrorHandler::StopEncode:
            error=-3;
            bSkipFile = true;
            break;
         }
      }

      // get percent done
      percent = inputModule.percentDone();

      // stuff all samples received into output module
      ret = outputModule.encodeSamples(sampleContainer);

      // catch errors
      if (handler!=NULL && ret<0)
      {
         switch(handler->handleError(infilename, outputModule.getModuleName(),
            -ret, outputModule.getLastError()))
         {
         case EncoderErrorHandler::Continue:
            break;
         case EncoderErrorHandler::SkipFile:
            error=4;
            bSkipFile = true;
            break;
         case EncoderErrorHandler::StopEncode:
            error=-4;
            bSkipFile = true;
            break;
         }
      }

      // check if we should stop the thread
      if (!running)
         break;

      // sleep if we should pause
      while(paused)
         Sleep(50);
   }
   while(true); // outer encoding loop
}

void EncoderImpl::WritePlaylistEntry(const CString& cszOutputFilename)
{
   FILE* fd = _tfopen(playlist_filename, _T("at"));
   fseek(fd, 0L, SEEK_END);
   if (fd == NULL)
      return;

   // get filename, without path
   CString cszFilename = cszOutputFilename;
   int iPos = cszFilename.ReverseFind('\\');
   if (iPos != -1)
      cszFilename.Delete(0, iPos+1);

   // write to playlist file
   _ftprintf(fd, _T("%s\n"), cszFilename);
   fclose(fd);
}
