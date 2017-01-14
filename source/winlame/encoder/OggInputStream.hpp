//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2014-2017 Michael Fink
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
/// \file OggInputStream.hpp
/// \brief Ogg input stream
//
#pragma once

#include <ogg/ogg.h>

namespace Encoder
{
   /// \brief wrapper for ogg input streams
   /// \details reads in bytes from passed FILE, and outputs ogg_packet's
   class OggInputStream
   {
   public:
      /// ctor
      OggInputStream(FILE* fd, bool bFreeFd)
         :m_fd(fd),
         m_freeFd(bFreeFd),
         m_endOfStream(false),
         m_streamInit(false)
      {
         ogg_sync_init(&m_sync);

         memset(&m_stream, 0, sizeof(m_stream));
         memset(&m_currentPage, 0, sizeof(m_currentPage));
      }

      /// dtor; auto-closes stream and file
      ~OggInputStream()
      {
         ogg_sync_destroy(&m_sync);
         ogg_stream_destroy(&m_stream);

         if (m_freeFd)
            fclose(m_fd);
      }

      /// returns file ptr
      FILE* GetFd() const { return m_fd; }

      /// reads more data from file into stream
      void ReadInput(size_t uiSize)
      {
         char* data = ogg_sync_buffer(&m_sync, uiSize);
         size_t uiRead = fread(data, sizeof(char), uiSize, m_fd);

         if (uiRead == 0)
            m_endOfStream = true;
         else
            ogg_sync_wrote(&m_sync, uiRead);
      }

      /// returns if stream is at its end
      bool IsEndOfStream() const
      {
         return m_endOfStream || feof(m_fd);
      }

      /// reads next packet
      /// \param[out] packet packet to read
      /// \retval false no packet available; read more bytes (or check IsEndOfStream())
      /// \retval true packet was read correctly
      bool ReadNextPacket(ogg_packet& packet)
      {
         // packet available?
         if (m_streamInit)
         {
            int iRet = ogg_stream_packetout(&m_stream, &packet);
            if (iRet == 1)
               return true;
         }

         if (1 != ogg_sync_pageout(&m_sync, &m_currentPage))
            return false;

         if (!m_streamInit)
         {
            ogg_stream_init(&m_stream, ogg_page_serialno(&m_currentPage));
            m_streamInit = true;
         }

         if (ogg_page_serialno(&m_currentPage) != m_stream.serialno)
            ogg_stream_reset_serialno(&m_stream, ogg_page_serialno(&m_currentPage));

         ogg_stream_pagein(&m_stream, &m_currentPage);

         return 1 == ogg_stream_packetout(&m_stream, &packet);
      }

   private:
      /// file to read from
      FILE* m_fd;

      /// indicates if m_fd is freed in dtor
      bool m_freeFd;

      /// indicates if stream is at end
      bool m_endOfStream;

      /// indicates if m_stream has been init'ed
      bool m_streamInit;

      /// sync state
      ogg_sync_state m_sync;

      /// stream state
      ogg_stream_state m_stream;

      /// current page
      ogg_page m_currentPage;
   };

} // namespace Encoder
