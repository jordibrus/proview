/* 
 * Proview   $Id: wb_c_dsup.cpp,v 1.1 2007-01-04 07:29:03 claes Exp $
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

/* wb_c_dsup.c -- work bench methods of the DSup class. */

#include <string.h>
#include "wb_pwrs.h"
#include "wb_ldh_msg.h"
#include "wb_pwrb_msg.h"
#include "pwr_baseclasses.h"
#include "wb_ldh.h"


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/

static pwr_tStatus PostCreate (
  ldh_tSesContext Session,
  pwr_tObjid	  Object,
  pwr_tObjid	  Father,
  pwr_tClassId	  Class
) {
  pwr_tStatus sts;
  int size;
  pwr_tAName Name;
  pwr_sAttrRef Attribute;
  
  /*
    If father of DSup has an "ActualValue" attribute, then make this DSup
    refer to this attribute.
  */

  sts = ldh_ObjidToName(Session, Father, ldh_eName_Hierarchy, Name,
    sizeof(Name), &size);
  if (EVEN(sts)) return PWRB__SUCCESS;

  strcat(Name, ".ActualValue");

  sts = ldh_NameToAttrRef(Session, Name, &Attribute);
  if (EVEN(sts)) {
    memset(&Attribute, 0, sizeof(Attribute));
  }

  sts = ldh_SetObjectPar(Session, Object, "RtBody", "Attribute", (char *)&Attribute,
    sizeof(Attribute));
  if (EVEN(sts)) return PWRB__SUCCESS;

  return PWRB__SUCCESS;
}

/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/

static pwr_tStatus PostMove (
  ldh_tSesContext Session,
  pwr_tObjid	  Object,
  pwr_tObjid	  Father,
  pwr_tClassId	  Class
) {
  pwr_tStatus sts;
  int size;
  pwr_tAName Name;
  pwr_sAttrRef Attribute;
  
  /*
    If father of DSup has an "ActualValue" attribute, then make this DSup
    refer to this attribute.
  */

  sts = ldh_ObjidToName(Session, Father, ldh_eName_Hierarchy, Name,
    sizeof(Name), &size);
  if (EVEN(sts)) return PWRB__SUCCESS;

  strcat(Name, ".ActualValue");

  sts = ldh_NameToAttrRef(Session, Name, &Attribute);
  if (EVEN(sts)) {
    memset(&Attribute, 0, sizeof(Attribute));
  }

  sts = ldh_SetObjectPar(Session, Object, "RtBody", "Attribute", (char *)&Attribute,
    sizeof(Attribute));
  if (EVEN(sts)) return PWRB__SUCCESS;

  return PWRB__SUCCESS;
}

/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindMethods(DSup) = {
  pwr_BindMethod(PostCreate),
  pwr_BindMethod(PostMove),
  pwr_NullMethod
};