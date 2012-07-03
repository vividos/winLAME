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
/// \file WizardPageHost.cpp
/// \brief Wizard page host window

// includes
#include "StdAfx.h"
#include "WizardPageHost.h"
#include "WizardPage.h"

void WizardPageHost::SetWizardPage(boost::shared_ptr<WizardPage> spCurrentPage)
{
   m_spCurrentPage = spCurrentPage;
}

int WizardPageHost::Run(HWND hWndParent)
{
   // note: parent must be disabled after creating dialog, and enabled before
   // destroying dialog.
   // see http://blogs.msdn.com/b/oldnewthing/archive/2004/02/27/81155.aspx

   Create(hWndParent, CWindow::rcDefault);

   ::EnableWindow(hWndParent, FALSE);

   ATLASSERT(m_spCurrentPage != NULL);
   InitPage();

   int iRet = CMessageLoop::Run();

   ::EnableWindow(hWndParent, TRUE);

   DestroyWindow();

   return iRet;
}

void WizardPageHost::InitPage()
{
   // find out rect for sub dialog
   CRect rc;
   CWindow wndFrame = GetDlgItem(IDC_WIZARDPAGE_STATIC_FRAME);
   wndFrame.GetWindowRect(&rc);
   ScreenToClient(&rc);

   HWND hWndPage = m_spCurrentPage->Create(m_hWnd);

   m_spCurrentPage->MoveWindow(&rc);
   m_spCurrentPage->ShowWindow(SW_SHOW);

   // set new caption
   CString cszCaption;
   cszCaption.LoadString(m_spCurrentPage->CaptionId());
   m_staticCaption.SetWindowText(cszCaption);

   // set new window name
   SetWindowText(cszCaption);

   AddTooltips(hWndPage);

   // configure buttons according to page
   ConfigWizardButtons(m_spCurrentPage->WizardPageType());
}

void WizardPageHost::ConfigWizardButtons(WizardPage::T_enWizardPageType enWizardPageType)
{
   // TODO string resources
   bool bEnableAction1 = true;
   switch(enWizardPageType)
   {
   case WizardPage::typeCancelNext:
      m_buttonAction2.SetWindowText(_T("Cancel"));
      m_buttonActionOK.SetWindowText(_T("&Next >"));
      bEnableAction1 = false;
      break;
   case WizardPage::typeCancelBackNext:
      m_buttonAction1.SetWindowText(_T("Cancel"));
      m_buttonAction2.SetWindowText(_T("< &Back"));
      m_buttonActionOK.SetWindowText(_T("&Next >"));
      bEnableAction1 = true;
      break;
   case WizardPage::typeCancelBackFinish:
      m_buttonAction1.SetWindowText(_T("Cancel"));
      m_buttonAction2.SetWindowText(_T("< &Back"));
      m_buttonActionOK.SetWindowText(_T("&Finish"));
      bEnableAction1 = true;
      break;
   case WizardPage::typeCancelOk:
      m_buttonAction2.SetWindowText(_T("Cancel"));
      m_buttonActionOK.SetWindowText(_T("OK"));
      bEnableAction1 = false;
      break;
   }

   m_buttonAction1.EnableWindow(bEnableAction1 ? TRUE : FALSE);
   m_buttonAction1.ShowWindow(bEnableAction1 ? SW_SHOW : SW_HIDE);
}

void WizardPageHost::AddTooltips(HWND hWnd)
{
   CWindow wndDialog(hWnd);
   CWindow wnd = wndDialog.GetWindow(GW_CHILD);

   // go through all child windows
   while (wnd.m_hWnd != NULL)
   {
      int id = wnd.GetDlgCtrlID();

      if (id != -1 && id != 0)
      {
         // try to load a text with same id
         CString cszToolText;
         cszToolText.LoadString(id);

         if (!cszToolText.IsEmpty())
            m_tooltipCtrl.AddTool(wnd.m_hWnd, (LPCTSTR)cszToolText);
      }

      wnd = wnd.GetWindow(GW_HWNDNEXT);
   }
}

LRESULT WizardPageHost::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   CenterWindow();

   // set icons
   HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME),
      IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
   SetIcon(hIcon, TRUE);
   HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_WINLAME),
      IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
   SetIcon(hIconSmall, FALSE);

   // create caption font
   CLogFont lf(AtlGetDefaultGuiFont());
   lf.lfHeight *= 2;

   m_fontCaption = lf.CreateFontIndirect();

   m_staticCaption.SetFont(m_fontCaption, FALSE);

   // tooltips
   m_tooltipCtrl.Create(m_hWnd);
   AddTooltips(m_hWnd);
   m_tooltipCtrl.Activate(TRUE);

   // enable resizing
   CRect rectPage;
   GetClientRect(&rectPage);
   m_sizePage.cx = rectPage.Width();
   m_sizePage.cy = rectPage.Height();

   DlgResize_Init(true, true);

   return 1;  // let the system set the focus
}

LRESULT WizardPageHost::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   bHandled = false;

   // resize page
   if (wParam == SIZE_MINIMIZED)
      return 0;

   int cx = GET_X_LPARAM(lParam) - m_sizePage.cx;
   int cy = GET_Y_LPARAM(lParam) - m_sizePage.cy;

   if (m_spCurrentPage != NULL)
   {
      CRect rc;
      m_spCurrentPage->GetWindowRect(&rc);
      ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rc, 2);
      rc.right += cx;
      rc.bottom += cy;

      m_spCurrentPage->MoveWindow(rc);
   }

   m_sizePage.cx += cx;
   m_sizePage.cy += cy;

   return 0;
}

LRESULT WizardPageHost::OnStaticColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
   UINT idCtrl = static_cast<UINT>(wParam);

   // we only have one static control: the caption
   CWindow wnd(reinterpret_cast<HWND>(lParam));
   if (idCtrl == IDC_WIZARDPAGE_STATIC_CAPTION)
   {
      CDCHandle obDC(reinterpret_cast<HDC>(wParam));
      obDC.SetTextColor(RGB(0, 0, 0));
      obDC.SetBkMode(OPAQUE);
      obDC.SetBkColor(TRANSPARENT);
   }

   return (LRESULT)GetStockObject(NULL_BRUSH);
}

LRESULT WizardPageHost::OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
   // draws all sorts of owner drawn items
   UINT idCtrl = static_cast<UINT>(wParam);

   if (idCtrl == IDC_WIZARDPAGE_STATIC_BACKGROUND)
   {
      LPDRAWITEMSTRUCT lpdis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);

      // owner-draw caption background
      ::SetBkColor(lpdis->hDC, RGB(255,255,255));
      ::ExtTextOut(lpdis->hDC, 0, 0, ETO_OPAQUE, &lpdis->rcItem, NULL, 0, NULL);
   }

   return 0;
}

LRESULT WizardPageHost::OnButtonClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   ATLASSERT(m_spCurrentPage != NULL);

   switch (m_spCurrentPage->WizardPageType())
   {
   case WizardPage::typeCancelNext:
      if (wID != IDOK)
         wID = IDCANCEL;
      break;

   case WizardPage::typeCancelBackNext:
      if (wID == IDC_WIZARDPAGE_BUTTON_ACTION1)
         wID = IDCANCEL;
      if (wID == IDC_WIZARDPAGE_BUTTON_ACTION2)
         wID = ID_WIZBACK;
      break;

   case WizardPage::typeCancelBackFinish:
      if (wID == IDC_WIZARDPAGE_BUTTON_ACTION1)
         wID = IDCANCEL;
      if (wID == IDC_WIZARDPAGE_BUTTON_ACTION2)
         wID = ID_WIZBACK;
      break;

   case WizardPage::typeCancelOk:
      if (wID != IDOK)
         wID = IDCANCEL;
      break;
   }

   // remove current page; when page doesn't set up new page, we leave
   boost::shared_ptr<WizardPage> spCurrentPage = m_spCurrentPage;
   m_spCurrentPage.reset();

   // send button press to current page
   spCurrentPage->SendMessage(WM_COMMAND, MAKELONG(wID, BN_CLICKED));

   // TODO check result code

   // clear last page
   spCurrentPage->DestroyWindow();

   // now check if there's another page
   if (m_spCurrentPage == NULL)
   {
      PostQuitMessage(wID);
   }
   else
   {
      // init next dialog
      InitPage();
   }

   return 0;
}
