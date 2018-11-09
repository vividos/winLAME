//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
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
//
#include "stdafx.h"
#include "WizardPageHost.hpp"
#include "WizardPage.hpp"
#include "App.hpp"
#include "AboutDlg.hpp"
#include "GeneralSettingsPage.hpp"

using UI::WizardPageHost;
using UI::WizardPage;

#define IDM_ABOUTBOX 16    ///< menu id for about box
#define IDM_OPTIONS 48     ///< menu id for options
#define IDM_APPMODE 64     ///< menu id for app mode change

WizardPageHost::WizardPageHost(bool isClassicMode)
   :m_isClassicMode(isClassicMode),
   m_isAppModeChanged(false)
{
}

void WizardPageHost::SetWizardPage(std::shared_ptr<WizardPage> spCurrentPage)
{
   m_spCurrentPage = spCurrentPage;
}

int WizardPageHost::Run(HWND hWndParent)
{
   if (IsClassicMode())
      _Module.AddMessageLoop(this);

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

   if (IsClassicMode())
      _Module.RemoveMessageLoop();

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
   CString caption;
   caption.LoadString(m_spCurrentPage->CaptionId());
   m_staticCaption.SetWindowText(caption);

   // set new window name
   CString title = _T("winLAME ") + App::Version() + " - " + caption;
   SetWindowText(title);

   AddTooltips(hWndPage);

   // configure buttons according to page
   ConfigWizardButtons(m_spCurrentPage->WizardPageType());

   GetDlgItem(IDC_WIZARDPAGE_STATIC_BACKGROUND).Invalidate();
   m_staticCaption.Invalidate();
}

void WizardPageHost::ConfigWizardButtons(WizardPage::T_enWizardPageType enWizardPageType)
{
   bool bEnableAction1 = true;
   switch (enWizardPageType)
   {
   case WizardPage::typeCancelNext:
      m_buttonAction2.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_CANCEL)));
      m_buttonActionOK.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_NEXT)));
      bEnableAction1 = false;
      break;

   case WizardPage::typeCancelBackNext:
      m_buttonAction1.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_CANCEL)));
      m_buttonAction2.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_BACK)));
      m_buttonActionOK.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_NEXT)));
      bEnableAction1 = true;
      break;

   case WizardPage::typeCancelBackFinish:
      m_buttonAction1.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_CANCEL)));
      m_buttonAction2.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_BACK)));
      m_buttonActionOK.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_FINISH)));
      bEnableAction1 = true;
      break;

   case WizardPage::typeCancelOk:
      m_buttonAction2.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_CANCEL)));
      m_buttonActionOK.SetWindowText(CString(MAKEINTRESOURCE(IDS_WIZARDPAGE_OK)));
      bEnableAction1 = false;
      break;

   default:
      ATLASSERT(false);
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

void WizardPageHost::ShowHelp()
{
   if (!App::Current().IsHelpAvailable())
      return;

   // get help topic path
   UINT dialogId = m_spCurrentPage->CaptionId();
   CString helpPath = MapDialogIdToHelpPath(dialogId);

   if (!helpPath.IsEmpty())
      m_htmlHelper.DisplayTopic(helpPath);
   else
      m_htmlHelper.DisplayTopic(_T("/html/pages/modernui.html"));
}

void WizardPageHost::SwitchToModernMode()
{
   m_isAppModeChanged = true;

   UISettings& settings = IoCContainer::Current().Resolve<UISettings>();

   settings.m_appMode = UISettings::modernMode;
   settings.StoreSettings();

   PostQuitMessage(0);
}

void WizardPageHost::AddSystemMenuEntries()
{
   // IDM_ABOUTBOX and IDM_OPTIONS must be in the system command range.
   ATLASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
   ATLASSERT(IDM_ABOUTBOX < 0xF000);
   ATLASSERT((IDM_OPTIONS & 0xFFF0) == IDM_OPTIONS);
   ATLASSERT(IDM_OPTIONS < 0xF000);
   ATLASSERT((IDM_APPMODE & 0xFFF0) == IDM_APPMODE);
   ATLASSERT(IDM_APPMODE < 0xF000);

   CMenu sysmenu;
   sysmenu.Attach(GetSystemMenu(FALSE));
   if (sysmenu != NULL)
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
}

BOOL WizardPageHost::PreTranslateMessage(MSG* pMsg)
{
   // relay mouse move messages to the tooltip ctrl
   if (pMsg->message == WM_MOUSEMOVE)
      m_tooltipCtrl.RelayEvent(pMsg);

   // handle dialog messages
   if (IsDialogMessage(pMsg))
      return TRUE;

   return CMessageLoop::PreTranslateMessage(pMsg);
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

   // set the button images
   m_helpIcon.Create(MAKEINTRESOURCE(IDB_HELP), 14, 0, RGB(192, 192, 192));
   CButton helpButton(GetDlgItem(IDC_WIZARDPAGE_HELP));
   helpButton.SetIcon(m_helpIcon.ExtractIcon(0));

   CImageList feedbackIcons;
   feedbackIcons.Create(16, 16, ILC_MASK | ILC_COLOR32, 0, 0);

   CBitmap bmpIcons;
   bmpIcons.LoadBitmap(IDR_MAINFRAME);
   feedbackIcons.Add(bmpIcons, RGB(0, 0, 0));

   CButton positiveButton(GetDlgItem(ID_FEEDBACK_POSITIVE));
   positiveButton.SetIcon(feedbackIcons.ExtractIcon(8));

   CButton negativeButton(GetDlgItem(ID_FEEDBACK_NEGATIVE));
   negativeButton.SetIcon(feedbackIcons.ExtractIcon(9));

   CButton switchModernButton(GetDlgItem(ID_VIEW_SWITCH_MODERN));
   switchModernButton.SetIcon(feedbackIcons.ExtractIcon(7));

   // set visibility of buttons
   if (!IsClassicMode())
   {
      switchModernButton.ShowWindow(SW_HIDE);
   }

   // don't show feedback buttons
   positiveButton.ShowWindow(SW_HIDE);
   negativeButton.ShowWindow(SW_HIDE);

   // check if help is available
   if (App::Current().IsHelpAvailable())
      m_htmlHelper.Init(m_hWnd, App::Current().HelpFilename());
   else
      GetDlgItem(IDC_WIZARDPAGE_HELP).ShowWindow(SW_HIDE);

   // create caption font
   CLogFont lf(AtlGetDefaultGuiFont());
   lf.lfHeight *= 2;

   m_fontCaption = lf.CreateFontIndirect();

   m_staticCaption.SetFont(m_fontCaption, FALSE);

   // tooltips
   m_tooltipCtrl.Create(m_hWnd);
   AddTooltips(m_hWnd);
   m_tooltipCtrl.Activate(TRUE);

   if (IsClassicMode())
      AddSystemMenuEntries();

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

      if (lpdis->itemState != ODS_DISABLED)
      {
         // owner-draw caption background
         ::SetBkColor(lpdis->hDC, RGB(255, 255, 255));
         ::ExtTextOut(lpdis->hDC, 0, 0, ETO_OPAQUE, &lpdis->rcItem, NULL, 0, NULL);
      }
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

   default:
      ATLASSERT(false);
      break;
   }

   // remove current page; when page doesn't set up new page, we leave
   std::shared_ptr<WizardPage> spCurrentPage = m_spCurrentPage;
   m_spCurrentPage.reset();

   // send button press to current page
   LRESULT result = spCurrentPage->SendMessage(WM_COMMAND, MAKELONG(wID, BN_CLICKED));

   // save last settings that were modified on the page
   if (wID != IDCANCEL)
   {
      UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
      settings.StoreSettings();
   }

   if (result == 1)
   {
      m_spCurrentPage = spCurrentPage;

      return 0; // message was rejected
   }

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

/// maps dialog id to help path
CString WizardPageHost::MapDialogIdToHelpPath(UINT dialogId)
{
   UINT helpId = 0;
   switch (dialogId)
   {
   case IDD_PAGE_INPUT_FILES: helpId = IDS_HTML_INPUT; break;
   case IDD_PAGE_INPUT_CD: helpId = IDS_HTML_INPUT_CD; break;
   case IDD_PAGE_OUTPUT_SETTINGS: helpId = IDS_HTML_OUTPUT; break;
   case IDD_PAGE_LAME_SETTINGS: helpId = IDS_HTML_LAME_SIMPLE; break;
   case IDD_PAGE_OPUS_SETTINGS: helpId = IDS_HTML_OPUS; break;
   case IDD_PAGE_LIBSNDFILE_SETTINGS: helpId = IDS_HTML_WAVE; break;
   case IDD_PAGE_OGGVORBIS_SETTINGS: helpId = IDS_HTML_OGGVORBIS; break;
   case IDD_PAGE_AAC_SETTINGS: helpId = IDS_HTML_AAC; break;
   case IDD_PAGE_WMA_SETTINGS: helpId = IDS_HTML_WMA; break;
   case IDD_PAGE_PRESET_SELECTION: helpId = IDS_HTML_PRESETS; break;
   case IDD_PAGE_FINISH: helpId = IDS_HTML_FINISH; break;
   case IDD_SETTINGS_CDREAD: helpId = IDS_HTML_SETTINGS_CDREAD; break;
   case IDD_SETTINGS_GENERAL: helpId = IDS_HTML_SETTINGS_GENERAL; break;
   case IDD_PAGE_CLASSIC_START: helpId = IDS_HTML_CLASSIC_START; break;
   case IDD_PAGE_CLASSIC_ENCODE: helpId = IDS_HTML_CLASSIC_ENCODE; break;
   default:
      ATLASSERT(false);
      break;
   }

   if (helpId == 0)
      return CString();

   CString helpPath;
   helpPath.LoadString(helpId);

   return helpPath;
}

LRESULT WizardPageHost::OnHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   ShowHelp();
   return 0;
}

LRESULT WizardPageHost::OnHelpButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   ShowHelp();
   return 0;
}

LRESULT WizardPageHost::OnViewSwitchToModern(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   SwitchToModernMode();
   return 0;
}

LRESULT WizardPageHost::OnFeedbackPositive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   extern LPCTSTR c_urlFeedbackPositive;

   ShellExecute(m_hWnd, _T("open"), c_urlFeedbackPositive, nullptr, nullptr, SW_SHOWNORMAL);
   return 0;
}

LRESULT WizardPageHost::OnFeedbackNegative(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   extern LPCTSTR c_urlFeedbackNegative;

   ShellExecute(m_hWnd, _T("open"), c_urlFeedbackNegative, nullptr, nullptr, SW_SHOWNORMAL);
   return 0;
}

LRESULT WizardPageHost::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   if ((wParam & 0xFFF0) == IDM_ABOUTBOX)
   {
      // show about dialog
      UI::AboutDlg dlg;

      UISettings& settings = IoCContainer::Current().Resolve<UISettings>();
      dlg.SetPresetsXmlFilename(settings.presets_filename);

      dlg.DoModal();
   }
   else
      if ((wParam & 0xFFF0) == IDM_OPTIONS)
      {
         WizardPageHost host;
         host.SetWizardPage(std::shared_ptr<WizardPage>(
            new GeneralSettingsPage(host,
               IoCContainer::Current().Resolve<UISettings>(),
               IoCContainer::Current().Resolve<LanguageResourceManager>())));
         host.Run(m_hWnd);
      }
      else
         if ((wParam & 0xFFF0) == IDM_APPMODE)
         {
            SwitchToModernMode();
         }
         else
            bHandled = false;

   return 0;
}
