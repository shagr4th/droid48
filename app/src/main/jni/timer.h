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

/* $Log: timer.h,v $
 * Revision 1.3  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.2  1994/11/02  14:51:27  ecd
 * new functions: set_accesstime()
 *
 * Revision 1.2  1994/11/02  14:51:27  ecd
 * new functions: set_accesstime()
 *
 *
 * $Id: timer.h,v 1.3 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _TIMER_H
#define _TIMER_H 1

#include "global.h"
#include "hp48.h"

#define NR_TIMERS	4

#define T1_TIMER	0
#define T2_TIMER	1
#define RUN_TIMER	2
#define IDLE_TIMER	3

typedef struct t1_t2_ticks {
  unsigned long t1_ticks;
  unsigned long t2_ticks;
} t1_t2_ticks;

extern void        reset_timer    __ProtoType__((int timer));
extern void        start_timer    __ProtoType__((int timer));
extern void        restart_timer  __ProtoType__((int timer));
extern void        stop_timer     __ProtoType__((int timer));
extern word_64     get_timer      __ProtoType__((int timer));
extern long        diff_timer     __ProtoType__((word_64 *t1, word_64 *t2));

extern t1_t2_ticks get_t1_t2      __ProtoType__((void));
extern void	   set_accesstime __ProtoType__((void));

#endif /* !_TIMER_H */
