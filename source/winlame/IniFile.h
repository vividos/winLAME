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
/// \file IniFile.h
/// \brief Ini file wrapper
//
#pragma once

/// ini file wrapper
class IniFile
{
public:
   /// ctor
   IniFile(const CString& iniFilename)
      :m_iniFilename(iniFilename)
   {
   }

   /// returns integer value from section and key
   int GetInt(LPCTSTR sectionName, LPCTSTR keyName, int defaultValue)
   {
      return ::GetPrivateProfileInt(sectionName, keyName, defaultValue, m_iniFilename);
   }

   /// returns string value from section and key
   CString GetString(LPCTSTR sectionName, LPCTSTR keyName, LPCTSTR defaultValue)
   {
      CString value;
      ::GetPrivateProfileString(sectionName, keyName, defaultValue, value.GetBuffer(512), 512, m_iniFilename);
      value.ReleaseBuffer();

      return value;
   }

   /// writes a string value to a section and key
   void WriteString(LPCTSTR sectionName, LPCTSTR keyName, LPCTSTR value)
   {
      ::WritePrivateProfileString(sectionName, keyName, value, m_iniFilename);
   }

private:
   /// ini filename
   CString m_iniFilename;
};
