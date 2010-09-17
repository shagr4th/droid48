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

/* $Log: mmu.h,v $
 * Revision 1.1  1995/01/11  18:11:25  ecd
 * Initial revision
 *
 *
 * $Id: mmu.h,v 1.1 1995/01/11 18:11:25 ecd Exp ecd $
 */
#ifndef _MMU_H
#define _MMU_H 1


#define NR_MCTL		6

#define MCTL_MMIO_SX	0
#define MCTL_SysRAM_SX	1
#define MCTL_PORT1_SX	2
#define MCTL_PORT2_SX	3
#define MCTL_EXTRA_SX	4
#define MCTL_SysROM_SX	5

#define MCTL_MMIO_GX	0
#define MCTL_SysRAM_GX	1
#define MCTL_BANK_GX	2
#define MCTL_PORT1_GX	3
#define MCTL_PORT2_GX	4
#define MCTL_SysROM_GX	5


#if 0
extern void	init_mmu	__ProtoType__((void));
extern void	reset_mmu	__ProtoType__((void));
#endif

#endif /* !_MMU_H */
