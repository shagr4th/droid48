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

/* $Log: append.h,v $
 * Revision 1.3  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.2  1994/12/07  20:16:41  ecd
 * deleted empty line at top of file
 *
 * Revision 1.2  1994/12/07  20:16:41  ecd
 * deleted empty line at top of file
 *
 * Revision 1.1  1994/12/07  10:16:15  ecd
 * Initial revision
 *
 *
 * $Id: append.h,v 1.3 1995/01/11 18:20:01 ecd Exp ecd $
 */
#ifndef _APPEND_H
#define _APPEND_H 1

#include "global.h"

extern char *	append_str __ProtoType__((char *buf, char *string));
extern char *	append_tab __ProtoType__((char *buf));
extern char *	append_tab_16 __ProtoType__((char *buf));

#endif /* !_APPEND_H */
