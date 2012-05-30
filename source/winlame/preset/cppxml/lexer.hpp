/*
   cppxml - a minimalistic c++ xml parser
   copyright (c) 2002 Michael Fink

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
/// \file lexer.hpp
/// \brief lexer declaration

// include guard
#pragma once

// needed includes
#include "cppxml.hpp"
#include <iosfwd>
#include <vector>


// namespace define
namespace cppxml
{

/// \ingroup cppxml
/// @{


/// xml lexical analyzer
class xmllexer
{
public:
   /// ctor
   xmllexer(std::istream &istr):istr(istr),cdata_mode(false),
      line(0),putback_char(-1){}

   /// returns next token
   cppxml::string get_next_token();

   /// puts back a token; the last put_back'ed token is the next returned
   void put_back(cppxml::string &token);

   /// returns if token string is a single literal character
   bool is_literal(cppxml::string &str);

protected:

   // internally used to recognize chars in the stream
   bool is_literal(int c);
   bool is_whitespace(int c);
   bool is_newline(int c);
   bool is_stringdelimiter(int c); // start-/endchar of a string

protected:
   /// cdata-mode doesn't care for whitespaces in generic strings
   bool cdata_mode;

   /// input stream
   std::istream &istr;

   /// current line
   int line;

   /// internally put_back'ed char
   int putback_char;

   /// list of put_back'ed token
   std::vector<cppxml::string> tokenstack;
};

/// @}

// end of namespace
};
