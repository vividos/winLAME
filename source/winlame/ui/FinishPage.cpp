//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2017 Michael Fink
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
/// \file FinishPage.cpp
/// \brief Finish page
//
#include "StdAfx.h"
#include "FinishPage.hpp"
#include "WizardPageHost.hpp"
#include "IoCContainer.hpp"
#include "PresetSelectionPage.hpp"
#include "OutputSettingsPage.hpp"
#include "UISettings.hpp"
#include "CDReadJob.hpp"
#include "CDRipTitleFormatManager.hpp"
#include "RedrawLock.hpp"

using namespace UI;

/// moves (or scales) up a window in Y direction
static void MoveUpWindow(CWindow& window, int deltaY, bool scaleUp)
{
   CRect rect;
   window.GetWindowRect(rect);

   rect.top -= deltaY;
   if (!scaleUp)
      rect.bottom -= deltaY;

   window.GetParent().ScreenToClient(rect);

   window.MoveWindow(rect);
}

FinishPage::FinishPage(WizardPageHost& pageHost)
   :WizardPage(pageHost, IDD_PAGE_FINISH, WizardPage::typeCancelBackFinish),
   m_uiSettings(IoCContainer::Current().Resolve<UISettings>())
{
}

LRESULT FinishPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);
   DlgResize_Init(false, false);

   m_iconLossy.SetIcon(LoadIcon(NULL, IDI_EXCLAMATION));
   m_iconOverwrite.SetIcon(LoadIcon(NULL, IDI_EXCLAMATION));

   bool warnLossyTranscoding = m_helper.IsLossyTranscoding();
   bool warnOverwriteOriginal = m_helper.IsOverwritingOriginalFiles();

   MoveAndHideWarnings(warnLossyTranscoding, warnOverwriteOriginal);

   SetupInputTracksList();
   UpdateInputTracksList();

   UpdateOutputModule();

   return 1;
}

LRESULT FinishPage::OnButtonOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_helper.AddTasks();

   return 0;
}

LRESULT FinishPage::OnButtonBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (m_uiSettings.preset_avail && m_uiSettings.m_iLastSelectedPresetIndex > 0)
   {
      m_pageHost.SetWizardPage(std::shared_ptr<WizardPage>(new PresetSelectionPage(m_pageHost)));
   }
   else
   {
      Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();
      int modid = moduleManager.GetOutputModuleID(m_uiSettings.output_module);

      OutputSettingsPage::SetWizardPageByOutputModule(m_pageHost, modid);
   }

   return 0;
}


void FinishPage::MoveAndHideWarnings(bool warnLossyTranscoding, bool warnOverwriteOriginal)
{
   int deltaInputTracks = 0;

   if (!warnLossyTranscoding)
   {
      m_iconLossy.ShowWindow(SW_HIDE);
      m_staticLossy.ShowWindow(SW_HIDE);

      CRect lossyRect, overwriteRect;
      m_iconLossy.GetWindowRect(lossyRect);
      m_iconOverwrite.GetWindowRect(overwriteRect);

      deltaInputTracks += overwriteRect.top - lossyRect.top;
   }

   if (!warnOverwriteOriginal)
   {
      m_iconOverwrite.ShowWindow(SW_HIDE);
      m_staticOverwrite.ShowWindow(SW_HIDE);

      CRect overwriteRect, bevelRect;
      m_iconOverwrite.GetWindowRect(overwriteRect);
      m_bevel1.GetWindowRect(bevelRect);

      deltaInputTracks += bevelRect.top - overwriteRect.top;
   }
   else
   {
      if (!warnLossyTranscoding)
      {
         MoveUpWindow(m_iconOverwrite, deltaInputTracks, false);
         MoveUpWindow(m_staticOverwrite, deltaInputTracks, false);
      }
   }

   if (deltaInputTracks > 0)
   {
      MoveUpWindow(m_bevel1, deltaInputTracks, false);
      MoveUpWindow(m_listInputTracks, deltaInputTracks, true);
   }
}

void FinishPage::SetupInputTracksList()
{
   m_listInputTracks.InsertColumn(0, _T("Track"));

   // task images
   m_taskImages.Create(16, 16, ILC_MASK | ILC_COLOR32, 0, 0);
   CBitmap bmpImages;
   // load bitmap, but always from main module (bmp not in translation dlls)
   bmpImages.Attach(::LoadBitmap(ModuleHelper::GetModuleInstance(), MAKEINTRESOURCE(IDB_BITMAP_TASKS)));
   m_taskImages.Add(bmpImages, RGB(0, 0, 0));

   m_listInputTracks.SetImageList(m_taskImages, LVSIL_SMALL);
}

void FinishPage::UpdateInputTracksList()
{
   RedrawLock lock(m_listInputTracks);

   if (m_uiSettings.m_bFromInputFilesPage)
   {
      for (int i = 0, iMax = m_uiSettings.encoderjoblist.size(); i < iMax; i++)
      {
         Encoder::EncoderJob& job = m_uiSettings.encoderjoblist[i];

         CString filename = job.InputFilename();

         m_listInputTracks.InsertItem(i, filename, 1); // icon 1: encoding
      }
   }
   else
   {
      unsigned int maxJobIndex = m_uiSettings.cdreadjoblist.size();
      for (unsigned int jobIndex = 0; jobIndex < maxJobIndex; jobIndex++)
      {
         Encoder::CDReadJob& cdReadJob = m_uiSettings.cdreadjoblist[jobIndex];

         const CDRipDiscInfo& discInfo = cdReadJob.DiscInfo();
         const CDRipTrackInfo& trackInfo = cdReadJob.TrackInfo();

         if (!trackInfo.m_isActive)
            continue;

         CString title = CDRipTitleFormatManager::FormatTitle(m_uiSettings, discInfo, trackInfo);

         m_listInputTracks.InsertItem(jobIndex, title, 2); // icon 2: CD extraction
      }
   }

   m_listInputTracks.SetColumnWidth(0, LVSCW_AUTOSIZE);
}

void FinishPage::UpdateOutputModule()
{
   Encoder::ModuleManager& moduleManager = IoCContainer::Current().Resolve<Encoder::ModuleManager>();

   ATLASSERT(m_uiSettings.output_module < moduleManager.GetOutputModuleCount());
   CString outputModuleName = moduleManager.GetOutputModuleName(m_uiSettings.output_module);

   m_editOutputModule.SetWindowText(outputModuleName);
}
