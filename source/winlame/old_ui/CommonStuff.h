/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2005 Michael Fink

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
/// \file CommonStuff.h
/// \brief commonly used functions and UI classes
/// \details contains commonly used functions and UI classes, such as a bevel line
/// control and functions to add tooltips and to browse for a folder
/// \ingroup userinterface
/// @{

// include guard
#pragma once

#include "Resource.h"
#include "BevelLine.hpp"
#include "AlternateColorsListCtrl.hpp"
#include "FixedValueSpinButtonCtrl.hpp"

using UI::BevelLine;
using UI::AlternateColorsListCtrl;
using UI::FixedValueSpinButtonCtrl;

// functions

/// \brief adds tool tips for every child dlg item found for hWnd
/// \details the tooltip text is the string resource that has the same ID value as
/// the control
void AddTooltips(HWND hWnd, CToolTipCtrl &ctrl);


/// lets the user browse for a folder
bool BrowseForFolder(HWND hParentWnd, CString &dirname,UINT captionid=0);


inline int AppMessageBox(HWND hWnd, LPCTSTR pszText, UINT nFlags)
{
   return AtlMessageBox(hWnd, pszText, IDS_APP_CAPTION, nFlags);
}

inline int AppMessageBox(HWND hWnd, UINT nResourceId, UINT nFlags)
{
   return AtlMessageBox(hWnd, nResourceId, IDS_APP_CAPTION, nFlags);
}

/// @}
