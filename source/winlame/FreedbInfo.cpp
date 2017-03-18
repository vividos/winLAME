//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2016 Michael Fink
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
/// \file FreedbInfo.cpp
/// \brief FreeDB CD infos
//
#include "stdafx.h"
#include "FreedbInfo.hpp"

FreedbInfo::FreedbInfo(const CString& entry)
{
   ParseEntry(entry);
   ParseTrackTitles();

   CString discTitle = DiscTitle();
   int pos = discTitle.Find(_T(" / "));

   if (pos != -1)
   {
      m_allEntries["DARTIST"] = discTitle.Left(pos);
      m_allEntries["DTITLE"] = discTitle.Mid(pos + 3);
   }
}

void FreedbInfo::ParseEntry(CString entry)
{
   entry.Replace(_T("\r\n"), _T("\n"));

   CString line;
   bool firstLine = true;
   while (!entry.IsEmpty() && entry.Find(_T(".")) != 0)
   {
      // get next line
      int lineEndPos = entry.Find(_T('\n'));
      if (lineEndPos == -1)
      {
         line = entry;
         entry.Empty();
      }
      else
      {
         line = entry.Left(lineEndPos);
         entry = entry.Mid(lineEndPos + 1);
      }

      line.Trim();

      // ignore first line (contains http code)
      if (firstLine)
      {
         firstLine = false;
         continue;
      }

      // check comment
      if (line.Find(_T("#")) == 0)
         continue;

      int equalSignPos = line.Find(_T('='));

      if (equalSignPos != -1)
      {
         m_allEntries.insert(
            std::make_pair(line.Left(equalSignPos), line.Mid(equalSignPos + 1)));
      }
      else
         ATLTRACE(_T("FreeDB: Invalid response line: %s"), line.GetString());
   }
}

void FreedbInfo::ParseTrackTitles()
{
   for (size_t trackIndex = 0; trackIndex < 256; trackIndex++)
   {
      CString entryName;
      entryName.Format(_T("TTITLE%Iu"), trackIndex);

      auto iter = m_allEntries.find(entryName);

      if (iter != m_allEntries.end())
         m_allTrackTitles.push_back(iter->second);
      else
         break;
   }
}

CString FreedbInfo::EntryOrDefault(LPCTSTR entryName, LPCTSTR defaultValue) const
{
   auto iter = m_allEntries.find(entryName);

   if (iter != m_allEntries.end())
      return iter->second;
   else
      return CString(defaultValue);
}
