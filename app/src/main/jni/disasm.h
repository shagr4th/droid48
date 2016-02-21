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

/* $Log: disasm.h,v $
 * Revision 1.3  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.2  1994/11/02  14:51:27  ecd
 * changed ALONZO to CLASS
 *
 * Revision 1.2  1994/11/02  14:51:27  ecd
 * changed ALONZO to CLASS
 *
 * Revision 1.1  1994/10/09  20:26:35  ecd
 * Initial revision
 *
 *
 * $Id: disasm.h,v 1.3 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _DISASM_H
#define _DISASM_H 1

#include "global.h"
#include "hp48.h"

#define HP_MNEMONICS            0
#define CLASS_MNEMONICS		1

extern int disassembler_mode;
extern char *mode_name[];

extern word_20 disassemble __ProtoType__((word_20 addr, char *out));

#endif /* !_DISASM_H */
