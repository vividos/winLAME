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
/// \file HttpClient.hpp
/// \brief Simple HTTP client
//
#pragma once

#include "Asio.hpp"
#include <string>

/// HTTP response
struct HttpResponse
{
   /// HTTP status code, e.g. 200 or 404
   unsigned int status_code;

   /// returned header lines
   std::vector<std::string> header_lines;

   /// response data returned after header lines
   std::vector<unsigned char> response_data;
};

/// Simple HTTP client
class HttpClient
{
public:
   /// ctor; creates new client
   HttpClient(const std::string& userAgent)
      :m_userAgent(userAgent)
   {
   }

   /// sends a request to host with given path and returns the HTTP response
   HttpResponse Request(std::string host, std::string path);

private:
   /// user agent to use for request
   std::string m_userAgent;

   /// IO service used for request
   boost::asio::io_service m_ioService;
};
