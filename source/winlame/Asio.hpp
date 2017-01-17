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
/// \file Asio.hpp
/// \brief Boost.Asio include
//
#pragma once

// don't link Regex and DateTime
#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

// asio but no winsock init
#define BOOST_ASIO_DETAIL_IMPL_WINSOCK_INIT_IPP

// ignore prefast warnings in Boost.Asio header files
#pragma warning(push)
#pragma warning(disable: 6001 6011 6031 6255 6258 6386 6387)

// Boost.Asio in version 1.59 outputs a warning when compiling with VS 2015
#pragma warning(disable: 4005) // 'BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT': macro redefinition

// includes
#include <boost/asio.hpp>

#pragma warning(pop)

// asio but no winsock init

/// \cond false
inline void boost::asio::detail::winsock_init_base::startup(boost::asio::detail::winsock_init_base::data &,unsigned char,unsigned char){}
inline void boost::asio::detail::winsock_init_base::cleanup(boost::asio::detail::winsock_init_base::data &){}
inline void boost::asio::detail::winsock_init_base::throw_on_error(boost::asio::detail::winsock_init_base::data &){}
/// \endcond
