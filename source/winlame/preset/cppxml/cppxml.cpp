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
/*! \file cppxml.cpp

   \brief node methods implementation

*/

// needed includes
#include "cppxml.hpp"
#include "parser.hpp"
#include <fstream>


// global functions

cppxml::xmlnode_ptr cppxml::get_nodelist_item(cppxml::xmlnodelist &nodelist,unsigned int index)
{
   cppxml::xmlnode_ptr node(NULL);

   cppxml::xmlnodelist::iterator iter = nodelist.begin();
   cppxml::xmlnodelist::iterator stop = nodelist.end();
   for(unsigned int i=0; i<index && iter!=stop; i++) iter++;

   if (iter!=nodelist.end())
      node = (*iter);

   return node;
}


// xmlnode methods

cppxml::xmlnode::xmlnode()
{
   type = nt_node;
}

cppxml::xmlnode::xmlnode(xmlnodetype mytype, cppxml::string myname,
   cppxml::string myvalue)
:type(mytype),name(myname),value(myvalue)
{
}

cppxml::string cppxml::xmlnode::get_attribute_value(const char *attrname)
{
   // try to find the key in the map
   cppxml::string key(attrname);
   xmlattrmap::const_iterator iter = attributes.find(key);

   cppxml::string empty("");
   return iter == attributes.end() ? empty : iter->second;
}

cppxml::xmlnodelist_ptr cppxml::xmlnode::select_nodes(const char *nodedesc)
{
   xmlnodelist_ptr nlp(new xmlnodelist);

   // retrieve nodes
   get_nodes(*nlp.get(),nodedesc,false);
   return nlp;
}

cppxml::xmlnode_ptr cppxml::xmlnode::select_single_node(const char *nodedesc)
{
   xmlnode_ptr node(NULL);

   // retrieve nodes
   xmlnodelist nodelist;
   get_nodes(nodelist,nodedesc,true);

   // only take first node found
   if (nodelist.size()>0)
      node = *(nodelist.begin());

   return node;
}

/// removes space chars from string
void cppxml_internal_trim_strings(cppxml::string &str)
{
   unsigned int pos;

   // trim front
   for(pos=0;pos<str.size() && str.at(pos)==' ';pos++);
   if (pos>0) str.erase(0,pos);

   // trim back
   for(pos=str.size()-1;pos>0 && str.at(pos)==' ';pos--);
   str.erase(pos+1);
}

void cppxml::xmlnode::get_nodes(
   cppxml::xmlnodelist &nodelist,const char *nodedesc,bool onlyone)
{
   // check if we should return an attribute node
   if (nodedesc[0]=='@')
   {
      // construct attr node from attr name/value
      cppxml::string value = get_attribute_value(nodedesc+1);
      cppxml::xmlnode_ptr attrnode(
         new xmlnode(nt_attr,cppxml::string(nodedesc+1),value));
      nodelist.push_back(attrnode);
      return;
   }

   // check if we should return child node(s)
   const char *pos=strchr(nodedesc,'/');
   if (pos!=NULL)
   {
      cppxml::string childnodename;
      childnodename.assign(nodedesc,pos-nodedesc);

      // get child nodes with specified name
      cppxml::xmlnodelist childnodes;
      get_nodes(childnodes,childnodename.c_str(),onlyone);

      cppxml::xmlnodelist::iterator iter,stop;
      iter = childnodes.begin();
      stop = childnodes.end();

      // go through all collected child nodes and process them further
      for(;iter!=stop;iter++)
      {
         cppxml::xmlnode_ptr node(*iter);
         node->get_nodes(nodelist,pos+1,onlyone);
      }
      return;
   }

   // no child node, but maybe attribute restrictions, e.g. [@attr = "val"]

   pos=strchr(nodedesc,'[');
   if (pos!=NULL)
   {
      cppxml::string childnodename;
      childnodename.assign(nodedesc,pos-nodedesc);

      // get all child nodes that match name
      cppxml::xmlnodelist childnodes;
      get_nodes(childnodes,childnodename.c_str(),onlyone);

      // parse attribute restriction
      cppxml::string attrname,attrvalue;

      const char *pos2=strchr(pos,'@');
      const char *pos3=strchr(pos,'=');
      const char *pos4=strchr(pos,']');

      if (pos2==NULL || pos3==NULL || pos4==NULL ||
         pos2>pos3 || pos2>pos4 || pos3>pos4)
         return; // the string syntax isn't correct

      attrname.assign(pos2+1,pos3-pos2-1);
      attrvalue.assign(pos3+1,pos4-pos3-1);

      // trim strings
      cppxml_internal_trim_strings(attrname);
      cppxml_internal_trim_strings(attrvalue);

      // remove '"' from value string
      if (attrvalue.size()>2)
      {
         attrvalue.erase(attrvalue.size()-1);
         attrvalue.erase(0,1);
      }
      else
         return; // no attr value found

      // go through all childnodes and add them, when restriction matches
      cppxml::xmlnodelist::iterator iter,stop;
      iter = childnodes.begin();
      stop = childnodes.end();

      for(int count=0;iter!=stop;iter++)
      {
         cppxml::xmlnode_ptr child(*iter);

         if (child->get_attribute_value(attrname.c_str())==attrvalue)
         {
            nodelist.push_back(child);
            count++;
         }

         // stop when only one node is needed
         if (count>0 && onlyone)
            break;
      }
      return;
   }

   // we have plain child nodes to collect
   {
      cppxml::xmlnodelist::iterator iter,stop;
      iter = children.begin();
      stop = children.end();

      // go through all child nodes
      for(int count=0;iter!=stop;iter++)
      {
         cppxml::xmlnode_ptr child(*iter);

         // check if child node name match
         if (child->get_name() == nodedesc)
         {
            // add node
            nodelist.push_back(child);
            count++;
         }

         // stop when only one node is needed
         if (count>0 && onlyone)
            break;
      }
   }
}

bool cppxml::xmlnode::read(std::istream &istr)
{
   cppxml::xmlnodebuilder nodebuilder(this);
   cppxml::xmlparser parser(nodebuilder,istr);
   return parser.parse(false);
}

bool cppxml::xmlnode::write(std::ostream &ostr, unsigned int indent)
{
   // check if cdata node
   if (type==nt_cdata)
   {
      ostr << value.c_str();
      return true;
   }

   // indent
   for(unsigned int i=0; i<indent; i++) ostr << ' ';

   // write start tag
   ostr << "<" << name.c_str();

   // write attributes
   if (attributes.size()>0)
   {
      xmlattrmap::iterator iter,stop;
      iter = attributes.begin();
      stop = attributes.end();

      for(;iter!=stop; iter++)
      {
         xmlattrmap::value_type attr = *iter;

         ostr << " " << attr.first.c_str() << "=" << "\"" <<
            attr.second.c_str() << "\"";
      }
   }

   // no child nodes
   if (children.size()==0)
   {
      ostr << "/>" << std::endl;
      return true;
   }

   ostr << ">";

   // output a newline when first one is not a cdata node
   if (children.front()->get_type()!=nt_cdata)
      ostr << std::endl;

   // write all child nodes
   {
      xmlnodelist::iterator iter,stop;
      iter = children.begin();
      stop = children.end();

      // write all child nodes
      for(;iter!=stop; iter++)
      {
         xmlnode_ptr &nodeptr = *iter;
         nodeptr->write(ostr,indent+1);
      }
   }

   // indent, but only if last one was no cdata node
   if (children.back()->get_type()!=nt_cdata)
   {
      for(unsigned int j=0; j<indent; j++) ostr << ' ';
   }

   // write end node
   ostr << "</" << name.c_str() << ">" << std::endl;

   return true;
}


// xmldocument methods

bool cppxml::xmldocument::load(std::istream &istr)
{
   cppxml::xmlnodebuilder nodebuilder(this);
   return nodebuilder.parse(istr);
}

bool cppxml::xmldocument::save(std::ostream &ostr)
{
   if (children.size()!=1)
      return false;

   // get root node and write it
   xmlnode_ptr nodeptr = *children.begin();
   return nodeptr->write(ostr);
}


// xmleventhandler methods

bool cppxml::xmleventhandler::parse(std::istream &istr)
{
   // instanciate parser and parse the document
   cppxml::xmlparser parser(*this,istr);
   return parser.parse();
}
