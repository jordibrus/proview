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
 */

typedef void *Widget;

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "glow.h"

#include "xtt_xnav.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "cow_wow_gtk.h"

#include "glow_growctx.h"
#include "glow_growapi.h"
#include "co_lng.h"
#include "xtt_ge_gtk.h"
#include "xtt_trend_gtk.h"
#include "xtt_sevhist_gtk.h"
#include "ge_graph_gtk.h"
#include "xtt_ev_gtk.h"
#include "xtt_evala_gtk.h"
#include "xtt_eveve_gtk.h"
#include "xtt_multiview_gtk.h"
#include "xtt_log.h"
#include "pwr_baseclasses.h"
#include "rt_xnav_msg.h"
#include "glow_msg.h"

gboolean XttMultiViewGtk::action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  XttMultiViewGtk *multiview = (XttMultiViewGtk *)data;

  if ( multiview->focustimer.disabled())
    return TRUE;

  //if ( multiview->graph)
  //  multiview->graph->set_inputfocus(1);

  multiview->focustimer.disable( 400);
  return FALSE;
}

void XttMultiViewGtk::set_size( int width, int height)
{
  int		default_width;
  int		default_height;
  GdkGeometry   geometry;
  XNav		*xnav = get_xnav();

  default_width = width + 20;
  default_height = height + 20;

  gtk_window_resize( GTK_WINDOW(toplevel), default_width, default_height);

  // This condition is due to a bug in Reflection X 11.0.5...
  if ( !xnav->gbl.no_graph_ratio) {
    // Note, equal min and max aspect will cause recursive resize on LXDE
    geometry.min_aspect = gdouble(default_width)/default_height;
    geometry.max_aspect = gdouble(default_width)/default_height * 1.02;
    gtk_window_set_geometry_hints( GTK_WINDOW(toplevel), GTK_WIDGET(toplevel),
    				   &geometry, GDK_HINT_ASPECT);
  }
}

void XttMultiViewGtk::activate_exit( GtkWidget *w, gpointer data)
{
  XttMultiViewGtk *multiview = (XttMultiViewGtk *)data;
  
  delete multiview;
}


void XttMultiViewGtk::action_resize( GtkWidget *w, GtkAllocation *allocation, gpointer data)
{
  // XttMultiView *multiview = (XttMultiView *)data;
}

XttMultiViewGtk::~XttMultiViewGtk()
{
  if ( close_cb)
    (close_cb)( parent_ctx, this);

  for ( unsigned int i = 0; i < MV_SIZE; i++) {
    if ( sala[i])
      delete sala[i];
  }
  for ( unsigned int i = 0; i < MV_SIZE; i++) {
    if ( seve[i])
      delete seve[i];
  }
  for ( unsigned int i = 0; i < MV_SIZE; i++) {
    if ( gectx[i])
      delete gectx[i];
  }
  for ( unsigned int i = 0; i < MV_SIZE; i++) {
    if ( mvctx[i])
      delete mvctx[i];
  }
  for ( unsigned int i = 0; i < MV_SIZE; i++) {
    if ( trend[i])
      delete trend[i];
  }
	  
  for ( unsigned int i = 0; i < MV_SIZE; i++) {
    if ( sevhist[i])
      delete sevhist[i];
  }
	  
  // delete widget;
  if ( !(options & ge_mOptions_Embedded))
    gtk_widget_destroy( toplevel);
}

void XttMultiViewGtk::pop()
{
  gtk_window_present( GTK_WINDOW(toplevel));
}

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer ge)
{
  XttMultiViewGtk::activate_exit(w, ge);

  return TRUE;
}

static void destroy_event( GtkWidget *w, gpointer data)
{
}

XttMultiViewGtk::XttMultiViewGtk( GtkWidget *mv_parent_wid, void *mv_parent_ctx, const char *mv_name, 
				  pwr_tAttrRef *mv_aref, int mv_width, int mv_height, 
				  int mv_x, int mv_y, unsigned int mv_options, pwr_tStatus *sts,
				  int (*mv_command_cb) (void *, char *, void *),
				  int (*mv_get_current_objects_cb) (void *, pwr_sAttrRef **, int **),
				  int (*mv_is_authorized_cb) (void *, unsigned int)) :
  XttMultiView( mv_parent_ctx, mv_name, mv_aref, mv_width,
		mv_height, mv_x, mv_y, mv_options, 
		mv_command_cb, mv_get_current_objects_cb, mv_is_authorized_cb), 
  parent_wid(mv_parent_wid)
{
  int	window_width = 600;
  int   window_height = 500;
  GdkGeometry   geometry;
  pwr_tStatus lsts;
  XNav 	*xnav = get_xnav();
  pwr_sClass_XttMultiView mv;

  memset( gectx, 0, sizeof(gectx));
  memset( mvctx, 0, sizeof(mvctx));
  memset( sala, 0, sizeof(sala));
  memset( seve, 0, sizeof(seve));
  memset( trend, 0, sizeof(trend));
  memset( sevhist, 0, sizeof(sevhist));
  memset( comp_widget, 0, sizeof(comp_widget));
  memset( exchange_widget, 0, sizeof(exchange_widget));

  if ( mv_width != 0 && mv_height != 0) {
    window_width = mv_width;
    window_height = mv_height;
  }
  else {
    window_width = 600;
    window_height = 500;
  }

  *sts = gdh_GetObjectInfoAttrref( &aref, &mv, sizeof(mv));
  if ( EVEN(*sts)) return;

  char *titleutf8 = g_convert( mv.Title, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);

  // Gtk
  if ( !(options & ge_mOptions_Embedded)) {
    toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
					 "default-height", window_height,
					 "default-width", window_width,
					 "title", titleutf8,
					 NULL);
    g_free( titleutf8);

    geometry.min_aspect = gdouble(window_width)/window_height;
    geometry.max_aspect = gdouble(window_width)/window_height * 1.02;
    gtk_window_set_geometry_hints( GTK_WINDOW(toplevel), GTK_WIDGET(toplevel),
  				 &geometry, GDK_HINT_ASPECT);

    g_signal_connect( toplevel, "delete_event", G_CALLBACK(delete_event), this);
    g_signal_connect( toplevel, "destroy", G_CALLBACK(destroy_event), this);
    g_signal_connect( toplevel, "focus-in-event", G_CALLBACK(action_inputfocus), this);

    CoWowGtk::SetWindowIcon( toplevel);
  }
  else {
    toplevel = parent_wid;
    box_widget = gtk_hbox_new( FALSE, 0);
  }
    
  if ( mv.Layout == pwr_eMultiViewLayoutEnum_Box) {
    GtkWidget *col_widget = gtk_hbox_new( FALSE, 0);

    rows = mv.Rows;
    cols = mv.Columns;

    bool escape = false;
    for ( int i = 0; i < cols; i++) {
      GtkWidget *row_widget = gtk_vbox_new( FALSE, 0);

      for ( int j = 0; j < rows; j++) {
	pwr_tFileName graph_name;
	int w, h, scrollbar, menu, type;

	if ( i*rows+j >= MV_SIZE) {
	  escape = true;
	  break;
	}

	w = mv.Action[i*rows+j].Width;
	h = mv.Action[i*rows+j].Height;
	scrollbar = (mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Scrollbars) ? 1 : 0;
	menu = (mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Menu) ? 1 : 0;
	strcpy( graph_name, mv.Action[i*rows+j].Action);
	type = mv.Action[i*rows+j].Type;

	switch ( type) {
	case pwr_eMultiViewContentEnum_AlarmList: {
	  if ( xnav->ev) {
	    sala[i*rows + j] = (EvAlaGtk *)xnav->ev->open_alarmlist_satellite( "No title", 
		    &lsts, w, h, 0, 0, mv.Action[i*rows+j].Object.Objid, ev_mAlaOptions_Embedded, toplevel);
	    if ( !sala[i*rows + j])
	      continue;
	    comp_widget[i*rows + j] = sala[i*rows + j]->get_widget();
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	  }
	  break;
	}
	case pwr_eMultiViewContentEnum_EventList: {
	  if ( xnav->ev) {
	    seve[i*rows + j] = (EvEveGtk *)xnav->ev->open_eventlist_satellite( "No title", 
		    &lsts, w, h, 0, 0, mv.Action[i*rows+j].Object.Objid, ev_mAlaOptions_Embedded, toplevel);
	    if ( !seve[i*rows + j])
	      continue;
	    comp_widget[i*rows + j] = seve[i*rows + j]->get_widget();
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	  }
	  break;
	}
	case pwr_eMultiViewContentEnum_Graph:
	case pwr_eMultiViewContentEnum_ObjectGraph: {
	  char *objectname_p = 0;
	  pwr_tAName objectname;
	  char *s;

	  if ( !cdh_ObjidIsNull(mv.Action[i*rows+j].Object.Objid)) {	    
	    lsts = gdh_AttrrefToName( &mv.Action[i*rows+j].Object, objectname, sizeof(objectname),
				     cdh_mName_volumeStrict);
	    if ( ODD(lsts))
	      objectname_p = objectname;
	    else
	      objectname_p = 0;
	  }

	  gectx[i*rows + j] = new XttGeGtk( toplevel, this, "No title", 
					    graph_name, scrollbar, menu, 0, w, h, mv_x, mv_y, 
					    1.0, objectname_p, 0, 0, 
					    ge_mOptions_Embedded, 0,
					    multiview_ge_command_cb, multiview_ge_get_current_objects_cb,
					    multiview_ge_is_authorized_cb);

	  gectx[i*rows + j]->close_cb = multiview_ge_close_cb;
	  gectx[i*rows + j]->help_cb = multiview_ge_help_cb;
	  gectx[i*rows + j]->display_in_xnav_cb = multiview_ge_display_in_xnav_cb;
	  gectx[i*rows + j]->popup_menu_cb = multiview_ge_popup_menu_cb;
	  gectx[i*rows + j]->call_method_cb = multiview_ge_call_method_cb;
	  gectx[i*rows + j]->sound_cb = multiview_ge_sound_cb;
	  gectx[i*rows + j]->eventlog_cb = multiview_ge_eventlog_cb;

	  comp_widget[i*rows + j] = gectx[i*rows + j]->get_graph_widget();

	  recall_buffer[i*rows + j].insert(graph_name, objectname_p);

	  if ( (s = strchr(graph_name, '.')))
	    *s = 0;	       
	  appl.insert( applist_eType_Graph, (void *)gectx[i*rows + j], pwr_cNObjid, graph_name,
		       objectname_p);
	

	  if ( mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Exchangeable) {
	    exchange_widget[i*rows+j] = gtk_hbox_new( FALSE, 0);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(exchange_widget[i*rows + j]), TRUE, TRUE, 0);
	  }
	  else
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	  break;
	}
	case pwr_eMultiViewContentEnum_MultiView: {
	  pwr_tAttrRef graph_aref;

	  lsts = gdh_NameToAttrref( pwr_cNObjid, graph_name, &graph_aref);
	  if ( EVEN(lsts)) break;

	  mvctx[i*rows + j] = new XttMultiViewGtk( toplevel, this, "No title", 
						   &graph_aref, w, h, mv_x, mv_y, 
						   ge_mOptions_Embedded, &lsts,
						   multiview_ge_command_cb, multiview_ge_get_current_objects_cb,
						   multiview_ge_is_authorized_cb);
	  
	  mvctx[i*rows + j]->close_cb = multiview_ge_close_cb;
	  mvctx[i*rows + j]->help_cb = multiview_ge_help_cb;
	  mvctx[i*rows + j]->display_in_xnav_cb = multiview_ge_display_in_xnav_cb;
	  mvctx[i*rows + j]->popup_menu_cb = multiview_ge_popup_menu_cb;
	  mvctx[i*rows + j]->call_method_cb = multiview_ge_call_method_cb;
	  mvctx[i*rows + j]->sound_cb = multiview_ge_sound_cb;
	  mvctx[i*rows + j]->eventlog_cb = multiview_ge_eventlog_cb;

	  comp_widget[i*rows + j] = (GtkWidget *)mvctx[i*rows + j]->get_widget();

	  recall_buffer[i*rows + j].insert(graph_name, 0);

	  appl.insert( applist_eType_MultiView, (void *)mvctx[i*rows + j], &aref, "", NULL);
	
	  if ( mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Exchangeable) {
	    exchange_widget[i*rows+j] = gtk_hbox_new( FALSE, 0);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(exchange_widget[i*rows + j]), TRUE, TRUE, 0);
	  }
	  else
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	  break;
	}
	case pwr_eMultiViewContentEnum_TrendCurve: {

	  int plotgroup_found = 0;
	  pwr_tAttrRef plotgroup;
	  pwr_tCid classid;
      	  GtkWidget *widget;
	  pwr_tAttrRef arefv[2];
	  int skip = 0;

	  lsts = gdh_GetAttrRefTid( &mv.Action[i*rows+j].Object, &classid);
	  if (EVEN(lsts)) break;

	  switch ( classid) {
	  case pwr_cClass_DsTrend:
	  case pwr_cClass_DsTrendCurve:
	    break;
	  case pwr_cClass_PlotGroup:
	    plotgroup_found = 1;
	    plotgroup = mv.Action[i*rows+j].Object;
	    arefv[0] = plotgroup;
	    break;
	  default:
	    skip = 1;
	  }

	  if ( skip)
	    break;

	  if ( plotgroup_found) {
	    trend[i*rows + j] = new XttTrendGtk( this, toplevel, (char *)"No title", &widget,
						 0, &plotgroup, w, h, (unsigned int)curve_mOptions_Embedded, sts);
	  }
	  else {
	    arefv[0] = mv.Action[i*rows+j].Object;
	    memset( &arefv[1], 0, sizeof(arefv[0]));
	    trend[i*rows + j] = new XttTrendGtk( this, toplevel, (char *)"No title", &widget,
						 arefv, 0, w, h, (unsigned int)curve_mOptions_Embedded, sts);
	  }
	  if ( EVEN(*sts)) break;

	  trend[i*rows + j]->close_cb = multiview_trend_close_cb;
	  trend[i*rows + j]->command_cb = multiview_trend_command_cb;
	  trend[i*rows + j]->help_cb = multiview_trend_help_cb;

	  comp_widget[i*rows + j] = widget;

	  // recall_buffer[i*rows + j].insert(graph_name, objectname_p);

	  appl.insert( applist_eType_Trend, (void *)trend[i*rows + j], &arefv[0], 
		       "",  NULL);
	

	  if ( mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Exchangeable) {
	    exchange_widget[i*rows+j] = gtk_hbox_new( FALSE, 0);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(exchange_widget[i*rows + j]), TRUE, TRUE, 0);
	  }
	  else
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	  break;
	}
	case pwr_eMultiViewContentEnum_SevHistory: {
	  pwr_tOid oidv[11];
	  pwr_tOName anamev[11];
	  pwr_tOName onamev[11];
	  bool sevhistobjectv[11];
	  pwr_tAttrRef attr_aref, sevhist_aref, histthread_aref;
	  pwr_tOid histthread_oid;
	  char server_node[40];
	  char *s;
	  pwr_tAName aname;
	  int plotgroup_found = 0;
	  int sevHistObjectFound = 0;
	  int oid_cnt = 0;
	  pwr_tCid classid;
	  int skip = 0;

	  if ( cdh_ObjidIsNull(mv.Action[i*rows+j].Object.Objid))
	    break;

	  GtkWidget *widget;
	  pwr_tAttrRef arefv[2];
	  pwr_tAttrRef plotgroup;
	  arefv[0] = mv.Action[i*rows+j].Object;
	  memset( &arefv[1], 0, sizeof(arefv[0]));


	  lsts = gdh_GetAttrRefTid( &arefv[0], &classid);
	  if (EVEN(lsts)) break;;

	  switch ( classid) {
	  case pwr_cClass_SevHist:
	    break;
	  case pwr_cClass_SevHistObject:
	    sevHistObjectFound = true;
	    break;
	  case pwr_cClass_PlotGroup:
	    plotgroup = mv.Action[i*rows+j].Object;
	    plotgroup_found = 1;
	    break;
	  default:
	    skip = 1;
	  }

	  if ( skip)
	    break;

	  if ( plotgroup_found) {
	    pwr_sClass_PlotGroup plot;
	    pwr_tCid cid;
	    int j;

	    lsts = gdh_GetObjectInfoAttrref( &plotgroup, &plot, sizeof(plot));
	    if ( EVEN(lsts)) break;
	
	    for ( j = 0; j < 20; j++) {
	      if ( cdh_ObjidIsNull( plot.YObjectName[j].Objid))
		break;
	  
	      sevhist_aref = plot.YObjectName[j];
	      lsts = gdh_GetAttrRefTid( &sevhist_aref, &cid);
	      if ( EVEN(lsts)) break;

	      if ( cid == pwr_cClass_SevHist) {
		lsts = gdh_ArefANameToAref( &sevhist_aref, "Attribute", &attr_aref);
		if ( EVEN(lsts)) break;
  
		lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
		if ( EVEN(lsts)) break;
  
		lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
		if ( EVEN(lsts)) break;

		s = strchr( aname, '.');
		if ( !s) break;
  
		*s = 0;
		strcpy( onamev[oid_cnt], aname);
		strcpy( anamev[oid_cnt], s+1);
		oidv[oid_cnt] = attr_aref.Objid;
		sevhistobjectv[oid_cnt] = false;
		oid_cnt++;
	      }
	      else if ( cid == pwr_cClass_SevHistObject) {
		lsts = gdh_ArefANameToAref( &sevhist_aref, "Object", &attr_aref);
		if ( EVEN(lsts)) break;
  
		lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
		if ( EVEN(lsts)) break;

		lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
		if ( EVEN(lsts)) break;

		s = strchr( aname, '.');
		if ( !s) {
		  //It is a complete object
		  anamev[oid_cnt][0] = '\0';
		}
		else {  
		  strcpy( anamev[oid_cnt], s+1);
		  *s = 0;
		}
		strcpy( onamev[oid_cnt], aname);
		oidv[oid_cnt] = attr_aref.Objid;
		sevhistobjectv[oid_cnt] = true;
		oid_cnt++;
	      }
	    }
	  }
	  else if ( sevHistObjectFound ) {
	    lsts = gdh_ArefANameToAref( &mv.Action[i*rows+j].Object, "Object", &attr_aref);
	    if ( EVEN(lsts)) break;

	    lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
	    if ( EVEN(lsts)) break;

	    lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
	    if ( EVEN(lsts)) break;

	    s = strchr( aname, '.');
	    if ( !s) {
	      //It is a complete object
	      anamev[oid_cnt][0] = '\0';
	    }
	    else {  
	      strcpy( anamev[oid_cnt], s+1);
	    }
	    oidv[oid_cnt] = attr_aref.Objid;
	    sevhistobjectv[oid_cnt] = true;
	    strcpy( onamev[oid_cnt], "");
	    sevhist_aref = mv.Action[i*rows+j].Object;
	    oid_cnt = 1;
	  }
	  else {
	    lsts = gdh_ArefANameToAref( &mv.Action[i*rows+j].Object, "Attribute", &attr_aref);
	    if ( EVEN(lsts)) break;
  
	    lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
	    if ( EVEN(lsts)) break;
  
	    lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
	    if ( EVEN(lsts)) break;

	    s = strchr( aname, '.');
	    if ( !s) break;
	    *s = 0;
  
	    strcpy( onamev[0], aname);
	    strcpy( anamev[0], s+1);
	    oidv[0] = attr_aref.Objid;
	    sevhistobjectv[0] = false;
	    oid_cnt = 1;
	    sevhist_aref = mv.Action[i*rows+j].Object;
	  }
	  
	  oidv[oid_cnt] = pwr_cNOid;

	  // Get server and connect to server
	  lsts = gdh_ArefANameToAref( &sevhist_aref, "ThreadObject", &attr_aref);
	  if ( EVEN(lsts)) break;

	  lsts = gdh_GetObjectInfoAttrref( &attr_aref, &histthread_oid, sizeof(histthread_oid));
	  if ( EVEN(lsts)) break;

	  histthread_aref = cdh_ObjidToAref( histthread_oid);
	  lsts = gdh_ArefANameToAref( &histthread_aref, "ServerNode", &attr_aref);
	  if ( EVEN(lsts)) break;

	  lsts = gdh_GetObjectInfoAttrref( &attr_aref, server_node, sizeof(server_node));
	  if ( EVEN(lsts)) break;

	  if ( !xnav->scctx) {
	    sevcli_init( &lsts, &xnav->scctx);
	    if ( EVEN(lsts)) break;
	  }
	  sevcli_set_servernode( &lsts, xnav->scctx, server_node);
	  if ( EVEN(lsts)) break;

	  sevhist[i*rows + j] = new XttSevHistGtk( this, toplevel, (char *)"No title", &widget,
					    oidv, anamev, onamev, sevhistobjectv, 
					    xnav->scctx, w, h, 
					    (unsigned int)curve_mOptions_Embedded, sts);
	  if ( EVEN(*sts)) break;

	  sevhist[i*rows + j]->help_cb = multiview_trend_help_cb;
	  sevhist[i*rows + j]->get_select_cb = multiview_sevhist_get_select_cb;

	  comp_widget[i*rows + j] = widget;

	  // recall_buffer[i*rows + j].insert(graph_name, objectname_p);

	  //appl.insert( applist_eType_Trend, (void *)trend[i*rows + j], &arefv[0], 
	  //	       "",  NULL);
	

	  if ( mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Exchangeable) {
	    exchange_widget[i*rows+j] = gtk_hbox_new( FALSE, 0);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(exchange_widget[i*rows + j]), TRUE, TRUE, 0);
	  }
	  else
	    gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(comp_widget[i*rows + j]), TRUE, TRUE, 0);
	  break;
	}
	default: ;
	}
	if ( (j + 1) % rows != 0 && mv.Options & pwr_mMultiViewOptionsMask_RowSeparators)
	  gtk_box_pack_start( GTK_BOX(row_widget), GTK_WIDGET(gtk_hseparator_new()), FALSE, FALSE, 0);
	  
      }
      gtk_box_pack_start( GTK_BOX(col_widget), GTK_WIDGET(row_widget), TRUE, TRUE, 0);

      if ( i != cols - 1 && mv.Options & pwr_mMultiViewOptionsMask_ColumnSeparators)
	gtk_box_pack_start( GTK_BOX(col_widget), GTK_WIDGET(gtk_vseparator_new()), FALSE, FALSE, 0);

      if ( escape)
	break;
    }
    if ( !(options & ge_mOptions_Embedded))
      gtk_container_add( GTK_CONTAINER(toplevel), col_widget);
    else
      gtk_box_pack_start( GTK_BOX(box_widget), GTK_WIDGET(col_widget), FALSE, FALSE, 0);
  }
  else if ( mv.Layout == pwr_eMultiViewLayoutEnum_Fix) {
  }
  else if ( mv.Layout == pwr_eMultiViewLayoutEnum_Table) {
  }


  if ( !(options & ge_mOptions_Embedded)) {
    gtk_widget_show_all( toplevel);

    if ( !(mv_x == 0 && mv_y == 0)) {
      // Set position
      gtk_window_move( GTK_WINDOW(toplevel), mv_x, mv_y);
    }
    else if ( !(mv.X == 0 && mv.Y == 0)) {
      // Set position from object
      gtk_window_move( GTK_WINDOW(toplevel), mv.X, mv.Y);
    }
  
    for ( int i = 0; i < MV_SIZE; i++) {
      if ( trend[i])
	trend[i]->setup();
    }
    for ( int i = 0; i < MV_SIZE; i++) {
      if ( sevhist[i])
	sevhist[i]->setup();
    }
    if ( options & ge_mOptions_FullScreen || 
	 mv.Options & pwr_mMultiViewOptionsMask_FullScreen)
      gtk_window_fullscreen( GTK_WINDOW(toplevel));
    else if ( options & ge_mOptions_Maximize || 
	      mv.Options & pwr_mMultiViewOptionsMask_Maximize)
      gtk_window_maximize( GTK_WINDOW(toplevel)); // TODO
    else if ( options & ge_mOptions_FullMaximize ||
	      mv.Options & pwr_mMultiViewOptionsMask_FullMaximize)
      gtk_window_maximize( GTK_WINDOW(toplevel));
    else if ( options & ge_mOptions_Iconify ||
	      mv.Options & pwr_mMultiViewOptionsMask_Iconify)
      gtk_window_iconify( GTK_WINDOW(toplevel));
    else if ( options & ge_mOptions_Invisible)
      g_object_set( toplevel, "visible", FALSE, NULL);
  }
  else {
    gtk_widget_set_size_request( box_widget, window_width, window_height);
  }

  *sts = XNAV__SUCCESS;
}

void *XttMultiViewGtk::get_widget() 
{ 
  if ( !(options & ge_mOptions_Embedded))
    return toplevel;
  else
    return box_widget;
}

int XttMultiViewGtk::set_subwindow_source( const char *name, char *source, char *object,
					   int insert, int cont)
{
  pwr_sClass_XttMultiView mv;
  pwr_tStatus sts;
  int x, y, w, h;
  int scrollbar;
  int menu;
  int type;
  char comp_name[80];
  char *sub_name;
   
  sts = gdh_GetObjectInfoAttrref( &aref, &mv, sizeof(mv));
  if ( EVEN(sts)) return sts;
  
  strncpy( comp_name, name, sizeof(comp_name));
  if ( (sub_name = strchr( comp_name, '.'))) {
    *sub_name = 0;
    sub_name++;
  }
  else
    sub_name = 0;

  for ( int i = 0; i < cols; i++) {
    for ( int j = 0; j < rows; j++) {
      if ( cdh_NoCaseStrcmp( comp_name, mv.Action[i*rows+j].Name) == 0) {
	if ( !sub_name) {
	  // Replace component
	  x = 0;
	  y = 0;
	  w = mv.Action[i*rows+j].Width;
	  h = mv.Action[i*rows+j].Height;
	  scrollbar = (mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Scrollbars) ? 1 : 0;
	  menu = (mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Menu) ? 1 : 0;
	  type = mv.Action[i*rows+j].Type;

	  if ( !(mv.Action[i*rows+j].Options & pwr_mMultiViewElemOptionsMask_Exchangeable))
	    return 0;

	  switch ( type) {
	  case pwr_eMultiViewContentEnum_Graph:
	  case pwr_eMultiViewContentEnum_ObjectGraph: {
	    XttGeGtk *ctx = new XttGeGtk( toplevel, this, "No title", 
					  source, scrollbar, menu, 0, w, h, x, y, 
					  1.0, object, 0, 0, 
					  ge_mOptions_Embedded, 0,
					  multiview_ge_command_cb, multiview_ge_get_current_objects_cb,
					  multiview_ge_is_authorized_cb);
	    
	    ctx->close_cb = multiview_ge_close_cb;
	    ctx->help_cb = multiview_ge_help_cb;
	    ctx->display_in_xnav_cb = multiview_ge_display_in_xnav_cb;
	    ctx->popup_menu_cb = multiview_ge_popup_menu_cb;
	    ctx->call_method_cb = multiview_ge_call_method_cb;
	    ctx->sound_cb = multiview_ge_sound_cb;
	    ctx->eventlog_cb = multiview_ge_eventlog_cb;
	    
	    GtkWidget *comp_w = ctx->get_graph_widget();
	  

	    appl.remove( (void *)gectx[i*rows+j]);
	    delete gectx[i*rows+j];

	    //gtk_container_remove(GTK_CONTAINER(exchange_widget[i*rows + j]), comp_widget[i*rows+j]);
	    gtk_widget_destroy( comp_widget[i*rows+j]);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_w), TRUE, TRUE, 0);
	    // gtk_container_add(GTK_CONTAINER(exchange_widget[i*rows + j]), comp_w);
	    gtk_widget_show_all( exchange_widget[i*rows + j]);
	    gtk_box_reorder_child( GTK_BOX(exchange_widget[i*rows+j]), comp_w, 0);

	    comp_widget[i*rows + j] = comp_w;	
	    gectx[i*rows+j] = ctx;	  
	    if ( insert)
	      recall_buffer[i*rows + j].insert( source, object);
	    appl.insert( applist_eType_Graph, (void *)gectx[i*rows + j], pwr_cNObjid, source,
			 object);
	    break;
	  }
	  case pwr_eMultiViewContentEnum_MultiView: {
	    pwr_tAttrRef source_aref;

	    sts = gdh_NameToAttrref( pwr_cNObjid, source, &source_aref);
	    if ( EVEN(sts)) break;

	    XttMultiViewGtk *ctx = new XttMultiViewGtk( toplevel, this, "No title", 
					  &source_aref, w, h, x, y, 
					  ge_mOptions_Embedded, &sts,
					  multiview_ge_command_cb, multiview_ge_get_current_objects_cb,
					  multiview_ge_is_authorized_cb);
	    
	    ctx->close_cb = multiview_ge_close_cb;
	    ctx->help_cb = multiview_ge_help_cb;
	    ctx->display_in_xnav_cb = multiview_ge_display_in_xnav_cb;
	    ctx->popup_menu_cb = multiview_ge_popup_menu_cb;
	    ctx->call_method_cb = multiview_ge_call_method_cb;
	    ctx->sound_cb = multiview_ge_sound_cb;
	    ctx->eventlog_cb = multiview_ge_eventlog_cb;
	    
	    GtkWidget *comp_w = (GtkWidget *)ctx->get_widget();
	  
	    appl.remove( (void *)mvctx[i*rows+j]);
	    delete mvctx[i*rows+j];

	    //gtk_container_remove(GTK_CONTAINER(exchange_widget[i*rows + j]), comp_widget[i*rows+j]);
	    gtk_widget_destroy( comp_widget[i*rows+j]);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_w), TRUE, TRUE, 0);
	    // gtk_container_add(GTK_CONTAINER(exchange_widget[i*rows + j]), comp_w);
	    gtk_widget_show_all( exchange_widget[i*rows + j]);
	    gtk_box_reorder_child( GTK_BOX(exchange_widget[i*rows+j]), comp_w, 0);

	    comp_widget[i*rows + j] = comp_w;	
	    mvctx[i*rows+j] = ctx;	  

	    if ( insert)
	      recall_buffer[i*rows + j].insert( source, object);
	    appl.insert( applist_eType_MultiView, (void *)mvctx[i*rows + j], &source_aref, "", NULL);
	    break;
	  }
	  case pwr_eMultiViewContentEnum_TrendCurve: {
	    int plotgroup_found = 0;
	    pwr_tAttrRef plotgroup;
	    pwr_tCid classid;
	    GtkWidget *comp_w;
	    pwr_tAttrRef arefv[2];
	    int skip = 0;
	    pwr_tStatus lsts;
	    pwr_tAttrRef object_aref;
	    
	    lsts = gdh_NameToAttrref( pwr_cNObjid, object, &object_aref);
	    if ( EVEN(lsts)) break;

	    lsts = gdh_GetAttrRefTid( &object_aref, &classid);
	    if (EVEN(lsts)) break;

	    switch ( classid) {
	    case pwr_cClass_DsTrend:
	    case pwr_cClass_DsTrendCurve:
	      break;
	    case pwr_cClass_PlotGroup:
	      plotgroup_found = 1;
	      plotgroup = object_aref;
	      arefv[0] = plotgroup;
	      break;
	    default:
	      skip = 1;
	    }
	    
	    if ( skip)
	      break;

	    XttTrendGtk *ctx;
	    if ( plotgroup_found) {
	      ctx = new XttTrendGtk( this, toplevel, (char *)"No title", &comp_w,
						 0, &plotgroup, w, h, (unsigned int)curve_mOptions_Embedded, &lsts);
	    }
	    else {
	      arefv[0] = object_aref;
	      memset( &arefv[1], 0, sizeof(arefv[0]));
	      ctx = new XttTrendGtk( this, toplevel, (char *)"No title", &comp_w,
						   arefv, 0, w, h, (unsigned int)curve_mOptions_Embedded, &lsts);
	    }
	    if ( EVEN(lsts)) break;

	    ctx->close_cb = multiview_trend_close_cb;
	    ctx->command_cb = multiview_trend_command_cb;
	    ctx->help_cb = multiview_trend_help_cb;
	    	
	    appl.remove( (void *)trend[i*rows+j]);
	    delete trend[i*rows+j];

	    gtk_widget_destroy( comp_widget[i*rows+j]);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_w), TRUE, TRUE, 0);
	    gtk_widget_show_all( exchange_widget[i*rows + j]);
	    ctx->setup();
	    gtk_box_reorder_child( GTK_BOX(exchange_widget[i*rows+j]), comp_w, 0);

	    comp_widget[i*rows + j] = comp_w;	
	    trend[i*rows+j] = ctx;	  

	    if ( insert)
	      recall_buffer[i*rows + j].insert( source, object);
	    appl.insert( applist_eType_Trend, (void *)trend[i*rows + j], &arefv[0], 
			 "",  NULL);

	    mv.Action[i*rows+j].Object = object_aref;
	    break;
	  }

	  case pwr_eMultiViewContentEnum_SevHistory: {
	    pwr_tOid oidv[11];
	    pwr_tOName anamev[11];
	    pwr_tOName onamev[11];
	    bool sevhistobjectv[11];
	    pwr_tAttrRef attr_aref, sevhist_aref;
	    char *s;
	    pwr_tAName aname;
	    int plotgroup_found = 0;
	    int sevHistObjectFound = 0;
	    int oid_cnt = 0;
	    pwr_tCid classid;
	    int skip = 0;
	    pwr_tStatus lsts;
	    GtkWidget *comp_w;
	    pwr_tAttrRef arefv[2];
	    pwr_tAttrRef plotgroup;
	    pwr_tAttrRef object_aref;
	    
	    lsts = gdh_NameToAttrref( pwr_cNObjid, object, &object_aref);
	    if ( EVEN(lsts)) break;

	    arefv[0] = object_aref;
	    memset( &arefv[1], 0, sizeof(arefv[0]));


	    lsts = gdh_GetAttrRefTid( &arefv[0], &classid);
	    if (EVEN(lsts)) break;;

	    switch ( classid) {
	    case pwr_cClass_SevHist:
	      break;
	    case pwr_cClass_SevHistObject:
	      sevHistObjectFound = true;
	      break;
	    case pwr_cClass_PlotGroup:
	      plotgroup = object_aref;
	      plotgroup_found = 1;
	      break;
	    default:
	      skip = 1;
	    }

	    if ( skip)
	      break;
	    
	    if ( plotgroup_found) {
	      pwr_sClass_PlotGroup plot;
	      pwr_tCid cid;
	      int j;

	      lsts = gdh_GetObjectInfoAttrref( &plotgroup, &plot, sizeof(plot));
	      if ( EVEN(lsts)) break;
	
	      for ( j = 0; j < 20; j++) {
		if ( cdh_ObjidIsNull( plot.YObjectName[j].Objid))
		  break;
		
		sevhist_aref = plot.YObjectName[j];
		lsts = gdh_GetAttrRefTid( &sevhist_aref, &cid);
		if ( EVEN(lsts)) break;

		if ( cid == pwr_cClass_SevHist) {
		  lsts = gdh_ArefANameToAref( &sevhist_aref, "Attribute", &attr_aref);
		  if ( EVEN(lsts)) break;
		  
		  lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
		  if ( EVEN(lsts)) break;
		  
		  lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
		  if ( EVEN(lsts)) break;
		  
		  s = strchr( aname, '.');
		  if ( !s) break;
  
		  *s = 0;
		  strcpy( onamev[oid_cnt], aname);
		  strcpy( anamev[oid_cnt], s+1);
		  oidv[oid_cnt] = attr_aref.Objid;
		  sevhistobjectv[oid_cnt] = false;
		  oid_cnt++;
		}
		else if ( cid == pwr_cClass_SevHistObject) {
		  lsts = gdh_ArefANameToAref( &sevhist_aref, "Object", &attr_aref);
		  if ( EVEN(lsts)) break;
		  
		  lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
		  if ( EVEN(lsts)) break;
		  
		  lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
		  if ( EVEN(lsts)) break;

		  s = strchr( aname, '.');
		  if ( !s) {
		    //It is a complete object
		    anamev[oid_cnt][0] = '\0';
		  }
		  else {  
		    strcpy( anamev[oid_cnt], s+1);
		    *s = 0;
		  }
		  strcpy( onamev[oid_cnt], aname);
		  oidv[oid_cnt] = attr_aref.Objid;
		  sevhistobjectv[oid_cnt] = true;
		  oid_cnt++;
		}
	      }
	    }
	    else if ( sevHistObjectFound ) {
	      lsts = gdh_ArefANameToAref( &object_aref, "Object", &attr_aref);
	      if ( EVEN(lsts)) break;

	      lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
	      if ( EVEN(lsts)) break;
	      
	      lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
	      if ( EVEN(lsts)) break;

	      s = strchr( aname, '.');
	      if ( !s) {
		//It is a complete object
		anamev[oid_cnt][0] = '\0';
	      }
	      else {  
		strcpy( anamev[oid_cnt], s+1);
	      }
	      oidv[oid_cnt] = attr_aref.Objid;
	      sevhistobjectv[oid_cnt] = true;
	      strcpy( onamev[oid_cnt], "");
	      sevhist_aref = object_aref;
	      oid_cnt = 1;
	    }
	    else {
	      lsts = gdh_ArefANameToAref( &object_aref, "Attribute", &attr_aref);
	      if ( EVEN(lsts)) break;
  
	      lsts = gdh_GetObjectInfoAttrref( &attr_aref, &attr_aref, sizeof(attr_aref));
	      if ( EVEN(lsts)) break;
  
	      lsts = gdh_AttrrefToName( &attr_aref, aname, sizeof(aname), cdh_mNName);
	      if ( EVEN(lsts)) break;

	      s = strchr( aname, '.');
	      if ( !s) break;
	      *s = 0;
  
	      strcpy( onamev[0], aname);
	      strcpy( anamev[0], s+1);
	      oidv[0] = attr_aref.Objid;
	      sevhistobjectv[0] = false;
	      oid_cnt = 1;
	      sevhist_aref = object_aref;
	    }
	  
	    oidv[oid_cnt] = pwr_cNOid;

	    XNav 	*xnav = get_xnav();
	    XttSevHistGtk *ctx;

	    if ( !xnav->scctx)
	      break;

	    ctx = new XttSevHistGtk( this, toplevel, (char *)"No title", &comp_w,
				     oidv, anamev, onamev, sevhistobjectv, 
				     xnav->scctx, w, h, 
				     (unsigned int)curve_mOptions_Embedded, &lsts);
	    if ( EVEN(lsts)) break;

	    ctx->help_cb = multiview_trend_help_cb;
	    ctx->get_select_cb = multiview_sevhist_get_select_cb;

	    appl.remove( (void *)trend[i*rows+j]);
	    delete sevhist[i*rows+j];

	    gtk_widget_destroy( comp_widget[i*rows+j]);
	    gtk_box_pack_start( GTK_BOX(exchange_widget[i*rows+j]), GTK_WIDGET(comp_w), TRUE, TRUE, 0);
	    gtk_widget_show_all( exchange_widget[i*rows + j]);
	    ctx->setup();
	    gtk_box_reorder_child( GTK_BOX(exchange_widget[i*rows+j]), comp_w, 0);

	    comp_widget[i*rows + j] = comp_w;	
	    sevhist[i*rows+j] = ctx;	  

	    //if ( insert)
	    //  recall_buffer[i*rows + j].insert( source, object);
	    //appl.insert( applist_eType_Trend, (void *)trend[i*rows + j], &arefv[0], 
	    //		 "",  NULL);

	    mv.Action[i*rows+j].Object = object_aref;
	  }
	  default: ;
	  }
	}
	else {
	  // Call set_window in component
	  type = mv.Action[i*rows+j].Type;

	  switch ( type) {
	  case pwr_eMultiViewContentEnum_Graph:
	  case pwr_eMultiViewContentEnum_ObjectGraph: {
	    gectx[i*rows+j]->set_subwindow_source( sub_name, source, object);
	    break;
	  }
	  case pwr_eMultiViewContentEnum_MultiView: {
	    mvctx[i*rows+j]->set_subwindow_source( sub_name, source, object,
						   insert);
	  }
	  default: ;
	  }
	}
      }
    }
  }
  if ( cont)
    return 1;
  else
    return GLOW__TERMINATED;
}

