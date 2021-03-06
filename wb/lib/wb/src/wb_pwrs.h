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

#ifndef wb_pwrs_h
#define wb_pwrs_h

/* wb_pwrs.h -- common include file for work bench methods. */

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif

#ifndef wb_ldh_h
#include "wb_ldh.h"
#endif

#ifndef NULL
#define NULL (void *) 0
#endif

#define pwrs_cSmdVersionStr "V2.7.0"

typedef struct {
  pwr_tObjName MethodName;
  pwr_tStatus (*Method)();
} pwr_sMethodBinding;

typedef struct {
  pwr_tObjName ClassName;
  pwr_sMethodBinding (*Methods)[];
} pwr_sClassBinding;

#if 1 //defined OS_LINUX
#define pwr_BindMethods(Class) pwr_sMethodBinding pwr_g ## Class ## _Methods[]
#define pwr_BindClasses(Type) pwr_sClassBinding pwr_g ## Type ## _ClassMethods[]
#define pwr_BindClass(Class) {#Class, (pwr_sMethodBinding(*)[]) pwr_g ## Class ## _Methods}
#define pwr_BindMethod(Method) {#Method, (pwr_tStatus (*)())Method}
#else
#error "Wrong OS"
#define pwr_BindMethods(Class) pwr_sMethodBinding pwr_g/**/Class/**/_Methods[]
#define pwr_BindClasses(Type) pwr_sClassBinding pwr_g/**/Type/**/_ClassMethods[]
#define pwr_BindClass(Class) {"Class", pwr_g/**/Class/**/_Methods}
#define pwr_BindMethod(Method) {"Method", (pwr_tStatus (*)())Method}
#endif


#define pwr_NullMethod {"", NULL}

#define pwr_NullClass {"", NULL}

/*----------------------------------------------------------------------------*\
  Prototypes for DbCallbacks
\*----------------------------------------------------------------------------*/

typedef pwr_tStatus (* wb_tMethodMenu)( ldh_sMenuCall *);
typedef pwr_tStatus (* wb_tMethodMenuFilter) (ldh_sMenuCall *, pwr_sMenuButton *);
typedef pwr_tStatus (* wb_tMethodAnteCreate) (ldh_tSesContext, pwr_tOid, pwr_tCid);
typedef pwr_tStatus (* wb_tMethodPostCreate) (ldh_tSesContext, pwr_tOid, pwr_tOid, pwr_tCid);
typedef pwr_tStatus (* wb_tMethodAnteMove)   (ldh_tSesContext, pwr_tOid, pwr_tOid, pwr_tOid);
typedef pwr_tStatus (* wb_tMethodPostMove)   (ldh_tSesContext, pwr_tOid, pwr_tOid, pwr_tCid);
typedef pwr_tStatus (* wb_tMethodAnteAdopt)  (ldh_tSesContext, pwr_tOid, pwr_tCid);
typedef pwr_tStatus (* wb_tMethodPostAdopt)  (ldh_tSesContext, pwr_tOid, pwr_tOid, pwr_tCid);
typedef pwr_tStatus (* wb_tMethodAnteUnadopt)(ldh_tSesContext, pwr_tOid, pwr_tOid, pwr_tCid);
typedef pwr_tStatus (* wb_tMethodPostUnadopt)(ldh_tSesContext, pwr_tOid, pwr_tOid, pwr_tCid);
typedef pwr_tStatus (* wb_tMethodPostRename) (ldh_tSesContext, pwr_tOid);
typedef pwr_tStatus (* wb_tMethodSyntaxCheck)(ldh_tSesContext, pwr_tAttrRef, int *, int *);
typedef pwr_tStatus (* wb_tMethodGetIoDeviceData)(pwr_tAttrRef, const char *, char *, int);
typedef pwr_tStatus (* wb_tMethodSetIoDeviceData)(pwr_tAttrRef, const char *, const char *);


#if defined OS_VMS
static pwr_tStatus AnteAdopt (
  ldh_tSesContext   Session,
  pwr_tObjid	    Father,   /* object about to adopt a new child */
  pwr_tClassId	    Class     /* class of new child */
);
static pwr_tStatus AnteCreate (
  ldh_tSesContext   Session,
  pwr_tObjid	    Father,   /* father of object to be created */
  pwr_tClassId	    Class     /* class of new child */
);
static pwr_tStatus AnteMove (
  ldh_tSesContext   Session,
  pwr_tObjid	    Object,   /* object about to be moved */
  pwr_tObjid	    Father,   /* new father */
  pwr_tObjid	    OldFather /* old father */
);
static pwr_tStatus AnteUnadopt (
  ldh_tSesContext   Session,
  pwr_tObjid	    Father,   /* object about to unadopt a child */
  pwr_tObjid	    Child,    /* child object to be unadopted */
  pwr_tClassId	    Class     /* class of child */
);
static pwr_tStatus PostAdopt (
  ldh_tSesContext   Session,
  pwr_tObjid	    Father,   /* father of adopted child */
  pwr_tObjid	    Child,    /* adopted child */
  pwr_tClassId	    Class     /* class of adopted child */
);
static pwr_tStatus PostCreate (
  ldh_tSesContext   Session,
  pwr_tObjid	    Object,   /* created object */
  pwr_tObjid	    Father,    /* father of created object */
  pwr_tClassId	    Class     /* class of father */
);
static pwr_tStatus PostMove (
  ldh_tSesContext   Session,
  pwr_tObjid	    Object,   /* moved object */
  pwr_tObjid	    Father,   /* new father */
  pwr_tClassId	    Class     /* class of father */
);
static pwr_tStatus PostUnadopt (
  ldh_tSesContext   Session,
  pwr_tObjid	    Father,   /* father of unadopted child */
  pwr_tObjid	    Child,    /* unadopted child */
  pwr_tClassId	    Class     /* class of child */
);
static pwr_tStatus SyntaxCheck (
  ldh_tSesContext   Session,
  pwr_tAttrRef	    Object,    /* object to check */
  int 		    *ErrorCount, /* accumulated error count */
  int               *WarningCount /* accumulated warning count */ 
);
#endif
#endif












