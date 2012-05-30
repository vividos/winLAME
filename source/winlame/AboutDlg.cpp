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

   $Id: AboutDlg.cpp,v 1.25 2010/01/08 18:05:50 vividos Exp $

*/
/*! \file AboutDlg.cpp

   \brief contains the about dialog box implementation

   GetHtmlString() loads an html from the resource, replaces %var% occurences
   and shows it in the about box.

*/

// needed includes
#include "stdafx.h"
#include "AboutDlg.h"
#include <cstdio>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// global functions

//! retrieves winLAME version number
void GetWinlameVersion(CString& cszVersion)
{
   // get exe file name
   CString cszFilename(GetCommandLine());
   {
      ATLASSERT(cszFilename.GetLength() > 2);

      // remove leading and ending '"'
      TCHAR cSeparator = _T(' ');
      if (cszFilename[0] == _T('\"'))
      {
         cSeparator = _T('\"');
         cszFilename = cszFilename.Mid(1);
      }

      int nPos = cszFilename.Find(cSeparator);
      ATLASSERT(cSeparator == _T('\"') && nPos != -1);
      if (nPos != -1)
         cszFilename = cszFilename.Left(nPos);
   }

   // allocate memory for the version info struct
   DWORD nDummy=0;
   DWORD nVerInfoSize = GetFileVersionInfoSize(const_cast<LPTSTR>(static_cast<LPCTSTR>(cszFilename)), &nDummy);
   if (nVerInfoSize == 0) return;

   std::vector<BYTE> vecVerInfo(nVerInfoSize);

   if (0 == GetFileVersionInfo(const_cast<LPTSTR>(static_cast<LPCTSTR>(cszFilename)), 0, nVerInfoSize, &vecVerInfo[0]))
      return;

   // retrieve version language
   LPVOID pVersion = NULL;
   UINT nVersionLen;

   BOOL bRet = VerQueryValue(&vecVerInfo[0], _T("\\VarFileInfo\\Translation"), &pVersion, &nVersionLen);
   if (!bRet) return;

   CString cszFileVersion;
   if (bRet && nVersionLen==4)
   {
      DWORD nLang = *(DWORD*)pVersion;

      cszFileVersion.Format(_T("\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion"),
         (nLang & 0xff00)>>8, nLang & 0xff, (nLang & 0xff000000)>>24, (nLang & 0xff0000)>>16);
   }
   else
      cszFileVersion.Format(_T("\\StringFileInfo\\%04X04B0\\FileVersion"),GetUserDefaultLangID());

   bRet = VerQueryValue(&vecVerInfo[0], const_cast<LPTSTR>(static_cast<LPCTSTR>(cszFileVersion)), &pVersion, &nVersionLen);
   if (bRet)
      cszVersion = (LPTSTR)pVersion;
}


// AboutDlg methods

void AboutDlg::GetHtmlString(CComBSTR& str)
{
   // load the html template string from resource

   // get handle to the html resource
   HRSRC hRsrc = ::FindResource(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDR_HTML_ABOUT), RT_HTML );
   if (hRsrc == NULL)
      return;

   HGLOBAL hResData = ::LoadResource(_Module.GetResourceInstance(), hRsrc );
   if (hResData == NULL)
      return;

   // get the html string in the custom resource
   LPCSTR pszText = (LPCSTR)::LockResource( hResData );
   if (pszText == NULL)
      return;

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
         varname = _T("N/A");
         GetWinlameVersion(varname);
      }
      else if (varname==_T("installedinputmodules"))
      {
         // retrieve list of installed input module names
         varname = _T("<ul>");
         int max = module_manager->getInputModuleCount();
         for(int i=0; i<max; i++)
         {
            varname += _T("<li>");
            varname += module_manager->getInputModuleName(i);
            varname += _T("</li>");
         }
         varname += _T("</ul>");
      }
      else if (varname==_T("installedoutputmodules"))
      {
         // retrieve list of installed output module names
         varname = _T("<ul>");
         int max = module_manager->getOutputModuleCount();
         for(int i=0; i<max; i++)
         {
            varname += _T("<li>");
            varname += module_manager->getOutputModuleName(i);
            varname += _T("</li>");
         }
         varname += _T("</ul>");
      }
      else if (varname==_T("lameversion"))
         module_manager->getModuleVersionString(varname,ID_OM_LAME,0);
      else if (varname==_T("lamecompiler"))
         module_manager->getModuleVersionString(varname,ID_OM_LAME,1);
      else if (varname==_T("lamecpufeat"))
         module_manager->getModuleVersionString(varname,ID_OM_LAME,2);
      else if (varname==_T("libsndfileversion"))
         module_manager->getModuleVersionString(varname,ID_IM_SNDFILE);
      else if (varname==_T("madversion"))
         module_manager->getModuleVersionString(varname,ID_IM_MAD,0);
      else if (varname==_T("madbuild"))
         module_manager->getModuleVersionString(varname,ID_IM_MAD,3);
      else if (varname==_T("vorbisversion"))
         module_manager->getModuleVersionString(varname,ID_OM_OGGV);
      else if (varname==_T("bassver"))
         module_manager->getModuleVersionString(varname,ID_IM_BASS);
      else if (varname==_T("flacver"))
         module_manager->getModuleVersionString(varname,ID_IM_FLAC);
      else if (varname==_T("stlportversion"))
      {
#ifdef _STLPORT_MAJOR
         varname.Format(_T("%u.%u.%u"),
            _STLPORT_MAJOR, _STLPORT_MINOR, _STLPORT_PATCHLEVEL);
#else
         varname = _T("5.0");
#endif
      }
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

   // hand over the string
   str = cszHtml;
}
