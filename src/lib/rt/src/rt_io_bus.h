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

#ifndef rt_io_bus_h
#define rt_io_bus_h

/* rt_io_bus.h -- includefile for io bus. */

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif

#ifndef PWR_BASECLASSES_H
#include "pwr_baseclasses.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef rt_io_supervise_h
#include "rt_io_supervise.h"
#endif

#ifndef rt_io_methods_h
#include "rt_io_methods.h"
#endif

#ifndef NULL
#define NULL (void *) 0
#endif

#if defined OS_OPENBSD
# ifdef swap16
#  undef swap16
# endif
# ifdef swap32
#  undef swap32
# endif
#endif

typedef enum {
  io_eAlignment_Packed,
  io_eAlignment_Powerlink
} io_eAlignment;

/*----------------------------------------------------------------------------*\
  Io functions
\*----------------------------------------------------------------------------*/


int is_diag( pwr_tAttrRef *aref);

pwr_tInt32 GetChanSize(pwr_eDataRepEnum rep);

unsigned short swap16(unsigned short in);

unsigned int swap32(unsigned int in);

pwr_tStatus io_bus_card_init( io_tCtx ctx,
			      io_sCard *cp, 
			      unsigned int *input_area_offset, 
			      unsigned int *input_area_chansize, 
			      unsigned int *output_area_offset, 
			      unsigned int *output_area_chansize, 
			      pwr_tByteOrderingEnum byte_order,
			      io_eAlignment alignment);

void io_bus_card_read( io_tCtx ctx,
		       io_sRack *rp, 
		       io_sCard *cp, 
		       void *input_area, 
		       void *diag_area,
		       pwr_tByteOrderingEnum byte_order,
		       pwr_tFloatRepEnum float_rep);

void io_bus_card_write( io_tCtx ctx,
			io_sCard *cp, 
			void *output_area, 
			pwr_tByteOrderingEnum byte_order,
			pwr_tFloatRepEnum float_rep);

#ifdef __cplusplus
}
#endif

#endif
