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

/* $Log: hp.h,v $
 * Revision 1.4  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.3  1994/11/02  14:51:27  ecd
 * minor fix
 *
 * Revision 1.3  1994/11/02  14:51:27  ecd
 * minor fix
 *
 * Revision 1.2  1994/10/06  16:28:03  ecd
 * changed char to unsigned
 *
 * Revision 1.1  1994/09/13  15:05:11  ecd
 * Initial revision
 *
 *
 * $Id: hp.h,v 1.4 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _HP_H
#define _HP_H 1

#include "bitmaps/hp.h"

#include "bitmaps/hp48sx.h"

#include "bitmaps/hp48gx.h"

#include "bitmaps/science.h"

#define gx_128K_ram_x_hot 1
#define gx_128K_ram_y_hot 8
#include "bitmaps/gx_128K_ram.h"

#define gx_silver_x_hot 0
#define gx_silver_y_hot 8
#include "bitmaps/gx_silver.h"

#define gx_green_x_hot 11
#define gx_green_y_hot 0
#include "bitmaps/gx_green.h"

#endif /* !_HP_H */
