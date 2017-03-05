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
/// \file HttpClient.cpp
/// \brief Simple HTTP client
//

// This file was taken from Boost.Asio examples, so it has another copyright:

//
// sync_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "HttpClient.hpp"
#include <istream>

using boost::asio::ip::tcp;

HttpResponse HttpClient::Request(std::string host, std::string path)
{
   // Get a list of endpoints corresponding to the server name.
   tcp::resolver resolver(m_ioService);
   tcp::resolver::query query(host, "http");
   tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

   // Try each endpoint until we successfully establish a connection.
   tcp::socket socket(m_ioService);
   boost::asio::connect(socket, endpoint_iterator);

   // Form the request. We specify the "Connection: close" header so that the
   // server will close the socket after transmitting the response. This will
   // allow us to treat all data up until the EOF as the content.
   boost::asio::streambuf request;
   std::ostream request_stream(&request);
   request_stream << "GET " << path << " HTTP/1.0\r\n";
   request_stream << "Host: " << host << "\r\n";
   request_stream << "User-Agent: " << m_userAgent << "\r\n";
   request_stream << "Accept: */*\r\n";
   request_stream << "Connection: close\r\n\r\n";

   // Send the request.
   boost::asio::write(socket, request);

   // Read the response status line. The response streambuf will automatically
   // grow to accommodate the entire line. The growth may be limited by passing
   // a maximum size to the streambuf constructor.
   boost::asio::streambuf response;
   boost::asio::read_until(socket, response, "\r\n");

   // Check that response is OK.
   HttpResponse httpResponse;
   std::istream response_stream(&response);

   std::string http_version;
   response_stream >> http_version;

   response_stream >> httpResponse.status_code;

   std::string status_message;
   std::getline(response_stream, status_message);
   if (!response_stream || http_version.substr(0, 5) != "HTTP/")
   {
      throw std::runtime_error("Invalid response");
   }

   if (httpResponse.status_code >= 400 && httpResponse.status_code <= 599)
   {
      std::stringstream message;
      message << "Response returned with status code " << httpResponse.status_code;
      throw std::runtime_error(message.str());
   }

   // Read the response headers, which are terminated by a blank line.
   boost::asio::read_until(socket, response, "\r\n\r\n");

   // Process the response headers.
   std::string header;
   while (std::getline(response_stream, header) && header != "\r")
      httpResponse.header_lines.push_back(header);

   // Read until EOF, writing data to output as we go.
   boost::system::error_code error;
   while (boost::asio::read(socket, response,
      boost::asio::transfer_at_least(1), error))
   {
      size_t data_size = response.size();
      if (data_size > 0)
      {
         std::copy(
            boost::asio::buffer_cast<const unsigned char*>(response.data()),
            boost::asio::buffer_cast<const unsigned char*>(response.data()) + response.size(),
            std::back_inserter(httpResponse.response_data));

         response.consume(data_size);
      }
   }

   if (error != boost::asio::error::eof)
      throw boost::system::system_error(error);

   return httpResponse;
}
