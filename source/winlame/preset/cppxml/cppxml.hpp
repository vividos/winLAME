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
/*! \file cppxml.hpp

   contains declarations for various types and classes that can be used to
   access cppxml's functionality. there are two methods to process an xml
   file:

   - load the xml document via xmldocument::load() and access nodes via the
     xmlnode class

   - derive from xmleventhandler, implement wanted virtual handler methods and
     call xmleventhandler::parse()

   \brief cppxml application program interface

*/
/*! \defgroup cppxml cppxml xml parser

   contains all classes and functions for the cppxml user

*/
/*! \ingroup cppxml */
/*! @{ */

// include guard
#pragma once

// switch off some warnings
#pragma warning( disable: 4786 )

// needed includes
#include <string>
#include <list>
#include <map>
#include <iostream>

// boost smart pointer stuff
#define BOOST_NO_MEMBER_TEMPLATES
#define BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#include "smart_ptr.hpp"   // the boost::shared_ptr class


//! cppxml namespace
namespace cppxml
{

//! string type
typedef std::string string;

//! xml tree node smartpointer
typedef boost::shared_ptr<class xmlnode> xmlnode_ptr;

//! list of node smart pointer
typedef std::list<xmlnode_ptr> xmlnodelist;

//! xml tree node list smartpointer
typedef boost::shared_ptr<xmlnodelist> xmlnodelist_ptr;

//! attribute map
typedef std::map<cppxml::string, cppxml::string> xmlattrmap;


//! function to retrieve a specific node in a xmlnodelist
xmlnode_ptr get_nodelist_item(xmlnodelist &nodelist,unsigned int index);


//! various types of nodes
typedef enum
{
   //! normal node
   nt_node,
   //! node is an attribute
   nt_attr,
   //! node is a cdata section
   nt_cdata,
} xmlnodetype;


//! xml tree node
class xmlnode
{
public:
   //! ctor
   xmlnode();

   //! ctor
   xmlnode(xmlnodetype type, cppxml::string name, cppxml::string value);

   // node info access

   //! returns type of node
   xmlnodetype get_type(){ return type; }

   //! returns name of node
   cppxml::string get_name(){ return name; }

   //! returns value of node
   cppxml::string get_value(){ return value; }

   //! sets node type
   void set_type(xmlnodetype t){ type=t; }

   //! sets node name
   void set_name(cppxml::string str){ name=str; }

   //! sets node value
   void set_value(cppxml::string str){ value=str; }

   // attribute access

   //! returns attribute list
   xmlattrmap &get_attribute_list(){ return attributes; }

   //! returns a specific attribute
   cppxml::string get_attribute_value(const char *attrname);

   // child node access

   //! returns a specific child node
   xmlnode_ptr get_child_node(unsigned int index);

   //! returns the list of all child nodes
   xmlnodelist &get_childnodes(){ return children; }

   //! select nodes with a certain name and returns it in a list
   xmlnodelist_ptr select_nodes(const char *nodedesc);

   //! returns first node matching the name
   xmlnode_ptr select_single_node(const char *nodedesc);

   // misc

   //! reads xml node from input stream
   bool read(std::istream &istr);

   //! writes node to output stream
   bool write(std::ostream &ostr, unsigned int indent=0);

protected:
   //! internal function to retrieve subnodes
   void get_nodes(cppxml::xmlnodelist &nodelist,
      const char *nodedesc,bool onlyone);

protected:
   xmlnodetype type;
   cppxml::string name;
   cppxml::string value;

   xmlnodelist children;
   xmlattrmap attributes;
};


//! xml document class
class xmldocument: public xmlnode
{
public:
   //! ctor
   xmldocument(){}

   //! loads an xml document from the stream
   bool load(std::istream &istr);

   //! saves an xml document to the stream
   bool save(std::ostream &ostr);
};


//! event handler
class xmleventhandler
{
public:
   //! ctor
   xmleventhandler(){}

   //! starts parsing using the event handler
   bool parse(std::istream &istr);

   // handler methods

   //! handler for tag start
   virtual bool have_tag_start(const cppxml::string &name){ return true; }

   //! handler for tag attribute
   virtual bool have_attribute(const cppxml::string &name,
      const cppxml::string &value ){ return true; }

   //! handler for cdata
   virtual bool have_cdata(const cppxml::string &cdata){ return true; }

   //! handler for tag end
   virtual bool have_tag_end(const cppxml::string &name){ return true; }

   //! handler for parsing instruction start
   virtual bool have_pi_start(const cppxml::string &name){ return true; }

   //! handler for parsing instruction end
   virtual bool have_pi_end(const cppxml::string &name){ return true; }
};

// end of namespace
};

//@}
