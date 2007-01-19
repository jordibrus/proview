/* 
 * Proview   $Id: xtt_block_gtk.cpp,v 1.1 2007-01-04 08:29:32 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
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
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* xtt_block_gtk.cpp -- Alarm blocking window in xtt. */

#include <gtk/gtk.h>

#include "pwr_class.h"
#include "pwr_privilege.h"
#include "rt_gdh.h"
#include "rt_mh_outunit.h"
#include "co_cdh.h"
#include "xtt_block_gtk.h"
#include "co_lng.h"
#include "co_wow_gtk.h"
#include "co_msg.h"

int BlockGtk::execute()
{
  mh_eEventPrio prio;
  pwr_tStatus sts;

  if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(toggleA)))
    prio = mh_eEventPrio_A;
  else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(toggleB)))
    prio = mh_eEventPrio_B;
  else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(toggleC)))
    prio = mh_eEventPrio_C;
  else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(toggleD)))
    prio = mh_eEventPrio_D;
  else
    prio = (mh_eEventPrio) 0;
  
  sts = mh_OutunitBlock( oar.Objid, prio);
  if (EVEN(sts)) {
    char msg[80];

    msg_GetMsg( sts, msg, sizeof(msg));
    wow->DisplayError( "Block Error", msg);
  }
  return sts;
}

void BlockGtk::update()
{
  pwr_tStatus    sts;
  mh_uEventInfo  block_level;

  sts = gdh_GetAlarmInfo( oar.Objid, NULL, NULL, (pwr_tUInt32 *) &block_level,
			  NULL, NULL);
  switch ( block_level.Event.Prio) {
  case mh_eEventPrio_A:
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(toggleA), TRUE);
    break;
  case mh_eEventPrio_B:
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(toggleB), TRUE);
    break;
  case mh_eEventPrio_C:
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(toggleC), TRUE);
    break;
  case mh_eEventPrio_D:
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(toggleD), TRUE);
    break;
  case 0:
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(toggleNo), TRUE);
    break;
  default:
    break;
  }
}

void BlockGtk::activate_apply( GtkWidget *w, gpointer data)
{
  Block *blk = (Block *)data;

  blk->execute();
}

void BlockGtk::activate_ok( GtkWidget *w, gpointer data)
{
  Block *blk = (Block *)data;
  pwr_tStatus sts;

  sts = blk->execute();
  if ( ODD(sts))
    delete blk;
}

void BlockGtk::activate_cancel( GtkWidget *w, gpointer data)
{
  Block *blk = (Block *)data;

  delete blk;
}

BlockGtk::~BlockGtk()
{
  delete wow;
  gtk_widget_destroy( toplevel);
}

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  BlockGtk::activate_cancel( w, data);
  return TRUE;
}

BlockGtk::BlockGtk( void *b_parent_ctx,
		    GtkWidget *b_parent_wid,
		    pwr_sAttrRef *b_oar,
		    char *name,
		    unsigned int priv,
		    pwr_tStatus *sts):
  Block( b_parent_ctx, b_oar, name, priv, sts), parent_wid(b_parent_wid)
{
  char 		title[400];
  pwr_tAName	aname;

  *sts = gdh_AttrrefToName( &oar, aname, sizeof(aname), cdh_mNName);
  if ( EVEN(*sts)) return;

  strcpy( title, name);
  strcat( title, "    ");
  strcat( title, aname);

  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
					     "default-height", 300,
					     "default-width", 500,
					     "title", title,
					     NULL);
  g_signal_connect( toplevel, "delete_event", G_CALLBACK(delete_event), this);

  CoWowGtk::SetWindowIcon( toplevel);

  toggleA = gtk_check_button_new_with_label( "A Alarm");
  toggleB = gtk_check_button_new_with_label( "B Alarm");
  toggleC = gtk_check_button_new_with_label( "C Alarm");
  toggleD = gtk_check_button_new_with_label( "D Alarm");
  toggleNo = gtk_check_button_new_with_label( "No Blocking");

  GtkWidget *toggle_vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(toggle_vbox), toggleA, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(toggle_vbox), toggleB, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(toggle_vbox), toggleC, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(toggle_vbox), toggleD, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(toggle_vbox), toggleNo, FALSE, FALSE, 7);

  buttonOk = gtk_button_new_with_label( "Ok");
  gtk_widget_set_size_request( buttonOk, 70, 25);
  g_signal_connect( buttonOk, "clicked", G_CALLBACK(activate_ok), this);

  buttonApply = gtk_button_new_with_label( "Apply");
  gtk_widget_set_size_request( buttonApply, 70, 25);
  g_signal_connect( buttonApply, "clicked", G_CALLBACK(activate_apply), this);

  GtkWidget *buttonCancel = gtk_button_new_with_label( "Cancel");
  gtk_widget_set_size_request( buttonCancel, 70, 25);
  g_signal_connect( buttonCancel, "clicked", G_CALLBACK(activate_cancel), this);

  GtkWidget *hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(hboxbuttons), buttonOk, FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(hboxbuttons), buttonApply, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(hboxbuttons), buttonCancel, FALSE, FALSE, 0);

  GtkWidget *vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), toggle_vbox, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(vbox), hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(toplevel), vbox);
  gtk_widget_show_all( toplevel);

  if ( !(priv & pwr_mPrv_RtEvents ||
	 priv & pwr_mPrv_System)) {
    gtk_widget_set_sensitive( buttonOk, FALSE);
    gtk_widget_set_sensitive( buttonApply, FALSE);
    gtk_widget_set_sensitive( toggleA, FALSE);
    gtk_widget_set_sensitive( toggleB, FALSE);
    gtk_widget_set_sensitive( toggleC, FALSE);
    gtk_widget_set_sensitive( toggleD, FALSE);
    gtk_widget_set_sensitive( toggleNo, FALSE);
  }

  wow = new CoWowGtk( parent_wid);

  update();

#if 0
  char		uid_filename[120] = {"xtt_block.uid"};
  char		*uid_filename_p = uid_filename;
  Arg 		args[20];
  int		msts;
  int		i;
  MrmHierarchy 	s_DRMh;
  MrmType 	dclass;
  char 		title[200];
  pwr_tAName	aname;

  static MrmRegisterArg	reglist[] = {
        { "blk_ctx", 0 },
	{"blk_activate_cancel",(caddr_t)activate_cancel },
	{"blk_activate_ok",(caddr_t)activate_ok },
	{"blk_activate_apply",(caddr_t)activate_apply },
	{"blk_create_ok",(caddr_t)create_ok },
	{"blk_create_apply",(caddr_t)create_apply },
	{"blk_create_toggleA",(caddr_t)create_toggleA },
	{"blk_create_toggleB",(caddr_t)create_toggleB },
	{"blk_create_toggleC",(caddr_t)create_toggleC },
	{"blk_create_toggleD",(caddr_t)create_toggleD },
	{"blk_create_toggleNo",(caddr_t)create_toggleNo }
	};
  static int	reglist_num = (sizeof reglist / sizeof reglist[0]);

  *sts = 1;

  Lng::get_uid( uid_filename, uid_filename);

  *sts = gdh_AttrrefToName( &oar, aname, sizeof(aname), cdh_mNName);
  if ( EVEN(*sts)) return;

  strcpy( title, name);
  strcat( title, "    ");
  strcat( title, aname);

  reglist[0].value = (caddr_t) this;

  // Gtk
  MrmInitialize();

  // Save the context structure in the widget
  i = 0;
  XtSetArg(args[i], XmNuserData, (unsigned int) this);i++;
  XtSetArg(args[i], XmNdeleteResponse, XmDO_NOTHING);i++;

  msts = MrmOpenHierarchy( 1, &uid_filename_p, NULL, &s_DRMh);
  if (msts != MrmSUCCESS) printf("can't open %s\n", uid_filename);

  MrmRegisterNames(reglist, reglist_num);

  parent_wid = XtCreatePopupShell( title, 
	  topLevelShellWidgetClass, parent_wid, args, i);

  msts = MrmFetchWidgetOverride( s_DRMh, "blk_window", parent_wid,
			name, args, 1, &toplevel, &dclass);
  if (msts != MrmSUCCESS)  printf("can't fetch %s\n", name);

  MrmCloseHierarchy(s_DRMh);

  i = 0;
  XtSetArg(args[i],XmNwidth,500);i++;
  XtSetArg(args[i],XmNheight,200);i++;
  XtSetValues( toplevel, args, i);

  XtManageChild( toplevel);

  XtPopup( parent_wid, XtGrabNone);

  if ( !(priv & pwr_mPrv_RtEvents ||
	 priv & pwr_mPrv_System)) {
    Arg	sensitive[1];
    // No access to block
    // XtUnmanageChild( ok);
    // XtUnmanageChild( apply);

    XtSetArg( sensitive[0],XmNsensitive, 0);
    XtSetValues( buttonOk, sensitive, 1);
    XtSetValues( buttonApply, sensitive, 1);
    XtSetValues( toggleA, sensitive, 1);
    XtSetValues( toggleB, sensitive, 1);
    XtSetValues( toggleC, sensitive, 1);
    XtSetValues( toggleD, sensitive, 1);
    XtSetValues( toggleNo, sensitive, 1);
  }

  wow = new CoWowGtk( parent_wid);

  update();

  // Connect the window manager close-button to exit
  flow_AddCloseVMProtocolCb( parent_wid, 
	(XtCallbackProc)activate_cancel, this);
#endif
}








