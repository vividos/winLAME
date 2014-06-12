/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2014 Michael Fink

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
/// \file InputPage.cpp
/// \brief contains the methods of the input page
/// \details code to delete selected list ctrl items, handle dropped files, insert files
/// into the list and the enter and leave page functions

// needed includes
#include "stdafx.h"
#include "InputPage.h"
#include "EncoderInterface.h"
#include "encoder/ModuleInterface.h"
#include "CDRipDlg.h"
#include "CDRipTrackManager.h"
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

#include "shellapi.h"
#undef ExtractIcon

// InputPage methods

LRESULT InputPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // create the image list
   ilIcons.Create(MAKEINTRESOURCE(IDB_BITMAP_BTNICONS),16,0,RGB(192,192,192));

   // set icons on buttons
   SendDlgItemMessage(IDC_INPUT_BUTTON_INFILESEL, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(0) );
   SendDlgItemMessage(IDC_INPUT_BUTTON_DELETE, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(1) );
   SendDlgItemMessage(IDC_INPUT_BUTTON_PLAY, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(2) );
   SendDlgItemMessage(IDC_INPUT_BUTTON_CDRIP, BM_SETIMAGE, IMAGE_ICON,
      (LPARAM)ilIcons.ExtractIcon(7) );

   HWND tmpWnd = GetDlgItem(IDC_INPUT_LIST_INPUTFILES);

   // subclass the list ctrl
   listctrl.SubclassWindow(tmpWnd);

   setsysimagelist = false;

   // enable drag & drop
   ::DragAcceptFiles(tmpWnd,TRUE);

   // find out width of the list ctrl
   RECT rc;
   listctrl.GetWindowRect(&rc);
   int width = rc.right-rc.left-4;

   // load strings
   CString col[4];
   col[0].LoadString(IDS_INPUT_LIST_COLNAME1);
   col[1].LoadString(IDS_INPUT_LIST_COLNAME2);
   col[2].LoadString(IDS_INPUT_LIST_COLNAME3);
   col[3].LoadString(IDS_INPUT_LIST_COLNAME4);

   // insert list ctrl columns
   double sizes[4] = { 0.57, 0.17, 0.15, 0.11 };

   LVCOLUMN lvColumn = { LVCF_TEXT | LVCF_WIDTH, 0, 0, NULL, 0, 0 };

   for(int i=0;i<4;i++)
   {
      lvColumn.cx = int(width*sizes[i]);
      lvColumn.pszText = const_cast<LPTSTR>((LPCTSTR)col[i]);
      listctrl.InsertColumn(i,&lvColumn);
   }

   // set extended list ctrl styles
   listctrl.SetExtendedListViewStyle(
      LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT );

   filterstring.Empty();

   // disable cd extract button if cd ripping is not available
   if (!CDRipDlg::IsCDExtractionAvail())
      ::EnableWindow(GetDlgItem(IDC_INPUT_BUTTON_CDRIP), FALSE);

   UpdateTimeCount();

   // enable resizing
   DlgResize_Init(false, true);

   return 1;  // let the system set the focus
}

LRESULT InputPage::OnButtonDeleteAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // delete all selected files in cdrip manager
   DeleteSelectedFromCDRipTrackManager();

   // delete all list items
   listctrl.DeleteSelectedListItems();

   UpdateTimeCount();

   return 0;
}

void InputPage::DeleteSelectedFromCDRipTrackManager()
{
   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();
   if (pManager->GetMaxTrackInfo()==0) return; // no tracks in list

   // get all selected items
   int pos = listctrl.GetNextItem(-1,LVIS_SELECTED);

   CString cszFilename;
   while (pos != -1)
   {
      cszFilename = listctrl.GetFileName(pos);
      if (0 == cszFilename.Find(g_pszCDRipPrefix))
      {
         unsigned long nIndex = _tcstoul(cszFilename.Mid(_tcslen(g_pszCDRipPrefix)), NULL, 10);
         ATLASSERT(nIndex < pManager->GetMaxTrackInfo());
         pManager->GetTrackInfo(nIndex).m_bActive = false; // switch inactive
      }

      pos = listctrl.GetNextItem(pos,LVIS_SELECTED);
   }
}

void InputPage::UpdateTimeCount()
{
   unsigned int nTime = listctrl.GetTotalLength();

   CString cszText;
   cszText.Format(IDS_INPUT_TIME_UU, nTime/60, nTime%60);

   SetDlgItemText(IDC_STATIC_TIMECOUNT, cszText);
}

LRESULT InputPage::OnButtonCDRip(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // first check if cd ripping is available
   if (!CDRipDlg::IsCDExtractionAvail())
      return 0;

   CDRipDlg dlg(pui->getUISettings(), *pui);
   if (IDOK == dlg.DoModal(m_hWnd))
   {
      // clean all files in filelist with cdrip prefix
      {
         CString cszFilename;
         for(int n=listctrl.GetItemCount()-1; n>=0; n--)
         {
            cszFilename = listctrl.GetFileName(n);

            if (cszFilename.Find(g_pszCDRipPrefix) == 0)
               listctrl.DeleteItem(n);
         }
      }

      // add new files to list
      CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();
      unsigned int nMax = pManager->GetMaxTrackInfo();

      CString cszTestCdaFilename;
      GetTempPath(MAX_PATH, cszTestCdaFilename.GetBuffer(MAX_PATH));
      cszTestCdaFilename.ReleaseBuffer();
      cszTestCdaFilename += _T("Track01.cda");

      FILE* fd = ::_tfopen(cszTestCdaFilename, _T("wb"));
      fclose(fd);

      CString cszCDRipUri;
      for(unsigned int n=0; n<nMax; n++)
      {
         CDRipTrackInfo& trackinfo = pManager->GetTrackInfo(n);

         cszCDRipUri.Format(_T("%s%u\\%u. %s"), g_pszCDRipPrefix, n,
            trackinfo.m_nTrackOnDisc+1, trackinfo.m_cszTrackTitle);

         InsertFileWithIcon(cszCDRipUri, cszTestCdaFilename, 44100, 44100*16*2, trackinfo.m_nTrackLength);
      }

      _tremove(cszTestCdaFilename);
   }
   return 0;
}

LRESULT InputPage::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // reflected from list ctrl; handle dropped filenames
   HDROP hDropInfo = (HDROP)wParam;

   UINT n = ::DragQueryFile(hDropInfo, UINT(-1), NULL, 0);

   TCHAR buffer[MAX_PATH];

   // clear input errors
   input_errors.Empty();

   // go through all available
   for (UINT i = 0; i<n; i++)
   {
      ::DragQueryFile(hDropInfo,i,buffer,MAX_PATH);
      InsertFilename(buffer);
   }

   ::DragFinish(hDropInfo);

   // redraw the windows after drop
   ::InvalidateRect(GetParent(), NULL, TRUE);

   // print out errors occured
   if (!input_errors.IsEmpty())
      AppMessageBox(m_hWnd, input_errors, MB_OK | MB_ICONEXCLAMATION);

   return 0;
}

LRESULT InputPage::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // delete key from list ctrl?
   if (VK_DELETE == (int)wParam)
   {
      int pos = listctrl.GetNextItem(-1,LVIS_SELECTED);

      // delete all selected files in cdrip manager
      DeleteSelectedFromCDRipTrackManager();

      // delete them all
      listctrl.DeleteSelectedListItems();

      // set selection on next item
      listctrl.SetItemState(pos,
         LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);

      UpdateTimeCount();
   }

   // insert key? fake button press
   if (VK_INSERT == (int)wParam)
      OnButtonInputFileSel(0,0,0,bHandled);

   // return key? play file
   if (VK_RETURN == (int)wParam)
      OnButtonPlay(0,0,0,bHandled);

   return 0;
}

LRESULT InputPage::OnListItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   // called when the selected item in the list changes

   // look if we have to disable the play and delete button
   BOOL enable = (0 != listctrl.GetSelectedCount());

   ::EnableWindow(GetDlgItem(IDC_INPUT_BUTTON_PLAY),enable);
   ::EnableWindow(GetDlgItem(IDC_INPUT_BUTTON_DELETE),enable);

   return 0;
}

LRESULT InputPage::OnDoubleClickedList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   // double-clicked on an item
   LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pnmh;

   if (lpnmlv->iItem != -1)
   {
      LPCTSTR filename = listctrl.GetFileName(lpnmlv->iItem);

      // play file
      PlayFile(filename);
   }

   return 0;
}

LRESULT InputPage::OnButtonPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // find out which file had the focus
   int index = listctrl.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

   LPCTSTR filename = listctrl.GetFileName(index);

   // play file
   PlayFile(filename);
   return 0;
}

void InputPage::PlayFile(LPCTSTR filename)
{
   ::ShellExecute( NULL, _T("play"), filename, NULL, NULL, SW_SHOW);
}

void InputPage::OpenFileDialog()
{
   // get filter string
   CString filter;
   if (filterstring.IsEmpty())
   {
      ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
      moduleManager.getFilterString(filterstring);

      CString cszText(MAKEINTRESOURCE(IDS_INPUT_FILTER_PLAYLISTS));
      filterstring += cszText;

      cszText.LoadString(IDS_INPUT_FILTER_CUESHEETS);
      filterstring += cszText;
      filterstring.Insert(filterstring.Find('|')+1,_T("*.m3u;*.pls;*.cue;"));

      cszText.LoadString(IDS_INPUT_FILTER_ALLFILES);
      filterstring += cszText + _T("|"); // add extra pipe char for end of filter
   }
   filter = filterstring;

   // exchange pipe char '|' with 0-char for commdlg
   for(int pos=filter.GetLength()-1; pos>=0; pos--)
      if (filter.GetAt(pos)=='|')
         filter.SetAt(pos,0);

   // load title
   CString title;
   title.LoadString(IDS_INPUT_INFILES_SELECT);

   // file dialog setup
   CFileDialog dlg(TRUE, NULL, NULL,
      OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST |
      OFN_PATHMUSTEXIST | OFN_EXPLORER,
      filter, m_hWnd );

   // modify the file buffer
   int buflen = _MAX_PATH*64;
   TCHAR *buffer = new TCHAR[buflen];

   CString& lastinputpath = pui->getUISettings().lastinputpath;

   // copy last input path to buffer, as init
   _tcsncpy(buffer,lastinputpath,buflen-1);
   buffer[buflen-1]=0;

   dlg.m_ofn.lpstrFile = buffer;
   dlg.m_ofn.nMaxFile = buflen;

   // do file dialog
   if (IDOK==dlg.DoModal())
   {
      // clear input errors
      input_errors.Empty();

      if (dlg.m_ofn.nFileExtension == 0)
      {
         // multiple file selection
         LPCTSTR start = buffer;

         // go to the first file
         while(*start++ != 0);

         // while not at end of the list
         while(*start != 0)
         {
            // construct pathname
            CString fname(buffer);
            if (fname.GetAt(fname.GetLength()-1)!='\\')
               fname += "\\";
            fname += start;
            InsertFilename(fname);

            // go to the next entry
            while(*start++ != 0);
         }

         // get the used directory
         lastinputpath = buffer;
         int len = lastinputpath.GetLength();
         if (len!=0 && lastinputpath[len-1]!='\\')
            lastinputpath += char('\\');

         // add last selected file
         --start;
         while(*(--start) != 0);
         lastinputpath+=(start+1);
      }
      else
      {
         // single file selection
         InsertFilename(buffer);

         // get the used directory
         lastinputpath = buffer;
      }

      // print out errors occured
      if (!input_errors.IsEmpty())
         AppMessageBox(m_hWnd, input_errors, MB_OK | MB_ICONEXCLAMATION);
   }

   delete[] buffer;
}

void InputPage::InsertFilename(LPCTSTR filename)
{
   // check if dropped item is a directory
   DWORD dwAttr = ::GetFileAttributes(filename);
   if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
   {
      CString cszPathStr(filename);
      cszPathStr += _T("\\*.*");

      _tfinddata_t fdata;
      long ffhnd;

      recursive = true;

      if (-1!=(ffhnd = _tfindfirst(cszPathStr, &fdata)))
      do
      {
         if (fdata.name[0]!='.')
         {
            CString fname(filename);
            fname += _T('\\');
            fname += fdata.name;

            InsertFilename(fname);
         }
      } while (0 == _tfindnext(ffhnd, &fdata));

      _findclose(ffhnd);

      recursive = false;

      return;
   }

   // check if the file is a playlist
   if (!recursive)
   {
      LPCTSTR pos = _tcsrchr(filename,'.');
      if (pos != NULL)
      do
      {
         if (0==_tcsicmp(pos+1,_T("m3u")))
         {
            // import m3u playlist
            ImportM3uPlaylist(filename);
         }
         else
         if (0==_tcsicmp(pos+1,_T("pls")))
         {
            // import pls playlist
            ImportPlsPlaylist(filename);
         }
         else
         if (0==_tcsicmp(pos+1,_T("cue")))
         {
            // import cue sheet
            ImportCueSheet(filename);
         }
         else
            break;

         // set playlist filename as output playlist, too
         UISettings &settings = pui->getUISettings();
         settings.create_playlist = true;

         // produce playlist filename
         CString plname(filename,pos-filename+1);
         plname += _T("m3u");

         plname.TrimRight(_T('\\'));

         settings.playlist_filename = plname;
         return;

      } while(false);
   }

   // find out infos of file
   CString errormsg;
   int samplerate=-1,bps=-1,length=-1;

   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();

   bool res = moduleManager.getAudioFileInfo(filename,length,bps,samplerate,errormsg);
   if (!res)
   {
      CString cszFirstText(MAKEINTRESOURCE(IDS_INPUT_ERRORS_OCCURED_ADDING));

      CString cszTemp;
      cszTemp.Format(IDS_INPUT_ERROR_OPEN_INPUT_FILE_SSS,
         !input_errors.IsEmpty() ? _T("") : cszFirstText,
         filename,
         errormsg);

      input_errors += cszTemp;
      return;
   }

   InsertFileWithIcon(filename, filename, samplerate, bps, length);
}

void InputPage::OnEnterPage()
{
   CString cszTestCdaFilename;
   GetTempPath(MAX_PATH, cszTestCdaFilename.GetBuffer(MAX_PATH));
   cszTestCdaFilename.ReleaseBuffer();
   cszTestCdaFilename += _T("Track01.cda");

   FILE* fd = _tfopen(cszTestCdaFilename, _T("wb"));
   fclose(fd);

   CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

   // insert all filenames into the list ctrl
   EncoderJobList& fnlist = pui->getUISettings().encoderjoblist;
   int max = fnlist.size();
   CString filename;
   for(int i=0;i<max;i++)
   {
      filename = fnlist[i].InputFilename();
      if (0 == filename.Find(g_pszCDRipPrefix))
      {
         unsigned long nIndex = _tcstoul(filename.Mid(_tcslen(g_pszCDRipPrefix)), NULL, 10);
         CDRipTrackInfo& trackinfo = pManager->GetTrackInfo(nIndex);

         InsertFileWithIcon(filename, cszTestCdaFilename, 44100, 44100*16*2, trackinfo.m_nTrackLength);
      }
      else
         InsertFilename(filename);
   }

   _tremove(cszTestCdaFilename);

   // set state of play button
   ::EnableWindow(GetDlgItem(IDC_INPUT_BUTTON_PLAY),max==0 ? FALSE : TRUE);
   ::EnableWindow(GetDlgItem(IDC_INPUT_BUTTON_DELETE),max==0 ? FALSE : TRUE);
}

bool InputPage::OnLeavePage()
{
   int max = listctrl.GetItemCount();

   // when no input files are chosen, refuse to leave the page
   if (max==0)
   {
      // pop up a message box
      AppMessageBox(m_hWnd, IDS_INPUT_NOINFILES, MB_OK | MB_ICONEXCLAMATION);
      return false;
   }

   // make room
   EncoderJobList& fnlist = pui->getUISettings().encoderjoblist;
   fnlist.clear();
   fnlist.reserve(max);

   CString fname;

   // put all filenames from the list ctrl into the vector
   for(int i=0;i<max;i++)
   {
      fname = listctrl.GetFileName(i);
      fnlist.push_back(EncoderJob(fname));
   }

   // check if any inactive track entries are in the cdrip manager
   {
      CDRipTrackManager* pManager = CDRipTrackManager::getCDRipTrackManager();

      unsigned int nMax = pManager->GetMaxTrackInfo();
      for(unsigned int n=0; n<nMax; n++)
      {
         if (!pManager->GetTrackInfo(n).m_bActive)
         {
            pManager->RemoveTrackInfo(n);
            --nMax;
            --n;
         }
      }
   }

   return true;
}

void InputPage::ImportM3uPlaylist(LPCTSTR filename)
{
   // open playlist
   USES_CONVERSION;
   std::ifstream plist(T2CA(filename),std::ios::in);
   if (!plist.is_open()) return;

   // find out pathname (for relative file refs)
   std::tstring path(filename);
   std::tstring::size_type pos = path.find_last_of('\\');
   if (pos!=std::tstring::npos)
      path.erase(pos);

   // read in all lines
   std::tstring line;

   while(!plist.eof())
   {
#ifdef UNICODE
      std::string line2;
      std::getline(plist,line2);
      line = A2CT(line2.c_str());
#else
      std::getline(plist,line);
#endif
      if (line.empty()) continue;
      if (line.at(0)=='#') continue;

      // line contains a file name

      // check if path is relative
      if (_tcschr(line.c_str(),':')==NULL && _tcsncmp(line.c_str(),_T("\\\\"),2)!=0)
      {
         if (line.size()>0 && line.at(0)!='\\')
         {
            // relative to playlist file
            line.insert(0,_T("\\"));
            line.insert(0,path);
         }
         else
         {
            // relative to drive
            line.insert(0,path.c_str(),2);
         }
      }

      // insert filename
      InsertFilename(line.c_str());
   }

   plist.close();
}

void InputPage::ImportPlsPlaylist(LPCTSTR filename)
{
   // open playlist
   USES_CONVERSION;
   std::ifstream plist(T2CA(filename),std::ios::in);
   if (!plist.is_open()) return;

   // find out pathname (for relative file refs)
   std::tstring path(filename);
   std::tstring::size_type pos = path.find_last_of('\\');
   if (pos!=std::tstring::npos)
      path.erase(pos);

   // read in all lines
   std::tstring line;

   while(!plist.eof())
   {
#ifdef UNICODE
      std::string line2;
      std::getline(plist,line2);
      line = A2CT(line2.c_str());
#else
      std::getline(plist,line);
#endif
      if (line.empty()) continue;
      if (_tcsncmp(line.c_str(),_T("File"),4)!=0) continue;

      // get file name after equal char
      std::tstring::size_type pos = line.find_first_of('=');
      if (pos==std::tstring::npos) continue;
      line.erase(0,pos+1);

      // check if path is relative
      if (_tcschr(line.c_str(),':')==NULL && _tcsncmp(line.c_str(),_T("\\\\"),2)!=0)
      {
         if (line.size()>0 && line.at(0)!='\\')
         {
            // relative to playlist file
            line.insert(0,_T("\\"));
            line.insert(0,path);
         }
         else
         {
            // relative to drive
            line.insert(0,path.c_str(),2);
         }
      }

      // insert filename
      InsertFilename(line.c_str());
   }

   plist.close();
}

void InputPage::ImportCueSheet(LPCTSTR filename)
{
   // open cue sheet
   USES_CONVERSION;
   std::ifstream sheet(T2CA(filename),std::ios::in);
   if (!sheet.is_open()) return;

   // find out pathname (for relative file refs)
   std::tstring path(filename);
   std::tstring::size_type pos = path.find_last_of('\\');
   if (pos!=std::tstring::npos)
      path.erase(pos);

   // read in all lines
   std::tstring line;

   while(!sheet.eof())
   {
#ifdef UNICODE
      std::string line2;
      std::getline(sheet,line2);
      line = A2CT(line2.c_str());
#else
      std::getline(sheet,line);
#endif
      if (line.empty()) continue;

      // trim
      while(line.at(0)==' ' && line.size()!=0) line.erase(0,1);
      if (line.empty()) continue;

      // file entry?
      if (line.find(_T("FILE"))==0)
      {
         // trim
         line.erase(0,5);
         while(line.at(0)==' ' && line.size()!=0) line.erase(0,1);
         if (line.empty()) continue;

         // determine endchar
         char endchar = ' ';
         if (line.at(0)=='\"') endchar = '\"';

         // search endchar
         std::tstring::size_type pos = line.find_first_of(endchar,1);
         if (pos==std::tstring::npos) continue;

         // cut out filename
         std::tstring fname;
         fname.assign( line.c_str()+(endchar==' ' ? 0 : 1),
            pos-(endchar==' ' ? 0 : 1));

         // insert filename
         InsertFilename(fname.c_str());
      }
   }

   sheet.close();
}

void InputPage::InsertFileWithIcon(LPCTSTR pszRealFilename, LPCTSTR pszFilenameForIcon,
   int nSamplefreq, int nBps, int nLength)
{
   // find out icon image
   SHFILEINFO sfi;
   HIMAGELIST hImageList = (HIMAGELIST)SHGetFileInfo(
      pszFilenameForIcon, 0, &sfi, sizeof(SHFILEINFO),
      SHGFI_SYSICONINDEX | SHGFI_SMALLICON );

   if (!setsysimagelist)
   {
      // set system wide image list
      listctrl.SetImageList(hImageList, LVSIL_SMALL);
      setsysimagelist = true;
   }

   listctrl.InsertFile(pszRealFilename, sfi.iIcon, nSamplefreq, nBps, nLength);

   UpdateTimeCount();
}
