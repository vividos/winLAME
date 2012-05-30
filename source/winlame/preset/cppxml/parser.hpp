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
/*! \file parser.hpp

   \brief nodebuilder and parser declaration

*/

// include guard
#pragma once

// needed includes
#include "cppxml.hpp"
#include <iosfwd>
#include "lexer.hpp"


// namespace define
namespace cppxml
{

/*! \ingroup cppxml */
/*! @{ */


//! node builder builds up a node structure
class xmlnodebuilder: public xmleventhandler
{
public:
   //! ctor
   xmlnodebuilder(xmlnode *node);

   // handler

   virtual bool have_tag_start(const cppxml::string &name);
   virtual bool have_attribute(const cppxml::string &name,
      const cppxml::string &value);
   virtual bool have_cdata(const cppxml::string &cdata);
   virtual bool have_tag_end(const cppxml::string &name);

   virtual bool have_pi_start(const cppxml::string &name);
   virtual bool have_pi_end(const cppxml::string &name);

protected:
   xmlnode *curnode;
   xmlnode *rootnode;

   std::vector<xmlnode*> nodestack;
};


//! error enum values
enum xmlparse_error
{
   xml_ok=0,
   xml_nonode=1,
};


//! parses the xml document
class xmlparser
{
public:
   //! ctor
   xmlparser(xmleventhandler &eventhandler,std::istream &istr)
      :eventhandler(eventhandler),lexer(istr){}

   //! starts parsing
   bool parse(bool fulldoc=true);

protected:
   //! parses header, such as processing instructions or doctype tags
   bool parse_header();

   //! parses a single node; calls itself recursively
   bool parse_node();

   //! parses tag attributes
   bool parse_attributes();

   //! parses a <!-- --> comment
   bool parse_comment();

protected:
   xmleventhandler &eventhandler;
   xmllexer lexer;
   xmlparse_error err;
};

//@}

// end of namespace
};
