/* 
 * Proview   $Id$
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

/* rt_io_m_velleman_k8055_board.c -- I/O methods for class Velleman_K8055_Board. */

#include "pwr.h"
#include "pwr_basecomponentclasses.h"
#include "pwr_otherioclasses.h"
#include "co_time.h"
#include "rt_io_base.h"
#include "rt_io_card_init.h"
#include "rt_io_card_close.h"
#include "rt_io_card_read.h"
#include "rt_io_card_write.h"
#include "rt_io_msg.h"

#if defined PWRE_CONF_LIBUSB

#include <libusb-1.0/libusb.h>
#include "rt_io_m_usb_agent.h"
#include "rt_io_m_velleman_k8055_board.h"


static pwr_tStatus IoCardInit( io_tCtx ctx,
			       io_sAgent *ap,
			       io_sRack *rp,
			       io_sCard *cp)
{
  ssize_t devnum;
  libusb_device **list;
  libusb_device *device = 0;
  int found = 0;
  int i;
  io_sLocalUSB_Agent *local_agent = (io_sLocalUSB_Agent *)ap->Local;
  io_sLocal_K8055 *local;
  pwr_sClass_Velleman_K8055_Board *op = (pwr_sClass_Velleman_K8055_Board *)cp->op;
  int sts;

  if ( !local_agent->libusb_ctx)
    return IO__INITFAIL;

  devnum = libusb_get_device_list( local_agent->libusb_ctx, &list);
  if ( devnum > 0) {

    for ( i = 0; i < devnum; i++) {
      struct libusb_device_descriptor desc;

      if ( libusb_get_device_descriptor( list[i], &desc) != 0)
	continue;

      if ( desc.idVendor == 0x10cf &&
	   desc.idProduct == 0x5500 + op->Super.Address) {
	device = list[i];
	found = 1;
	break;
      }
    }
  }

  if ( !found) {
    errh_Error( "Init Velleman K8055, device not found '%s'", cp->Name);
    op->Status = IO__NODEVICE;
    ((pwr_sClass_Velleman_K8055 *)rp->op)->Status = op->Status;
    return IO__INITFAIL;
  }
  
  local = (io_sLocal_K8055 *) calloc( 1, sizeof(io_sLocal_K8055));
  cp->Local = local;

  sts = libusb_open( device, &local->libusb_device);
  if ( sts != 0) {
    op->Status = IO__INITFAIL;
    ((pwr_sClass_Velleman_K8055 *)rp->op)->Status = op->Status;
    local->libusb_device = 0;
    return IO__INITFAIL;
  }

  if ( libusb_kernel_driver_active( local->libusb_device, 0) != 0)
    libusb_detach_kernel_driver( local->libusb_device, 0);

  sts = libusb_claim_interface( local->libusb_device, 0);
  if ( sts < 0) {
    errh_Error( "K8055 Claim interface failed, sts %d, '%s'", sts, ap->Name);
    op->Status = IO__INITFAIL;
    ((pwr_sClass_Velleman_K8055 *)rp->op)->Status = op->Status;
    return IO__INITFAIL;
  }


  for ( i = 0; i < 2; i++) {
    if ( cp->chanlist[i].sop)
      io_AiRangeToCoef( &cp->chanlist[i]);
  }
  for ( i = 0; i < 2; i++) {
    if ( cp->chanlist[i+7].sop)
      io_AoRangeToCoef( &cp->chanlist[i+7]);
  }


  errh_Info( "Init of Velleman K8055 '%s'", cp->Name);
  op->Status = IO__SUCCESS;

  // Rack has no methods, set status
  if ( ((pwr_sClass_Velleman_K8055 *)rp->op)->Status == 0)
    ((pwr_sClass_Velleman_K8055 *)rp->op)->Status = op->Status;

  return IO__SUCCESS;
}

static pwr_tStatus IoCardClose( io_tCtx ctx,
			        io_sAgent *ap,
			        io_sRack *rp,
			        io_sCard *cp)
{
  io_sLocalUSB_Agent *local_agent = (io_sLocalUSB_Agent *)ap->Local;
  io_sLocal_K8055 *local = (io_sLocal_K8055 *)cp->Local;

  if ( !local_agent->libusb_ctx)
    return IO__SUCCESS;

  if ( local->libusb_device)
    libusb_close( local->libusb_device);

  if ( cp->Local)
    free( cp->Local);

  return IO__SUCCESS;
}

static pwr_tStatus IoCardRead( io_tCtx ctx,
			       io_sAgent *ap,
			       io_sRack	*rp,
			       io_sCard	*cp)
{
  io_sLocal_K8055 *local = (io_sLocal_K8055 *)cp->Local;
  pwr_sClass_Velleman_K8055_Board *op = (pwr_sClass_Velleman_K8055_Board *)cp->op;
  unsigned char data[9];
  char endpoint = 0x81;
  int size = 8;
  int tsize;
  unsigned char m;
  int sts;
  int i;
  pwr_tUInt32 error_count = op->Super.ErrorCount;

  // You have to read twice to get the latest ?????
  sts = libusb_interrupt_transfer( local->libusb_device, endpoint, data, 8, &tsize, 20);
  sts = libusb_interrupt_transfer( local->libusb_device, endpoint, data, 8, &tsize, 20);
  if ( sts != 0 || tsize != size) {
    op->Super.ErrorCount++;
    return IO__SUCCESS;
  }
  else {

    // Handle Ai
    for ( i = 0; i < 2; i++) {
      if ( cp->chanlist[i].sop) {
	io_sChannel *chanp = &cp->chanlist[i];
	pwr_sClass_ChanAi *cop = (pwr_sClass_ChanAi *)chanp->cop;
	pwr_sClass_Ai *sop = (pwr_sClass_Ai *)chanp->sop;
	pwr_tFloat32 actvalue;
	int ivalue = data[i+2];

	if ( cop->CalculateNewCoef)
	  // Request to calculate new coefficients
	  io_AiRangeToCoef( chanp);

	io_ConvertAi( cop, ivalue, &actvalue);

	// Filter
	if ( sop->FilterType == 1 &&
	     sop->FilterAttribute[0] > 0 &&
	     sop->FilterAttribute[0] > ctx->ScanTime) {
	  actvalue = *(pwr_tFloat32 *)chanp->vbp + ctx->ScanTime / sop->FilterAttribute[0] *
	    (actvalue - *(pwr_tFloat32 *)chanp->vbp);
	}

	*(pwr_tFloat32 *)chanp->vbp = actvalue;
	sop->SigValue = cop->SigValPolyCoef1 * ivalue + cop->SigValPolyCoef0;
	sop->RawValue = ivalue;
      }
    }

    // Handle Di
    for ( i = 0; i < 5; i++) {
      switch ( i) {
      case 0: m = 16; break;
      case 1: m = 32; break;
      case 2: m = 1; break;
      case 3: m = 64; break;
      case 4: m = 128; break;
      }
      if ( cp->chanlist[i+2].sop)
	*(pwr_tBoolean *)cp->chanlist[i+2].vbp = ((data[0] & m) != 0);
    }
  }

  if ( op->Super.ErrorCount >= op->Super.ErrorSoftLimit && 
       error_count < op->Super.ErrorSoftLimit) {
    errh_Warning( "IO Card ErrorSoftLimit reached, '%s'", cp->Name);
  }
  if ( op->Super.ErrorCount >= op->Super.ErrorHardLimit) {
    errh_Error( "IO Card ErrorHardLimit reached '%s', IO stopped", cp->Name);
    ctx->Node->EmergBreakTrue = 1;
    return IO__ERRDEVICE;
  }    

  return IO__SUCCESS;
}


static pwr_tStatus IoCardWrite( io_tCtx ctx,
			       io_sAgent *ap,
			       io_sRack	*rp,
			       io_sCard	*cp)
{
  io_sLocal_K8055 *local = (io_sLocal_K8055 *)cp->Local;
  pwr_sClass_Velleman_K8055_Board *op = (pwr_sClass_Velleman_K8055_Board *)cp->op;
  unsigned char data[9];
  char endpoint = 0x1;
  int size = 8;
  int tsize;
  unsigned char m;
  int i;
  int sts;
  pwr_tUInt32 error_count = op->Super.ErrorCount;

  memset( data, 0, sizeof(data));
  data[0] = 0x5;

  // Handle Do
  m = 1;
  unsigned char do_value = 0;
  for ( i = 0; i < 8; i++) {
    if ( cp->chanlist[i+9].sop) {
      if ( *(pwr_tBoolean *)cp->chanlist[i+9].vbp)
	do_value |= m;
    }
    m = m << 1;
  }
  data[1] = do_value;

  // Handle Ao
  for ( i = 0; i < 2; i++) {
    if ( cp->chanlist[i+7].sop) {
      io_sChannel *chanp = &cp->chanlist[i+7];
      pwr_sClass_ChanAo *cop = (pwr_sClass_ChanAo *)chanp->cop;


      if ( cop->CalculateNewCoef)
	// Request to calculate new coefficients
	io_AoRangeToCoef( chanp);

      float fvalue = *(pwr_tFloat32 *)chanp->vbp * cop->OutPolyCoef1 + cop->OutPolyCoef0;
      int ivalue = (int)fvalue;
      if ( ivalue < 0)
	ivalue = 0;
      else if (ivalue > 255)
	ivalue = 255;

      data[i+2] = ivalue;
    }
  }

  sts = libusb_interrupt_transfer( local->libusb_device, endpoint, data, size, &tsize, 20);
  if ( sts != 0 || tsize != size) {
    op->Super.ErrorCount++;
    return IO__SUCCESS;
  }

  if ( op->Super.ErrorCount >= op->Super.ErrorSoftLimit && 
       error_count < op->Super.ErrorSoftLimit) {
    errh_Warning( "IO Card ErrorSoftLimit reached, '%s'", cp->Name);
  }
  if ( op->Super.ErrorCount >= op->Super.ErrorHardLimit) {
    errh_Error( "IO Card ErrorHardLimit reached '%s', IO stopped", cp->Name);
    ctx->Node->EmergBreakTrue = 1;
    return IO__ERRDEVICE;
  }    

  return IO__SUCCESS;
}

#else
static pwr_tStatus IoCardInit( io_tCtx ctx,io_sAgent *ap,io_sRack *rp,io_sCard *cp) {return 0;}
static pwr_tStatus IoCardClose( io_tCtx ctx,io_sAgent *ap,io_sRack *rp,io_sCard *cp) {return 0;}
static pwr_tStatus IoCardRead( io_tCtx ctx,io_sAgent *ap,io_sRack *rp,io_sCard *cp) {return 0;}
static pwr_tStatus IoCardWrite( io_tCtx ctx,io_sAgent *ap,io_sRack *rp,io_sCard *cp) {return 0;}
#endif

/*  Every method should be registred here. */

pwr_dExport pwr_BindIoMethods(Velleman_K8055_Board) = {
  pwr_BindIoMethod(IoCardInit),
  pwr_BindIoMethod(IoCardClose),
  pwr_BindIoMethod(IoCardRead),
  pwr_BindIoMethod(IoCardWrite),
  pwr_NullMethod
};