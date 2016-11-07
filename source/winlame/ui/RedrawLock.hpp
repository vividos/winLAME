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
/// \file RedrawLock.hpp
/// \brief Redraw lock for updating windows
//
#pragma once

namespace UI
{
   /// locks a CWindow based object with SetRedraw to prevent flickering while updating
   class RedrawLock
   {
   public:
      /// locks window to prevent redraws
      RedrawLock(CWindow& window)
         :m_window(window)
      {
         window.SetRedraw(false);
      }

      /// dtor; unlocks window again
      ~RedrawLock()
      {
         m_window.SetRedraw(true);
      }

   private:
      /// window to lock/unlock
      CWindow& m_window;
   };

} // namespace UI
