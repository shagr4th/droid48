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

/* $Log: errors.c,v $
 * Revision 1.1  1994/12/07  10:15:47  ecd
 * Initial revision
 *
 *
 * $Id: errors.c,v 1.1 1994/12/07 10:15:47 ecd Exp ecd $
 */

#include <stdio.h>

#include "global.h"
#include "resources.h"

#include <android/log.h>
#include "x48.h"

char errbuf[1024] = { 0, };
char fixbuf[1024] = { 0, };

void
#ifdef __FunctionProto__
fatal_exit(void)
#else
fatal_exit()
#endif
{
 /* if (quiet)
    exit (1);

  if (errbuf[0] == '\0')
    {
      LOGE( "%s: FATAL ERROR, exit.\n", progname);
      exit (1);
    }

  LOGE( "%s: FATAL ERROR, exit.\n  - %s\n", progname, errbuf);

  if (fixbuf[0] != '\0')
    LOGE( "  - %s\n", fixbuf);

  exit (1);*/
}


