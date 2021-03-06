/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2013 SSAB EMEA AB.
 *
 * This file is part of Proview.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 **/

/* ge_attr.cpp -- Display object attributes */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "co_cdh.h"
#include "co_time.h"

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"

#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"
#include "flow_msg.h"

#include "ge_attr.h"
#include "ge_attrnav.h"
#include "ge_dyn.h"
#include "ge_msg.h"

Attr::~Attr()
{
}

Attr::Attr(
  void			*a_parent_ctx,
  void 			*a_object,
  attr_sItem  		*itemlist,
  int			item_cnt ) :
  parent_ctx(a_parent_ctx), input_open(0), object(a_object), 
  close_cb(0), redraw_cb(0), get_subgraph_info_cb(0), get_dyn_info_cb(0),
  reconfigure_attr_cb(0),
  store_cb(0), recall_cb(0), set_data_cb(0), client_data(0), recall_idx(-1),
  original_data(0)
{
}

void Attr::store()
{
  if ( store_cb)
    (store_cb)( parent_ctx, object);
}

void Attr::recall_next()
{
  int sts;
  GeDyn *old_data;
  int idx;

  if ( recall_idx == -1)
    idx = 0;
  else
    idx = recall_idx + 1;

  if ( recall_cb)
    sts = (recall_cb)( parent_ctx, object, 
				     idx, &old_data);
  if ( ODD(sts))
  {
    if ( recall_idx == -1 && !original_data) {
      original_data = old_data;
      recall_idx = 0;
    }
    recall_idx = idx;
    reconfigure_attr_c((void *)this);
  }
}

void Attr::recall_prev()
{
  int sts;
  GeDyn *old_p;

  if ( recall_idx < 0)
    return;
  recall_idx--;
  if ( recall_idx == -1) {
    // Get original data
    if ( set_data_cb) {
      (set_data_cb)( parent_ctx, object, 
			       original_data);
      Attr::reconfigure_attr_c((void *)this);
      original_data = 0;
    }
  }
  else {
    if ( recall_cb) {
      sts = (recall_cb)( parent_ctx, object, 
				  recall_idx, &old_p);
      if ( ODD(sts)) {
        Attr::reconfigure_attr_c((void *)this);
	delete old_p;
      }
      else
        recall_idx++;
    }
  }
}

int Attr::get_plant_select_c( void *attr_ctx, char *value, int size)
{
  Attr *attr = (Attr *) attr_ctx;
  if ( attr->get_plant_select_cb)
    return attr->get_plant_select_cb( attr->parent_ctx, value, size);  
  return 0;
}

int Attr::get_current_colors_c( void *attr_ctx, glow_eDrawType *fill_color, 
				       glow_eDrawType *border_color, glow_eDrawType *text_color)
{
  Attr *attr = (Attr *) attr_ctx;
  if ( attr->get_current_colors_cb)
    return attr->get_current_colors_cb( attr->parent_ctx, fill_color, border_color, text_color);  
  return 0;
}

int Attr::get_current_color_tone_c( void *attr_ctx, glow_eDrawType *color_tone)
{
  Attr *attr = (Attr *) attr_ctx;
  if ( attr->get_current_color_tone_cb)
    return attr->get_current_color_tone_cb( attr->parent_ctx, color_tone);
  return 0;
}

int Attr::get_subgraph_info_c( void *attr_ctx, char *name, 
	attr_sItem **itemlist, int *item_cnt)
{
  Attr *attr = (Attr *) attr_ctx;
  if ( attr->get_subgraph_info_cb)
    return attr->get_subgraph_info_cb( attr->parent_ctx, name, itemlist, 
		item_cnt);  
  return 0;
}

int Attr::get_dyn_info_c( void *attr_ctx, GeDyn *dyn, 
	attr_sItem **itemlist, int *item_cnt)
{
  Attr *attr = (Attr *) attr_ctx;
  if ( attr->get_dyn_info_cb)
    return attr->get_dyn_info_cb( attr->parent_ctx, dyn, itemlist, 
		item_cnt);  
  return 0;
}

void Attr::change_value_c( void *attr)
{
  ((Attr *)attr)->change_value();
}

int Attr::reconfigure_attr_c( void *attr)
{
  return ((Attr *)attr)->reconfigure_attr();
}

void Attr::message( void *attr, char severity, const char *message)
{
  ((Attr *)attr)->message( severity, message);
}


