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

/* $Log: mkcard.c,v $
 * Revision 1.1  1995/01/11  18:11:25  ecd
 * Initial revision
 *
 *
 * $Id: mkcard.c,v 1.1 1995/01/11 18:11:25 ecd Exp ecd $
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

unsigned char *mem;

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

  if (NULL == (fp = fopen(name, "w")))
    {
      printf( "can\'t open %s\n", name);
      return 0;
    }

  if (fwrite(mem, 1, (size_t)size, fp) != size)
    {
      printf( "can\'t write %s\n", name);
      fclose(fp);
      return 0;
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
  long size;
  char *name;
  char *asize;
  unsigned char *core;

  if (argc < 2)
    {
      printf( "usage: %s [32K | 128K | 1M | 2M | 4M] file-name\n",
              argv[0]);
      exit (1);
    }

  name = argv[2];
  asize = argv[1];
  if (!strcmp(asize, "32K"))
    size = 0x8000;
  else if (!strcmp(asize, "128K"))
    size = 0x20000;
  else if (!strcmp(asize, "256K"))
    size = 0x40000;
  else if (!strcmp(asize, "512K"))
    size = 0x80000;
  else if (!strcmp(asize, "1M"))
    size = 0x100000;
  else if (!strcmp(asize, "2M"))
    size = 0x200000;
  else if (!strcmp(asize, "4M"))
    size = 0x400000;
  else
    {
      printf(
              "%s: size must be one of 32K, 128K, 256K, 512K, 1M, 2M, or 4M\n",
              argv[0]);
      exit (1);
    }

  if ((core = (unsigned char *)malloc(size)) == NULL) {
    printf( "%s: can\'t malloc %ld bytes\n", argv[0], size);
    exit (1);
  }
  memset(core, 0, size);

  if (!write_mem_file(name, core, size))
    {
      printf( "%s: can\'t write to %s\n", argv[0], name);
      exit (1);
    }

  exit (0);
}

