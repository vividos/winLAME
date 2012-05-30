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
/*! \file CDRipPage.cpp

   \brief contains implementation of the output settings page

*/

// needed includes
#include "stdafx.h"
#include "CDRipPage.h"
#include "CDRipTrackManager.h"
#include "basscd.h"
#include <memory>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CDRipPage methods

CDRipPage::CDRipPage()
{
   IDD = IDD_DLG_CDRIP;
   captionID = IDS_DLG_CAP_CDRIP;
   descID = IDS_DLG_DESC_CDRIP;
   helpID = IDS_HTML_CDRIP;

   m_hEventStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
   m_hWorkerThread = NULL;
   m_lInDestroyHandler = 0;
}

CDRipPage::~CDRipPage()
{
   ::InterlockedIncrement(&m_lInDestroyHandler);

   SetEvent(m_hEventStop);

   WaitForSingleObject(m_hWorkerThread, INFINITE);
   m_hWorkerThread = NULL;

   CloseHandle(m_hEventStop);
}

LRESULT CDRipPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // subclass bevel lines
   bevel1.SubclassWindow(GetDlgItem(IDC_CDRIP_BEVEL1));

   m_lcTracks.SubclassWindow(GetDlgItem(IDC_CDRIP_LIST_TRACKS));

   CString cszText(MAKEINTRESOURCE(IDS_CDRIP_PAGE_COLUMN_TRACK));
   m_lcTracks.InsertColumn(0, cszText, LVCFMT_LEFT, 40, 0);
   cszText.LoadString(IDS_CDRIP_PAGE_COLUMN_TITLE);
   m_lcTracks.InsertColumn(1, cszText, LVCFMT_LEFT, 270, 0);
   cszText.LoadString(IDS_CDRIP_PAGE_COLUMN_STATUS);
   m_lcTracks.InsertColumn(2, cszText, LVCFMT_LEFT, 100, 0);

   m_lcTracks.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

   m_bFinishedAllTracks = false;

   m_pcProgress = GetDlgItem(IDC_CDRIP_PROGRESS);
   m_pcProgress.SetRange(0, 100);

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT CDRipPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ::InterlockedIncrement(&m_lInDestroyHandler);

   SetEvent(m_hEventStop);

   WaitForSingleObject(m_hWorkerThread, INFINITE);
   m_hWorkerThread = NULL;

   return 0;
}

void CDRipPage::OnEnterPage()
{
   UISettings& settings = pui->getUISettings();

   PostMessage(WM_LOCK_NEXT_BUTTON);

   // "start encoding after extraction" check
   bool bValue = settings.cdrip_autostart_encoding;
   SendDlgItemMessage(IDC_CDRIP_CHECK_AUTOSTART_ENC, BM_SETCHECK,
      bValue ? BST_CHECKED : BST_UNCHECKED);

   // refresh tracks list
   {
      m_lcTracks.SetRedraw(FALSE);
      m_lcTracks.DeleteAllItems();

      CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

      unsigned int nMax = pManager->GetMaxTrackInfo();
      for(unsigned int n=0; n<nMax; n++)
      {
         CDRipTrackInfo& trackinfo = pManager->GetTrackInfo(n);
         if (!trackinfo.m_bActive)
            continue;

         CString cszText;
         cszText.Format(_T("%u."), trackinfo.m_nTrackOnDisc+1);
         int nItem = m_lcTracks.InsertItem(m_lcTracks.GetItemCount(), cszText);
         m_lcTracks.SetItemText(nItem, 1, trackinfo.m_cszTrackTitle);

         cszText.LoadString(IDS_CDRIP_PAGE_STATUS_NOT_PROCESSED);
         m_lcTracks.SetItemText(nItem, 2, cszText);
      }

      m_lcTracks.SetRedraw(TRUE);
   }
}

bool CDRipPage::OnLeavePage()
{
   pui->getUISettings().cdrip_autostart_encoding =
      BST_CHECKED==SendDlgItemMessage(IDC_CDRIP_CHECK_AUTOSTART_ENC, BM_GETCHECK);

   pui->getUISettings().last_page_was_cdrip_page = true;

   SetEvent(m_hEventStop);

   WaitForSingleObject(m_hWorkerThread, INFINITE);
   m_hWorkerThread = NULL;

   return true;
}

LRESULT CDRipPage::OnButtonStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

   CDRipDiscInfo& discinfo = pManager->GetDiscInfo();

   // check if disc is inserted
   if (FALSE == BASS_CD_IsReady(discinfo.m_nDiscDrive))
   {
      AppMessageBox(m_hWnd, IDS_CDRIP_PAGE_ERROR_NO_CD_IN_DRIVE, MB_OK | MB_ICONEXCLAMATION);
      return 0;
   }

   // check disc ID
   CString cszCDID(BASS_CD_GetID(discinfo.m_nDiscDrive, BASS_CDID_CDDB));
   if (cszCDID != discinfo.m_cszCDID)
   {
      AppMessageBox(m_hWnd, IDS_CDRIP_PAGE_ERROR_WRONG_CD, MB_OK | MB_ICONEXCLAMATION);
      return 0;
   }

   ::ResetEvent(m_hEventStop);

   ::EnableWindow(GetDlgItem(IDC_CDRIP_START), FALSE);
   pui->lockWizardButtons(true);

   DWORD nThreadId = 0;
   m_hWorkerThread = ::CreateThread(NULL, 0, ThreadProc, reinterpret_cast<LPVOID>(this), 0, &nThreadId);

   return 0;
}

DWORD CDRipPage::ThreadProc(void* pData)
{
   CDRipPage* pThis = reinterpret_cast<CDRipPage*>(pData);
   pThis->ExtractAudio();

   if (0 == ::InterlockedCompareExchange(&pThis->m_lInDestroyHandler, 1, 1))
   {
      pThis->pui->lockWizardButtons(false);

      HWND hWndButtonNext = ::GetDlgItem(pThis->GetParent(), IDC_MDLG_NEXT);
      ::PostMessage(hWndButtonNext, BM_CLICK, 0, 0);
   }

   return 0;
}

void CDRipPage::ExtractAudio()
{
   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

   CDRipDiscInfo& discinfo = pManager->GetDiscInfo();

   unsigned int nMax = pManager->GetMaxTrackInfo();
   for(unsigned int n=0; n<nMax; n++)
   {
      m_lcTracks.EnsureVisible(n, FALSE);

      CDRipTrackInfo& trackinfo = pManager->GetTrackInfo(n);

      CString cszText(MAKEINTRESOURCE(IDS_CDRIP_PAGE_STATUS_PROCESSING));
      m_lcTracks.SetItemText(n, 2, cszText);

      CString cszDiscTrackTitle;
      if (discinfo.m_bVariousArtists)
         cszDiscTrackTitle = trackinfo.m_cszTrackTitle;
      else
      {
         cszDiscTrackTitle.Format(_T("%s - %s"),
            discinfo.m_cszDiscTitle, trackinfo.m_cszTrackTitle);
      }

      cszDiscTrackTitle.Replace(_T(" / "), _T(" - "));
      cszDiscTrackTitle.Replace(_T("\\"), _T(""));
      cszDiscTrackTitle.Replace(_T("/"), _T(""));
      cszDiscTrackTitle.Replace(_T(":"), _T(""));
      cszDiscTrackTitle.Replace(_T("*"), _T(""));
      cszDiscTrackTitle.Replace(_T("?"), _T(""));
      cszDiscTrackTitle.Replace(_T("\""), _T(""));
      cszDiscTrackTitle.Replace(_T("<"), _T(""));
      cszDiscTrackTitle.Replace(_T(">"), _T(""));
      cszDiscTrackTitle.Replace(_T("|"), _T(""));
      cszDiscTrackTitle.Replace(_T("{"), _T("("));
      cszDiscTrackTitle.Replace(_T("}"), _T(")"));

      CString cszGuid;
      cszGuid.Format(_T("{%08x-%08x}"), ::GetCurrentProcessId(), ::GetCurrentThreadId());

      // add slash to temp folder if necessary
      CString cszTempFolder(pui->getUISettings().cdrip_temp_folder);
      if (cszTempFolder.Right(1) != _T("\\"))
         cszTempFolder += _T("\\");

      CString cszTempFilename;
      cszTempFilename.Format(_T("%s\\(%02u) %s%s.wav"),
         cszTempFolder,
         trackinfo.m_nTrackOnDisc+1,
         cszDiscTrackTitle, cszGuid);

      trackinfo.m_cszRippedFilename = cszTempFilename;

      bool bRet = ExtractTrack(discinfo, trackinfo, cszTempFilename);
      if (!bRet)
      {
//         m_lcTracks.SetItemText(n, 2, _T("cancelled"));
         return;
      }

/*
      // rewrite all entries in the filenames list with this entry number
      CString cszTrackUriStart;
      cszTrackUriStart.Format(_T("%s%u\\"), g_pszCDRipPrefix, n);

      FilenameList& fnlist = pui->getUISettings().filenamelist;
      unsigned int nMax2 = fnlist.size();
      for(unsigned int i=0; i<nMax2; i++)
      {
         if (0 == fnlist[i].find(cszTrackUriStart))
         {
            // found one
            fnlist[i] = cszTempFilename;
         }
      }
*/
      cszText.LoadString(IDS_CDRIP_PAGE_STATUS_FINISHED);
      m_lcTracks.SetItemText(n, 2, cszText);
   }

   m_bFinishedAllTracks = true;
}

#include "encoder/WaveOutputModule.h"

bool CDRipPage::ExtractTrack(CDRipDiscInfo& discinfo, CDRipTrackInfo& trackinfo, const CString& cszTempFilename)
{
   DWORD nTrackLength = BASS_CD_GetTrackLength(discinfo.m_nDiscDrive, trackinfo.m_nTrackOnDisc);
   nTrackLength /= 100L;
   DWORD nCurLength = 0;

   BASS_Init(0, 44100, 0, m_hWnd, NULL);

   HSTREAM hStream = BASS_CD_StreamCreate(discinfo.m_nDiscDrive, trackinfo.m_nTrackOnDisc,
      BASS_STREAM_DECODE);

   DWORD nError = BASS_ErrorGetCode();

   if (hStream == 0)
      return false;

   WaveOutputModule outmod;

   if (!outmod.isAvailable())
   {
      AppMessageBox(m_hWnd, IDS_CDRIP_PAGE_WAVE_OUTPUT_NOT_AVAIL, MB_OK | MB_ICONSTOP);
      return false;
   }

   SettingsManager mgr;
   TrackInfo outtrackinfo;
   SampleContainer samplecont;
   {
      samplecont.setInputModuleTraits(16, SamplesInterleaved, 44100, 2);
      outtrackinfo.ResetInfos();
      mgr.setValue(WaveRawAudioFile, 0);
      mgr.setValue(WaveWriteWavEx, 0);
      mgr.setValue(WaveOutputFormat, 0); // SF_FORMAT_PCM_16
      mgr.setValue(WaveFileFormat, 0);   // FILE_WAV

      outmod.prepareOutput(mgr);
      int nRet = outmod.initOutput(cszTempFilename, mgr, outtrackinfo, samplecont);
      if (nRet < 0)
      {
         CString cszText;
         cszText.Format(IDS_CDRIP_PAGE_ERROR_CREATE_OUTPUT_FILE_S, cszTempFilename);
         AppMessageBox(m_hWnd, cszText, MB_OK | MB_ICONSTOP);
         return false;
      }
   }

   const unsigned int nBufferSize = 65536;

   unsigned int nCurPos = 100;

   signed short* pBuffer = new signed short[nBufferSize];
   std::auto_ptr<signed short> apAutoDeleteBuffer(pBuffer);

   bool bFinished = true;

   while(BASS_ACTIVE_STOPPED != BASS_ChannelIsActive(hStream))
   {
      DWORD nAvail = BASS_ChannelGetData(hStream, pBuffer, nBufferSize*sizeof(pBuffer[0]));

      if (nAvail == 0)
         break; // couldn't read more samples

      samplecont.putSamplesInterleaved(pBuffer, nAvail / sizeof(pBuffer[0]) / 2);

      nCurLength += nAvail;

      if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventStop, 0))
      {
         ResetEvent(m_hEventStop);
         bFinished = false;
         break;
      }

      unsigned int nNewPos = nCurLength/nTrackLength;
      if (nNewPos != nCurPos)
      {
         if (m_pcProgress.IsWindow())
            m_pcProgress.SetPos(nNewPos > 100 ? 100 : nNewPos);
         nCurPos = nNewPos;
      }

      int nRet = outmod.encodeSamples(samplecont);
      if (nRet < 0)
         break;
   }

   BASS_StreamFree(hStream);
   BASS_Free();

   outmod.doneOutput();

   return bFinished;
}
