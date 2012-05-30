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
/// \file lexer.cpp
/// \brief lexer implementation

// needed includes
#include "lexer.hpp"
#include <fstream>
#include <cstdio>


cppxml::string cppxml::xmllexer::get_next_token()
{
   cppxml::string ret;
   if (tokenstack.size()>0)
   {
      ret=tokenstack.back();
      tokenstack.pop_back();
      return ret;
   }

   bool finished = false;
   int c=0;

   do
   {
      // get char
      if (putback_char==-1)
         c = istr.get();
      else
      {
         c = putback_char;
         putback_char = -1;
      }

      // is it a literal?
      if (is_literal(c))
      {
         cdata_mode = false;
         if (ret.size()==0)
         {
            ret=c;
            if (c=='>') cdata_mode = true;
            break;
         }
         putback_char = c;
         break;
      }

      if (!cdata_mode)
      {
         // string delimiter?
         if (is_stringdelimiter(c))
         {
            ret = c;
            int delim = c;
            do
            {
               c = istr.get();
               if (c==EOF) break;
               ret += c;
            } while (c != delim);

            break;
         }
      }

      // whitespace?
      if (is_whitespace(c))
      {
         if (ret.length()==0)
            continue;
         else
            if (!cdata_mode)
               break;
      }

      // newline char?
      if (is_newline(c))
      {
         if (ret.length()==0)
            continue;
      }

      // add to generic string
      ret += c;
   }
   while (!finished);

   return ret;
}

void cppxml::xmllexer::put_back(cppxml::string &token)
{
   tokenstack.push_back(token);
}

bool cppxml::xmllexer::is_literal(cppxml::string &str)
{
   return (str.size()==1 && is_literal(str.at(0)));
}

// returns if we have a literal char
bool cppxml::xmllexer::is_literal(int c)
{
   switch(c)
   {
   case '?':
   case '=':
   case '!':
   case '/':
      if (cdata_mode)
         return false;
   case EOF:
   case '<':
   case '>':
      return true;
   }
   return false;
}

// returns if we have a white space char
bool cppxml::xmllexer::is_whitespace(int c)
{
   switch(c)
   {
   case ' ':
   case '\t':
      return true;
   }
   return false;
}

// returns if we have a newline
bool cppxml::xmllexer::is_newline(int c)
{
   switch(c)
   {
   case '\n':
      line++;
   case '\r':
      return true;
   }
   return false;
}

// returns if we have a string delimiter (separating " and ')
bool cppxml::xmllexer::is_stringdelimiter(int c)
{
   switch(c)
   {
   case '\"':
   case '\'':
      return true;
   }
   return false;
}
