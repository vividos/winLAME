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
/// \file CommandLineParser.cpp
/// \brief Command line parser
//

// includes
#include "stdafx.h"
#include "CommandLineParser.hpp"

bool CommandLineParser::GetNext(CString& param)
{
   if (m_commandLine[0] == 0)
      return false;

   // find out next stopper
   char stopper = ' ';
   if (m_commandLine[0] == '\"')
   {
      m_commandLine++;
      stopper = '\"';
   }

   // search for stopper
   int max = lstrlen(m_commandLine);
   int i;
   for (i = 0; i<max; i++)
      if (m_commandLine[i] == stopper)
         break;

   param = CString(m_commandLine, i);

   // move string pointer
   m_commandLine += i;
   if (m_commandLine[0] != 0 && stopper != ' ')
      m_commandLine++;

   // eat space chars
   while (m_commandLine[0] == ' ')
      m_commandLine++;

   return true;
}
