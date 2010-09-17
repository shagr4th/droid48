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

/* $Log: romio.h,v $
 * Revision 1.1  1995/01/11  18:11:25  ecd
 * Initial revision
 *
 *
 * $Id: romio.h,v 1.1 1995/01/11 18:11:25 ecd Exp ecd $
 */

#ifndef _ROMIO_H
#define _ROMIO_H 1

#include "global.h"

#define ROM_SIZE_SX 0x080000
#define ROM_SIZE_GX 0x100000

extern unsigned int opt_gx;
extern unsigned int rom_size;

extern int read_rom_file __ProtoType__((char *name,
                                        unsigned char **mem,
                                        int *size));

#endif /* !_ROMIO_H */
