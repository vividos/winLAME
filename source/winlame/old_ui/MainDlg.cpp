/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2014 Michael Fink
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
/// \file MainDlg.cpp
/// \brief implementation of main dialog methods
/// \details contains implementation of the methods for the main dialog, such as
/// drawing the caption bar, switching between pages and finding out where
/// the chm help file resides

// needed includes
#include "stdafx.h"
#include "MainDlg.h"
#include "CommonStuff.h"
#include "InputPage.h"
#include "OutputPage.h"
#include "EncodePage.h"
#include "AboutDlg.hpp"
#include "OptionsDlg.h"
#include "App.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <shlobj.h>

#define IDM_ABOUTBOX 16    ///< menu id for about box
#define IDM_OPTIONS 48     ///< menu id for options
#define IDM_APPMODE 64     ///< menu id for app mode change

// MainDlg methods

MainDlg::MainDlg(UISettings& settings_, LanguageResourceManager& langResourceManager)
:m_bKeyDownEscape(false),
 settings(settings_),
 m_langResourceManager(langResourceManager),
   m_isAppModeChanged(false)
{
   // load icons
   wndicon = ::LoadIcon(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_ICON_WINLAME));
   wndicon_small = (HICON)::LoadImage(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDI_ICON_WINLAME),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

   // load accelerator table
   actable = ::LoadAccelerators(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDR_ACTABLE));

   currentpage = -1;

   // insert some pages
   pages.push_back(new InputPage);
   pages.push_back(new OutputPage);
   pages.push_back(new EncodePage);
}

MainDlg::~MainDlg()
{
   // destroy loaded icon
   ::DestroyIcon(wndicon);
   ::DestroyIcon(wndicon_small);

   // destroy the table
   if (actable!=NULL)
      ::DestroyAcceleratorTable(actable);

   // delete all page pointers
   int max = pages.size();
   for(int i=0; i<max; i++)
      delete pages[i];
}

LRESULT MainDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // center main window
   CenterWindow();

   // set caption
   CString cszCaption = _T("winLAME ") + App::Version();
   SetWindowText(cszCaption);

   // set icon
   SetIcon(wndicon,TRUE);
   SetIcon(wndicon_small,FALSE);
   ShowWindow(SW_SHOW);

   // create tooltip control
   tooltips.Create(m_hWnd);

   // add tool tips for every control
   AddTooltips(m_hWnd,tooltips);

   // activate tooltips
   tooltips.Activate(TRUE);

   // activate page 0
   ActivatePage(0);

   // add menu items to system menu

   // IDM_ABOUTBOX and IDM_OPTIONS must be in the system command range.
   ATLASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
   ATLASSERT(IDM_ABOUTBOX < 0xF000);
   ATLASSERT((IDM_OPTIONS & 0xFFF0) == IDM_OPTIONS);
   ATLASSERT(IDM_OPTIONS < 0xF000);
   ATLASSERT((IDM_APPMODE & 0xFFF0) == IDM_APPMODE);
   ATLASSERT(IDM_APPMODE < 0xF000);

   CMenu sysmenu;
   sysmenu.Attach(GetSystemMenu(FALSE));
   if (sysmenu!=NULL)
   {
      sysmenu.AppendMenu(MF_SEPARATOR);

      // add chage app mode menu entry
      {
         CString cszMenuEntry;

         cszMenuEntry.LoadString(IDS_COMMON_APPMODE_MODERN);
         sysmenu.AppendMenu(MF_STRING, IDM_APPMODE, cszMenuEntry);
      }

      // add options menu entry
      {
         CString cszMenuEntry;

         cszMenuEntry.LoadString(IDS_COMMON_OPTIONS);
         sysmenu.AppendMenu(MF_STRING, IDM_OPTIONS, cszMenuEntry);
      }

      // add about box menu entry
      {
         CString cszMenuEntry;

         cszMenuEntry.LoadString(IDS_COMMON_ABOUTBOX);
         sysmenu.AppendMenu(MF_STRING, IDM_ABOUTBOX, cszMenuEntry);
      }
   }
   sysmenu.Detach();

   // enable resizing
   CRect rectPage;
   GetClientRect(&rectPage);
   m_sizePage.cx = rectPage.Width();
   m_sizePage.cy = rectPage.Height();

   DlgResize_Init(true, true);

   return 1;  // let the system set the focus
}

LRESULT MainDlg::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   if ((wParam & 0xFFF0)==IDM_ABOUTBOX)
   {
      // show about dialog
      UI::AboutDlg dlg;
      dlg.SetPresetsXmlFilename(settings.presets_filename);
      dlg.DoModal();
   }
   else
   if ((wParam & 0xFFF0)==IDM_OPTIONS)
   {
      OptionsDlg dlg(settings, m_langResourceManager);
      dlg.DoModal(m_hWnd);
   }
   else
   if ((wParam & 0xFFF0)==IDM_APPMODE)
   {
      m_isAppModeChanged = true;
      settings.m_appMode = UISettings::modernMode;
      PostQuitMessage(0);
   }
   else
      bHandled = false;

   return 0;
}

void MainDlg::RunDialog()
{
   // collect file names from the command line
   GetCommandLineFiles();

   // create the dialog
   Create(NULL,CWindow::rcDefault);

   // check if input or output modules are available
   ModuleManager& moduleManager = IoCContainer::Current().Resolve<ModuleManager>();
   if (0==moduleManager.getInputModuleCount() ||
       0==moduleManager.getOutputModuleCount() )
   {
      // show warning message box
      AppMessageBox(m_hWnd, IDS_ERR_NOMODULES, MB_OK | MB_ICONSTOP);

      // exit winLAME
      PostQuitMessage(0);
   }

   // runs the message loop of the modeless dialog
   MSG msg;

   for(;;)
   {
      // retrieve next message
      BOOL bRet = ::GetMessage(&msg, NULL, 0, 0);

      // quit message loop on WM_QUIT
      if (!bRet)
         break;

      // deliver message
      if(!PreTranslateMessage(&msg) && !IsDialogMessage(&msg))
      {
         ::TranslateMessage(&msg);
         ::DispatchMessage(&msg);
      }
   }

   // destroy the dialog window
   DestroyWindow();
}

void MainDlg::DrawCaptionBar(HDC &hDC, RECT &rc)
{
   // paint background white
   ::SetBkColor(hDC, RGB(255,255,255));
   ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

   // get default font and logfont struct
   HFONT font = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
   LOGFONT lf;
   ::GetObject(font,sizeof(LOGFONT), &lf);

   // create normal font
   _tcscpy(lf.lfFaceName,_T("MS Shell Dlg"));
//   lf.lfHeight = 14;
   HFONT normfont = ::CreateFontIndirect(&lf);

   // create font with height 24
   lf.lfHeight = 24;
   HFONT bigfont = ::CreateFontIndirect(&lf);

   // set some modes and color
   SetBkMode(hDC,TRANSPARENT);
   SetTextColor(hDC,RGB(0,0,0));

   // load caption string
   CString caption;
   caption.LoadString(pages[currentpage]->captionID);

   rc.top+=3; rc.left+=7;

   // draw text one
   ::SelectObject(hDC,bigfont);
   ::DrawText(hDC,caption,caption.GetLength(), &rc, DT_VCENTER | DT_LEFT );

   // load description string
   CString desc;
   desc.LoadString(pages[currentpage]->descID);

   rc.top+=22+3; rc.left+=5+2;

   // draw text two
   ::SelectObject(hDC,normfont);
   ::DrawText(hDC, desc, desc.GetLength(), &rc, DT_VCENTER | DT_LEFT );

   // delete the created fonts
   ::DeleteObject(bigfont);
   ::DeleteObject(normfont);
}

bool MainDlg::ActivatePage(int page)
{
   // activates a page

   int oldpage_index = currentpage;

   if (currentpage != -1)
   {
      // deactivate the previous active page
      PageBase* oldpage = pages[oldpage_index];
      if (!oldpage->OnLeavePage())
         return false;

      oldpage->DestroyWindow();
      if (oldpage->ShouldRemovePage())
      {
         deleteWizardPage(oldpage_index);
         if (page > currentpage)
            page--;
      }
   }

   // set current page
   if (currentpage == oldpage_index)
      currentpage=page;

   // find out rect for sub dialog
   RECT rc;
   HWND tmpWnd = GetDlgItem(IDC_MDLG_FRAME);
   ::GetWindowRect( tmpWnd, &rc );
   ScreenToClient(&rc);

   // create the modeless sub dialog
   PageBase *newpage = pages[currentpage];
   newpage->pui = this;
   newpage->Create(m_hWnd);
   newpage->MoveWindow(&rc);
   newpage->ShowWindow(SW_SHOW);
   newpage->OnEnterPage();

   // add tool tips for every control
   AddTooltips(newpage->m_hWnd,tooltips);

   // enable or disable the buttons below
   lockWizardButtons(false);

   // force redraw of the fancy color bar
   ::InvalidateRect(GetDlgItem(IDC_MDLG_CAPTIONBAR), NULL, TRUE);

   return true;
}

void MainDlg::GetCommandLineFiles()
{
   // get command line
   LPTSTR lpszCommandLine = ::GetCommandLine();

   bool first=true;

   while(lpszCommandLine[0]!=0)
   {
      // find out next stopper
      char stopper=' ';
      if (lpszCommandLine[0]=='\"')
      {
         lpszCommandLine++;
         stopper='\"';
      }

      // search for stopper
      int max = lstrlen(lpszCommandLine);
      int i;
      for(i=0;i<max;i++)
         if (lpszCommandLine[i]==stopper)
            break;

      if (first)
      {
         // skip first string
         first = false;
      }
      else
      {
         // insert filename
         CString fname(lpszCommandLine, i);
         settings.encoderjoblist.push_back(EncoderJob(fname));
      }

      // move string pointer
      lpszCommandLine += i;
      if (lpszCommandLine[0]!=0)
         lpszCommandLine += (stopper==' ' ? 1 : 2);

      // eat space chars
      while (lpszCommandLine[0] == ' ')
         lpszCommandLine++;
   }
}

LRESULT MainDlg::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   bHandled = false;

   // resize page
   if (wParam != SIZE_MINIMIZED)
   {
      int cx = GET_X_LPARAM(lParam) - m_sizePage.cx;
      int cy = GET_Y_LPARAM(lParam) - m_sizePage.cy;

      CRect rc;
      pages[currentpage]->GetWindowRect(&rc);
      ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rc, 2);
      rc.right += cx;
      rc.bottom += cy;

      pages[currentpage]->MoveWindow(rc);

      m_sizePage.cx += cx;
      m_sizePage.cy += cy;
   }

   return 0;
}
