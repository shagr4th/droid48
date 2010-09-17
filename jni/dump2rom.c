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

/* $Log: dump2rom.c,v $
 * Revision 1.7  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.6  1994/12/07  20:20:50  ecd
 * minor changes
 *
 * Revision 1.6  1994/12/07  20:20:50  ecd
 * minor changes
 *
 * Revision 1.5  1994/11/28  02:00:51  ecd
 * improved trapping of EOF
 *
 * Revision 1.4  1994/11/02  14:40:38  ecd
 * support for "compressed" rom files added
 *
 * Revision 1.3  1994/09/18  15:29:22  ecd
 * turned off unused rcsid message
 *
 * Revision 1.2  1994/09/13  16:57:00  ecd
 * changed to plain X11
 *
 * Revision 1.1  1994/09/07  12:57:36  ecd
 * Initial revision
 *
 * $Id: dump2rom.c,v 1.7 1995/01/11 18:20:01 ecd Exp ecd $
 */


#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef SUNOS
#include <memory.h>
#endif
#include <fcntl.h>

unsigned char *core;

#define DEFAULT_ROM_FILE "rom.dump"

int
#ifdef __FunctionProto__
write_mem_file(char *name, unsigned char *mem, int size)
#else
write_mem_file(name, mem, size)
char *name;
unsigned char *mem;
int size;
#endif
{
  FILE *fp;
  unsigned char *tmp_mem;
  unsigned char byte;
  int i, j;

  if (NULL == (fp = fopen(name, "w")))
    {
      LOGE( "can\'t open %s\n", name);
      return 0;
    }

  if (NULL == (tmp_mem = (unsigned char *)malloc((size_t)size / 2)))
    {
      for (i = 0, j = 0; i < size / 2; i++)
        {
          byte = (mem[j++] & 0x0f);
          byte |= (mem[j++] << 4) & 0xf0;
          if (1 != fwrite(&byte, 1, 1, fp))
            {
              LOGE( "can\'t write %s\n", name);
              fclose(fp);
              return 0;
            }
        }
    }
  else
    {
      for (i = 0, j = 0; i < size / 2; i++)
        {
          tmp_mem[i] = (mem[j++] & 0x0f);
          tmp_mem[i] |= (mem[j++] << 4) & 0xf0;
        }

      if (fwrite(tmp_mem, 1, (size_t)size / 2, fp) != size / 2)
        {
          LOGE( "can\'t write %s\n", name);
          fclose(fp);
          free(tmp_mem);
          return 0;
       }

       free(tmp_mem);
    }

  fclose(fp);
  return 1;
}

int
#ifdef __FunctionProto__
main(int argc, char **argv)
#else
main(argc, argv)
int argc;
char **argv;
#endif
{
  FILE *dump;
  long addr, size;
  int ch, i, gx, error;

  if (argc < 2) {
    LOGE( "usage: %s hp48-dump-file\n", argv[0]);
    exit (1);
  }

  if ((dump = fopen(argv[1], "r")) == NULL) {
    LOGE( "%s: can\'t open %s\n", argv[0], argv[1]);
    exit (1);
  }

  if ((core = (unsigned char *)malloc(0x100000)) == NULL) {
    LOGE( "%s: can\'t malloc %d bytes\n", argv[0], 0x100000);
    exit (1);
  }
  memset(core, 0, 0x100000);

  gx = 0;
  error = 0;
  while (1) {
    addr = 0;
    for (i = 0; i < 5; i++) {
      addr <<= 4;
      if ((ch = fgetc(dump)) < 0) {
        error = 1;
        break;
      }
      if (ch >= '0' && ch <= '9') {
        addr |= ch - '0';
      } else if (ch >= 'A' && ch <= 'F') {
        addr |= ch - 'A' + 10;
      } else {
        LOGE( "%s: Illegal char %c at %lx\n", argv[0], ch, addr);
        error = 1;
        break;
      }
    }
    if (error)
      break;
    if (addr >= 0x80000)
      gx = 1;
    if ((ch = fgetc(dump)) < 0) {
      LOGE( "%s: Unexpected EOF at %lx\n", argv[0], addr);
      break;
    }
    if (ch != ':') {
      LOGE( "%s: Illegal char %c, expected \':\' at %lx\n",
                      argv[0], ch, addr);
      break;
    }
    for (i = 0; i < 16; i++) {
      if ((ch = fgetc(dump)) < 0) {
        LOGE( "%s: Unexpected EOF at %lx\n", argv[0], addr);
        error = 1;
        break;
      }
      if (ch >= '0' && ch <= '9') {
        core[addr++] = ch - '0';
      } else if (ch >= 'A' && ch <= 'F') {
        core[addr++] = ch - 'A' + 10;
      } else {
        LOGE( "%s: Illegal char %c at %lx\n", argv[0], ch, addr);
        error = 1;
        break;
      }
    }
    if (error)
      break;
    if ((ch = fgetc(dump)) < 0)
      break;
    if (ch != '\n') {
      LOGE( "%s: Illegal char %c, expected \'\\n\' at %lx\n",
                      argv[0], ch, addr);
      break;
    }
  }

  if (!gx)
    if (core[0x29] == 0x0)
      gx = 1;
  if (gx)
    size = 0x100000;
  else
    size = 0x80000;
  if (!write_mem_file(DEFAULT_ROM_FILE, core, size))
    {
      LOGE( "%s: can\'t write to %s\n", argv[0], DEFAULT_ROM_FILE);
      exit (1);
    }

  exit (0);
}

