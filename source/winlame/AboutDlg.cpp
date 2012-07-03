/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2012 Michael Fink

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
/// \file AboutDlg.cpp
/// \brief about dialog implementation
/// \details GetHtmlString() loads an html from the resource, replaces %var%
/// occurences and shows it in the about box.

// needed includes
#include "stdafx.h"
#include "AboutDlg.h"
#include "ModuleInterface.h"
#include "App.h"

// AboutDlg methods

LRESULT AboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CenterWindow(GetParent());

   // set window icon
   wndicon = (HICON)::LoadImage(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDI_ICON_WINLAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
   SetIcon(wndicon, FALSE);

   // create child control
   CRect rc;
   ::GetWindowRect(GetDlgItem(IDC_ABOUT_FRAME), &rc);
   ScreenToClient(&rc);
   m_browser.Create(m_hWnd, rc, NULL, WS_CHILD | WS_HSCROLL, 0, IDC_ABOUT_BROWSER);
   m_browser.ShowWindow(SW_SHOW);

   // host webbrowser control
   CComBSTR htmlstring = GetAboutHtmlText();
   m_browser.CreateControl(htmlstring);

   CDialogResize<AboutDlg>::DlgResize_Init(true, true);

   return 1;
}

LRESULT AboutDlg::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   // exits dialog
   m_browser.DestroyWindow();
   EndDialog(0);

   // delete resources
   ::DestroyIcon(wndicon);

   return 0;
}

CString AboutDlg::GetAboutHtmlText()
{
   // load the html template string from resource

   // get handle to the html resource
   HRSRC hRsrc = ::FindResource(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDR_HTML_ABOUT), RT_HTML);
   if (hRsrc == NULL)
      return _T("");

   HGLOBAL hResData = ::LoadResource(_Module.GetResourceInstance(), hRsrc );
   if (hResData == NULL)
      return _T("");

   // get the html string in the custom resource
   LPCSTR pszText = (LPCSTR)::LockResource( hResData );
   if (pszText == NULL)
      return _T("");

   CString cszHtml(_T("MSHTML:"));

   DWORD dwHtmlSize = SizeofResource(_Module.GetResourceInstance(), hRsrc );

   // add using loaded ansi text and size; may not be 0 terminated; fixes bug with blank about box
   cszHtml += CString(pszText, dwHtmlSize/sizeof(CHAR));

   // replace all appearances of %var%, where var can be some keywords
   int iPos = 0;

   while(-1 != (iPos = cszHtml.Find(_T('%'))))
   {
      // find out next variable name
      int iPos2 = cszHtml.Find('%', iPos+1);

      if (iPos2 == -1)
         break;

      CString varname = cszHtml.Mid(iPos+1, iPos2-iPos-1);

      // check for keywords
      if (varname==_T("winlamever"))
      {
         // find out winlame version from version resource
         varname = App::Version();
      }
      else if (varname==_T("installedinputmodules"))
      {
         // retrieve list of installed input module names
         varname = _T("<ul>");
         int max = m_moduleManager.getInputModuleCount();
         for(int i=0; i<max; i++)
         {
            varname += _T("<li>");
            varname += m_moduleManager.getInputModuleName(i);
            varname += _T("</li>");
         }
         varname += _T("</ul>");
      }
      else if (varname==_T("installedoutputmodules"))
      {
         // retrieve list of installed output module names
         varname = _T("<ul>");
         int max = m_moduleManager.getOutputModuleCount();
         for(int i=0; i<max; i++)
         {
            varname += _T("<li>");
            varname += m_moduleManager.getOutputModuleName(i);
            varname += _T("</li>");
         }
         varname += _T("</ul>");
      }
      else if (varname==_T("lameversion"))
         m_moduleManager.getModuleVersionString(varname,ID_OM_LAME,0);
      else if (varname==_T("lamecompiler"))
         m_moduleManager.getModuleVersionString(varname,ID_OM_LAME,1);
      else if (varname==_T("lamecpufeat"))
         m_moduleManager.getModuleVersionString(varname,ID_OM_LAME,2);
      else if (varname==_T("libsndfileversion"))
         m_moduleManager.getModuleVersionString(varname,ID_IM_SNDFILE);
      else if (varname==_T("madversion"))
         m_moduleManager.getModuleVersionString(varname,ID_IM_MAD,0);
      else if (varname==_T("madbuild"))
         m_moduleManager.getModuleVersionString(varname,ID_IM_MAD,3);
      else if (varname==_T("vorbisversion"))
         m_moduleManager.getModuleVersionString(varname,ID_OM_OGGV);
      else if (varname==_T("bassver"))
         m_moduleManager.getModuleVersionString(varname,ID_IM_BASS);
      else if (varname==_T("flacver"))
         m_moduleManager.getModuleVersionString(varname,ID_IM_FLAC);
      else if (varname==_T("wtlversion"))
      {
         varname.Format(_T("%u.%u"),
            _WTL_VER >> 8, (_WTL_VER >> 4)&15, _WTL_VER & 15);
      }
      else if (varname==_T("presetsxml"))
      {
         varname = m_cszPresetsXmlFilename;
      }

      // replace text with new one
      cszHtml.Delete(iPos, iPos2-iPos+1);
      cszHtml.Insert(iPos, varname);
   }

   return cszHtml;
}
