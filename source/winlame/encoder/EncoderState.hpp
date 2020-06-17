//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2020 Michael Fink
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
/// \file EncoderState.hpp
/// \brief encoder state
//
#pragma once

#include <atomic>

namespace Encoder
{
   /// encoder state
   struct EncoderState
   {
      /// ctor
      EncoderState()
         :m_running(false),
         m_paused(false),
         m_finished(false),
         m_percent(0.f),
         m_errorCode(0)
      {
      }

      /// copy ctor
      EncoderState(const EncoderState& otherState)
         :m_running((bool)otherState.m_running),
         m_paused((bool)otherState.m_paused),
         m_finished((bool)otherState.m_finished),
         m_percent((float)otherState.m_percent),
         m_encodingDescription(otherState.m_encodingDescription),
         m_errorCode((int)otherState.m_errorCode)
      {
      }

      /// move ctor
      EncoderState(const EncoderState&& otherState) noexcept
         :m_running((bool)otherState.m_running),
         m_paused((bool)otherState.m_paused),
         m_finished((bool)otherState.m_finished),
         m_percent((float)otherState.m_percent),
         m_encodingDescription(otherState.m_encodingDescription),
         m_errorCode((int)otherState.m_errorCode)
      {
      }

      /// assignment copy operator
      EncoderState& operator=(const EncoderState& otherState)
      {
         m_running = (bool)otherState.m_running;
         m_paused = (bool)otherState.m_paused;
         m_finished = (bool)otherState.m_finished;
         m_percent = (float)otherState.m_percent;
         m_encodingDescription = otherState.m_encodingDescription;
         m_errorCode = (int)otherState.m_errorCode;

         return *this;
      }

      /// assignment move operator
      EncoderState& operator=(const EncoderState&& otherState) noexcept
      {
         m_running = (bool)otherState.m_running;
         m_paused = (bool)otherState.m_paused;
         m_finished = (bool)otherState.m_finished;
         m_percent = (float)otherState.m_percent;
         m_encodingDescription = otherState.m_encodingDescription;
         m_errorCode = (int)otherState.m_errorCode;

         return *this;
      }

      /// indicates if encoder is running
      std::atomic<bool> m_running;

      /// indicates if encoder is paused
      std::atomic<bool> m_paused;

      /// indicates if encoder has been finished
      std::atomic<bool> m_finished;

      /// indicates percent done
      float m_percent;

      /// encoding description
      CString m_encodingDescription;

      /// \brief current error code
      /// \details 0 means no error, a positive integer indictates an error, a
      /// negative one is a fatal error and should stop the whole encoding
      /// process
      std::atomic<int> m_errorCode;
   };

} // namespace Encoder
