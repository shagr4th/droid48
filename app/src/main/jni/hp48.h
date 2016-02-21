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

/* $Log: hp48.h,v $
 * Revision 1.11  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.10  1994/11/28  02:19:22  ecd
 * added function serial_baud()
 *
 * Revision 1.10  1994/11/28  02:19:22  ecd
 * added function serial_baud()
 *
 * Revision 1.9  1994/11/02  14:51:27  ecd
 * minor changes
 *
 * Revision 1.8  1994/10/09  20:26:35  ecd
 * changed display_t: display.skip --> display.nibs_per_line
 *
 * Revision 1.7  1994/10/05  08:33:22  ecd
 * added do_interupt definition
 *
 * Revision 1.6  1994/09/30  12:32:49  ecd
 * added some fields for REALTIME support,
 * added Scheduler stuff
 *
 * Revision 1.5  1994/09/18  15:31:58  ecd
 * started Real Time support
 *
 * Revision 1.4  1994/09/13  16:58:42  ecd
 * changed to plain X11
 *
 * Revision 1.3  1994/08/31  18:25:23  ecd
 * added read_nibble_crc
 *
 * Revision 1.2  1994/08/27  11:29:48  ecd
 * changed keyboard interrupt handling.
 *
 * Revision 1.1  1994/08/26  11:09:18  ecd
 * Initial revision
 *
 *
 *
 * $Id: hp48.h,v 1.11 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _HP48_H
#define _HP48_H 1

#include "global.h"



#include <sys/time.h>

#include "mmu.h"

#define RAM_SIZE_SX 0x10000
#define RAM_SIZE_GX 0x40000

#define P_FIELD          0
#define WP_FIELD         1
#define XS_FIELD         2
#define X_FIELD          3
#define S_FIELD          4
#define M_FIELD          5
#define B_FIELD          6
#define W_FIELD          7
#define A_FIELD         15
#define IN_FIELD        16
#define OUT_FIELD       17
#define OUTS_FIELD      18

#define DEC             10
#define HEX             16

#define NR_RSTK		 8
#define NR_PSTAT	16

typedef unsigned char  word_1;
typedef unsigned char  word_4;
typedef unsigned char  word_8;
typedef unsigned short word_12;
typedef unsigned short word_16;
typedef long	       word_20;
typedef long	       word_32;
typedef struct word_64 {
  unsigned long hi, lo;
} word_64;

typedef struct keystate_t {
  short rows[9];
} keystate_t;

typedef struct display_t {

  int  on;

  long disp_start;
  long disp_end;

  int  offset;
  int  lines;
  int  nibs_per_line;

  int  contrast;

  long menu_start;
  long menu_end;

  int  annunc;

} display_t;

typedef struct mem_cntl_t {
  short unconfigured;
  word_20 config[2];
} mem_cntl_t;

typedef struct saturn_t {

  unsigned long magic;
  char          version[4];

  unsigned char A[16], B[16], C[16], D[16];

  word_20       d[2];

#define D0	d[0]
#define D1	d[1]

  word_4        P;
  word_20       PC;

  unsigned char R0[16], R1[16], R2[16], R3[16], R4[16];
  unsigned char IN[4];
  unsigned char OUT[3];

  word_1        CARRY;

  unsigned char PSTAT[NR_PSTAT];
  unsigned char XM, SB, SR, MP;

  word_4	hexmode;

  word_20	rstk[NR_RSTK];
  short         rstkp;

  keystate_t    keybuf;

  unsigned char intenable;
  unsigned char int_pending;
  unsigned char kbd_ien;

  word_4	disp_io;

  word_4	contrast_ctrl;
  word_8	disp_test;

  word_16	crc;

  word_4	power_status;
  word_4	power_ctrl;

  word_4	mode;

  word_8	annunc;

  word_4	baud;

  word_4	card_ctrl;
  word_4	card_status;

  word_4	io_ctrl;
  word_4	rcs;
  word_4	tcs;

  word_8	rbr;
  word_8	tbr;

  word_8	sreq;

  word_4	ir_ctrl;

  word_4	base_off;

  word_4	lcr;
  word_4	lbr;

  word_4	scratch;

  word_4	base_nibble;

  word_20       disp_addr;
  word_12       line_offset;
  word_8	line_count;

  word_16       unknown;

  word_4	t1_ctrl;
  word_4	t2_ctrl;

  word_20       menu_addr;

  word_8	unknown2;

  char		timer1;		/* may NOT be unsigned !!! */
  word_32	timer2;

  long		t1_instr;
  long		t2_instr;

  short		t1_tick;
  short		t2_tick;
  long		i_per_s;

  short		bank_switch;
  mem_cntl_t	mem_cntl[NR_MCTL];

  unsigned char *rom;
  unsigned char *ram;
  unsigned char *port1;
  unsigned char *port2;

} saturn_t;

#define NIBBLES_PER_ROW 0x22

extern char    files_path    [256];
extern char    rom_filename  [256];
extern char    ram_filename  [256];
extern char    conf_filename  [256];
extern char    port1_filename [256];
extern char    port2_filename [256];

extern int		got_alarm;

extern int		set_t1;
extern long		sched_timer1;
extern long		sched_timer2;

extern int		adj_time_pending;
extern long		sched_adjtime;
extern long		schedule_event;

extern display_t	display;
extern void		init_display __ProtoType__((void));

extern saturn_t		saturn;

extern int		exit_emulator __ProtoType__((void));
extern int		init_emulator __ProtoType__((void));
extern void		init_active_stuff __ProtoType__((void));

extern int		serial_init __ProtoType__((void));
extern void		serial_baud __ProtoType__((int baud));
extern void		transmit_char __ProtoType__((void));
extern void		receive_char __ProtoType__((void));

extern void		do_kbd_int  __ProtoType__((void));
extern void		do_interupt __ProtoType__((void));

extern void		(*write_nibble) __ProtoType__((long addr, int val));
extern int		(*read_nibble) __ProtoType__((long addr));
extern int		(*read_nibble_crc) __ProtoType__((long addr));

extern int		emulate	__ProtoType__((void));
extern int		step_instruction __ProtoType__((void));
extern void		schedule __ProtoType__((void));

extern int              read_rom __ProtoType__((const char *fname));
extern int              read_files __ProtoType__((void));
extern int              write_files __ProtoType__((void));

extern void		load_addr __ProtoType__((word_20 *dat, long addr,
						 int n));
#endif /* !_HP48_H */
