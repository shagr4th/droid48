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

/* $Log: version.h,v $
 * Revision 1.2  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.1  1994/11/28  02:27:59  ecd
 * Initial revision
 *
 * Revision 1.1  1994/11/28  02:27:59  ecd
 * Initial revision
 *
 *
 * $Id: version.h,v 1.2 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _VERSION_H
#define _VERSION_H 1

extern int   VERSION_MAJOR;
extern int   VERSION_MINOR;
extern int   PATCHLEVEL;
extern int   COMPILE_VERSION;
extern char *COMPILE_TIME;
extern char *COMPILE_BY;

#endif /* !_VERSION_H */
