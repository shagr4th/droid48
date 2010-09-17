/*
 *  This file is part of x48, an emulator of the HP-48sx Calculator.
 *  Copyright (C) 1994  Eddie C. Dost  (ecd@dressler.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $Log: register.c,v $
 * Revision 1.6  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.5  1994/10/06  16:30:05  ecd
 * changed char to unsigned
 *
 * Revision 1.5  1994/10/06  16:30:05  ecd
 * changed char to unsigned
 *
 * Revision 1.4  1994/10/01  10:12:53  ecd
 * changed get_start and get_end to be inline functions
 *
 * Revision 1.3  1994/09/18  15:29:22  ecd
 * turned off unused rcsid message
 *
 * Revision 1.2  1994/09/13  16:57:00  ecd
 * changed to plain X11
 *
 * Revision 1.1  1994/08/26  11:09:02  ecd
 * Initial revision
 *
 * $Id: register.c,v 1.6 1995/01/11 18:20:01 ecd Exp ecd $
 */


#include "global.h"

#include <stdlib.h>
#include <stdio.h>

#include "hp48.h"
#include "hp48_emu.h"

extern long nibble_masks[16];

static int start_fields[] = {
  -1,  0,  2,  0, 15,  3,  0,  0,
  -1,  0,  2,  0, 15,  3,  0,  0, 
   0,  0,  0
};

static int end_fields[] = {
  -1, -1,  2,  2, 15, 14,  1, 15,
  -1, -1,  2,  2, 15, 14,  1,  4,
   3,  2,  0
};

static inline int
#ifdef __FunctionProto__
get_start(int code)
#else
get_start(code)
int code;
#endif
{
  int s;

  if ((s = start_fields[code]) == -1) {
    s = saturn.P;
  }
  return s;
}

static inline int
#ifdef __FuntionProto__
get_end(int code)
#else
get_end(code)
int code;
#endif
{
  int e;

  if ((e = end_fields[code]) == -1) {
    e = saturn.P;
  }
  return e;
}

void
#ifdef __FunctionProto__
add_register(unsigned char *res, unsigned char *r1,
             unsigned char *r2, int code)
#else
add_register(res, r1, r2, code)
unsigned char *res;
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int t, c, i, s, e;

  s = get_start(code);
  e = get_end(code);
  c = 0;
  for (i = s; i <= e; i++) {
    t = r1[i] + r2[i] + c;
    if (t < (int)saturn.hexmode) {
      res[i] = t & 0xf;
      c = 0;
    } else {
      res[i] = (t - saturn.hexmode) & 0xf;
      c = 1;
    }
  }
  if (c)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
add_p_plus_one(unsigned char *r)
#else
add_p_plus_one(r)
unsigned char *r;
#endif
{
  int t, c, i, s, e;

  s = 0;
  e = 4;
  c = saturn.P + 1;
  for (i = s; i <= e; i++) {
    t = r[i] + c;
    if (t < 16) {
      r[i] = t & 0xf;
      c = 0;
    } else {
      r[i] = (t - 16) & 0xf;
      c = 1;
    }
  }
  if (c)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
sub_register(unsigned char *res, unsigned char *r1,
             unsigned char *r2, int code)
#else
sub_register(res, r1, r2, code)
unsigned char *res;
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int t, c, i, s, e;

  s = get_start(code);
  e = get_end(code);
  c = 0;
  for (i = s; i <= e; i++) {
    t = r1[i] - r2[i] - c;
    if (t >= 0) {
      res[i] = t & 0xf;
      c = 0;
    } else {
      res[i] = (t + saturn.hexmode) & 0xf;
      c = 1;
    }
  }
  if (c)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
complement_2_register(unsigned char *r, int code)
#else
complement_2_register(r, code)
unsigned char *r;
int code;
#endif
{
  int t, c, carry, i, s, e;

  s = get_start(code);
  e = get_end(code);
  c = 1;
  carry = 0;
  for (i = s; i <= e; i++) {
    t = (saturn.hexmode - 1) - r[i] + c;
    if (t < (int)saturn.hexmode) {
      r[i] = t & 0xf;
      c = 0;
    } else {
      r[i] = (t - saturn.hexmode) & 0xf;
      c = 1;
    }
    carry += r[i];
  }
  if (carry)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
complement_1_register(unsigned char *r, int code)
#else
complement_1_register(r, code)
unsigned char *r;
int code;
#endif
{
  int t, i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = s; i <= e; i++) {
    t = (saturn.hexmode - 1) - r[i];
    r[i] = t & 0xf;
  }
  saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
inc_register(unsigned char *r, int code)
#else
inc_register(r, code)
unsigned char *r;
int code;
#endif
{
  int t, c, i, s, e;

  s = get_start(code);
  e = get_end(code);
  c = 1;
  for (i = s; i <= e; i++) {
    t = r[i] + c;
    if (t < (int)saturn.hexmode) {
      r[i] = t & 0xf;
      c = 0;
      break;
    } else {
      r[i] = (t - saturn.hexmode) & 0xf;
      c = 1;
    }
  }
  if (c)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
add_register_constant(unsigned char *r, int code, int val)
#else
add_register_constant(r, code, val)
unsigned char *r;
int code;
int val;
#endif
{
  int t, c, i, s, e;

  s = get_start(code);
  e = get_end(code);
  c = val;
  for (i = s; i <= e; i++) {
    t = r[i] + c;
    if (t < 16) {
      r[i] = t & 0xf;
      c = 0;
      break;
    } else {
      r[i] = (t - 16) & 0xf;
      c = 1;
    }
  }
  if (c)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
dec_register(unsigned char *r, int code)
#else
dec_register(r, code)
unsigned char *r;
int code;
#endif
{
  int t, c, i, s, e;

  s = get_start(code);
  e = get_end(code);
  c = 1;
  for (i = s; i <= e; i++) {
    t = r[i] - c;
    if (t >= 0) {
      r[i] = t & 0xf;
      c = 0;
      break;
    } else {
      r[i] = (t + saturn.hexmode) & 0xf;
      c = 1;
    }
  }
  if (c)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
sub_register_constant(unsigned char *r, int code, int val)
#else
sub_register_constant(r, code, val)
unsigned char *r;
int code;
int val;
#endif
{
  int t, c, i, s, e;

  s = get_start(code);
  e = get_end(code);
  c = val;
  for (i = s; i <= e; i++) {
    t = r[i] - c;
    if (t >= 0) {
      r[i] = t & 0xf;
      c = 0;
      break;
    } else {
      r[i] = (t + 16) & 0xf;
      c = 1;
    }
  }
  if (c)
    saturn.CARRY = 1;
  else
    saturn.CARRY = 0;
}

void
#ifdef __FunctionProto__
zero_register(unsigned char *r, int code)
#else
zero_register(r, code)
unsigned char *r;
int code;
#endif
{
  int i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = s; i <= e; i++)
    r[i] = 0;
}

void
#ifdef __FunctionProto__
or_register(unsigned char *res, unsigned char *r1,
            unsigned char *r2, int code)
#else
or_register(res, r1, r2, code)
unsigned char *res;
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = s; i <= e; i++) {
    res[i] = (r1[i] | r2[i]) & 0xf;
  }
}

void
#ifdef __FunctionProto__
and_register(unsigned char *res, unsigned char *r1,
             unsigned char *r2, int code)
#else
and_register(res, r1, r2, code)
unsigned char *res;
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = s; i <= e; i++) {
    res[i] = (r1[i] & r2[i]) & 0xf;
  }
}

void
#ifdef __FunctionProto__
copy_register(unsigned char *to, unsigned char *from, int code)
#else
copy_register(to, from, code)
unsigned char *to;
unsigned char *from;
int code;
#endif
{
  int i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = s; i <= e; i++)
    to[i] = from[i];
}

void
#ifdef __FunctionProto__
exchange_register(unsigned char *r1, unsigned char *r2, int code)
#else
exchange_register(r1, r2, code)
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int t, i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = s; i <= e; i++) {
    t = r1[i];
    r1[i] = r2[i];
    r2[i] = t;
  }
}

void
#ifdef __FunctionProto__
exchange_reg(unsigned char *r, word_20 *d, int code)
#else
exchange_reg(r, d, code)
unsigned char *r;
word_20 *d;
int code;
#endif
{
  int t, i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = s; i <= e; i++) {
    t = r[i];
    r[i] = (*d >> (i * 4)) & 0x0f;
    *d &= ~nibble_masks[i];
    *d |= t << (i * 4);
  }
}

void
#ifdef __FunctionProto__
shift_left_register(unsigned char *r, int code)
#else
shift_left_register(r, code)
unsigned char *r;
int code;
#endif
{
  int i, s, e;

  s = get_start(code);
  e = get_end(code);
  for (i = e; i > s; i--) {
    r[i] = r[i-1] & 0x0f;
  }
  r[s] = 0;
}

void
#ifdef __FunctionProto__
shift_left_circ_register(unsigned char *r, int code)
#else
shift_left_circ_register(r, code)
unsigned char *r;
int code;
#endif
{
  int t, i, s, e;

  s = get_start(code);
  e = get_end(code);
  t = r[e] & 0x0f;
  for (i = e; i > s; i--) {
    r[i] = r[i-1] & 0x0f;
  }
  r[s] = t;
}

void
#ifdef __FunctionProto__
shift_right_register(unsigned char *r, int code)
#else
shift_right_register(r, code)
unsigned char *r;
int code;
#endif
{
  int i, s, e;

  s = get_start(code);
  e = get_end(code);
  if (r[s] & 0x0f)
    saturn.SB = 1;
  for (i = s; i < e; i++) {
    r[i] = r[i+1] & 0x0f;
  }
  r[e] = 0;
}

void
#ifdef __FunctionProto__
shift_right_circ_register(unsigned char *r, int code)
#else
shift_right_circ_register(r, code)
unsigned char *r;
int code;
#endif
{
  int t, i, s, e;

  s = get_start(code);
  e = get_end(code);
  t = r[s] & 0x0f;
  for (i = s; i < e; i++) {
    r[i] = r[i+1] & 0x0f;
  }
  r[e] = t;
  if (t)
    saturn.SB = 1;
}

void
#ifdef __FunctionProto__
shift_right_bit_register(unsigned char *r, int code)
#else
shift_right_bit_register(r, code)
unsigned char *r;
int code;
#endif
{
  int t, i, s, e, sb;

  s = get_start(code);
  e = get_end(code);
  sb = 0;
  for (i = e; i >= s; i--) {
    t = (((r[i] >> 1) & 7) | (sb << 3)) & 0x0f;
    sb = r[i] & 1;
    r[i] = t;
  }
  if (sb)
    saturn.SB = 1;
}
 
int
#ifdef __FunctionProto__
is_zero_register(unsigned char *r, int code)
#else
is_zero_register(r, code)
unsigned char *r;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 1;
  for (i = s; i <= e; i++)
    if ((r[i] & 0xf) != 0) {
      z = 0;
      break;
    }
  return z;
}

int
#ifdef __FunctionProto__
is_not_zero_register(unsigned char *r, int code)
#else
is_not_zero_register(r, code)
unsigned char *r;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 0;
  for (i = s; i <= e; i++)
    if ((r[i] & 0xf) != 0) {
      z = 1;
      break;
    }
  return z;
}

int
#ifdef __FunctionProto__
is_equal_register(unsigned char *r1, unsigned char *r2, int code)
#else
is_equal_register(r1, r2, code)
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 1;
  for (i = s; i <= e; i++)
    if ((r1[i] & 0xf) != (r2[i] & 0xf)) {
      z = 0;
      break;
    }
  return z;
}

int
#ifdef __FunctionProto__
is_not_equal_register(unsigned char *r1, unsigned char *r2, int code)
#else
is_not_equal_register(r1, r2, code)
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 0;
  for (i = s; i <= e; i++)
    if ((r1[i] & 0xf) != (r2[i] & 0xf)) {
      z = 1;
      break;
    }
  return z;
}

int
#ifdef __FunctionProto__
is_less_register(unsigned char *r1, unsigned char *r2, int code)
#else
is_less_register(r1, r2, code)
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 0;
  for (i = e; i >= s; i--) {
    if ((int)(r1[i] & 0xf) < (int)(r2[i] & 0xf)) {
      z = 1;
      break;
    }
    if ((int)(r1[i] & 0xf) > (int)(r2[i] & 0xf)) {
      z = 0;
      break;
    }
  }
  return z;
}

int
#ifdef __FunctionProto__
is_less_or_equal_register(unsigned char *r1, unsigned char *r2, int code)
#else
is_less_or_equal_register(r1, r2, code)
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 1;
  for (i = e; i >= s; i--) {
    if ((int)(r1[i] & 0xf) < (int)(r2[i] & 0xf)) {
      z = 1;
      break;
    }
    if ((int)(r1[i] & 0xf) > (int)(r2[i] & 0xf)) {
      z = 0;
      break;
    }
  }
  return z;
}

int
#ifdef __FunctionProto__
is_greater_register(unsigned char *r1, unsigned char *r2, int code)
#else
is_greater_register(r1, r2, code)
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 0;
  for (i = e; i >= s; i--) {
    if ((int)(r1[i] & 0xf) > (int)(r2[i] & 0xf)) {
      z = 1;
      break;
    }
    if ((int)(r1[i] & 0xf) < (int)(r2[i] & 0xf)) {
      z = 0;
      break;
    }
  }
  return z;
}

int
#ifdef __FunctionProto__
is_greater_or_equal_register(unsigned char *r1, unsigned char *r2, int code)
#else
is_greater_or_equal_register(r1, r2, code)
unsigned char *r1;
unsigned char *r2;
int code;
#endif
{
  int z, i, s, e;

  s = get_start(code);
  e = get_end(code);
  z = 1;
  for (i = e; i >= s; i--) {
    if ((int)(r1[i] & 0xf) < (int)(r2[i] & 0xf)) {
      z = 0;
      break;
    }
    if ((int)(r1[i] & 0xf) > (int)(r2[i] & 0xf)) {
      z = 1;
      break;
    }
  }
  return z;
}

