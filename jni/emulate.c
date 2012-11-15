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

/*
 * $Log: emulate.c,v $
 * Revision 1.16  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.15  1994/12/07  20:20:50  ecd
 * fiddled around with sound
 *
 * Revision 1.15  1994/12/07  20:20:50  ecd
 * fiddled around with sound
 *
 * Revision 1.14  1994/11/28  02:00:51  ecd
 * added TRAP instruction 64001
 * played with output register ...
 * removed some unused switch statements
 *
 * Revision 1.13  1994/11/04  03:42:34  ecd
 * changed includes
 *
 * Revision 1.12  1994/11/02  19:13:04  ecd
 * fixed missing log
 *
 *
 * $Id: emulate.c,v 1.16 1995/01/11 18:20:01 ecd Exp ecd $
 */


#include "global.h"

#include <stdio.h>

#include "hp48.h"
#include "hp48_emu.h"
#include "device.h"
#include "timer.h"

#include "debugger.h"

#if 0
#define DEBUG_TIMER
#define DEBUG_SCHED
#define DEBUG_DISP_SCHED
#endif

static long	jumpaddr;

unsigned long	instructions = 0;
unsigned long	old_instr = 0;

int		rece_instr = 0;
int		device_check = 0;

int		adj_time_pending = 0;

int		set_t1;

long		schedule_event = 0;

#define		SrvcIoStart		0x3c0
#define		SrvcIoEnd		0x5ec

#define		SCHED_INSTR_ROLLOVER	0x3fffffff
#define		SCHED_RECEIVE		0x7ff
#define		SCHED_ADJTIME		0x1ffe
#define		SCHED_TIMER1		0x1e00
#define		SCHED_TIMER2		0xf
#define		SCHED_STATISTICS	0x7ffff
#define		SCHED_NEVER		0x7fffffff

#define		NR_SAMPLES 10

long		sched_instr_rollover = SCHED_INSTR_ROLLOVER;
long		sched_receive = SCHED_RECEIVE;
long		sched_adjtime = SCHED_ADJTIME;
long		sched_timer1 = SCHED_TIMER1;
long		sched_timer2 = SCHED_TIMER2;
long		sched_statistics = SCHED_STATISTICS;
long		sched_display = SCHED_NEVER;

unsigned long	t1_i_per_tick;
unsigned long	t2_i_per_tick;
unsigned long	s_1;
unsigned long	s_16;
unsigned long	old_s_1;
unsigned long	old_s_16;
unsigned long	delta_t_1;
unsigned long	delta_t_16;
unsigned long	delta_i;
word_64		run;

static word_20 jumpmasks[] = {
  0xffffffff, 0xfffffff0, 0xffffff00, 0xfffff000,
  0xffff0000, 0xfff00000, 0xff000000, 0xf0000000
};

int
#ifdef __FunctionProto__
decode_group_80(void)
#else
decode_group_80()
#endif
{
  int t, op3, op4, op5, op6;
  unsigned char *REG;
  long addr;
  op3 = read_nibble(saturn.PC + 2);
//LOGI("-------%d", op3);
  switch (op3) {
    case 0:	/* OUT=CS */
      saturn.PC += 3;
      copy_register(saturn.OUT, saturn.C, OUTS_FIELD);
#if 1
      check_out_register();
#endif
      return 0;
    case 1:	/* OUT=C */
      saturn.PC += 3;
      copy_register(saturn.OUT, saturn.C, OUT_FIELD);
#if 1
      check_out_register();
#endif
      return 0;
    case 2:	/* A=IN */
      saturn.PC += 3;
      do_in();
      copy_register(saturn.A, saturn.IN, IN_FIELD);
      return 0;
    case 3:	/* C=IN */
      saturn.PC += 3;
      do_in();
      copy_register(saturn.C, saturn.IN, IN_FIELD);
      return 0;
    case 4:	/* UNCNFG */
      saturn.PC += 3;
      do_unconfigure();
      return 0;
    case 5:	/* CONFIG */
      saturn.PC += 3;
      do_configure();
      return 0;
    case 6:	/* C=ID */
      saturn.PC += 3;
      return get_identification();
    case 7:	/* SHUTDN */
      saturn.PC += 3;
      do_shutdown();
      return 0;
    case 8:
      op4 = read_nibble(saturn.PC + 3);
      switch (op4) {
        case 0:		/* INTON */
          saturn.PC += 4;
          do_inton();
	  return 0;
        case 1:		/* RSI... */
          op5 = read_nibble(saturn.PC + 4);
          saturn.PC += 5;
          do_reset_interrupt_system();
	  return 0;
        case 2:		/* LA... */
          op5 = read_nibble(saturn.PC + 4);
          load_constant(saturn.A, op5 + 1, saturn.PC + 5);
          saturn.PC += 6 + op5;
	  return 0;
        case 3:		/* BUSCB */
          saturn.PC += 4;
	  return 0;
        case 4:		/* ABIT=0 */
          op5 = read_nibble(saturn.PC + 4);
          saturn.PC += 5;
          clear_register_bit(saturn.A, op5);
	  return 0;
        case 5:		/* ABIT=1 */
          op5 = read_nibble(saturn.PC + 4);
          saturn.PC += 5;
          set_register_bit(saturn.A, op5);
	  return 0;
        case 8:		/* CBIT=0 */
          op5 = read_nibble(saturn.PC + 4);
          saturn.PC += 5;
          clear_register_bit(saturn.C, op5);
	  return 0;
        case 9:		/* CBIT=1 */
          op5 = read_nibble(saturn.PC + 4);
          saturn.PC += 5;
          set_register_bit(saturn.C, op5);
	  return 0;
        case 6:		/* ?ABIT=0 */
        case 7:		/* ?ABIT=1 */
        case 0xa:	/* ?CBIT=0 */
        case 0xb:	/* ?CBIT=1 */
          op5 = read_nibble(saturn.PC + 4);
	  if (op4 < 8)
            REG = saturn.A;
          else
            REG = saturn.C;
          if (op4 == 6 || op4 == 0xa) 
            t = 0;
          else
            t = 1;
          saturn.CARRY = (get_register_bit(REG, op5) == t)?1:0;
	  if (saturn.CARRY) {
	    saturn.PC += 5;
            op6 = read_nibbles(saturn.PC, 2);
      	    if (op6) {
              if (op6 & 0x80)
                op6 |= jumpmasks[2];
              jumpaddr = (saturn.PC + op6) & 0xfffff;
	      saturn.PC = jumpaddr;
            } else {
	      saturn.PC = pop_return_addr();
            }
	  } else {
            saturn.PC += 7;
	  }
          return 0;
        case 0xc:	/* PC=(A) */
          addr = dat_to_addr(saturn.A);
          jumpaddr = read_nibbles(addr, 5);
          saturn.PC = jumpaddr;
          return 0;
        case 0xd:	/* BUSCD */
          saturn.PC += 4;
	  return 0;
        case 0xe:	/* PC=(C) */
          addr = dat_to_addr(saturn.C);
          jumpaddr = read_nibbles(addr, 5);
          saturn.PC = jumpaddr;
          return 0;
        case 0xf:	/* INTOFF */
          saturn.PC += 4;
          do_intoff();
	  return 0;
        default:
          return 1;
      }
    case 9:	/* C+P+1 */
      saturn.PC += 3;
      add_p_plus_one(saturn.C);
      return 0;
    case 0xa:	/* RESET */
      saturn.PC += 3;
      do_reset();
      return 0;
    case 0xb:	/* BUSCC */
      saturn.PC += 3;
      return 0;
    case 0xc:   /* C=P n */
      op4 = read_nibble(saturn.PC + 3);
      saturn.PC += 4;
      set_register_nibble(saturn.C, op4, saturn.P);
      return 0;
    case 0xd:	/* P=C n */
      op4 = read_nibble(saturn.PC + 3);
      saturn.PC += 4;
      saturn.P = get_register_nibble(saturn.C, op4);
      return 0;
    case 0xe:	/* SREQ? */
      saturn.PC += 3;
      saturn.C[0] = 0;
      saturn.SR = 0;
      return 0;
    case 0xf:	/* CPEX n */
      op4 = read_nibble(saturn.PC + 3);
      saturn.PC += 4;
      t = get_register_nibble(saturn.C, op4);
      set_register_nibble(saturn.C, op4, saturn.P);
      saturn.P = t;
      return 0;
    default:
      return 1;
  }
}

int
#ifdef __FunctionProto__
decode_group_1(void)
#else
decode_group_1()
#endif
{
  int op, op2, op3, op4;

  op2 = read_nibble(saturn.PC + 1);
  switch (op2) {
    case 0:
      op3 = read_nibble(saturn.PC + 2);
      switch (op3) {
        case 0:		/* saturn.R0=A */
          saturn.PC += 3;
          copy_register(saturn.R0, saturn.A, W_FIELD);
          return 0;
        case 1:		/* saturn.R1=A */
        case 5:
          saturn.PC += 3;
          copy_register(saturn.R1, saturn.A, W_FIELD);
          return 0;
        case 2:		/* saturn.R2=A */
        case 6:
          saturn.PC += 3;
          copy_register(saturn.R2, saturn.A, W_FIELD);
          return 0;
        case 3:		/* saturn.R3=A */
        case 7:
          saturn.PC += 3;
          copy_register(saturn.R3, saturn.A, W_FIELD);
          return 0;
        case 4:		/* saturn.R4=A */
          saturn.PC += 3;
          copy_register(saturn.R4, saturn.A, W_FIELD);
          return 0;
        case 8:		/* saturn.R0=C */
          saturn.PC += 3;
          copy_register(saturn.R0, saturn.C, W_FIELD);
          return 0;
        case 9:		/* saturn.R1=C */
        case 0xd:
          saturn.PC += 3;
          copy_register(saturn.R1, saturn.C, W_FIELD);
          return 0;
        case 0xa:	/* saturn.R2=C */
        case 0xe:
          saturn.PC += 3;
          copy_register(saturn.R2, saturn.C, W_FIELD);
          return 0;
        case 0xb:	/* saturn.R3=C */
        case 0xf:
          saturn.PC += 3;
          copy_register(saturn.R3, saturn.C, W_FIELD);
          return 0;
        case 0xc:	/* saturn.R4=C */
          saturn.PC += 3;
          copy_register(saturn.R4, saturn.C, W_FIELD);
          return 0;
        default:
          return 1;
      }
    case 1:
      op3 = read_nibble(saturn.PC + 2);
      switch (op3) {
        case 0:		/* A=R0 */
          saturn.PC += 3;
          copy_register(saturn.A, saturn.R0, W_FIELD);
          return 0;
        case 1:		/* A=R1 */
        case 5:
          saturn.PC += 3;
          copy_register(saturn.A, saturn.R1, W_FIELD);
          return 0;
        case 2:		/* A=R2 */
        case 6:
          saturn.PC += 3;
          copy_register(saturn.A, saturn.R2, W_FIELD);
          return 0;
        case 3:		/* A=R3 */
        case 7:
          saturn.PC += 3;
          copy_register(saturn.A, saturn.R3, W_FIELD);
          return 0;
        case 4:		/* A=R4 */
          saturn.PC += 3;
          copy_register(saturn.A, saturn.R4, W_FIELD);
          return 0;
        case 8:		/* C=R0 */
          saturn.PC += 3;
          copy_register(saturn.C, saturn.R0, W_FIELD);
          return 0;
        case 9:		/* C=R1 */
        case 0xd:
          saturn.PC += 3;
          copy_register(saturn.C, saturn.R1, W_FIELD);
          return 0;
        case 0xa:	/* C=R2 */
        case 0xe:
          saturn.PC += 3;
          copy_register(saturn.C, saturn.R2, W_FIELD);
          return 0;
        case 0xb:	/* C=R3 */
        case 0xf:
          saturn.PC += 3;
          copy_register(saturn.C, saturn.R3, W_FIELD);
          return 0;
        case 0xc:	/* C=R4 */
          saturn.PC += 3;
          copy_register(saturn.C, saturn.R4, W_FIELD);
          return 0;
        default:
          return 1;
      }
    case 2:
      op3 = read_nibble(saturn.PC + 2);
      switch (op3) {
        case 0:		/* AR0EX */
          saturn.PC += 3;
          exchange_register(saturn.A, saturn.R0, W_FIELD);
          return 0;
        case 1:		/* AR1EX */
        case 5:
          saturn.PC += 3;
          exchange_register(saturn.A, saturn.R1, W_FIELD);
          return 0;
        case 2:		/* AR2EX */
        case 6:
          saturn.PC += 3;
          exchange_register(saturn.A, saturn.R2, W_FIELD);
          return 0;
        case 3:		/* AR3EX */
        case 7:
          saturn.PC += 3;
          exchange_register(saturn.A, saturn.R3, W_FIELD);
          return 0;
        case 4:		/* AR4EX */
          saturn.PC += 3;
          exchange_register(saturn.A, saturn.R4, W_FIELD);
          return 0;
        case 8:		/* CR0EX */
          saturn.PC += 3;
          exchange_register(saturn.C, saturn.R0, W_FIELD);
          return 0;
        case 9:		/* CR1EX */
        case 0xd:
          saturn.PC += 3;
          exchange_register(saturn.C, saturn.R1, W_FIELD);
          return 0;
        case 0xa:	/* CR2EX */
        case 0xe:
          saturn.PC += 3;
          exchange_register(saturn.C, saturn.R2, W_FIELD);
          return 0;
        case 0xb:	/* CR3EX */
        case 0xf:
          saturn.PC += 3;
          exchange_register(saturn.C, saturn.R3, W_FIELD);
          return 0;
        case 0xc:	/* CR4EX */
          saturn.PC += 3;
          exchange_register(saturn.C, saturn.R4, W_FIELD);
          return 0;
        default:
          return 1;
      }
    case 3:
      op3 = read_nibble(saturn.PC + 2);
      switch (op3) {
        case 0:		/* D0=A */
          saturn.PC += 3;
	  register_to_address(saturn.A, &saturn.D0, 0);
          return 0;
        case 1:		/* D1=A */
          saturn.PC += 3;
	  register_to_address(saturn.A, &saturn.D1, 0);
          return 0;
        case 2:		/* AD0EX */
	  saturn.PC += 3;
	  exchange_reg(saturn.A, &saturn.D0, A_FIELD);
	  return 0;
        case 3:		/* AD1EX */
	  saturn.PC += 3;
	  exchange_reg(saturn.A, &saturn.D1, A_FIELD);
	  return 0;
        case 4:		/* D0=C */
          saturn.PC += 3;
	  register_to_address(saturn.C, &saturn.D0, 0);
          return 0;
        case 5:		/* D1=C */
          saturn.PC += 3;
	  register_to_address(saturn.C, &saturn.D1, 0);
          return 0;
        case 6:		/* CD0EX */
	  saturn.PC += 3;
	  exchange_reg(saturn.C, &saturn.D0, A_FIELD);
	  return 0;
        case 7:		/* CD1EX */
	  saturn.PC += 3;
	  exchange_reg(saturn.C, &saturn.D1, A_FIELD);
	  return 0;
        case 8:		/* D0=AS */
          saturn.PC += 3;
	  register_to_address(saturn.A, &saturn.D0, 1);
          return 0;
        case 9:		/* saturn.D1=AS */
          saturn.PC += 3;
	  register_to_address(saturn.A, &saturn.D1, 1);
          return 0;
        case 0xa:	/* AD0XS */
	  saturn.PC += 3;
	  exchange_reg(saturn.A, &saturn.D0, IN_FIELD);
          return 0;
        case 0xb:	/* AD1XS */
	  saturn.PC += 3;
	  exchange_reg(saturn.A, &saturn.D1, IN_FIELD);
          return 0;
        case 0xc:	/* D0=CS */
          saturn.PC += 3;
	  register_to_address(saturn.C, &saturn.D0, 1);
          return 0;
        case 0xd:	/* D1=CS */
          saturn.PC += 3;
	  register_to_address(saturn.C, &saturn.D1, 1);
          return 0;
        case 0xe:	/* CD0XS */
	  saturn.PC += 3;
	  exchange_reg(saturn.C, &saturn.D0, IN_FIELD);
          return 0;
        case 0xf:	/* CD1XS */
	  saturn.PC += 3;
	  exchange_reg(saturn.C, &saturn.D1, IN_FIELD);
          return 0;
        default:
          return 1;
      }
    case 4:
      op3 = read_nibble(saturn.PC + 2);
      op = op3 < 8 ? 0xf : 6;
      switch(op3 & 7) {
        case 0:	/* DAT0=A */
	  saturn.PC += 3;
	  store(saturn.D0, saturn.A, op);
          return 0;
	case 1:	/* DAT1=A */
	  saturn.PC += 3;
	  store(saturn.D1, saturn.A, op);
          return 0;
	case 2:	/* A=DAT0 */
	  saturn.PC += 3;
	  recall(saturn.A, saturn.D0, op);
          return 0;
	case 3:	/* A=DAT1 */
	  saturn.PC += 3;
	  recall(saturn.A, saturn.D1, op);
          return 0;
	case 4:	/* DAT0=C */
	  saturn.PC += 3;
	  store(saturn.D0, saturn.C, op);
          return 0;
	case 5:	/* DAT1=C */
	  saturn.PC += 3;
	  store(saturn.D1, saturn.C, op);
          return 0;
	case 6:	/* C=DAT0 */
	  saturn.PC += 3;
	  recall(saturn.C, saturn.D0, op);
          return 0;
	case 7:	/* C=DAT1 */
	  saturn.PC += 3;
	  recall(saturn.C, saturn.D1, op);
          return 0;
	default:
	  return 1;
      }
    case 5:
      op3 = read_nibble(saturn.PC + 2);
      op4 = read_nibble(saturn.PC + 3);
      if (op3 >= 8) {
	switch(op3 & 7) {
          case 0:	/* DAT0=A */
	    saturn.PC += 4;
	    store_n(saturn.D0, saturn.A, op4+1);
            return 0;
	  case 1:	/* DAT1=A */
	    saturn.PC += 4;
	    store_n(saturn.D1, saturn.A, op4+1);
            return 0;
	  case 2:	/* A=DAT0 */
	    saturn.PC += 4;
	    recall_n(saturn.A, saturn.D0, op4+1);
            return 0;
	  case 3:	/* A=DAT1 */
	    saturn.PC += 4;
	    recall_n(saturn.A, saturn.D1, op4+1);
            return 0;
	  case 4:	/* DAT0=C */
	    saturn.PC += 4;
	    store_n(saturn.D0, saturn.C, op4+1);
            return 0;
	  case 5:	/* DAT1=C */
	    saturn.PC += 4;
	    store_n(saturn.D1, saturn.C, op4+1);
            return 0;
	  case 6:	/* C=DAT0 */
	    saturn.PC += 4;
	    recall_n(saturn.C, saturn.D0, op4+1);
            return 0;
	  case 7:	/* C=DAT1 */
	    saturn.PC += 4;
	    recall_n(saturn.C, saturn.D1, op4+1);
            return 0;
	  default:
	    return 1;
        }
      } else {
	switch(op3) {
          case 0:	/* DAT0=A */
	    saturn.PC += 4;
	    store(saturn.D0, saturn.A, op4);
            return 0;
	  case 1:	/* DAT1=A */
	    saturn.PC += 4;
	    store(saturn.D1, saturn.A, op4);
            return 0;
	  case 2:	/* A=DAT0 */
	    saturn.PC += 4;
	    recall(saturn.A, saturn.D0, op4);
            return 0;
	  case 3:	/* A=DAT1 */
	    saturn.PC += 4;
	    recall(saturn.A, saturn.D1, op4);
            return 0;
	  case 4:	/* DAT0=C */
	    saturn.PC += 4;
	    store(saturn.D0, saturn.C, op4);
            return 0;
	  case 5:	/* DAT1=C */
	    saturn.PC += 4;
	    store(saturn.D1, saturn.C, op4);
            return 0;
	  case 6:	/* C=DAT0 */
	    saturn.PC += 4;
	    recall(saturn.C, saturn.D0, op4);
            return 0;
	  case 7:	/* C=DAT1 */
	    saturn.PC += 4;
	    recall(saturn.C, saturn.D1, op4);
            return 0;
	  default:
	    return 1;
        }
      }
    case 6:
      op3 = read_nibble(saturn.PC + 2);
      saturn.PC += 3;
      add_address(&saturn.D0, op3+1);
      return 0;
    case 7:
      op3 = read_nibble(saturn.PC + 2);
      saturn.PC += 3;
      add_address(&saturn.D1, op3+1);
      return 0;
    case 8:
      op3 = read_nibble(saturn.PC + 2);
      saturn.PC += 3;
      add_address(&saturn.D0, -(op3+1));
      return 0;
    case 9:
      load_addr(&saturn.D0, saturn.PC+2, 2);
      saturn.PC += 4;
      return 0;
    case 0xa:
      load_addr(&saturn.D0, saturn.PC+2, 4);
      saturn.PC += 6;
      return 0;
    case 0xb:
      load_addr(&saturn.D0, saturn.PC+2, 5);
      saturn.PC += 7;
      return 0;
    case 0xc:
      op3 = read_nibble(saturn.PC + 2);
      saturn.PC += 3;
      add_address(&saturn.D1, -(op3+1));
      return 0;
    case 0xd:
      load_addr(&saturn.D1, saturn.PC+2, 2);
      saturn.PC += 4;
      return 0;
    case 0xe:
      load_addr(&saturn.D1, saturn.PC+2, 4);
      saturn.PC += 6;
      return 0;
    case 0xf:
      load_addr(&saturn.D1, saturn.PC+2, 5);
      saturn.PC += 7;
      return 0;
    default:
      return 1;
  }
}

inline int
#ifdef __FunctionProto__
decode_8_thru_f(int op1)
#else
decode_8_thru_f(op1)
int op1;
#endif
{
  int op2, op3, op4, op5, op6;

  op2 = read_nibble(saturn.PC + 1);
 // LOGI("----- %d", op2);
  switch (op1) {
    case 8:
      switch (op2) {
        case 0:
          return decode_group_80();
        case 1:
          op3 = read_nibble(saturn.PC + 2);
          switch (op3) {
            case 0:	/* ASLC */
              saturn.PC += 3;
              shift_left_circ_register(saturn.A, W_FIELD);
              return 0;
            case 1:	/* BSLC */
              saturn.PC += 3;
              shift_left_circ_register(saturn.B, W_FIELD);
              return 0;
            case 2:	/* CSLC */
              saturn.PC += 3;
              shift_left_circ_register(saturn.C, W_FIELD);
              return 0;
            case 3:	/* DSLC */
              saturn.PC += 3;
              shift_left_circ_register(saturn.D, W_FIELD);
              return 0;
            case 4:	/* ASRC */
              saturn.PC += 3;
              shift_right_circ_register(saturn.A, W_FIELD);
              return 0;
            case 5:	/* BSRC */
              saturn.PC += 3;
              shift_right_circ_register(saturn.B, W_FIELD);
              return 0;
            case 6:	/* CSRC */
              saturn.PC += 3;
              shift_right_circ_register(saturn.C, W_FIELD);
              return 0;
            case 7:	/* DSRC */
              saturn.PC += 3;
              shift_right_circ_register(saturn.D, W_FIELD);
              return 0;
            case 8:	/* R = R +/- CON */
              op4 = read_nibble(saturn.PC + 3);
              op5 = read_nibble(saturn.PC + 4);
              op6 = read_nibble(saturn.PC + 5);
              if (op5 < 8) {	/* PLUS */
                switch (op5 & 3) {
                  case 0:	/* A=A+CON */
                    saturn.PC += 6;
                    add_register_constant(saturn.A, op4, op6+1);
                    return 0;
                  case 1:	/* B=B+CON */
                    saturn.PC += 6;
                    add_register_constant(saturn.B, op4, op6+1);
                    return 0;
                  case 2:	/* C=C+CON */
                    saturn.PC += 6;
                    add_register_constant(saturn.C, op4, op6+1);
                    return 0;
                  case 3:	/* D=D+CON */
                    saturn.PC += 6;
                    add_register_constant(saturn.D, op4, op6+1);
                    return 0;
                  default:
                    return 1;
                }
              } else {		/* MINUS */
                switch (op5 & 3) {
                  case 0:	/* A=A-CON */
                    saturn.PC += 6;
                    sub_register_constant(saturn.A, op4, op6+1);
                    return 0;
                  case 1:	/* B=B-CON */
                    saturn.PC += 6;
                    sub_register_constant(saturn.B, op4, op6+1);
                    return 0;
                  case 2:	/* C=C-CON */
                    saturn.PC += 6;
                    sub_register_constant(saturn.C, op4, op6+1);
                    return 0;
                  case 3:	/* D=D-CON */
                    saturn.PC += 6;
                    sub_register_constant(saturn.D, op4, op6+1);
                    return 0;
                  default:
                    return 1;
                }
              }
            case 9:	/* R SRB FIELD */
              op4 = read_nibble(saturn.PC + 3);
              op5 = read_nibble(saturn.PC + 4);
              switch (op5 & 3) {
                case 0:
                  saturn.PC += 5;
                  shift_right_bit_register(saturn.A, op4);
                  return 0;
                case 1:
                  saturn.PC += 5;
                  shift_right_bit_register(saturn.B, op4);
                  return 0;
                case 2:
                  saturn.PC += 5;
                  shift_right_bit_register(saturn.C, op4);
                  return 0;
                case 3:
                  saturn.PC += 5;
                  shift_right_bit_register(saturn.D, op4);
                  return 0;
                default:
                  return 1;
              }
            case 0xa:	/* R = R FIELD, etc. */
              op4 = read_nibble(saturn.PC + 3);
              op5 = read_nibble(saturn.PC + 4);
              op6 = read_nibble(saturn.PC + 5);
              switch (op5) {
                case 0:
                  switch (op6) {
                    case 0:		/* saturn.R0=A */
                      saturn.PC += 6;
                      copy_register(saturn.R0, saturn.A, op4);
                      return 0;
                    case 1:		/* saturn.R1=A */
                    case 5:
                      saturn.PC += 6;
                      copy_register(saturn.R1, saturn.A, op4);
                      return 0;
                    case 2:		/* saturn.R2=A */
                    case 6:
                      saturn.PC += 6;
                      copy_register(saturn.R2, saturn.A, op4);
                      return 0;
                    case 3:		/* saturn.R3=A */
                    case 7:
                      saturn.PC += 6;
                      copy_register(saturn.R3, saturn.A, op4);
                      return 0;
                    case 4:		/* saturn.R4=A */
                      saturn.PC += 6;
                      copy_register(saturn.R4, saturn.A, op4);
                      return 0;
                    case 8:		/* saturn.R0=C */
                      saturn.PC += 6;
                      copy_register(saturn.R0, saturn.C, op4);
                      return 0;
                    case 9:		/* saturn.R1=C */
                    case 0xd:
                      saturn.PC += 6;
                      copy_register(saturn.R1, saturn.C, op4);
                      return 0;
                    case 0xa:	/* saturn.R2=C */
                    case 0xe:
                      saturn.PC += 6;
                      copy_register(saturn.R2, saturn.C, op4);
                      return 0;
                    case 0xb:	/* saturn.R3=C */
                    case 0xf:
                      saturn.PC += 6;
                      copy_register(saturn.R3, saturn.C, op4);
                      return 0;
                    case 0xc:	/* saturn.R4=C */
                      saturn.PC += 6;
                      copy_register(saturn.R4, saturn.C, op4);
                      return 0;
                    default:
                      return 1;
                  }
                case 1:
                  switch (op6) {
                    case 0:		/* A=R0 */
                      saturn.PC += 6;
                      copy_register(saturn.A, saturn.R0, op4);
                      return 0;
                    case 1:		/* A=R1 */
                    case 5:
                      saturn.PC += 6;
                      copy_register(saturn.A, saturn.R1, op4);
                      return 0;
                    case 2:		/* A=R2 */
                    case 6:
                      saturn.PC += 6;
                      copy_register(saturn.A, saturn.R2, op4);
                      return 0;
                    case 3:		/* A=R3 */
                    case 7:
                      saturn.PC += 6;
                      copy_register(saturn.A, saturn.R3, op4);
                      return 0;
                    case 4:		/* A=R4 */
                      saturn.PC += 6;
                      copy_register(saturn.A, saturn.R4, op4);
                      return 0;
                    case 8:		/* C=R0 */
                      saturn.PC += 6;
                      copy_register(saturn.C, saturn.R0, op4);
                      return 0;
                    case 9:		/* C=R1 */
                    case 0xd:
                      saturn.PC += 6;
                      copy_register(saturn.C, saturn.R1, op4);
                      return 0;
                    case 0xa:	/* C=R2 */
                    case 0xe:
                      saturn.PC += 6;
                      copy_register(saturn.C, saturn.R2, op4);
                      return 0;
                    case 0xb:	/* C=R3 */
                    case 0xf:
                      saturn.PC += 6;
                      copy_register(saturn.C, saturn.R3, op4);
                      return 0;
                    case 0xc:	/* C=R4 */
                      saturn.PC += 6;
                      copy_register(saturn.C, saturn.R4, op4);
                      return 0;
                    default:
                      return 1;
                  }
                case 2:
                  switch (op6) {
                    case 0:		/* AR0EX */
                      saturn.PC += 6;
                      exchange_register(saturn.A, saturn.R0, op4);
                      return 0;
                    case 1:		/* AR1EX */
                    case 5:
                      saturn.PC += 6;
                      exchange_register(saturn.A, saturn.R1, op4);
                      return 0;
                    case 2:		/* AR2EX */
                    case 6:
                      saturn.PC += 6;
                      exchange_register(saturn.A, saturn.R2, op4);
                      return 0;
                    case 3:		/* AR3EX */
                    case 7:
                      saturn.PC += 6;
                      exchange_register(saturn.A, saturn.R3, op4);
                      return 0;
                    case 4:		/* AR4EX */
                      saturn.PC += 6;
                      exchange_register(saturn.A, saturn.R4, op4);
                      return 0;
                    case 8:		/* CR0EX */
                      saturn.PC += 6;
                      exchange_register(saturn.C, saturn.R0, op4);
                      return 0;
                    case 9:		/* CR1EX */
                    case 0xd:
                      saturn.PC += 6;
                      exchange_register(saturn.C, saturn.R1, op4);
                      return 0;
                    case 0xa:	/* CR2EX */
                    case 0xe:
                      saturn.PC += 6;
                      exchange_register(saturn.C, saturn.R2, op4);
                      return 0;
                    case 0xb:	/* CR3EX */
                    case 0xf:
                      saturn.PC += 6;
                      exchange_register(saturn.C, saturn.R3, op4);
                      return 0;
                    case 0xc:	/* CR4EX */
                      saturn.PC += 6;
                      exchange_register(saturn.C, saturn.R4, op4);
                      return 0;
                    default:
                      return 1;
                  }
                default:
                  return 1;
              }
            case 0xb:
              op4 = read_nibble(saturn.PC + 3);
              switch(op4) {
                case 2:		/* PC=A */
                  jumpaddr = dat_to_addr(saturn.A);
                  saturn.PC = jumpaddr;
                  return 0;
                case 3:		/* PC=C */
                  jumpaddr = dat_to_addr(saturn.C);
                  saturn.PC = jumpaddr;
                  return 0;
                case 4:		/* A=PC */
                  saturn.PC += 4;
                  addr_to_dat(saturn.PC, saturn.A);
                  return 0;
                case 5:		/* C=PC */
                  saturn.PC += 4;
                  addr_to_dat(saturn.PC, saturn.C);
                  return 0;
                case 6:		/* APCEX */
		  saturn.PC += 4;
                  jumpaddr = dat_to_addr(saturn.A);
                  addr_to_dat(saturn.PC, saturn.A);
                  saturn.PC = jumpaddr;
                  return 0;
                case 7: 	/* CPCEX */
		  saturn.PC += 4;
                  jumpaddr = dat_to_addr(saturn.C);
                  addr_to_dat(saturn.PC, saturn.C);
                  saturn.PC = jumpaddr;
                  return 0;
                default:
                  return 1;
              }
            case 0xc:	/* ASRB */
              saturn.PC += 3;
              shift_right_bit_register(saturn.A, W_FIELD);
              return 0;
            case 0xd:	/* BSRB */
              saturn.PC += 3;
              shift_right_bit_register(saturn.B, W_FIELD);
              return 0;
            case 0xe:	/* CSRB */
              saturn.PC += 3;
              shift_right_bit_register(saturn.C, W_FIELD);
              return 0;
            case 0xf:	/* DSRB */
              saturn.PC += 3;
              shift_right_bit_register(saturn.D, W_FIELD);
              return 0;
            default:
              return 1;
          }
        case 2:
          op3 = read_nibble(saturn.PC + 2);
          saturn.PC += 3;
          clear_hardware_stat(op3);
          return 0;
        case 3:
          op3 = read_nibble(saturn.PC + 2);
          saturn.CARRY = is_zero_hardware_stat(op3);
	  if (saturn.CARRY) {
	    saturn.PC += 3;
            op4 = read_nibbles(saturn.PC, 2);
      	    if (op4) {
              if (op4 & 0x80)
                op4 |= jumpmasks[2];
              jumpaddr = (saturn.PC + op4) & 0xfffff;
	      saturn.PC = jumpaddr;
            } else {
	      saturn.PC = pop_return_addr();
            }
	  } else {
            saturn.PC += 5;
	  }
          return 0;
        case 4:
        case 5:
          op3 = read_nibble(saturn.PC + 2);
	  if (op2 == 4) {
	    saturn.PC += 3;
            clear_program_stat(op3);
          } else {
	    saturn.PC += 3;
	    set_program_stat(op3);
          }
          return 0;
        case 6:
        case 7:
          op3 = read_nibble(saturn.PC + 2);
	  if (op2 == 6)
            saturn.CARRY = (get_program_stat(op3) == 0)?1:0;
          else
            saturn.CARRY = (get_program_stat(op3) != 0)?1:0;
	  if (saturn.CARRY) {
	    saturn.PC += 3;
            op4 = read_nibbles(saturn.PC, 2);
      	    if (op4) {
              if (op4 & 0x80)
                op4 |= jumpmasks[2];
              jumpaddr = (saturn.PC + op4) & 0xfffff;
	      saturn.PC = jumpaddr;
            } else {
	      saturn.PC = pop_return_addr();
            }
	  } else {
            saturn.PC += 5;
	  }
          return 0;
        case 8:
        case 9:
          op3 = read_nibble(saturn.PC + 2);
	  if (op2 == 8)
            saturn.CARRY = (saturn.P != op3)?1:0;
          else
            saturn.CARRY = (saturn.P == op3)?1:0;
	  if (saturn.CARRY) {
	    saturn.PC += 3;
            op4 = read_nibbles(saturn.PC, 2);
      	    if (op4) {
              if (op4 & 0x80)
                op4 |= jumpmasks[2];
              jumpaddr = (saturn.PC + op4) & 0xfffff;
	      saturn.PC = jumpaddr;
            } else {
	      saturn.PC = pop_return_addr();
            }
	  } else {
            saturn.PC += 5;
	  }
          return 0;
        case 0xa:
          op3 = read_nibble(saturn.PC + 2);
          switch(op3) {
            case 0:	/* ?A=B */
	      saturn.CARRY = is_equal_register(saturn.A, saturn.B, A_FIELD);
              break;
            case 1:	/* ?B=C */
	      saturn.CARRY = is_equal_register(saturn.B, saturn.C, A_FIELD);
              break;
            case 2:	/* ?A=C */
	      saturn.CARRY = is_equal_register(saturn.A, saturn.C, A_FIELD);
              break;
            case 3:	/* ?C=D */
	      saturn.CARRY = is_equal_register(saturn.C, saturn.D, A_FIELD);
              break;
            case 4:	/* ?A#B */
	      saturn.CARRY = is_not_equal_register(saturn.A, saturn.B, A_FIELD);
              break;
            case 5:	/* ?B#C */
	      saturn.CARRY = is_not_equal_register(saturn.B, saturn.C, A_FIELD);
              break;
            case 6:	/* ?A#C */
	      saturn.CARRY = is_not_equal_register(saturn.A, saturn.C, A_FIELD);
              break;
            case 7:	/* ?C#D */
	      saturn.CARRY = is_not_equal_register(saturn.C, saturn.D, A_FIELD);
              break;
            case 8:	/* ?A=0 */
	      saturn.CARRY = is_zero_register(saturn.A, A_FIELD);
              break;
            case 9:	/* ?B=0 */
	      saturn.CARRY = is_zero_register(saturn.B, A_FIELD);
              break;
            case 0xa:	/* ?C=0 */
	      saturn.CARRY = is_zero_register(saturn.C, A_FIELD);
              break;
            case 0xb:	/* ?D=0 */
	      saturn.CARRY = is_zero_register(saturn.D, A_FIELD);
              break;
            case 0xc:	/* ?A#0 */
	      saturn.CARRY = is_not_zero_register(saturn.A, A_FIELD);
              break;
            case 0xd:	/* ?B#0 */
	      saturn.CARRY = is_not_zero_register(saturn.B, A_FIELD);
              break;
            case 0xe:	/* ?C#0 */
	      saturn.CARRY = is_not_zero_register(saturn.C, A_FIELD);
              break;
            case 0xf:	/* ?D#0 */
	      saturn.CARRY = is_not_zero_register(saturn.D, A_FIELD);
              break;
            default:
              return 1;
          }
	  if (saturn.CARRY) {
	    saturn.PC += 3;
            op4 = read_nibbles(saturn.PC, 2);
      	    if (op4) {
              if (op4 & 0x80)
                op4 |= jumpmasks[2];
              jumpaddr = (saturn.PC + op4) & 0xfffff;
	      saturn.PC = jumpaddr;
            } else {
	      saturn.PC = pop_return_addr();
            }
	  } else {
            saturn.PC += 5;
	  }
          return 0;
	case 0xb:
          op3 = read_nibble(saturn.PC + 2);
          switch (op3) {
            case 0:	/* ?A>B */
	      saturn.CARRY = is_greater_register(saturn.A, saturn.B, A_FIELD);
              break;
            case 1:	/* ?B>C */
	      saturn.CARRY = is_greater_register(saturn.B, saturn.C, A_FIELD);
              break;
            case 2:	/* ?C>A */
	      saturn.CARRY = is_greater_register(saturn.C, saturn.A, A_FIELD);
              break;
            case 3:	/* ?D>C */
	      saturn.CARRY = is_greater_register(saturn.D, saturn.C, A_FIELD);
              break;
            case 4:	/* ?A<B */
	      saturn.CARRY = is_less_register(saturn.A, saturn.B, A_FIELD);
              break;
            case 5:	/* ?B<C */
	      saturn.CARRY = is_less_register(saturn.B, saturn.C, A_FIELD);
              break;
            case 6:	/* ?C<A */
	      saturn.CARRY = is_less_register(saturn.C, saturn.A, A_FIELD);
              break;
            case 7:	/* ?D<C */
	      saturn.CARRY = is_less_register(saturn.D, saturn.C, A_FIELD);
              break;
            case 8:	/* ?A>=B */
	      saturn.CARRY = is_greater_or_equal_register(saturn.A, saturn.B, A_FIELD);
              break;
            case 9:	/* ?B>=C */
	      saturn.CARRY = is_greater_or_equal_register(saturn.B, saturn.C, A_FIELD);
              break;
            case 0xa:	/* ?C>=A */
	      saturn.CARRY = is_greater_or_equal_register(saturn.C, saturn.A, A_FIELD);
              break;
            case 0xb:	/* ?D>=C */
	      saturn.CARRY = is_greater_or_equal_register(saturn.D, saturn.C, A_FIELD);
              break;
            case 0xc:	/* ?A<=B */
	      saturn.CARRY = is_less_or_equal_register(saturn.A, saturn.B, A_FIELD);
              break;
            case 0xd:	/* ?B<=C */
	      saturn.CARRY = is_less_or_equal_register(saturn.B, saturn.C, A_FIELD);
              break;
            case 0xe:	/* ?C<=A */
	      saturn.CARRY = is_less_or_equal_register(saturn.C, saturn.A, A_FIELD);
              break;
            case 0xf:	/* ?D<=C */
	      saturn.CARRY = is_less_or_equal_register(saturn.D, saturn.C, A_FIELD);
              break;
            default:
              return 1;
          }
	  if (saturn.CARRY) {
	    saturn.PC += 3;
            op4 = read_nibbles(saturn.PC, 2);
      	    if (op4) {
              if (op4 & 0x80)
                op4 |= jumpmasks[2];
              jumpaddr = (saturn.PC + op4) & 0xfffff;
	      saturn.PC = jumpaddr;
            } else {
	      saturn.PC = pop_return_addr();
            }
	  } else {
            saturn.PC += 5;
	  }
          return 0;
	case 0xc:
          op3 = read_nibbles(saturn.PC + 2, 4);
          if (op3 & 0x8000)
            op3 |= jumpmasks[4];
          jumpaddr = (saturn.PC + op3 + 2) & 0xfffff;
          saturn.PC = jumpaddr;
	  return 0;
	case 0xd:
          op3 = read_nibbles(saturn.PC + 2, 5);
          jumpaddr = op3;
          saturn.PC = jumpaddr;
	  return 0;
	case 0xe:
          op3 = read_nibbles(saturn.PC + 2, 4);
          if (op3 & 0x8000)
            op3 |= jumpmasks[4];
          jumpaddr = (saturn.PC + op3 + 6) & 0xfffff;
	  push_return_addr(saturn.PC + 6);
          saturn.PC = jumpaddr;
	  return 0;
	case 0xf:
          op3 = read_nibbles(saturn.PC + 2, 5);
          jumpaddr = op3;
	  push_return_addr(saturn.PC + 7);
          saturn.PC = jumpaddr;
	  return 0;
        default:
          return 1;
      }
    case 9:
      op3 = read_nibble(saturn.PC + 2);
      if (op2 < 8) {
        switch(op3) {
          case 0:	/* ?A=B */
	    saturn.CARRY = is_equal_register(saturn.A, saturn.B, op2);
            break;
          case 1:	/* ?B=C */
	    saturn.CARRY = is_equal_register(saturn.B, saturn.C, op2);
            break;
          case 2:	/* ?A=C */
	    saturn.CARRY = is_equal_register(saturn.A, saturn.C, op2);
            break;
          case 3:	/* ?C=D */
	    saturn.CARRY = is_equal_register(saturn.C, saturn.D, op2);
            break;
          case 4:	/* ?A#B */
	    saturn.CARRY = is_not_equal_register(saturn.A, saturn.B, op2);
            break;
          case 5:	/* ?B#C */
	    saturn.CARRY = is_not_equal_register(saturn.B, saturn.C, op2);
            break;
          case 6:	/* ?A#C */
	    saturn.CARRY = is_not_equal_register(saturn.A, saturn.C, op2);
            break;
          case 7:	/* ?C#D */
	    saturn.CARRY = is_not_equal_register(saturn.C, saturn.D, op2);
            break;
          case 8:	/* ?A=0 */
	    saturn.CARRY = is_zero_register(saturn.A, op2);
            break;
          case 9:	/* ?B=0 */
	    saturn.CARRY = is_zero_register(saturn.B, op2);
            break;
          case 0xa:	/* ?C=0 */
	    saturn.CARRY = is_zero_register(saturn.C, op2);
            break;
          case 0xb:	/* ?D=0 */
	    saturn.CARRY = is_zero_register(saturn.D, op2);
            break;
          case 0xc:	/* ?A#0 */
	    saturn.CARRY = is_not_zero_register(saturn.A, op2);
            break;
          case 0xd:	/* ?B#0 */
	    saturn.CARRY = is_not_zero_register(saturn.B, op2);
            break;
          case 0xe:	/* ?C#0 */
	    saturn.CARRY = is_not_zero_register(saturn.C, op2);
            break;
          case 0xf:	/* ?D#0 */
            saturn.CARRY = is_not_zero_register(saturn.D, op2);
            break;
          default:
            return 1;
        }
      } else {
        op2 &= 7;
        switch (op3) {
          case 0:	/* ?A>B */
	    saturn.CARRY = is_greater_register(saturn.A, saturn.B, op2);
            break;
          case 1:	/* ?B>C */
	    saturn.CARRY = is_greater_register(saturn.B, saturn.C, op2);
            break;
          case 2:	/* ?C>A */
	    saturn.CARRY = is_greater_register(saturn.C, saturn.A, op2);
            break;
          case 3:	/* ?D>C */
	    saturn.CARRY = is_greater_register(saturn.D, saturn.C, op2);
            break;
          case 4:	/* ?A<B */
	    saturn.CARRY = is_less_register(saturn.A, saturn.B, op2);
            break;
          case 5:	/* ?B<C */
	    saturn.CARRY = is_less_register(saturn.B, saturn.C, op2);
            break;
          case 6:	/* ?C<A */
	    saturn.CARRY = is_less_register(saturn.C, saturn.A, op2);
            break;
          case 7:	/* ?D<C */
	    saturn.CARRY = is_less_register(saturn.D, saturn.C, op2);
            break;
          case 8:	/* ?A>=B */
	    saturn.CARRY = is_greater_or_equal_register(saturn.A, saturn.B, op2);
            break;
          case 9:	/* ?B>=C */
	    saturn.CARRY = is_greater_or_equal_register(saturn.B, saturn.C, op2);
            break;
          case 0xa:	/* ?C>=A */
	    saturn.CARRY = is_greater_or_equal_register(saturn.C, saturn.A, op2);
            break;
          case 0xb:	/* ?D>=C */
	    saturn.CARRY = is_greater_or_equal_register(saturn.D, saturn.C, op2);
            break;
          case 0xc:	/* ?A<=B */
	    saturn.CARRY = is_less_or_equal_register(saturn.A, saturn.B, op2);
            break;
          case 0xd:	/* ?B<=C */
	    saturn.CARRY = is_less_or_equal_register(saturn.B, saturn.C, op2);
            break;
          case 0xe:	/* ?C<=A */
	    saturn.CARRY = is_less_or_equal_register(saturn.C, saturn.A, op2);
            break;
          case 0xf:	/* ?D<=C */
	    saturn.CARRY = is_less_or_equal_register(saturn.D, saturn.C, op2);
            break;
          default:
            return 1;
        }
      }
      if (saturn.CARRY) {
	saturn.PC += 3;
        op4 = read_nibbles(saturn.PC, 2);
      	if (op4) {
          if (op4 & 0x80)
            op4 |= jumpmasks[2];
          jumpaddr = (saturn.PC + op4) & 0xfffff;
	  saturn.PC = jumpaddr;
        } else {
	  saturn.PC = pop_return_addr();
        }
      } else {
        saturn.PC += 5;
      }
      return 0;
    case 0xa:
      op3 = read_nibble(saturn.PC + 2);
      if (op2 < 8) {
	switch(op3) {
	  case 0:	/* A=A+B */
	    saturn.PC += 3;
	    add_register(saturn.A, saturn.A, saturn.B, op2);
	    return 0;
	  case 1:	/* B=B+C */
	    saturn.PC += 3;
	    add_register(saturn.B, saturn.B, saturn.C, op2);
	    return 0;
	  case 2:	/* C=C+A */
	    saturn.PC += 3;
	    add_register(saturn.C, saturn.C, saturn.A, op2);
	    return 0;
	  case 3:	/* D=D+C */
	    saturn.PC += 3;
	    add_register(saturn.D, saturn.D, saturn.C, op2);
	    return 0;
	  case 4:	/* A=A+A */
	    saturn.PC += 3;
	    add_register(saturn.A, saturn.A, saturn.A, op2);
	    return 0;
	  case 5:	/* B=B+B */
	    saturn.PC += 3;
	    add_register(saturn.B, saturn.B, saturn.B, op2);
	    return 0;
	  case 6:	/* C=C+C */
	    saturn.PC += 3;
	    add_register(saturn.C, saturn.C, saturn.C, op2);
	    return 0;
	  case 7:	/* D=D+D */
	    saturn.PC += 3;
	    add_register(saturn.D, saturn.D, saturn.D, op2);
	    return 0;
	  case 8:	/* B=B+A */
	    saturn.PC += 3;
	    add_register(saturn.B, saturn.B, saturn.A, op2);
	    return 0;
	  case 9:	/* C=C+B */
	    saturn.PC += 3;
	    add_register(saturn.C, saturn.C, saturn.B, op2);
	    return 0;
	  case 0xa:	/* A=A+C */
	    saturn.PC += 3;
	    add_register(saturn.A, saturn.A, saturn.C, op2);
	    return 0;
	  case 0xb:	/* C=C+D */
	    saturn.PC += 3;
	    add_register(saturn.C, saturn.C, saturn.D, op2);
	    return 0;
	  case 0xc:	/* A=A-1 */
	    saturn.PC += 3;
	    dec_register(saturn.A, op2);
	    return 0;
	  case 0xd:	/* B=B-1 */
	    saturn.PC += 3;
	    dec_register(saturn.B, op2);
	    return 0;
	  case 0xe:	/* C=C-1 */
	    saturn.PC += 3;
	    dec_register(saturn.C, op2);
	    return 0;
	  case 0xf:	/* D=D-1 */
	    saturn.PC += 3;
	    dec_register(saturn.D, op2);
	    return 0;
	  default:
	    return 1;
	}
      } else {
	op2 &= 7;
	switch(op3) {
	  case 0:	/* A=0 */
	    saturn.PC += 3;
	    zero_register(saturn.A, op2);
	    return 0;
	  case 1:	/* B=0 */
	    saturn.PC += 3;
	    zero_register(saturn.B, op2);
	    return 0;
	  case 2:	/* C=0 */
	    saturn.PC += 3;
	    zero_register(saturn.C, op2);
	    return 0;
	  case 3:	/* D=0 */
	    saturn.PC += 3;
	    zero_register(saturn.D, op2);
	    return 0;
	  case 4:	/* A=B */
	    saturn.PC += 3;
	    copy_register(saturn.A, saturn.B, op2);
	    return 0;
	  case 5:	/* B=C */
	    saturn.PC += 3;
	    copy_register(saturn.B, saturn.C, op2);
	    return 0;
	  case 6:	/* C=A */
	    saturn.PC += 3;
	    copy_register(saturn.C, saturn.A, op2);
	    return 0;
	  case 7:	/* D=C */
	    saturn.PC += 3;
	    copy_register(saturn.D, saturn.C, op2);
	    return 0;
	  case 8:	/* B=A */
	    saturn.PC += 3;
	    copy_register(saturn.B, saturn.A, op2);
	    return 0;
	  case 9:	/* C=B */
	    saturn.PC += 3;
	    copy_register(saturn.C, saturn.B, op2);
	    return 0;
	  case 0xa:	/* A=C */
	    saturn.PC += 3;
	    copy_register(saturn.A, saturn.C, op2);
	    return 0;
	  case 0xb:	/* C=D */
	    saturn.PC += 3;
	    copy_register(saturn.C, saturn.D, op2);
	    return 0;
	  case 0xc:	/* ABEX */
	    saturn.PC += 3;
	    exchange_register(saturn.A, saturn.B, op2);
	    return 0;
	  case 0xd:	/* BCEX */
	    saturn.PC += 3;
	    exchange_register(saturn.B, saturn.C, op2);
	    return 0;
	  case 0xe:	/* ACEX */
	    saturn.PC += 3;
	    exchange_register(saturn.A, saturn.C, op2);
	    return 0;
	  case 0xf:	/* CDEX */
	    saturn.PC += 3;
	    exchange_register(saturn.C, saturn.D, op2);
	    return 0;
	  default:
	    return 1;
	}
      }
    case 0xb:
      op3 = read_nibble(saturn.PC + 2);
      if (op2 < 8) {
        switch (op3) {
          case 0:	/* A=A-B */
            saturn.PC += 3;
            sub_register(saturn.A, saturn.A, saturn.B, op2);
            return 0;
          case 1:	/* B=B-C */
            saturn.PC += 3;
            sub_register(saturn.B, saturn.B, saturn.C, op2);
            return 0;
          case 2:	/* C=C-A */
            saturn.PC += 3;
            sub_register(saturn.C, saturn.C, saturn.A, op2);
            return 0;
          case 3:	/* D=D-C */
            saturn.PC += 3;
            sub_register(saturn.D, saturn.D, saturn.C, op2);
            return 0;
          case 4:	/* A=A+1 */
            saturn.PC += 3;
	    inc_register(saturn.A, op2);
            return 0;
          case 5:	/* B=B+1 */
            saturn.PC += 3;
	    inc_register(saturn.B, op2);
            return 0;
          case 6:	/* C=C+1 */
            saturn.PC += 3;
	    inc_register(saturn.C, op2);
            return 0;
          case 7:	/* D=D+1 */
            saturn.PC += 3;
	    inc_register(saturn.D, op2);
            return 0;
          case 8:	/* B=B-A */
            saturn.PC += 3;
            sub_register(saturn.B, saturn.B, saturn.A, op2);
            return 0;
          case 9:	/* C=C-B */
            saturn.PC += 3;
            sub_register(saturn.C, saturn.C, saturn.B, op2);
            return 0;
          case 0xa:	/* A=A-C */
            saturn.PC += 3;
            sub_register(saturn.A, saturn.A, saturn.C, op2);
            return 0;
          case 0xb:	/* C=C-D */
            saturn.PC += 3;
            sub_register(saturn.C, saturn.C, saturn.D, op2);
            return 0;
          case 0xc:	/* A=B-A */
            saturn.PC += 3;
            sub_register(saturn.A, saturn.B, saturn.A, op2);
            return 0;
          case 0xd:	/* B=C-B */
            saturn.PC += 3;
            sub_register(saturn.B, saturn.C, saturn.B, op2);
            return 0;
          case 0xe:	/* C=A-C */
            saturn.PC += 3;
            sub_register(saturn.C, saturn.A, saturn.C, op2);
            return 0;
          case 0xf:	/* D=C-D */
            saturn.PC += 3;
            sub_register(saturn.D, saturn.C, saturn.D, op2);
            return 0;
          default:
            return 1;
        }
      } else {
	op2 &= 7;
        switch (op3) {
          case 0:	/* ASL */
            saturn.PC += 3;
            shift_left_register(saturn.A, op2);
            return 0;
          case 1:	/* BSL */
            saturn.PC += 3;
            shift_left_register(saturn.B, op2);
            return 0;
          case 2:	/* CSL */
            saturn.PC += 3;
            shift_left_register(saturn.C, op2);
            return 0;
          case 3:	/* DSL */
            saturn.PC += 3;
            shift_left_register(saturn.D, op2);
            return 0;
          case 4:	/* ASR */
            saturn.PC += 3;
            shift_right_register(saturn.A, op2);
            return 0;
          case 5:	/* BSR */
            saturn.PC += 3;
            shift_right_register(saturn.B, op2);
            return 0;
          case 6:	/* CSR */
            saturn.PC += 3;
            shift_right_register(saturn.C, op2);
            return 0;
          case 7:	/* DSR */
            saturn.PC += 3;
            shift_right_register(saturn.D, op2);
            return 0;
          case 8:	/* A=-A */
            saturn.PC += 3;
	    complement_2_register(saturn.A, op2);
            return 0;
          case 9:	/* B=-B */
            saturn.PC += 3;
	    complement_2_register(saturn.B, op2);
            return 0;
          case 0xa:	/* C=-C */
            saturn.PC += 3;
	    complement_2_register(saturn.C, op2);
            return 0;
          case 0xb:	/* D=-D */
            saturn.PC += 3;
	    complement_2_register(saturn.D, op2);
            return 0;
          case 0xc:	/* A=-A-1 */
            saturn.PC += 3;
	    complement_1_register(saturn.A, op2);
            return 0;
          case 0xd:	/* B=-B-1 */
            saturn.PC += 3;
	    complement_1_register(saturn.B, op2);
            return 0;
          case 0xe:	/* C=-C-1 */
            saturn.PC += 3;
	    complement_1_register(saturn.C, op2);
            return 0;
          case 0xf:	/* D=-D-1 */
            saturn.PC += 3;
	    complement_1_register(saturn.D, op2);
            return 0;
          default:
            return 1;
        }
      }
    case 0xc:
      switch(op2) {
        case 0:		/* A=A+B */
	  saturn.PC += 2;
	  add_register(saturn.A, saturn.A, saturn.B, A_FIELD);
          return 0;
        case 1:		/* B=B+C */
	  saturn.PC += 2;
	  add_register(saturn.B, saturn.B, saturn.C, A_FIELD);
          return 0;
        case 2:		/* C=C+A */
	  saturn.PC += 2;
	  add_register(saturn.C, saturn.C, saturn.A, A_FIELD);
          return 0;
        case 3:		/* D=D+C */
	  saturn.PC += 2;
	  add_register(saturn.D, saturn.D, saturn.C, A_FIELD);
          return 0;
        case 4:		/* A=A+A */
	  saturn.PC += 2;
	  add_register(saturn.A, saturn.A, saturn.A, A_FIELD);
          return 0;
        case 5:		/* B=B+B */
	  saturn.PC += 2;
	  add_register(saturn.B, saturn.B, saturn.B, A_FIELD);
          return 0;
        case 6:		/* C=C+C */
	  saturn.PC += 2;
	  add_register(saturn.C, saturn.C, saturn.C, A_FIELD);
          return 0;
        case 7:		/* D=D+D */
	  saturn.PC += 2;
	  add_register(saturn.D, saturn.D, saturn.D, A_FIELD);
          return 0;
        case 8:		/* B=B+A */
	  saturn.PC += 2;
	  add_register(saturn.B, saturn.B, saturn.A, A_FIELD);
          return 0;
        case 9:		/* C=C+B */
	  saturn.PC += 2;
	  add_register(saturn.C, saturn.C, saturn.B, A_FIELD);
          return 0;
        case 0xa:	/* A=A+C */
	  saturn.PC += 2;
	  add_register(saturn.A, saturn.A, saturn.C, A_FIELD);
          return 0;
        case 0xb:	/* C=C+D */
	  saturn.PC += 2;
	  add_register(saturn.C, saturn.C, saturn.D, A_FIELD);
          return 0;
        case 0xc:	/* A=A-1 */
	  saturn.PC += 2;
	  dec_register(saturn.A, A_FIELD);
	  return 0;
        case 0xd:	/* B=B-1 */
	  saturn.PC += 2;
	  dec_register(saturn.B, A_FIELD);
	  return 0;
        case 0xe:	/* C=C-1 */
	  saturn.PC += 2;
	  dec_register(saturn.C, A_FIELD);
	  return 0;
        case 0xf:	/* D=D-1 */
	  saturn.PC += 2;
	  dec_register(saturn.D, A_FIELD);
	  return 0;
        default:
          return 1;
      }
    case 0xd:
      switch(op2) {
	case 0:		/* A=0 */
	  saturn.PC += 2;
	  zero_register(saturn.A, A_FIELD);
	  return 0;
	case 1:		/* B=0 */
	  saturn.PC += 2;
	  zero_register(saturn.B, A_FIELD);
	  return 0;
	case 2:		/* C=0 */
	  saturn.PC += 2;
	  zero_register(saturn.C, A_FIELD);
	  return 0;
	case 3:		/* D=0 */
	  saturn.PC += 2;
	  zero_register(saturn.D, A_FIELD);
	  return 0;
        case 4:		/* A=B */
	  saturn.PC += 2;
	  copy_register(saturn.A, saturn.B, A_FIELD);
	  return 0;
	case 5:		/* B=C */
	  saturn.PC += 2;
	  copy_register(saturn.B, saturn.C, A_FIELD);
	  return 0;
	case 6:		/* C=A */
	  saturn.PC += 2;
	  copy_register(saturn.C, saturn.A, A_FIELD);
	  return 0;
	case 7:		/* D=C */
	  saturn.PC += 2;
	  copy_register(saturn.D, saturn.C, A_FIELD);
	  return 0;
	case 8:		/* B=A */
	  saturn.PC += 2;
	  copy_register(saturn.B, saturn.A, A_FIELD);
	  return 0;
	case 9:		/* C=B */
	  saturn.PC += 2;
	  copy_register(saturn.C, saturn.B, A_FIELD);
	  return 0;
	case 0xa:	/* A=C */
	  saturn.PC += 2;
	  copy_register(saturn.A, saturn.C, A_FIELD);
	  return 0;
	case 0xb:	/* C=D */
	  saturn.PC += 2;
	  copy_register(saturn.C, saturn.D, A_FIELD);
	  return 0;
	case 0xc:	/* ABEX */
	  saturn.PC += 2;
	  exchange_register(saturn.A, saturn.B, A_FIELD);
	  return 0;
	case 0xd:	/* BCEX */
	  saturn.PC += 2;
	  exchange_register(saturn.B, saturn.C, A_FIELD);
	  return 0;
	case 0xe:	/* ACEX */
	  saturn.PC += 2;
	  exchange_register(saturn.A, saturn.C, A_FIELD);
	  return 0;
	case 0xf:	/* CDEX */
	  saturn.PC += 2;
	  exchange_register(saturn.C, saturn.D, A_FIELD);
	  return 0;
	default:
	  return 1;
      }
    case 0xe:
      switch (op2) {
        case 0:	/* A=A-B */
          saturn.PC += 2;
          sub_register(saturn.A, saturn.A, saturn.B, A_FIELD);
          return 0;
        case 1:	/* B=B-C */
          saturn.PC += 2;
          sub_register(saturn.B, saturn.B, saturn.C, A_FIELD);
          return 0;
        case 2:	/* C=C-A */
          saturn.PC += 2;
          sub_register(saturn.C, saturn.C, saturn.A, A_FIELD);
          return 0;
        case 3:	/* D=D-C */
          saturn.PC += 2;
          sub_register(saturn.D, saturn.D, saturn.C, A_FIELD);
          return 0;
        case 4:	/* A=A+1 */
          saturn.PC += 2;
	  inc_register(saturn.A, A_FIELD);
          return 0;
        case 5:	/* B=B+1 */
          saturn.PC += 2;
	  inc_register(saturn.B, A_FIELD);
          return 0;
        case 6:	/* C=C+1 */
          saturn.PC += 2;
	  inc_register(saturn.C, A_FIELD);
          return 0;
        case 7:	/* D=D+1 */
          saturn.PC += 2;
	  inc_register(saturn.D, A_FIELD);
          return 0;
        case 8:	/* B=B-A */
          saturn.PC += 2;
          sub_register(saturn.B, saturn.B, saturn.A, A_FIELD);
          return 0;
        case 9:	/* C=C-B */
          saturn.PC += 2;
          sub_register(saturn.C, saturn.C, saturn.B, A_FIELD);
          return 0;
        case 0xa:	/* A=A-C */
          saturn.PC += 2;
          sub_register(saturn.A, saturn.A, saturn.C, A_FIELD);
          return 0;
        case 0xb:	/* C=C-D */
          saturn.PC += 2;
          sub_register(saturn.C, saturn.C, saturn.D, A_FIELD);
          return 0;
        case 0xc:	/* A=B-A */
          saturn.PC += 2;
          sub_register(saturn.A, saturn.B, saturn.A, A_FIELD);
          return 0;
        case 0xd:	/* B=C-B */
          saturn.PC += 2;
          sub_register(saturn.B, saturn.C, saturn.B, A_FIELD);
          return 0;
        case 0xe:	/* C=A-C */
          saturn.PC += 2;
          sub_register(saturn.C, saturn.A, saturn.C, A_FIELD);
          return 0;
        case 0xf:	/* D=C-D */
          saturn.PC += 2;
          sub_register(saturn.D, saturn.C, saturn.D, A_FIELD);
          return 0;
        default:
          return 1;
      }
    case 0xf:
      switch (op2) {
        case 0:	/* ASL */
          saturn.PC += 2;
          shift_left_register(saturn.A, A_FIELD);
          return 0;
        case 1:	/* BSL */
          saturn.PC += 2;
          shift_left_register(saturn.B, A_FIELD);
          return 0;
        case 2:	/* CSL */
          saturn.PC += 2;
          shift_left_register(saturn.C, A_FIELD);
          return 0;
        case 3:	/* DSL */
          saturn.PC += 2;
          shift_left_register(saturn.D, A_FIELD);
          return 0;
        case 4:	/* ASR */
          saturn.PC += 2;
          shift_right_register(saturn.A, A_FIELD);
          return 0;
        case 5:	/* BSR */
          saturn.PC += 2;
          shift_right_register(saturn.B, A_FIELD);
          return 0;
        case 6:	/* CSR */
          saturn.PC += 2;
          shift_right_register(saturn.C, A_FIELD);
          return 0;
        case 7:	/* DSR */
          saturn.PC += 2;
          shift_right_register(saturn.D, A_FIELD);
          return 0;
        case 8:	/* A=-A */
          saturn.PC += 2;
	  complement_2_register(saturn.A, A_FIELD);
          return 0;
        case 9:	/* B=-B */
          saturn.PC += 2;
	  complement_2_register(saturn.B, A_FIELD);
          return 0;
        case 0xa:	/* C=-C */
          saturn.PC += 2;
	  complement_2_register(saturn.C, A_FIELD);
          return 0;
        case 0xb:	/* D=-D */
          saturn.PC += 2;
	  complement_2_register(saturn.D, A_FIELD);
          return 0;
        case 0xc:	/* A=-A-1 */
          saturn.PC += 2;
	  complement_1_register(saturn.A, A_FIELD);
          return 0;
        case 0xd:	/* B=-B-1 */
          saturn.PC += 2;
	  complement_1_register(saturn.B, A_FIELD);
          return 0;
        case 0xe:	/* C=-C-1 */
          saturn.PC += 2;
	  complement_1_register(saturn.C, A_FIELD);
          return 0;
        case 0xf:	/* D=-D-1 */
          saturn.PC += 2;
	  complement_1_register(saturn.D, A_FIELD);
          return 0;
        default:
          return 1;
      }
    default:
      return 1;
  }
}

inline int
#ifdef __FunctionProto__
step_instruction(void)
#else
step_instruction()
#endif
{
  int op0, op1, op2, op3;
  int stop = 0;

  jumpaddr = 0;

  op0 = read_nibble(saturn.PC);
 // LOGI("----- %d", op0);
  switch (op0) {
    case 0:
      op1 = read_nibble(saturn.PC + 1);
      switch (op1) {
        case 0:	/* RTNSXM */
          saturn.XM = 1;
	  saturn.PC = pop_return_addr();
	  break;
        case 1:	/* RTN */
	  saturn.PC = pop_return_addr();
	  break;
        case 2:	/* RTNSC */
	  saturn.CARRY = 1;
	  saturn.PC = pop_return_addr();
	  break;
        case 3:	/* RTNCC */
	  saturn.CARRY = 0;
	  saturn.PC = pop_return_addr();
	  break;
        case 4:	/* SETHEX */
	  saturn.PC += 2;
	  saturn.hexmode = HEX;
	  break;
        case 5:	/* SETDEC */
	  saturn.PC += 2;
	  saturn.hexmode = DEC;
	  break;
        case 6:	/* RSTK=C */
          jumpaddr = dat_to_addr(saturn.C);
     	  push_return_addr(jumpaddr);
          saturn.PC += 2;
	  break;
        case 7:	/* C=RSTK */
          saturn.PC += 2;
	  jumpaddr = pop_return_addr();
          addr_to_dat(jumpaddr, saturn.C);
	  break;
        case 8:	/* CLRST */
          saturn.PC += 2;
          clear_status();
	  break;
        case 9:	/* C=ST */
          saturn.PC += 2;
          status_to_register(saturn.C);
	  break;
        case 0xa:	/* ST=C */
          saturn.PC += 2;
          register_to_status(saturn.C);
	  break;
        case 0xb:	/* CSTEX */
          saturn.PC += 2;
          swap_register_status(saturn.C);
	  break;
        case 0xc:	/* P=P+1 */
	  saturn.PC += 2;
          if (saturn.P == 0xf) {
            saturn.P = 0;
            saturn.CARRY = 1;
          } else {
            saturn.P += 1;
            saturn.CARRY = 0;
          }
	  break;
        case 0xd:	/* P=P-1 */
	  saturn.PC += 2;
          if (saturn.P == 0) {
            saturn.P = 0xf;
            saturn.CARRY = 1;
          } else {
            saturn.P -= 1;
            saturn.CARRY = 0;
          }
	  break;
        case 0xe:
          op2 = read_nibble(saturn.PC + 2);
          op3 = read_nibble(saturn.PC + 3);
          switch(op3) {
            case 0:	/* A=A&B */
              saturn.PC += 4;
              and_register(saturn.A, saturn.A, saturn.B, op2);
              break;
            case 1:	/* B=B&C */
              saturn.PC += 4;
              and_register(saturn.B, saturn.B, saturn.C, op2);
              break;
            case 2:	/* C=C&A */
              saturn.PC += 4;
              and_register(saturn.C, saturn.C, saturn.A, op2);
              break;
            case 3:	/* D=D&C */
              saturn.PC += 4;
              and_register(saturn.D, saturn.D, saturn.C, op2);
              break;
            case 4:	/* B=B&A */
              saturn.PC += 4;
              and_register(saturn.B, saturn.B, saturn.A, op2);
              break;
            case 5:	/* C=C&B */
              saturn.PC += 4;
              and_register(saturn.C, saturn.C, saturn.B, op2);
              break;
            case 6:	/* A=A&C */
              saturn.PC += 4;
              and_register(saturn.A, saturn.A, saturn.C, op2);
              break;
            case 7:	/* C=C&D */
              saturn.PC += 4;
              and_register(saturn.C, saturn.C, saturn.D, op2);
              break;
            case 8:	/* A=A!B */
              saturn.PC += 4;
              or_register(saturn.A, saturn.A, saturn.B, op2);
              break;
            case 9:	/* B=B!C */
              saturn.PC += 4;
              or_register(saturn.B, saturn.B, saturn.C, op2);
              break;
            case 0xa:	/* C=C!A */
              saturn.PC += 4;
              or_register(saturn.C, saturn.C, saturn.A, op2);
              break;
            case 0xb:	/* D=D!C */
              saturn.PC += 4;
              or_register(saturn.D, saturn.D, saturn.C, op2);
              break;
            case 0xc:	/* B=B!A */
              saturn.PC += 4;
              or_register(saturn.B, saturn.B, saturn.A, op2);
              break;
            case 0xd:	/* C=C!B */
              saturn.PC += 4;
              or_register(saturn.C, saturn.C, saturn.B, op2);
              break;
            case 0xe:	/* A=A!C */
              saturn.PC += 4;
              or_register(saturn.A, saturn.A, saturn.C, op2);
              break;
            case 0xf:	/* C=C!D */
              saturn.PC += 4;
              or_register(saturn.C, saturn.C, saturn.D, op2);
              break;
            default:
              stop = 1;
              break;
          }
	  break;
        case 0xf:	/* RTI */
          do_return_interupt();
	  break;
        default:
          stop = 1;
          break;
      }
      break;
    case 1:
      stop = decode_group_1();
      break;
    case 2:
      op2 = read_nibble(saturn.PC + 1);
      saturn.PC += 2;
      saturn.P = op2;
      break;
    case 3:
      op2 = read_nibble(saturn.PC + 1);
      load_constant(saturn.C, op2 + 1, saturn.PC + 2);
      saturn.PC += 3 + op2;
      break;
    case 4:
      op2 = read_nibbles(saturn.PC + 1, 2);
      if (op2 == 0x02) {
        saturn.PC += 3;
      } else if (saturn.CARRY != 0) {
        if (op2) {
          if (op2 & 0x80)
            op2 |= jumpmasks[2];
          jumpaddr = (saturn.PC + op2 + 1) & 0xfffff;
          saturn.PC = jumpaddr;
        } else {
	  saturn.PC = pop_return_addr();
        }
      } else {
        saturn.PC += 3;
      }
      break;
    case 5:
      if (saturn.CARRY == 0) {
	op2 = read_nibbles(saturn.PC + 1, 2);
        if (op2) {
          if (op2 & 0x80)
            op2 |= jumpmasks[2];
          jumpaddr = (saturn.PC + op2 + 1) & 0xfffff;
          saturn.PC = jumpaddr;
        } else {
	  saturn.PC = pop_return_addr();
        }
      } else {
        saturn.PC += 3;
      }
      break;
    case 6:
      op2 = read_nibbles(saturn.PC + 1, 3);
      if (op2 == 0x003) {
        saturn.PC += 4;
      } else if(op2 == 0x004) {
        op3 = read_nibbles(saturn.PC + 4, 1);
        saturn.PC += 5;
        if (op3 != 0) {
          enter_debugger |= TRAP_INSTRUCTION;
	  return 1;
        }
      } else {
        if (op2 & 0x800)
          op2 |= jumpmasks[3];
        jumpaddr = (op2 + saturn.PC + 1) & 0xfffff;
        saturn.PC = jumpaddr;
      }
      break;
    case 7:
      op2 = read_nibbles(saturn.PC + 1, 3);
      if (op2 & 0x800)
        op2 |= jumpmasks[3];
      jumpaddr = (op2 + saturn.PC + 4) & 0xfffff;
      push_return_addr(saturn.PC+4);
      saturn.PC = jumpaddr;
      break;
    default:
      stop = decode_8_thru_f(op0);
      break;
  }
//  LOGI("-----2");
  instructions++;
  if (stop) {
    enter_debugger |= ILLEGAL_INSTRUCTION;
  }
  return stop;
}

inline void
#ifdef __FunctionProto__
schedule(void)
#else
schedule()
#endif
{
  t1_t2_ticks		ticks;
  unsigned long		steps;
  static unsigned long	old_stat_instr;
  static unsigned long	old_sched_instr;

  steps = instructions - old_sched_instr;
  old_sched_instr = instructions;

#ifdef DEBUG_SCHED
  fprintf(stderr, "schedule called after %ld instructions\n", steps);
#endif

  if ((sched_timer2 -= steps) <= 0) {
    if (!saturn.intenable) {
      sched_timer2 = SCHED_TIMER2;
    } else {
      sched_timer2 = saturn.t2_tick;
    }
    saturn.t2_instr += steps;
    if (saturn.t2_ctrl & 0x01) {
      saturn.timer2--;
    }
    if (saturn.timer2 == 0 && (saturn.t2_ctrl & 0x02)) {
      saturn.t2_ctrl |= 0x08;
      do_interupt();
    }
  }
  schedule_event = sched_timer2;

#ifdef DEBUG_SCHED
  fprintf(stderr, "next timer 2 step: %ld, event: %ld\n",
          sched_timer2, schedule_event);
#endif

  if (device_check) {
    device_check = 0;
    if ((sched_display -= steps) <= 0) {
      if (device.display_touched) device.display_touched -= steps;
      if (device.display_touched < 0) device.display_touched = 1;
#ifdef DEBUG_DISP_SCHED
      fprintf(stderr, "check_device: disp_when %d, disp_touched %d\n",
              sched_display, device.display_touched);
#endif
    }
    check_devices();
    sched_display = SCHED_NEVER;
    if (device.display_touched) {
      if (device.display_touched < sched_display)
        sched_display = device.display_touched - 1;
      if (sched_display < schedule_event) schedule_event = sched_display;
    }
  }

  if ((sched_receive -= steps) <= 0) {
    sched_receive = SCHED_RECEIVE;
    if ((saturn.rcs & 0x01) == 0) {
      receive_char();
    }
  }
  if (sched_receive < schedule_event) schedule_event = sched_receive;

#ifdef DEBUG_SCHED
  fprintf(stderr, "next receive: %ld, event: %ld\n",
          sched_receive, schedule_event);
#endif

  if ((sched_adjtime -= steps) <= 0) {

    sched_adjtime = SCHED_ADJTIME;

    if (saturn.PC < SrvcIoStart || saturn.PC > SrvcIoEnd) {

      ticks = get_t1_t2();
      if (saturn.t2_ctrl & 0x01) {
        saturn.timer2 = ticks.t2_ticks;
      }

      if ((saturn.t2_ctrl & 0x08) == 0 && saturn.timer2 <= 0) {
        if (saturn.t2_ctrl & 0x02) {
          saturn.t2_ctrl |= 0x08;
          do_interupt();
        }
      }

      adj_time_pending = 0;

      saturn.timer1 = set_t1 - ticks.t1_ticks;
      if ((saturn.t1_ctrl & 0x08) == 0 && saturn.timer1 <= 0) {
        if (saturn.t1_ctrl & 0x02) {
          saturn.t1_ctrl |= 0x08;
          do_interupt();
        }
      }
      saturn.timer1 &= 0x0f;

    } else {

      adj_time_pending = 1;

    }
  }
  if (sched_adjtime < schedule_event) schedule_event = sched_adjtime;

#ifdef DEBUG_SCHED
  fprintf(stderr, "next adjtime: %ld, event: %ld\n",
          sched_adjtime, schedule_event);
#endif

  if ((sched_timer1 -= steps) <= 0) {
    if (!saturn.intenable) {
      sched_timer1 = SCHED_TIMER1;
    } else {
      sched_timer1 = saturn.t1_tick;
    }
    saturn.t1_instr += steps;
    saturn.timer1 = (saturn.timer1 - 1) & 0xf;
    if (saturn.timer1 == 0 && (saturn.t1_ctrl & 0x02)) {
      saturn.t1_ctrl |= 0x08;
      do_interupt();
    }
  }
  if (sched_timer1 < schedule_event) schedule_event = sched_timer1;

#ifdef DEBUG_SCHED
  fprintf(stderr, "next timer 1 step: %ld, event: %ld\n",
          sched_timer1, schedule_event);
#endif

  if ((sched_statistics -= steps) <= 0) {
    sched_statistics = SCHED_STATISTICS;
    run = get_timer(RUN_TIMER);
    s_1 = (run.hi << 19) | (run.lo >> 13);
    s_16 = (run.hi << 23) | (run.lo >> 9);
    delta_t_1 = s_1 - old_s_1;
    delta_t_16 = s_16 - old_s_16;
    old_s_1 = s_1;
    old_s_16 = s_16;
    delta_i = instructions - old_stat_instr;
    old_stat_instr = instructions;
    if (delta_t_1 > 0) {
      t1_i_per_tick = ((NR_SAMPLES - 1) * t1_i_per_tick +
                      (delta_i / delta_t_16)) / NR_SAMPLES;
      t2_i_per_tick = t1_i_per_tick / 512;
      saturn.i_per_s = ((NR_SAMPLES - 1) * saturn.i_per_s +
                       (delta_i / delta_t_1)) / NR_SAMPLES;
    } else {
      t1_i_per_tick = 8192;
      t2_i_per_tick = 16;
    }
    saturn.t1_tick = t1_i_per_tick;
    saturn.t2_tick = t2_i_per_tick;

#ifdef DEBUG_TIMER
    if (delta_t_1 > 0) {
#if 0
      fprintf(stderr, "I/s = %ld, T1 I/TICK = %d (%ld), T2 I/TICK = %d (%ld)\n",
              saturn.i_per_s, saturn.t1_tick, t1_i_per_tick,
              saturn.t2_tick, t2_i_per_tick);
#else
      fprintf(stderr, "I/s = %ld, T1 I/TICK = %d, T2 I/TICK = %d (%ld)\n",
              saturn.i_per_s, saturn.t1_tick, saturn.t2_tick, t2_i_per_tick);
#endif
    }
#endif
  }
  if (sched_statistics < schedule_event)
    schedule_event = sched_statistics;

#ifdef DEBUG_SCHED
  fprintf(stderr, "next statistics: %ld, event: %ld\n",
          sched_statistics, schedule_event);
#endif

  if ((sched_instr_rollover -= steps) <= 0) {
    sched_instr_rollover = SCHED_INSTR_ROLLOVER;
    instructions = 1;
    old_sched_instr = 1;
    reset_timer(RUN_TIMER);
    reset_timer(IDLE_TIMER);
    start_timer(RUN_TIMER);
  }
  if (sched_instr_rollover < schedule_event)
    schedule_event = sched_instr_rollover;

#ifdef DEBUG_SCHED
  fprintf(stderr, "next instruction rollover: %ld, event: %ld\n",
          sched_instr_rollover, schedule_event);
#endif

  schedule_event--;
/*
  if (got_alarm) {
    got_alarm = 0;
#ifdef HAVE_XSHM
    if (disp.display_update) refresh_display();
#endif
    GetEvent();
    //usleep(500);
  }
*/

  GetEvent();
}

int
#ifdef __FunctionProto__
emulate(void)
#else
emulate()
#endif
{
  struct timeval  tv;
  struct timeval  tv2;
#ifndef SOLARIS
  struct timezone tz;
#endif

  reset_timer(T1_TIMER);
  reset_timer(RUN_TIMER);
  reset_timer(IDLE_TIMER);

  start_timer(T1_TIMER);
  set_accesstime();

  start_timer(RUN_TIMER);

  sched_timer1 = t1_i_per_tick = saturn.t1_tick;
  sched_timer2 = t2_i_per_tick = saturn.t2_tick;

  set_t1 = saturn.timer1;

/*

   do {
    step_instruction();
	
    if (--schedule_event <= 0) {
      schedule();
    }
  } while (exit_state);
*/

  do {
	  
    step_instruction();


    /* gettimeofday(&tv, &tz);
    while ((tv.tv_sec == tv2.tv_sec) && ((tv.tv_usec - tv2.tv_usec) < 2)) {
	gettimeofday(&tv, &tz);
    }
    tv2.tv_usec = tv.tv_usec;
    tv2.tv_sec = tv.tv_sec;

    usleep(3);
*/
    if (schedule_event-- == 0)
      {
        schedule();
      }
  } while (!enter_debugger && exit_state); // exit_state

  return 0;
}
