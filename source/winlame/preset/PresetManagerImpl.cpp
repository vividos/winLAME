/*
   winLAME - a frontend for the LAME encoding engine
   Copyright (c) 2000-2004 Michael Fink

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
/// \file PresetManagerImpl.cpp
/// \brief contains the preset manager implementation
/// \ingroup preset

// needed includes
#include "stdafx.h"
#include "PresetManagerImpl.h"
#include "PropertyDlg.h"
#include <fstream>

// debug helper
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


PresetManagerInterface *PresetManagerInterface::getPresetManager()
{
   return new PresetManagerImpl;
}

// PresetManagerInterface methods

PresetManagerImpl::PresetManagerImpl()
{
   xmlchanged=false;
}

bool PresetManagerImpl::loadPreset(LPCTSTR filename)
{
   // remember filename
   xmlfilename = filename;

   // load xml document
   USES_CONVERSION;
   std::ifstream istr(T2CA(filename));
   if (!istr.is_open())
      return false;

   bool ret = doc.load(istr);
   istr.close();
   return ret;
}

bool PresetManagerImpl::mergePreset(LPCTSTR filename)
{
   // not implemented

   return true;
}

void PresetManagerImpl::savePreset()
{
   if (xmlchanged)
   {
      // save preset xml file
      USES_CONVERSION;
      std::ofstream ostr(T2CA(xmlfilename.c_str()));
      doc.save(ostr);
      ostr.close();

      xmlchanged = false;
   }
}

void PresetManagerImpl::setFacility(LPCTSTR facname)
{
   USES_CONVERSION;
   facility = facname;

   std::tstring nodepath(_T("presets/facility[@name = \""));
   nodepath += facname;
   nodepath += _T("\"]");

   cppxml::xmlnode_ptr facnode = doc.select_single_node(T2CA(nodepath.c_str()));
   if (facnode.get()==NULL)
   {
      // at least, create empty list
      presets_list = cppxml::xmlnodelist_ptr(new cppxml::xmlnodelist);
   }
   else
      presets_list = facnode->select_nodes("preset");
}

int PresetManagerImpl::getPresetCount()
{
   // return length of presets list
   return presets_list->size();
}

std::tstring PresetManagerImpl::getPresetName(int index)
{
   std::tstring ret(_T("- name not found -"));

   // search for preset node
   cppxml::xmlnode_ptr node(cppxml::get_nodelist_item(*presets_list,index));
   if (node.get()==NULL) return ret;

   // get name
   USES_CONVERSION;
   ret = A2CT(node->get_attribute_value("name").c_str());

   return ret;
}

std::tstring PresetManagerImpl::getPresetDescription(int index)
{
   std::tstring ret(_T("- no description available -"));

   // search for preset node
   cppxml::xmlnode_ptr node(cppxml::get_nodelist_item(*presets_list,index));
   if (node.get()==NULL) return ret;

   // get comment node
   cppxml::xmlnode_ptr comment = node->select_single_node("comment/cdata");
   if (comment.get()==NULL) return ret;

   USES_CONVERSION;
   ret = A2CT(comment->get_value().c_str());

   // trim description text
   bool first = true;
   std::tstring::size_type pos, len=ret.length();
   for(pos=0;pos<len;pos++)
   {
      // trim leading whitespaces
      int dist=0;
      while(pos<len && (ret.at(pos)==' ' || ret.at(pos)=='\t' ||
            (first && ret.at(pos)=='\n') ))
         dist++,pos++;

      first = false;

      if (dist!=0)
      {
         pos-=dist;
         ret.erase(pos,dist);
         len-=dist;
      }

      if (pos>=len) break;

      // search for \n
      while(pos<len && ret.at(pos)!='\n')
         pos++;

      if (pos>=len) break;

      // search back for spaces
      dist=0;
      while(pos<len && (ret.at(pos-dist-1)==' ' || ret.at(pos)=='\t'))
         dist++;

      if (pos>=len) break;

      if (dist>0)
      {
         pos-=dist;
         ret.erase(pos,dist);
         len-=dist;
      }

      // insert \r before \n
      ret.insert((len++,pos++),1,'\r');
   }

   // trim last \r\n
   if (ret.size()>0 && 0==_tcscmp(ret.c_str()+(ret.size()-2),_T("\r\n")))
      ret.erase(ret.size()-2);

   return ret;
}

void PresetManagerImpl::setSettings(int index, SettingsManager &settings_mgr)
{
   // search for preset node
   cppxml::xmlnode_ptr node(cppxml::get_nodelist_item(*presets_list,index));
   if (node.get()==NULL) return;

   // get all "values" nodes
   cppxml::xmlnodelist_ptr values = node->select_nodes("value");

   // go through all nodes and set them in the settings manager
   {
      cppxml::xmlnodelist::iterator iter2, stop;
      iter2 = values->begin();
      stop = values->end();

      for(;iter2!=stop; iter2++)
      {
         cppxml::xmlnode_ptr node(*iter2);

         // get attribute value and ID
         cppxml::string name = node->get_attribute_value("name");

         USES_CONVERSION;
         int valueID = mgr_variables.lookupID(A2CT(name.c_str()));
         if (valueID == -1) continue; // not found

         // get cdata: variable's value
         cppxml::xmlnode_ptr valuenode = node->get_childnodes().front();
         if (valuenode.get()==NULL) continue; // not found

         cppxml::string value = valuenode->get_value();

         // set name ID and value
         settings_mgr.setValue(valueID,atoi(value.c_str()));
      }
   }
}

void PresetManagerImpl::setDefaultSettings(SettingsManager &settings_mgr)
{
   for(int i=VarFirst; i<VarLast; i++)
   {
      int def = mgr_variables.lookupDefaultValue(i);
      settings_mgr.setValue(i,def);
   }
}

void PresetManagerImpl::editSettingsDialog(int index)
{
   // search for preset node
   cppxml::xmlnode_ptr node(cppxml::get_nodelist_item(*presets_list,index));
   if (node.get()==NULL) return;

   editing_preset = node;

   // show edit dialog
   PropertyDlg dlg;
   dlg.SetPropertyManagerInterface(this);
   dlg.DoModal();
}

// interface implementation for the PropertyManagerInterface

int PresetManagerImpl::GetGroupCount()
{
   return 1;
}

std::tstring PresetManagerImpl::GetGroupName(int group)
{
   // look up description for facility
   std::tstring ret;
   int id = mgr_facility.lookupID(facility.c_str());
   if (id!=-1)
      ret = mgr_facility.lookupDescription(id);
   return ret;
}

int PresetManagerImpl::GetItemCount(int group)
{
   return editing_preset->select_nodes("value")->size();
}

std::tstring PresetManagerImpl::GetItemName(int group, int index)
{
   std::tstring ret(_T("- no item name -"));

   // search node
   cppxml::xmlnode_ptr node(editLookupNode(group,index));
   if (node.get()==NULL) return ret; // not found

   // get name of node
   USES_CONVERSION;
   int id = mgr_variables.lookupID( A2CT(node->get_attribute_value("name").c_str()) );
   if (id!=-1)
      ret = mgr_variables.lookupDescription(id);

   return ret;
}

std::tstring PresetManagerImpl::GetItemValue(int group, int index)
{
   std::tstring ret(_T("- no item value -"));

   // search node
   cppxml::xmlnode_ptr node(editLookupNode(group,index));
   if (node.get()==NULL) return ret; // not found

   // get cdata of fist child node
   cppxml::xmlnodelist::iterator iter = node->get_childnodes().begin();
   USES_CONVERSION;
   ret = A2CT((*iter)->get_value().c_str());

   return ret;
}

void PresetManagerImpl::SetItemValue(int group, int index, std::tstring val)
{
   // search node
   cppxml::xmlnode_ptr node(editLookupNode(group,index));
   if (node.get()==NULL) return; // not found

   // set cdata of fist child node
   cppxml::xmlnodelist::iterator iter = node->get_childnodes().begin();

   USES_CONVERSION;
   (*iter)->set_value(T2CA(val.c_str()));
}

cppxml::xmlnode_ptr PresetManagerImpl::editLookupNode(int group, int index)
{
   cppxml::xmlnode_ptr node(NULL);

   // get list of "value" nodes
   cppxml::xmlnodelist_ptr nodelist = editing_preset->select_nodes("value");

   if (nodelist->size()>0)
   {
      // search for node
      cppxml::xmlnodelist::iterator iter = nodelist->begin();
      for(int i=0; i<index; i++) iter++;

      // search for node
      cppxml::xmlnode_ptr node(cppxml::get_nodelist_item(*nodelist,index));
      return node;
   }

   return node;
}
