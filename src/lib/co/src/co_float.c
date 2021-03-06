/** 
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

/**
  @file co_float.c

  Converts an IEEE float to a VAX float and vice verca.
 */


#include "co_float.h"

/* Vax f-float on a little endian machine.


   3             2 2             1 1 1           0 0 0           0
   1             4 3             6 5 4           8 7 6           0
  +---------------+---------------+-+-------------+-+-------------+
  |       fraction 15 <- 0        |s|  exponent   : | f 22 <- 16  |
  +---------------+---------------+-+-------------+-+-------------+
   1             0 0             0   0           0 0 2           1
   5             8 7             0   7           1 0 2           6


*/

union vax_f_le {
  unsigned int	i;
  struct {
    unsigned int    f22_16	:  7;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
    unsigned int    f15_0	: 16;
  } b;
};

/* Vax f-float on a big endian machine.



  +-+-------------+   +-+-------------+   +---------------+   +---------------+
  : | f 22 <- 16  |   |s|  exponent   :   : 15 <- 0       |   |      fraction :
  +-+-------------+   +-+-------------+   +---------------+   +---------------+
   0 2           1       0           0     0             0     1             0
   0 2           6       7           1     7             0     5             8

   0             0 0             1 1 1           2 2             3
   0             7 8             5 6 7           3 4             1
  +---------------+---------------+-+-------------+-+-------------+
  |       fraction 0 -> 15        |s|  exponent   : | f 16 -> 22  |
  +---------------+---------------+-+-------------+-+-------------+
   0             0 0             1   0             0 1           2
   0             7 8             5   0             7 6           2

*/

union vax_f_be {
  unsigned int	i;
  struct {
    unsigned int    f0_15	: 16;
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f16_22	:  7;
  } b;
};

/* IEEE single on a little endian machine.

   3             2 2             1 1             0 0             0
   1             4 3             6 5             8 7             0
  +-+-------------+-+-------------+---------------+---------------+
  |s|  exponent   : |        fraction 22 <- 0     :               |
  +-+-------------+-+-------------+---------------+---------------+
     0             0 2           1 1                             0
     7             0 2           6 5                             0

*/

union i3e_s_le {
  unsigned int	i;
  struct {
    unsigned int    f22_0	: 23;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
  } b;
  struct {
    unsigned int    f15_0	: 16;
    unsigned int    f22_16	:  7;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
  } v;
};

/* IEEE single on a big endian machine.

   0             0 0             1 1             2 2             3
   0             7 8             5 6             3 4             1
  +-+-------------+-+-------------+---------------+---------------+
  |s|  exponent   : |        fraction 0 -> 22     :               |
  +-+-------------+-+-------------+---------------+---------------+
     0           0 0 0           0 0                             2
     0           6 7 0           6 7                             2

*/

union i3e_s_be {
  unsigned int	i;
  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f0_22	: 23;
  } b;
  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f0_6	:  7;
    unsigned int    f7_22	: 16;
  } v;
};

#define VAX_F_BIAS    0x81
#define I3E_S_BIAS    0x7f
#define VAX_D_BIAS    0x81
#define VAX_G_BIAS    0x401
#define I3E_D_BIAS    0x3ff


#define IBYTE0(i) ((i >> 0x18) & 0x000000ff) 
#define IBYTE1(i) ((i >> 0x08) & 0x0000ff00) 
#define IBYTE2(i) ((i << 0x08) & 0x00ff0000) 
#define IBYTE3(i) ((i << 0x18) & 0xff000000) 

#define ENDIAN_SWAP_INT(t, s)\
  {int i = *(int *)s; *(int *)t = (IBYTE0(i) | IBYTE1(i) | IBYTE2(i) | IBYTE3(i));}





void 
co_vaxf2ieee(co_eBO sbo,
             co_eBO tbo,
             const char *sp,
             char *tp)
{
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    union vax_f_le v;
    union i3e_s_le *i3ep;    
#else    
    union vax_f_be v;
    union i3e_s_be *i3ep;    
#endif

    if (sbo != co_dHostByteOrder) {
        ENDIAN_SWAP_INT(&v.i, (int *)sp);
    }
    else
        v.i = *(int *)sp;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    i3ep = (union i3e_s_le *) tp;
#else
    i3ep = (union i3e_s_be *) tp;
#endif

    if (v.b.f22_16 == 0x7f && v.b.exp == 0xff && v.b.f15_0 == 0xffff) {  /* High value.  */
        i3ep->i = 0, i3ep->v.exp = 0xff;
    } else if (v.b.f22_16 == 0 && v.b.exp == 0 && v.b.f15_0 == 0) {  /* Low value.  */
        i3ep->i = 0;
    } else {
        i3ep->v.exp     = v.b.exp - VAX_F_BIAS + I3E_S_BIAS;
        i3ep->v.f15_0   = v.b.f15_0;
        i3ep->v.f22_16  = v.b.f22_16;
    }

    i3ep->b.sign = v.b.sign;

    if (tbo != co_dHostByteOrder) {   
        ENDIAN_SWAP_INT((int* )tp, (int *)tp);
    }    

}


void 
co_ieee2vaxf(co_eBO sbo,
             co_eBO tbo,
             const char *sp,
             char *tp)
{
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    union vax_f_le	*vp;
    union i3e_s_le	i3e;
#else
    union vax_f_be	*vp;
    union i3e_s_be	i3e;
#endif

    if (sbo != co_dHostByteOrder) {
        ENDIAN_SWAP_INT(&i3e.i, (int *)sp);
    }
    else
        i3e.i = *(int *)sp;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    vp = (union vax_f_le *) tp;
#else
    vp = (union vax_f_be *) tp;
#endif
    
    if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0xff) {  /* High value.  */
        vp->b.f22_16 = 0x7f;
        vp->b.exp    = 0xff;
        vp->b.f15_0  = 0xffff;

    } else if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0x00) {  /* Low value.  */
        vp->i = 0;

        /* -0 is valid for IEEE, this is not the case for VAX. 
         * Clear the sign bit 
         */
        i3e.i = 0;

    } else {
        vp->b.exp    = i3e.v.exp - I3E_S_BIAS + VAX_F_BIAS;
        vp->b.f22_16 = i3e.v.f22_16;
        vp->b.f15_0  = i3e.v.f15_0;
    }

    vp->b.sign = i3e.b.sign;

    if (tbo != co_dHostByteOrder) {   
        ENDIAN_SWAP_INT((int* )tp, (int *)tp);
    }    

}



#if 0 /* Double precision */


/* Vax d-float on a big endian machine  */

union vax_d {
  struct {
    unsigned int    lo;
    unsigned int    hi;
  } i;
  struct {
    unsigned int    f48_54	:  7;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
    unsigned int    f32_47	: 16;
    unsigned int    f16_31	: 16;
    unsigned int    f0_15	: 16;
  } b;

  /* This struct is used when moving to ieee double.  */

  struct {
    unsigned int    f48_51	:  4;
    unsigned int    f52_54	:  3;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
    unsigned int    f32_47	: 16;

    unsigned int    f16_19	:  4;
    unsigned int    f20_31	: 12;
    unsigned int    f0_15	: 16;
  } e;
};

/* Vax g-float on a big endian machine  */

union vax_g {
  struct {
    unsigned int    lo;
    unsigned int    hi;
  } i;
  struct {
    unsigned int    f48_51	:  4;
    unsigned int    exp		: 11;
    unsigned int    sign	:  1;
    unsigned int    f32_47	: 16;
    unsigned int    f16_31	: 16;
    unsigned int    f0_15	: 16;
  } b;

  /* This struct is used when moving to ieee double.  */

  struct {
    unsigned int    f48_51	:  4;
    unsigned int    exp		: 11;
    unsigned int    sign	:  1;
    unsigned int    f32_47	: 16;

    unsigned int    f16_19	:  4;
    unsigned int    f20_31	: 12;
    unsigned int    f0_15	: 16;
  } e;
};

/* IEEE double float on a big endian machine */

union i3e_d {
  struct {
    unsigned int    lo;
    unsigned int    hi;
  } i;
  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		: 11;
    unsigned int    f0_19	: 20;
    unsigned int    f20_51	: 32;
  } b;

  /* This struct is used when moving from vax d,g float. */

  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		: 11;
    unsigned int    f0_15	: 16;
    unsigned int    f16_19	:  4;

    unsigned int    f20_31	: 12;
    unsigned int    f32_47	: 16;
    unsigned int    f48_51	:  4;
  } v;
};


static pwr_tBoolean
dvms_dfloat (
  char			*p,
  gdb_sAttribute	*ap
)
{
  int			i;
  union vax_d		v;
  union i3e_d		*i3ep;

  p += ap->offs;

  for (i = ap->elem; i > 0; i--) {
    v = *((union vax_d *)p);
    i3ep = (union i3e_d *)p;
    ENDIAN_SWAP_INT(&v.i.lo);
    ENDIAN_SWAP_INT(&v.i.hi);

    if (v.b.f48_54 == 0x7f && v.b.exp == 0xff && v.b.f32_47 == 0xffff && v.i.hi == 0xffffffff) {  /* High value.  */
      i3ep->i.lo = i3ep->i.hi = 0, i3ep->b.exp = 0x7ff;
    } else if (v.b.f48_54 == 0 && v.b.exp == 0 && v.i.hi == 0) {  /* Low value.  */
      i3ep->i.lo = i3ep->i.hi = 0;
    } else {
      i3ep->b.exp    = v.b.exp - VAX_D_BIAS + I3E_D_BIAS;
      i3ep->v.f0_15  = v.e.f0_15;
      i3ep->v.f16_19 = v.e.f16_19;
      i3ep->v.f20_31 = v.e.f20_31;
      i3ep->v.f32_47 = v.e.f32_47;
      i3ep->v.f48_51 = v.e.f48_51;
    }

    i3ep->b.sign = v.b.sign;
    p += sizeof(float);
  }

  return TRUE;
}


static pwr_tBoolean
dvms_gfloat (
  char			*p,
  gdb_sAttribute	*ap
)
{
  int			i;
  union vax_g		v;
  union i3e_d		*i3ep;

  p += ap->offs;

  for (i = ap->elem; i > 0; i--) {
    v = *((union vax_g *)p);
    i3ep = (union i3e_d *)p;
    ENDIAN_SWAP_INT(&v.i.lo);
    ENDIAN_SWAP_INT(&v.i.hi);

    if (v.b.f48_51 == 0xf && v.b.exp == 0x7ff && v.b.f32_47 == 0xffff && v.i.hi == 0xffffffff) {  /* High value.  */
      i3ep->i.lo = i3ep->i.hi = 0, i3ep->b.exp = 0x7ff;
    } else if (v.b.f48_51 == 0 && v.b.exp == 0 && v.i.hi == 0) {  /* Low value.  */
      i3ep->i.lo = i3ep->i.hi = 0;
    } else {
      i3ep->b.exp    = v.b.exp - VAX_G_BIAS + I3E_D_BIAS;
      i3ep->v.f0_15  = v.e.f0_15;
      i3ep->v.f16_19 = v.e.f16_19;
      i3ep->v.f20_31 = v.e.f20_31;
      i3ep->v.f32_47 = v.e.f32_47;
      i3ep->v.f48_51 = v.e.f48_51;
    }

    i3ep->b.sign = v.b.sign;
    p += sizeof(float);
  }

  return TRUE;
}

#endif
