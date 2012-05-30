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
/// \file parser.cpp
/// \brief parser implementation

// needed includes
#include "parser.hpp"
#include <algorithm>


// xmlnodebuilder methods

cppxml::xmlnodebuilder::xmlnodebuilder(xmlnode *node)
:curnode(node),rootnode(node)
{
}

bool cppxml::xmlnodebuilder::have_tag_start(const cppxml::string &name)
{
   // create new node
   xmlnode_ptr node(new xmlnode);

   // add node to current node
   xmlnodelist &children = curnode->get_childnodes();
   children.push_back(node);

   // remember curnode on nodestack
   nodestack.push_back(curnode);

   // new current node
   curnode = node.get();

   // set node name
   curnode->set_name(name);

   return true;
}

bool cppxml::xmlnodebuilder::have_attribute(const cppxml::string &name,
   const cppxml::string &value)
{
   // insert attribute pair into attributes
   xmlattrmap &attrlist = curnode->get_attribute_list();
   attrlist.insert(
      std::make_pair<cppxml::string, cppxml::string>(name,value));

   return true;
}

bool cppxml::xmlnodebuilder::have_cdata(const cppxml::string &cdata)
{
   // create new cdata node
   xmlnode_ptr node(new xmlnode);

   node->set_type(nt_cdata);
   node->set_name("cdata");
   node->set_value(cdata);

   // insert cdata node
   xmlnodelist &children = curnode->get_childnodes();
   children.push_back(node);

   return true;
}

bool cppxml::xmlnodebuilder::have_tag_end(const cppxml::string &name)
{
   // restore last curnode
   if (nodestack.size()>0)
   {
      curnode = nodestack.back();
      nodestack.pop_back();
   }
   return true;
}

bool cppxml::xmlnodebuilder::have_pi_start(const cppxml::string &name)
{
   // remember curnode on nodestack
   nodestack.push_back(curnode);

   // set root node as current node
   curnode = rootnode;

   return true;
}

bool cppxml::xmlnodebuilder::have_pi_end(const cppxml::string &name)
{
   // restore back last node
   curnode = nodestack.back();
   nodestack.pop_back();

   return true;
}


// xmlparser methods

bool cppxml::xmlparser::parse(bool fulldoc)
{
   bool ret = true;

   err = xml_ok;

   ret &= fulldoc ? parse_header() : true;
   ret &= parse_node();
   return ret;
}

bool cppxml::xmlparser::parse_header()
{
   cppxml::string token,token2;

   while(true)
   {
      token = lexer.get_next_token();

      if (token != "<")
         return false; // open '<' expected

      // token after opening < is a literal?
      token2 = lexer.get_next_token();
      if (!lexer.is_literal(token2))
      {
         // generic string encountered: assume no pi and doctype tags
         lexer.put_back(token2);
         lexer.put_back(token);
         return true;
      }

      // now check for the literal
      switch(token2.at(0))
      {
      case '!':
         {
            // parse comment or doctype tag
            token = lexer.get_next_token();

            if (!lexer.is_literal(token))
            {
               // now a doctype tag or a comment may follow
               if (strncmp(token.c_str(),"--",2)==0)
                  parse_comment();
               else
               {
//                  cppxml::string doctypestr(token3.get_generic());

                  // make uppercase
                  std::transform(token.begin(),token.end(),token.begin(),toupper);

                  if (token == "DOCTYPE")
                  {
                     // TODO parse doctype tag

                     // read the complete tag until the closing >
                     while (token != ">")
                        token = lexer.get_next_token();
                  }
                  else
                     return false; // unknown stuff encountered
               }
            }
            else
               return false; // pi or doctype encountered

            break;
         }
      case '?':
         {
            // parse processing instruction
            token = lexer.get_next_token();

            if (lexer.is_literal(token))
               return false; // pi or doctype expected

            token.insert(0,"?");
            eventhandler.have_pi_start(token);

            if (!parse_attributes())
               return false;

            token = lexer.get_next_token();
            if (token != "?")
               return false; // pi or doctype expected

            token = lexer.get_next_token();
            if (token != ">")
               return false; // closing '>' expected

            eventhandler.have_pi_end(token);
            break;
         }
      default:
         // unknown literal encountered
         return false; // pi or doctype expected
;
      } // end switch

   } // end while

   return true;
}

bool cppxml::xmlparser::parse_node()
{
   cppxml::string token,token2,tagname;

   token = lexer.get_next_token();

/* if (token.size()==1 && token.at(0)==EOF)
      return false; // end of stream
*/

   // loop when we encounter a comment
   bool again;
   do
   {
      again = false;

      // check if we have a literal
      if (!lexer.is_literal(token))
      {
         cppxml::string cdata;

         // parse cdata section(s) and return
         do
         {
            cdata += token;
            token = lexer.get_next_token();

         } while(!lexer.is_literal(token));

         lexer.put_back(token);

         // handle cdata event
         eventhandler.have_cdata(cdata);
         return true;
      }

      // no cdata, try to continue parsing node content

      if (token != "<")
         return false; // expected cdata or open tag

      // get node name
      token2 = lexer.get_next_token();

      if (lexer.is_literal(token2))
      {
         // check the following literal
         switch(token2.at(0))
         {
            // closing '</...>' follows
         case '/':
            // return, we have a closing node with no more content
            lexer.put_back(token2);
            lexer.put_back(token);
            err = xml_nonode;
            return true;

            // comment follows
         case '!':
            token = lexer.get_next_token();

            if (token=="[CDATA[")
            {
               // parse cdata section
               cppxml::string cdata;

               token = lexer.get_next_token();
               do
               {
                  cdata += token;
                  token = lexer.get_next_token();

               } while(token!="]]");

               token = lexer.get_next_token();
            }
            else
               parse_comment();

            // get next token
            token = lexer.get_next_token();

            // parse again, until we encounter some useful data
            again = true;
            break;

         default:
            return false; // tagname was expected
         }
      }
      tagname = token2;

   } while (again);

   // at this point, we're sure we have an open tag

   // notify event handler
   eventhandler.have_tag_start(token2);

   // parse attributes
   if (!parse_attributes())
      return false;

   // check for leaf
   token = lexer.get_next_token();

   if (token == "/")
   {
      // node has finished
      token = lexer.get_next_token();
      if (token != ">")
         return false; // closing '>' expected

      // notify handler
      eventhandler.have_tag_end(tagname);
      return true;
   }

   // now a closing bracket must follow
   if (token != ">")
      return false; // closing '>' expected

   // loop to parse all subnodes
   while (true)
   {
      // try to parse possible sub nodes
      if (!parse_node())
         return false;

      // check if no more subnodes were available
      if (err==xml_nonode)
      {
         err=xml_ok; // reset error
         break;
      }
   }

   // parse end tag
   token = lexer.get_next_token();
   token2 = lexer.get_next_token();

   if (token != "<" && token2 != "/")
      return false; // closing tag expected

   // check closing tagname
   token = lexer.get_next_token();

   // check if open and close tag names are identical
   if (tagname!=token)
      return false; // closing tagname mismatch

   token = lexer.get_next_token();
   if (token != ">")
      return false; // closing '>' expected

   // notify handler
   eventhandler.have_tag_end(tagname);

   return true;
}

bool cppxml::xmlparser::parse_attributes()
{
   cppxml::string token;

   while(true)
   {
      // read attribute name
      token = lexer.get_next_token();

      if (lexer.is_literal(token))
      {
         // no attributes
         lexer.put_back(token);
         break;
      }
      cppxml::string name(token);

      // check for equal sign
      token = lexer.get_next_token();
      if (token != "=")
         return false; // equal expected

      // read attribute value
      token = lexer.get_next_token();
      if (lexer.is_literal(token))
         return false; // attribute value expected

      // remove "" or '' from attribute value
      cppxml::string value(token);
      value.erase(0,1);
      value.erase(value.length()-1,1);

      // call event handler
      eventhandler.have_attribute(name,value);
   }
   return true;
}

bool cppxml::xmlparser::parse_comment()
{
   cppxml::string token;

   // get token until comment is over
   while (true)
   {
      token = lexer.get_next_token();

      if (token.size()>=2 &&
         strcmp(token.c_str()+token.size()-2,"--")==0)
      {
         token = lexer.get_next_token();
         if (token==">")
            break;
      }
   }
   return true;
}
