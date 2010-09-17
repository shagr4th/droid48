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

/* $Log: debugger.h,v $
 * Revision 1.4  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.3  1994/11/28  02:19:22  ecd
 * catch TRAP instructions
 *
 * Revision 1.3  1994/11/28  02:19:22  ecd
 * catch TRAP instructions
 *
 * Revision 1.2  1994/11/02  14:51:27  ecd
 * added breakpoint related stuff
 *
 * Revision 1.1  1994/10/04  15:12:38  ecd
 * Initial revision
 *
 *
 * $Id: debugger.h,v 1.4 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _DEBUGGER_H
#define _DEBUGGER_H	1

#include "global.h"
#include "hp48.h"

#define	USER_INTERRUPT		1
#define	ILLEGAL_INSTRUCTION	2
#define	BREAKPOINT_HIT		4
#define	TRAP_INSTRUCTION	8

/*
 * exec_flags values
 */
#define EXEC_BKPT		1

extern int	enter_debugger;
extern int	in_debugger;
extern int	exec_flags;

extern void	init_debugger	__ProtoType__((void));
extern int	debug		__ProtoType__((void));
extern int	emulate_debug	__ProtoType__((void));

extern char    *str_nibbles	__ProtoType__((word_20 addr, int n));

#endif /* !_DEBUGGER_H */
