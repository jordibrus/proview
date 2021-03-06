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

/* rt_plc_loop.c -- PLC Loop Routines
   Link options for VMS
     AXP:   /sysexe or alpha$loadable_images:sys$base_image/share
     VAX    sys$system:sys.stb/selective_search  */

#if defined OS_VMS
#include <starlet.h>
#include <lib$routines.h>
#endif

#if defined OS_ELN
#include libdef
#endif

#include "rt_plc_loop.h"

extern void plc_ConvMSToLI(pwr_tUInt32 Time, pwr_tVaxTime *TimeLI);


#ifdef OS_VMS
static int GetVmsUpTime (unsigned int *Seconds, unsigned int *Ticks)
{
  extern unsigned int EXE$GL_ABSTIM;
  extern unsigned int EXE$GL_ABSTIM_TICS;

  *Seconds =  EXE$GL_ABSTIM;
  *Ticks =    EXE$GL_ABSTIM_TICS;

   return 1;
}
#endif

#ifdef OS_ELN
/*
 * plc_LoopInit()
 *
 * Description: T
 *  Gets the Uptime which will be used as NextTime the first time
 *  plc_LoopWait is called. 
 *
 */ 
pwr_tStatus plc_LoopInit (pwr_tVaxTime *NextTime)
{
    pwr_tStatus sts;

    ker$get_uptime(&sts, NextTime);
    return sts;
}

/*
 * plc_LoopWait()
 *
 * Description:
 *  Returns FALSE if a slip is detected.
 */ 
pwr_tBoolean plc_LoopWait (
    int		*WaitResult,     /* Set to 1 when Event */
    pwr_tVaxTime	*DeltaTime,
    pwr_tVaxTime	*NextTime,
    unsigned long Event
)
{
    pwr_tStatus sts;
    pwr_tVaxTime NextLoop, UpTime, DiffTime;
    pwr_tBoolean Result;

    sts = lib$add_times(NextTime, DeltaTime, &NextLoop);
    *NextTime = NextLoop;
    ker$get_uptime(&sts, &UpTime);
    
    sts = lib$sub_times(NextTime, &UpTime, &DiffTime);
    if (sts == LIB$_NEGTIM) 
	Result = FALSE; /* Slip */
    else  if (NextTime->high == UpTime.high && NextTime->low == UpTime.low)
	Result = TRUE; /* Equal Times */
    else {
	if (Event)
	    ker$wait_any(&sts, WaitResult, &DiffTime, Event);
	else
	    ker$wait_any(&sts, WaitResult, &DiffTime);
	Result = TRUE;
    }	

    return Result;
}
#endif

#ifdef OS_VMS
/*
 * plc_LoopGetVmsUpTime()
 *
 * Description: 
 *  Gets the VMS uptime
 *
 */ 
pwr_tStatus plc_LoopGetVmsUpTime (
  unsigned int *Seconds,	 
  unsigned int *Ticks	/* 1 tick = 10 ms */
) {
  void *argv[3];
  pwr_tStatus sts;

  argv[0] = (void *) 2;
  argv[1] = Seconds;
  argv[2] = Ticks;
  sts = sys$cmexec(&GetVmsUpTime, argv);

  return sts;
}
/*
 * plc_LoopInit()
 *
 * Description: 
 *  Inits the plc_sLoopWait struct
 *  This routine must be called before plc_LoopWait
 *
 */ 
pwr_tStatus plc_LoopInit (
  plc_sLoopWait *p
) {
  pwr_tStatus sts;
  unsigned int Seconds;

  sts = lib$get_ef(&p->TimerFlag);
  if (EVEN(sts))
    return sts;

  sts = plc_LoopGetVmsUpTime(&Seconds, &p->LastTick);
  p->LastLoop = p->LastTick;
  p->LoopOflw = 0;
  p->TickOflw = 0;

  return sts;
}

/*
 * plc_LoopWait()
 *
 * Description:
 *  Returns False if a slip is detected.
 *  The plc_sLoopWait structure must be initialized with a call to
 *  plc_LoopInit.
 */ 
pwr_tBoolean plc_LoopWait (
  plc_sLoopWait	  *p,
  unsigned int	  DeltaTime /* ms */
) {
  unsigned int Next, Tick, Seconds;
  pwr_tVaxTime    WaitTime;
  pwr_tUInt32  Diff;

  Next = p->LastLoop + DeltaTime / 10;
  if (Next < p->LastLoop)
    p->LoopOflw = 1;
  p->LastLoop = Next;
  
  plc_LoopGetVmsUpTime (&Seconds, &Tick);
  if (Tick < p->LastTick)
    p->TickOflw = 1;
  p->LastTick = Tick;

  /* Reset overflow flags if both flags have been set */
  if (p->LoopOflw && p->TickOflw)
    p->LoopOflw = p->TickOflw = 0; 
  

  /* Return if there is a slip */ 
  if (!p->LoopOflw && !p->TickOflw) {
    if (Tick > Next) 
      return FALSE;

  } else if (!p->LoopOflw && p->TickOflw) 
      return FALSE;


  Diff = (Next - Tick) * 10;	  /* ms */
  plc_ConvMSToLI(Diff, &WaitTime);
  sys$clref(p->TimerFlag);
  sys$setimr(p->TimerFlag, &WaitTime, 0, 0, 0);
  sys$waitfr(p->TimerFlag);

  return TRUE;  
}
#endif

