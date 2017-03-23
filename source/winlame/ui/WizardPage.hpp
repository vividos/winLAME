//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2012 Michael Fink
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file WizardPage.hpp
/// \brief Wizard page base class
//
#pragma once

namespace UI
{
// forward references
class WizardPageHost;

/// base class for a wizard dialog page
class WizardPage: public CDialogImpl<WizardPage>
{
public:
   /// wizard page type
   enum T_enWizardPageType
   {
      typeCancelNext,         ///< use for a first page in a row
      typeCancelBackNext,     ///< use for a page in the middle of a row
      typeCancelBackFinish,   ///< use for a page at the end of a row
      typeCancelOk,           ///< use for a dialog page with wizard style
   };

   /// ctor
   WizardPage(WizardPageHost& pageHost, UINT uiDialogId, T_enWizardPageType enWizardPageType)
      :m_pageHost(pageHost),
       IDD(uiDialogId),
       m_enWizardPageType(enWizardPageType)
   {
   }
   /// dtor
   ~WizardPage()
   {
   }

   /// returns caption string id
   UINT CaptionId() const { return IDD; }

   /// returns wizard page type
   T_enWizardPageType WizardPageType() const { return m_enWizardPageType; }

   friend HWND CDialogImpl<WizardPage>::Create(HWND, LPARAM);

   /// creates the dialog; hides the version in CDialogImpl<T>
   HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
   {
      return CDialogImpl<WizardPage>::Create(hWndParent, dwInitParam);
   }

   // message map
   BEGIN_MSG_MAP(WizardPage)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

protected:
   /// ref to page host
   WizardPageHost& m_pageHost;

   /// dialog id
   UINT IDD;

   /// wizard page type
   T_enWizardPageType m_enWizardPageType;
};

} // namespace UI
