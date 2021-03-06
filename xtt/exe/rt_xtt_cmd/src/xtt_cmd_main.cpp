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

/* xtt_cmd.c -- command file processing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pwr.h"
#include "pwr_class.h"
#include "co_dcli.h"

#include "rt_load.h"
#include "wb_foe_msg.h"
#include "co_dcli_input.h"

#include "flow.h"
#include "flow_ctx.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "xtt_xnav.h"
#include "xtt_item.h"
#include "rt_xtt_cmd.h"

xnav_sStartMenu XttCmd::alarm_menu[] = {
	{ "Alarm List", xnav_eItemType_Command,	menu_ePixmap_List, (void *) "show alarm"},
	{ "Event List", xnav_eItemType_Command,	menu_ePixmap_List,	(void *) "show event"},
	{ "Blocked Alarms", xnav_eItemType_Command,	menu_ePixmap_List,	(void *) "show block"},
	{ "Historical List", xnav_eItemType_Command, menu_ePixmap_List,	(void *) "show hist"},
	{ "", 0, 0, NULL}};

xnav_sStartMenu XttCmd::nethandler_menu[] = {
	{ "Link", xnav_eItemType_Command, menu_ePixmap_Map, (void *) "show link"},
	{ "Subscription Client", xnav_eItemType_Command, menu_ePixmap_Map, (void *) "show subcli"},
	{ "Subscription Server", xnav_eItemType_Command, menu_ePixmap_Map, (void *) "show subsrv"},
	{ "", 0, 0, NULL}};
xnav_sStartMenu XttCmd::communication_menu[] = {
	{ "RemNode", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show remnode"},
	{ "RemTrans", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show remtrans"},
	{ "", 0, 0, NULL}};
xnav_sStartMenu XttCmd::logging_menu[] = {
	{ "Logging entry 1", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=1"},
	{ "Logging entry 2", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=2"},
	{ "Logging entry 3", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=3"},
	{ "Logging entry 4", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=4"},
	{ "Logging entry 5", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=5"},
	{ "Logging entry 6", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=6"},
	{ "Logging entry 7", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=7"},
	{ "Logging entry 8", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=8"},
	{ "Logging entry 9", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=9"},
	{ "Logging entry 10", xnav_eItemType_Command,	menu_ePixmap_Map, (void *) "show logging/entry=10"},
	{ "", 0, 0, NULL}};
xnav_sStartMenu XttCmd::system_menu[] = {
	{ "Nethandler", xnav_eItemType_Menu,	menu_ePixmap_Map, 	(void *)&XttCmd::nethandler_menu},
	{ "Communication", xnav_eItemType_Menu,	menu_ePixmap_Map, 	(void *)&XttCmd::communication_menu},
	{ "Device", 	xnav_eItemType_Command,	menu_ePixmap_Map, 	(void *) "show device"},
	{ "PlcThread", 	xnav_eItemType_Command,	menu_ePixmap_Map, 	(void *) "show plcthread"},
	{ "PlcPgm", 	xnav_eItemType_Command,	menu_ePixmap_Map,	(void *) "show plcpgm"},
	{ "Logging", 	xnav_eItemType_Menu,	menu_ePixmap_Map,	(void *)&XttCmd::logging_menu},
	{ "System Messages", xnav_eItemType_Command, menu_ePixmap_List,	(void *) "open consolelog"},
	{ "System Status", 	xnav_eItemType_Command,	menu_ePixmap_Map,	(void *) "show nodeinfo"},
	{ "Nodes", 	xnav_eItemType_Command,	menu_ePixmap_Map,	(void *) "show nodeobjects"},
	{ "Volumes", 	xnav_eItemType_Command,	menu_ePixmap_Map,	(void *) "show volumes"},
	{ "", 0, 0, NULL}};
xnav_sStartMenu XttCmd::root_menu[] = {
	{ "Database", 	xnav_eItemType_Command,	menu_ePixmap_Map, 	(void *) "show database"},
	{ "Alarm", 	xnav_eItemType_Menu,	menu_ePixmap_Map, 	(void *)&XttCmd::alarm_menu},
	{ "Store",	xnav_eItemType_Command,	menu_ePixmap_Map, 	(void *) "show file"},
	{ "System",	xnav_eItemType_Menu,	menu_ePixmap_Map, 	(void *)&XttCmd::system_menu},
	{ "Close", 	xnav_eItemType_Command,	menu_ePixmap_Leaf,	(void *) "exit"},
	{ "", 0, 0, NULL}};

void XttCmd::usage()
{
  cout << endl << endl <<
    "xttc        Proview Runtime Commands" << endl << endl << 
    "Arguments:" << endl <<
    "  -h             Print usage." << endl <<
    "  -q             Quiet. Don't print license information." << endl <<
    "  -i             Hide navigator." << endl << endl <<
    "  Other arguments are treated as a command and passed to the command parser" << endl <<
    "  If a command is given as an argument, the command will be executed and the" << endl <<
    "  program is then terminated." << endl <<
    "  If no command is given, xttc will prompt for a command." << endl << endl <<
    "Examples:" << endl <<
    "  $ xttc" << endl <<
    "  xttc>" << endl << endl <<
    "  " << endl;
}

void XttCmd::message_cb( void *ctx, char severity, const char *msg)
{
  if ( strcmp( msg, "") != 0)
    printf( "XNAV-'%c'-MSG, %s\n", severity, msg);
}

void XttCmd::close_cb( void *ctx, int terminate)
{
  XttCmd *cmd = (XttCmd *) ctx;

  dcli_input_end( &cmd->chn, cmd->recall_buf);
  exit(0);
}

XttCmd::XttCmd() :
  xnav(0)
{
}
