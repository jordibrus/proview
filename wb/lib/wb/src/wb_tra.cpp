/* 
 * Proview   $Id: wb_tra.cpp,v 1.1 2007-01-04 07:29:04 claes Exp $
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
 **/

/* wb_tra.c

   Gets pointers to rtdb for some objects under a plc program
   and update the aspect of theses objects on the screen 
   depending of the value of these objects in rtdb.

   The objects are traced  with two different way:
   1. At the start of the tracing , for all "the object that can to 
      be higligthed" the pointer to the rtdb are taken.

   2. When the trace function is already running an other object can be
      traced by connecting an analyse object to one of his parameter.  */

#define TRA_MAX_TRACEMETHOD 10 

#define	BEEP	    putchar( '\7' );

/*_Include files_________________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"

#include "flow_ctx.h"
#include "flow_api.h"
#include "wb_ldh.h"
#include "wb_vldh.h"
#include "wb_goen.h"
#include "wb_gcg.h"
#include "wb_gre.h"
#include "wb_foe.h"
#include "co_cdh.h"
#include "rt_gdh.h"
#include "wb_foe_msg.h"
#include "wb_vldh_msg.h"
#include "rt_gdh_msg.h"
#include "wb_tra.h"



/*_procedure declarations ____________________________________*/

static int trace_flow_cb( FlowCtx *ctx, flow_tEvent event);
static void trace_changevalue (
    WGre	    *gre,
    flow_tNode	    fnode
);
static pwr_tStatus	trace_aanalyse_set_value(
    WFoe *foe,
    char	*valuestr
);

typedef pwr_tStatus (*tra_tMethod)( WGre *, vldh_t_node, char *, char *, char *, 
					 flow_eTraceType *, int *);

static pwr_tStatus trace_get_attr_m0( 	WGre		*gre, 
						vldh_t_node	node, 
						char		*debug_par,
						char		*object_str, 
						char		*attr_str,
						flow_eTraceType	*trace_type,
						int		*inverted)
{ return TRA__DISCARD;}
static pwr_tStatus trace_get_attr_m1(	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted);
static pwr_tStatus trace_get_attr_m2( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted);
static pwr_tStatus trace_get_attr_m3( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted);
static pwr_tStatus trace_get_attr_m4( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted);
static pwr_tStatus trace_get_attr_m5( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted);
static pwr_tStatus trace_get_attr_m7( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted);
static pwr_tStatus trace_get_attr_m9( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted);
pwr_tStatus trace_get_attr_mno( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{ return TRA__NOMETHOD;}

tra_tMethod trace_get_attr_m[TRA_MAX_TRACEMETHOD] = {
    trace_get_attr_m0,
    trace_get_attr_m1,
    trace_get_attr_m2,
    trace_get_attr_m3,
    trace_get_attr_m4,
    trace_get_attr_m5,
    trace_get_attr_mno,
    trace_get_attr_m7,
    trace_get_attr_mno,
    trace_get_attr_m9
};

int trace_get_attributes( 	WGre		*gre, 
				vldh_t_node	node, 
				char		*object_str, 
				char		*attr_str,
				flow_eTraceType	*trace_type,
				int		*inverted)
{
  int			sts, size; 
  pwr_tClassId		bodyclass;
  pwr_sGraphPlcNode 	*graphbody;

  sts = ldh_GetClassBody( node->hn.wind->hw.ldhses,
		node->ln.cid, "GraphPlcNode", 
		&bodyclass, (char **)&graphbody, &size);
  if (EVEN(sts)) return sts;

  if ( graphbody->tracemethod  >= TRA_MAX_TRACEMETHOD) 
    return TRA__BADMETHOD;

  sts = (trace_get_attr_m[graphbody->tracemethod])( gre, node, 
		graphbody->debugpar, object_str, attr_str, trace_type, inverted);
  return sts;
}


/*************************************************************************
*
* Name:		trace_getm1()
*
* Description:
* Get the trace information for objects that refers to other objects:
*  getdi, getdo , getdv , getai , getao , getav , 
*  stodo , stodv , stoao , stoav ,  
*  setdo , setdv , resdo , resdv .
*
*  Notice : 1. debug par is not the parameter to be debuged ,
* 					(life is sometimes more complicated):
*   o: As usual debug par give the name of a parameter under the rtbody
* ( of the object with tracemethod 1 ) but,
*   a: The parameter refered by debug par is supposed to have the flags
*   rtvirtual and devbodyref sets. 
*   b: The pgmname of this parameter is the pgmname of the parameter 
*   to trace in the refered object.
*   c: the value of the first barn of type objdid under the devbody
* ( of the object with tracemethod 1 ) is the objdid of the refered object.
*
*  Notice 2: annotation is irrelevant for these nodes.
*  Notice 3: These nodes use only one tranode. So nb_ptr is irrelevant.
**************************************************************************/

static pwr_tStatus trace_get_attr_m1( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{
  pwr_tStatus	sts;
  pwr_eType	par_type;

  *inverted = 0;

  /* Get the object and parameter that should be traced */
  sts = gcg_get_debug( node, debug_par, object_str, attr_str, &par_type);
  if ( EVEN(sts)) return sts;

  switch( par_type) {
    case pwr_eType_Boolean:
      *trace_type = flow_eTraceType_Boolean;
      break;
    case pwr_eType_Int32:
      *trace_type = flow_eTraceType_Int32;
      break;
    case pwr_eType_Float32:
      *trace_type = flow_eTraceType_Float32;
      break;
    default:
      *trace_type = flow_eTraceType_Int32;
      break;
  }
  return TRA__SUCCESS ;
}


/*************************************************************************
*
* Name:		trace_getm2()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* tra_ctx	tractx		I	trace context
* tra_t_tranode	*tranode_ptr    O	pointer to a tranode to fill up.
* int		*nb_ptr		IO	irrelevant for this method
*
* Description:
* Tracemethod for the objects of type input which refers to another
* object trough the connection .
* ( ie the id refered object is not saved in the object ( as it is 
* in the case of the tracemethod 2 ).
* Notice1 : this method is quite similar to getm8.
* Notice2 : debug par refers a parameter of type RTvirtual 
* which IS NOT of type objdid.
* As it is today, only : setcond  object is concerned by this method.
**************************************************************************/

static pwr_tStatus trace_get_attr_m2( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{
  pwr_tStatus	sts;
  pwr_eType	par_type;
  int		par_inverted;

  /* Get the object and parameter that should be traced */
  sts = gcg_get_debug_virtual( node, debug_par, object_str, attr_str, 
		&par_type, &par_inverted);
  if ( EVEN(sts)) return sts;

  *inverted = par_inverted;
  switch( par_type) {
    case pwr_eType_Boolean:
      *trace_type = flow_eTraceType_Boolean;
      break;
    case pwr_eType_Int32:
      *trace_type = flow_eTraceType_Int32;
      break;
    case pwr_eType_Float32:
      *trace_type = flow_eTraceType_Float32;
      break;
    default:
      *trace_type = flow_eTraceType_Int32;
      break;
  }
  return TRA__SUCCESS ;
}


/*************************************************************************
*
* Name:		trace_getm3 ()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* tra_ctx	tractx		I	trace context
* tra_t_tranode	*tranode_ptr    O	pointer to a tranode to fill up.
* int		*nb_ptr		IO	irrelevant for this method
*
* Description:
* In this method the parameter to trace is a barn of the rtbody which name
* is given by 'debugpar' in the graphplcnode.
* tracing method for the following objects :
* and,	or,	xor,	
*   	edge,	sr_s,	sr_r,	dsup,	asup,	
* 	trans,	reset_so 
**************************************************************************/

static pwr_tStatus trace_get_attr_m3( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{
  pwr_tStatus		sts; 
  int			size; 
  ldh_sParDef 		*bodydef;
  int 			i ,rows;
  int			found;
  pwr_tOName   		name;
  pwr_eType		par_type;
  
  *inverted = 0;

  /* Get the name of the object */
  sts = ldh_ObjidToName( node->hn.wind->hw.ldhses, 
		node->ln.oid, ldh_eName_Hierarchy, name, sizeof(name), 
		&size); 
  if( EVEN(sts) ) return sts;

  sts = ldh_GetObjectBodyDef( node->hn.wind->hw.ldhses,
	      node->ln.cid, "RtBody", 1, &bodydef, &rows);
  if( EVEN(sts) ) return sts;

  found = 0;
  for ( i = 0; i < rows; i++)
  {
    if ( strcmp(bodydef[i].ParName, debug_par) == 0) 
    {
      found = 1;
      break;
    }
  }

  if ( !found )
  {
    free((char *) bodydef);
    return  TRA__NOPAR;
  }
  strcpy( object_str, name);
  strcpy( attr_str, debug_par);

  par_type = bodydef[i].Par->Param.Info.Type;
  switch( par_type) {
    case pwr_eType_Boolean:
      *trace_type = flow_eTraceType_Boolean;
      break;
    case pwr_eType_Int32:
      *trace_type = flow_eTraceType_Int32;
      break;
    case pwr_eType_Float32:
      *trace_type = flow_eTraceType_Float32;
      break;
    default:
      *trace_type = flow_eTraceType_Int32;
      break;
  }

  free((char *) bodydef);
  return TRA__SUCCESS ; 
}


/*************************************************************************
*
* Name:		trace_getm4()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Tracemethod for objects handling digital parameters:
*	stodp, setdp, resdp.
*
*	The referenced object and its parameter for a xxxdp is stored
*	this way:
*	- the object to be traced is stored in the parameter Object
*	- the parameter to bre traced in this object is stored in the parameter
*	  "Parameter" in the xxxdp object.
*
**************************************************************************/

static pwr_tStatus trace_get_attr_m4( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{
  pwr_tAName   		aname;
  pwr_tStatus		sts;
  int			size;
  pwr_sAttrRef		*objarp;
  pwr_sAttrRef		objar;
  char			parname[120];
  pwr_sTypeDef 		*tdef;
  pwr_tTid		tid;
  char			*np, *s;

  *inverted = 0;

  /* Get the objdid stored in the parameter Object */
  switch ( node->ln.cid) {
  case pwr_cClass_GetAp:
    strcpy( parname, "ApObject");
    break;
  case pwr_cClass_GetDp:
    strcpy( parname, "DpObject");
    break;
  default:
    strcpy( parname, "Object");
  }
  sts = ldh_GetObjectPar( node->hn.wind->hw.ldhses,  
		node->ln.oid, "DevBody", parname,
		(char **)&objarp, &size); 
  if ( EVEN(sts)) return sts;

  objar = *objarp;
  free((char *) objarp);

  sts = ldh_GetAttrRefTid( node->hn.wind->hw.ldhses,
	&objar, &tid);
  if( EVEN(sts) ) return sts;

  if ( cdh_tidIsCid( tid))
    return TRA__NOPAR;

  /* Get the name of the node */
  sts = ldh_AttrRefToName( node->hn.wind->hw.ldhses,  
			   &objar, cdh_mNName, &np,
			   &size);
  if( EVEN(sts)) return sts;
  strcpy( aname, np);

  s = strrchr( aname, '.');
  if ( !s) return TRA__NOPAR;
  strcpy( attr_str, s + 1);
  *s = 0;
  strcpy( object_str, aname);

  sts = ldh_GetObjectBody( node->hn.wind->hw.ldhses,
			   cdh_TypeIdToObjid(tid), "SysBody", (void **)&tdef, &size);
  if ( EVEN(sts)) return sts;

  switch( tdef->Type) {
    case pwr_eType_Boolean:
      *trace_type = flow_eTraceType_Boolean;
      break;
    case pwr_eType_Int32:
      *trace_type = flow_eTraceType_Int32;
      break;
    case pwr_eType_Float32:
      *trace_type = flow_eTraceType_Float32;
      break;
    default:
      *trace_type = flow_eTraceType_Int32;
      break;
  }
  free((char *) tdef);
  return TRA__SUCCESS;
}

/*************************************************************************
*
* Name:		trace_getm5()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Tracemethod for Disable.
*
*	The method should trace the DisableAttr attribute for the
*       referenced object.
*
**************************************************************************/

static pwr_tStatus trace_get_attr_m5( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{
  pwr_tAName	       	aname;
  pwr_tStatus		sts;
  int			size;
  pwr_sAttrRef		*objarp;
  pwr_sAttrRef		objar, disar;
  char			*np, *s;
  ldh_sAttrRefInfo	info;

  *inverted = 0;

  /* Get the attrref stored in the attribute Object */
  sts = ldh_GetObjectPar( node->hn.wind->hw.ldhses,  
		node->ln.oid, "DevBody", "Object",
		(char **)&objarp, &size); 
  if ( EVEN(sts)) return sts;

  objar = *objarp;
  free((char *) objarp);

  sts = ldh_GetAttrRefInfo( node->hn.wind->hw.ldhses,
	&objar, &info);
  if( EVEN(sts)) return sts;

  if ( !info.flags & PWR_MASK_DISABLEATTR)
    return TRA__NOPAR;

  disar = cdh_ArefToDisableAref( &objar);

  /* Get the name of the node */
  sts = ldh_AttrRefToName( node->hn.wind->hw.ldhses,  
			   &disar, cdh_mNName, &np,
			   &size);
  if( EVEN(sts)) return sts;
  strcpy( aname, np);

  s = strrchr( aname, '.');
  if ( !s) return TRA__NOPAR;

  strcpy( attr_str, s + 1);
  *s = 0;
  strcpy( object_str, aname);

  *trace_type = flow_eTraceType_Boolean;
  return TRA__SUCCESS;
}


/*************************************************************************
*
* Name:		trace_getm7 ()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* tra_ctx	tractx		I	trace context
* tra_t_tranode	*tranode_ptr    O	pointer to a tranode to fill up.
* int		*nb_ptr		IO	irrelevant for this method
*
* Description:
* tracing method for the following objects :
* step , initstep , substep , ssbegin , ssend , order 
* the name of relevant parameter ( in rtdb ) is debugpar in the graphplcnode
* it differs from getm3 by the fact that the param is an array element
* SG 02.06.91 This method is the same as getm3.
* FOR TEST ONLY keeps it to verify that the information in rdb is relevant
* ie that there is a parameter under rt body which name is given by debugpar
**************************************************************************/

static pwr_tStatus trace_get_attr_m7( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{
  pwr_tStatus		sts;
  int			size;
  ldh_sParDef 		*bodydef;
  int 			i;
  int 			rows;
  int			found;
  char 			array_name[32];
  size_t		pos1;
  size_t		pos2;
  size_t		len;
  int 			offset;
  pwr_tOName   		name;
  pwr_eType		par_type;

  *inverted = 0;

  /* The size of the parameter is in the runtime body of the object */

  /* get the name of the object */
  sts = ldh_ObjidToName( node->hn.wind->hw.ldhses, 
    	node->ln.oid, ldh_eName_Hierarchy,
    	name, sizeof(name), &size); 
  if( EVEN(sts) ) return sts;

  sts = ldh_GetObjectBodyDef( node->hn.wind->hw.ldhses,
    	node->ln.cid, "RtBody", 1, &bodydef, &rows);
  if( EVEN(sts) ) return sts;

  /* take away the brackets and get the offset */
  pos1 = strcspn( debug_par, "[");
  pos2 = strcspn( debug_par, "]");
  len = strlen(debug_par);
  offset = debug_par[pos1+1] - '0';
  if (pos1 == len || pos2 == len || pos2 != pos1+2 || offset > 9 || offset < 0)
    return TRA__BADARRAY ;
  else 
  {
    /* the format was ok save the relevent information */
    strcpy( array_name, debug_par);
    array_name[pos1] = '\0';
  }

  found = 0;
  for ( i = 0; i < rows; i++)
  {
    if ( strcmp(bodydef[i].ParName, array_name) == 0  &&
	 bodydef[i].Par->Param.Info.Flags & PWR_MASK_ARRAY)
    {
      found = 1;
      break;
    }
  }
  if ( !found )
  {
    free((char *) bodydef);
    return  TRA__NOPAR;
  }
  strcpy( object_str, name);
  strcpy( attr_str, debug_par);

  par_type = bodydef[i].Par->Param.Info.Type;
  switch( par_type) {
    case pwr_eType_Boolean:
      *trace_type = flow_eTraceType_Boolean;
      break;
    case pwr_eType_Int32:
      *trace_type = flow_eTraceType_Int32;
      break;
    case pwr_eType_Float32:
      *trace_type = flow_eTraceType_Float32;
      break;
    default:
      *trace_type = flow_eTraceType_Int32;
      break;
  }

  free((char *) bodydef);
  return TRA__SUCCESS; 
}


/*************************************************************************
*
* Name:		trace_getm8()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* tra_ctx	tractx		I	trace context
* tra_t_tranode	*tranode_ptr 	I	pointer to the tranode to fill up.
* int		*nb_ptr		IO	pointer to the nb of tra nodes to create.
*					irrelevant for this method
*
* Description:
* Tracemethod for a aanalyseobject which refers to a parameter to another 
* object trough the connection between the analyse node and this other object.
* Notice: This method does not create an additionnal tranode. So nb_ptr  
* is irrelevant.
**************************************************************************/
pwr_tStatus trace_get_attr_con( 	WGre		*gre, 
					vldh_t_con	con,
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type)
{
  vldh_t_node	conn_node;
  pwr_tStatus	sts;
  pwr_tClassId	cid;
  ldh_sParDef 	*bodydef;
  int 		i ,rows;
  int		found;
  pwr_eType	par_type;
  pwr_tObjid	objdid;

  /* Get the object and parameter that is connected to the source object */
  sts = gcg_get_connected_parameter( con->hc.source_node, 
		con->lc.source_point, &conn_node,
		object_str, attr_str);
  if ( EVEN(sts)) return sts;

  /* Get the trace type */
  sts = ldh_NameToObjid( conn_node->hn.wind->hw.ldhses,
		&objdid, object_str);
  sts = ldh_GetObjectClass( conn_node->hn.wind->hw.ldhses,
		objdid, &cid);
  sts = ldh_GetObjectBodyDef( conn_node->hn.wind->hw.ldhses,
	      cid, "RtBody", 1, &bodydef, &rows);
  if( EVEN(sts) ) return sts;

  found = 0;
  for ( i = 0; i < rows; i++)
  {
    if ( strcmp(bodydef[i].ParName, attr_str) == 0) 
    {
      found = 1;
      break;
    }
  }

  if ( !found )
  {
    free((char *) bodydef);
    return  TRA__NOPAR;
  }

  par_type = bodydef[i].Par->Param.Info.Type;
  switch( par_type) {
    case pwr_eType_Boolean:
      *trace_type = flow_eTraceType_Boolean;
      break;
    case pwr_eType_Int32:
      *trace_type = flow_eTraceType_Int32;
      break;
    case pwr_eType_Float32:
      *trace_type = flow_eTraceType_Float32;
      break;
    default:
      *trace_type = flow_eTraceType_Int32;
      break;
  }

  return TRA__SUCCESS;
}


/*************************************************************************
*
* Name:		trace_getm9 ()
*
* Type		int
*
* Description:
* tracing method for the following objects:
* GetAattr, GetDattr, GetIattr, StoDattr, SetDattr, ResDattr
**************************************************************************/

static pwr_tStatus trace_get_attr_m9( 	WGre		*gre, 
					vldh_t_node	node, 
					char		*debug_par,
					char		*object_str, 
					char		*attr_str,
					flow_eTraceType	*trace_type,
					int		*inverted)
{
  pwr_tStatus		sts;
  char			*attribute;
  int			size;

  *inverted = 0;

  /* In class editor, object is always $host */
  strcpy( object_str, "$host");

  /* Get attribute from Attribute */
  sts = ldh_GetObjectPar( node->hn.wind->hw.ldhses,  
		node->ln.oid, "DevBody", "Attribute",
		&attribute, &size); 
  if ( EVEN(sts)) return sts;

  strcpy( attr_str, attribute);
  free((char *) attribute);

  switch( node->ln.cid) {
  case pwr_cClass_GetAattr:
    *trace_type = flow_eTraceType_Float32;
    break;
  case pwr_cClass_GetIattr:
    *trace_type = flow_eTraceType_Int32;
    break;
  default:
    *trace_type = flow_eTraceType_Boolean;
  }
  return TRA__SUCCESS; 
}


pwr_tStatus trace_simsetup( WFoe *foe) 
{
  flow_tCtx ctx = foe->gre->flow_ctx;

  flow_DisableEventAll( ctx);
  flow_EnableEvent( ctx, flow_eEvent_MB1PressCtrl, flow_eEventType_MoveNode, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2Press, flow_eEventType_CreateCon, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2DoubleClick, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShift, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB3Press, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1ClickCtrl, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShiftCtrl, flow_eEventType_CallBack, 
	trace_flow_cb);
  return 1;
}

pwr_tStatus trace_trasetup( WFoe *foe)
{
  flow_tCtx ctx = foe->gre->flow_ctx;

  flow_DisableEventAll( ctx);
  flow_EnableEvent( ctx, flow_eEvent_MB1PressCtrl, flow_eEventType_MoveNode, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2Press, flow_eEventType_CreateCon, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2DoubleClick, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShift, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB3Press, flow_eEventType_CallBack, 
	trace_flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1ClickCtrl, flow_eEventType_CallBack, 
	trace_flow_cb);
  return 1;
}


static int trace_connect_bc( flow_tObject object, char *name, char *attr, 
	flow_eTraceType type, void **p)
{
  pwr_tAName   	attr_str;
  int		size;
  vldh_t_node 	vnode;
  pwr_tSubid	*subid_p, subid;
  int		sts;

  printf( "Connecting %s.%s\n", name, attr);

  if ( strcmp( name, "") == 0 || strcmp( attr, "") == 0)
    return 1;

  switch( type)
  {
    case flow_eTraceType_Boolean:
      size = sizeof(pwr_tBoolean);
      break;
    case flow_eTraceType_Int32:
      size = sizeof(pwr_tInt32);
      break;
    case flow_eTraceType_Float32:
      size = sizeof(pwr_tFloat32);
      break;
    default:
      size = sizeof(pwr_tInt32);
  }

  strcpy( attr_str, name);
  strcat( attr_str, ".");
  strcat( attr_str, attr);  

  if ( flow_GetObjectType( object) == flow_eObjectType_Node)
  {
    if ( flow_GetNodeGroup( object) == flow_eNodeGroup_Trace)
    {
      sts = gdh_RefObjectInfo( attr_str, p, &subid, size);
      if ( EVEN(sts)) return sts;
      
      subid_p = (pwr_tSubid *) calloc( 1, sizeof(pwr_tSubid));
      *subid_p = subid;
      flow_SetUserData( object, (void *) subid_p);
    }
    else
    {
      flow_GetUserData( object, (void **) &vnode);
      sts = gdh_RefObjectInfo( attr_str, p, &vnode->hn.trace_subid, size);
      if ( EVEN(sts)) return sts;
    }
  }
  return 1;
}

static int trace_disconnect_bc( flow_tObject object)
{
  pwr_tSubid	*subid_p;
  vldh_t_node	vnode;
  int 		sts;
  flow_tTraceObj name;
  flow_tTraceAttr attr;
  flow_eTraceType type;
  int		inverted;

  printf( "DisConnecting something...\n");
  if ( flow_GetObjectType( object) == flow_eObjectType_Node)
  {
    if ( flow_GetNodeGroup( object) == flow_eNodeGroup_Trace)
    {
      flow_GetUserData( object, (void **) &subid_p);
      sts = gdh_UnrefObjectInfo( *subid_p);
      free( (char *) subid_p);
    }
    else
    {
      flow_GetTraceAttr( object, name, attr, &type, &inverted);
      if ( !( strcmp( name, "") == 0 || strcmp( attr, "") == 0)) {
        flow_GetUserData( object, (void **) &vnode);
        sts = gdh_UnrefObjectInfo( vnode->hn.trace_subid);
      }
    }
  }
  return 1;
}

int trace_start( WFoe *foe)
{
  static 	int gdh_initialized = 0;
  WGre 	*gre = foe->gre;
  int	  	sts;
  double 	f_width, f_height;

  if ( !gre->trace_started) {
    if ( !gdh_initialized) {
      sts = gdh_Init("wb_trace");
      if (EVEN(sts)) {
        foe->message( "Unable to attach to Proview runtime");
        WFoe::error_msg( sts); 
        return sts;
      }
    }

    gre->set_trace_attributes( 0);

    trace_trasetup( foe);

    flow_ResetHighlightAll( gre->flow_ctx);
    flow_SelectClear( gre->flow_ctx);
    sts = flow_TraceInit( gre->flow_ctx, trace_connect_bc, 
		trace_disconnect_bc, NULL);
    if ( EVEN(sts))
      return sts;


    gre->trace_start();

    /* Create node and con classes for trace */
    if ( !gre->trace_analyse_nc) {
      f_width = 4*GOEN_F_GRID;
      f_height = GOEN_F_GRID;
      flow_CreateNodeClass( gre->flow_ctx, "TraceNode", flow_eNodeGroup_Trace, 
		&gre->trace_analyse_nc);
      flow_AddRect( gre->trace_analyse_nc, 0, 0, f_width, f_height, 
		flow_eDrawType_Line, 1, flow_mDisplayLevel_1);
      flow_AddAnnot( gre->trace_analyse_nc, f_width/8, 0.7*f_height, 0,
		flow_eDrawType_TextHelvetica, 4, flow_eAnnotType_OneLine,
		flow_mDisplayLevel_1);
      flow_AddConPoint( gre->trace_analyse_nc, 0, 0.5*f_height, 0, 
		flow_eDirection_Left);
      flow_AddConPoint( gre->trace_analyse_nc, f_width, 0.5*f_height, 1,
		flow_eDirection_Right);

      flow_CreateConClass( gre->flow_ctx, "TraceCon", 
		flow_eConType_Straight, flow_eCorner_Right,
		flow_eDrawType_Line, 1, 0, 0, 0, flow_eConGroup_Trace,
		&gre->trace_con_cc);	  
    }
  }
  return TRA__SUCCESS;
}

int trace_stop( WFoe *foe)
{
  WGre *gre = foe->gre;

  if ( gre->trace_started) {
    flow_TraceClose( gre->flow_ctx);
    flow_ResetHighlightAll( gre->flow_ctx);
    flow_SelectClear( gre->flow_ctx);
    flow_RemoveTraceObjects( gre->flow_ctx);
    gre->trace_stop();
  }
  return 1;
}

int trace_create_analyse( 	WGre *gre, 
				double x, 
				double y, 
				vldh_t_node source, 
				int source_conpoint)
{
  static int 		idx = 0;
  vldh_t_conobject 	dummy_con;
  flow_tTraceObj      	object_str;
  flow_tTraceAttr      	attr_str;
  flow_eTraceType	trace_type;
  flow_tNode		n1;
  flow_tCon		c1;
  char			name[80];
  int			sts;

  if ( gre->trace_started)
  {
    /* Create a trace object */
    sprintf( name, "Trace%d", idx++);

    dummy_con.hc.source_node = source;
    dummy_con.lc.source_point = source_conpoint;
    sts = trace_get_attr_con( gre, &dummy_con, "", object_str, attr_str, 
		&trace_type);
    if (EVEN(sts)) return sts;

    flow_CreateNode( gre->flow_ctx, name, gre->trace_analyse_nc, 
		x, y, NULL, &n1);
    flow_SetTraceAttr( n1, object_str, attr_str, trace_type, 0);

    flow_CreateCon( gre->flow_ctx, name, gre->trace_con_cc, 
	  	source->hn.node_id, n1, source_conpoint, 0, NULL, &c1,
		0, NULL, NULL);
  }
  return TRA__SUCCESS;
}



/*************************************************************************
*
* Name:		int	trace_flow_cb()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Callback from flow.
**************************************************************************/
static int trace_flow_cb( FlowCtx *ctx, flow_tEvent event)
{
  WGre	*gre;
  vldh_t_node	source, dest;
  void		*vobject;
  double	ll_x, ll_y, ur_x, ur_y, width;
  int		subwindow_nr;

  flow_GetCtxUserData( ctx, (void **)&gre);

  gre->search_rectangle_delete();

  if ( event->any.type == flow_eEventType_CreateCon)
  {
    if ( flow_GetNodeGroup( event->con_create.source_object) == 
		flow_eNodeGroup_Trace)
      return 1;
    flow_GetUserData( event->con_create.source_object, (void **) &source);
    if ( event->con_create.dest_object)
    {
      if ( flow_GetNodeGroup( event->con_create.dest_object) == 
		flow_eNodeGroup_Trace)
        return 1;
      flow_GetUserData( event->con_create.dest_object, (void **) &dest);
    }
    else
      dest = 0;
    (gre->gre_con_created) (gre, 
		event->con_create.x, event->con_create.y, 
		source, event->con_create.source_conpoint,
		dest, event->con_create.dest_conpoint);	
  }
  switch ( event->event)
  {
    case flow_eEvent_Init:
      break;
    case flow_eEvent_MB2DoubleClick:
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
	  if ( flow_GetNodeGroup( event->object.object) == 
		flow_eNodeGroup_Trace)
	  {
	    flow_DeleteNodeCons( event->object.object);
	    flow_DeleteNode( event->object.object);
	  }
          break;
        default:
          ;
      }
      break;
    case flow_eEvent_MB1PressCtrl:
    {
      vldh_t_node	vnode;
      vldh_t_con	vcon;
      double		pos_x, pos_y;
      double		*x_arr, *y_arr;
      int		i, num;

      /* Object moved */
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
	  if ( flow_GetNodeGroup( event->object.object) == 
		flow_eNodeGroup_Trace)
	    break;
          flow_GetUserData( event->object.object, (void **) &vnode);
	  flow_GetNodePosition( event->object.object, &pos_x, &pos_y);
	  vnode->ln.x = pos_x;
	  vnode->ln.y = pos_y;
	  vldh_nodemodified( vnode);

	  (gre->gre_node_moved) (gre);
          break;
        case flow_eObjectType_Con:
	  if ( flow_GetConGroup( event->object.object) == 
		flow_eConGroup_Trace)
	    break;
          flow_GetUserData( event->object.object, (void **) &vcon);
	  flow_GetConPosition( event->object.object, &x_arr, &y_arr, &num);
	  for ( i = 0; i < num; i++)
	  {
	    vcon->lc.point[i].x = x_arr[i];
	    vcon->lc.point[i].y = y_arr[i];
	  }
	  vcon->lc.point_count = num;
	  vldh_conmodified( vcon);
          break;
        default:
          ;
      }
      break;
    }
    case flow_eEvent_MB1DoubleClick:
      /* Open attribute editor */
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
	  if ( flow_GetNodeGroup( event->object.object) == 
		flow_eNodeGroup_Trace)
	    break;
          gre->popupmenu_mode = GRE_POPUPMENUMODE_OBJECT;
          flow_GetUserData( event->object.object, &vobject);
	  (gre->gre_attribute) (gre, (vldh_t_node) vobject);
          break;
        default:
          ;
      }
      break;
    case flow_eEvent_MB1DoubleClickShift:
      /* Open subwindow */
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
	  if ( flow_GetNodeGroup( event->object.object) == 
		flow_eNodeGroup_Trace)
	    break;
	  gre->popupmenu_mode = GRE_POPUPMENUMODE_OBJECT;
	  flow_MeasureNode( event->object.object, &ll_x,&ll_y,&ur_x,&ur_y);
          width = ur_x - ll_x;
	  if ( event->object.x > ( ll_x + width / 2 ))	
	    subwindow_nr = 1;
	  else				
	    subwindow_nr = 2;
          flow_GetUserData( event->object.object, &vobject);
	  (gre->gre_subwindow) ( gre, (vldh_t_node)vobject, subwindow_nr);
          break;
        default:
          ;
      }
      break;
    case flow_eEvent_MB3Press:
    {
      /* Popup menu */
      int		x_pix, y_pix;
      vldh_t_node	current_node;
      flow_tObject	*select_list;
      int		select_cnt;

      if ( event->object.object_type != flow_eObjectType_Node)
        break;
      if ( flow_GetNodeGroup( event->object.object) == flow_eNodeGroup_Trace)
        break;

      flow_PositionToPixel( gre->flow_ctx, event->object.x,
			event->object.y, &x_pix, &y_pix); 
	  
      gre->get_popup_position( &x_pix, &y_pix);

      current_node = 0;

      /* If there is no selected object, select current object */
      if ( gre->popupmenu_mode == GRE_POPUPMENUMODE_OBJECT)
      {
        flow_GetSelectList( ctx, &select_list, &select_cnt);
        if ( !select_cnt)
        {
          if ( event->object.object_type == flow_eObjectType_Node)
            /* Get the current object */
            flow_GetUserData( event->object.object, (void **)&current_node);
        }	
      }

      (gre->gre_popupmenu) (gre, x_pix, y_pix, 
			gre->popupmenu_mode, current_node);
      break;
    }
    case flow_eEvent_MB1ClickCtrl:
    {
      char			help_title[32];
      vldh_t_node		node;
      vldh_t_con		con;
      int			size, sts;

      if ( event->object.object_type == flow_eObjectType_Node)
      {
	if ( flow_GetNodeGroup( event->object.object) == 
		flow_eNodeGroup_Trace)
	  break;
        flow_GetUserData( event->object.object, (void **)&node);
	sts = ldh_ObjidToName(
		(node->hn.wind)->hw.ldhses,
	         cdh_ClassIdToObjid( node->ln.cid), ldh_eName_Object,
		 help_title, sizeof( help_title), &size);
	WFoe::error_msg(sts);
	if ( EVEN(sts)) return 1;
	(gre->gre_help) ( gre, help_title);
      }
      if ( event->object.object_type == flow_eObjectType_Con)
      {
	if ( flow_GetConGroup( event->object.object) == 
		flow_eConGroup_Trace)
	  break;
        flow_GetUserData( event->object.object, (void **)&con);
	sts = ldh_ObjidToName( 
			(gre->wind)->hw.ldhses, 
	         	cdh_ClassIdToObjid( con->lc.cid), ldh_eName_Object,
		        help_title, sizeof( help_title), &size);
	WFoe::error_msg(sts);
	if ( EVEN(sts)) return 1;
	(gre->gre_help) ( gre, help_title);
      }
      break;
    }
    case flow_eEvent_MB1DoubleClickShiftCtrl:
    {
      trace_changevalue( gre, event->object.object);
      break;
    }
    default:
      ;
  }
  return 1;
}

int	trace_save( WGre *gre)
{
  pwr_tFileName filename;
  int sts;

  if (!gre->trace_started)
    return 0;
  sprintf( filename, "pwrp_exe:pwr_%s.trc",
		vldh_IdToStr(0, gre->wind->lw.oid));
  sts = flow_SaveTrace( gre->flow_ctx, filename);  
  return sts;
}

int	trace_restore( WGre *gre)
{
  pwr_tFileName filename;
  int sts;

  if (!gre->trace_started)
    return 0;
  sprintf( filename, "pwrp_exe:pwr_%s.trc",
		vldh_IdToStr(0, gre->wind->lw.oid));
  sts = flow_OpenTrace( gre->flow_ctx, filename);
  return sts;
}


static void trace_changevalue (
    WGre	    *gre,
    flow_tNode	    fnode
)
{
  WFoe		*foe;
  ldh_tSesContext	ldhses;
  pwr_tStatus 		sts;
  char			name[200];
  pwr_tBoolean		value;
  flow_tTraceObj       	object_str;
  flow_tTraceAttr      	attr_str;
  flow_eTraceType	trace_type;
  int			inverted;

  foe = (WFoe *)gre->parent_ctx;
  ldhses = (gre->wind)->hw.ldhses ;

  /* take away the old messages */
  if ( foe->msg_label_id != 0 ) foe->message( "");

  if ( flow_GetNodeGroup( fnode) == flow_eNodeGroup_Trace)
  {
    gre->trace_changenode = fnode;

    /* Get a value */
    foe->get_textinput( "Enter value : ", &trace_aanalyse_set_value);
    return;
  }
  else
  {	    
    /* Toggle the value, start to get the current value */
    flow_GetTraceAttr( fnode, object_str, attr_str, &trace_type, &inverted);
    strcpy( name, object_str);
    strcat( name, ".");
    strcat( name, attr_str);
    switch ( trace_type)
    {
      case flow_eTraceType_Boolean:
        sts = gdh_GetObjectInfo( name, &value, sizeof(value)); 
        if (EVEN(sts))
        {
          foe->message( "Unable to set value");
          WFoe::error_msg( sts); 
          return;
        }

        /* Toggle the value */
        if ( value == 0)
          value = 1;
        else
          value = 0;

        sts = gdh_SetObjectInfo( name, &value, sizeof(value));
        if (EVEN(sts))
        {
          foe->message( "Unable to set value");
          WFoe::error_msg( sts); 
          return;
         }
         break;
       default:
        foe->message( "Unable to toggle value");
    }
  }
}

/*************************************************************************
*
* Name:		trace_aanalyse_set_value ()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
**************************************************************************/

static pwr_tStatus	trace_aanalyse_set_value(
    WFoe *foe,
    char	*valuestr
)
{
  ldh_tSesContext	ldhses;
  pwr_tStatus 		sts;
  char			name[200];
  pwr_tBoolean		boolean_value;
  pwr_tFloat32		float_value;
  flow_tTraceObj       	object_str;
  flow_tTraceAttr      	attr_str;
  flow_eTraceType	trace_type;
  int			inverted;

  ldhses = ((foe->gre)->wind)->hw.ldhses ;

  flow_GetTraceAttr( foe->gre->trace_changenode, object_str, attr_str, &trace_type,
		     &inverted);
  strcpy( name, object_str);
  strcat( name, ".");
  strcat( name, attr_str);
  switch ( trace_type)
  {
    case flow_eTraceType_Boolean:
      /* Convert to Boolean */
      if ( sscanf( valuestr, "%d", &boolean_value) != 1)
      {
        foe->message( "Syntax error");
        return 0;
      }

      sts = gdh_SetObjectInfo( name, &boolean_value, sizeof(boolean_value));
      if (EVEN(sts))
      {
        foe->message( "Unable to set value");
        WFoe::error_msg( sts); 
        return 1;
       }
       break;
    case flow_eTraceType_Float32:
      /* Convert to float */
      if ( sscanf( valuestr, "%f", &float_value) != 1)
      {
        foe->message( "Syntax error");
        return 0;
      }

      sts = gdh_SetObjectInfo( name, &float_value, sizeof(float_value));
      if (EVEN(sts))
      {
        foe->message( "Unable to set value");
        WFoe::error_msg( sts); 
        return 1;
       }
       break;
     default:
      foe->message( "Unable to set value");
  }
  return 1;
}
