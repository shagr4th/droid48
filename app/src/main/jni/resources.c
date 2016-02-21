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

/* $Log: resources.c,v $
 * Revision 1.3  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.2  1994/12/07  20:20:50  ecd
 * more resource get functions
 *
 * Revision 1.2  1994/12/07  20:20:50  ecd
 * more resource get functions
 *
 * Revision 1.1  1994/12/07  10:15:47  ecd
 * Initial revision
 *
 *
 * $Id: resources.c,v 1.3 1995/01/11 18:20:01 ecd Exp ecd $
 */

/* xscreensaver, Copyright (c) 1992 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

#include "global.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "resources.h"
#include "disasm.h"
#include "errors.h"


int	verbose;
int	quiet;
int     useTerminal;
int     useSerial;
char   *serialLine;
int     useXShm;
int     useDebugger;
int     initialize;
int     resetOnStartup;
char   *romFileName;
char   *homeDirectory;

void
#ifdef __FunctionProto__
get_resources(void)
#else
get_resources()
#endif
{
 verbose = 0;
  quiet = 0;

  useXShm = 0;

  useTerminal = 0;
  useSerial = 0;
  serialLine = 0;

  initialize = 1;
  resetOnStartup = 0;
  romFileName = "rom";
  //homeDirectory = get_string_resource("homeDirectory", "HomeDirectory");

  useDebugger = 0;
  disassembler_mode =0;
}

