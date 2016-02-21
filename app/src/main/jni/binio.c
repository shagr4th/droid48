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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "global.h"
#include "binio.h"
#include "rpl.h"
#include "hp48_emu.h"

typedef word_20      DWORD;
typedef unsigned char BYTE;
typedef unsigned int  UINT;

void
Npeek(BYTE *a, DWORD d, UINT s)
{
  load_address(a, d, s);
}


static DWORD 
Npack(BYTE *a, UINT s)
{
	DWORD r = 0;

	while (s--) r = (r<<4)|a[s];
	return r;
}


DWORD 
RPL_ObjectSize(BYTE *o)
{
	DWORD n, l = 0;

	n = Npack(o, 5); // read prolog
	switch (n)
	{
	case DOBINT:   l = 10; break; // System Binary
	case DOREAL:   l = 21; break; // Real
	case DOEREAL:  l = 26; break; // Long Real
	case DOCMP:    l = 37; break; // Complex
	case DOECMP:   l = 47; break; // Long Complex
	case DOCHAR:   l =  7; break; // Character
	case DOEXT1:   l = 15; break; // Extended Pointer
	case DOROMP:   l = 11; break; // XLIB Name
	case DOLIST: // List
	case DOSYMB: // Algebraic
	case DOEXT:  // Unit
	case DOCOL:  // Program
		n=5;
		do { l+=n; o+=n; n=RPL_ObjectSize(o); } while (n);
		l += 5;
		break;
	case SEMI: l =  0; break; // SEMI
	case DOIDNT: // Global Name
	case DOLAM:  // Local Name
	case DOTAG:  // Tagged
		n = 7 + Npack(o+5,2)*2;
		l = n + RPL_ObjectSize(o+n);
		break;
	case DORRP: // Directory
		n = Npack(o+8,5);
		if (n==0) // empty dir
		{
			l=13;
		}
		else
		{
			l = 8+n;
			n = Npack(o+l,2)*2 + 4;
			l += n;
			l += RPL_ObjectSize(o+l);
		}
		break;
	case DOARRY:    // Array
	case DOLNKARRY: // Linked Array
	case DOCSTR:    // String
	case DOHSTR:    // Binary Integer
	case DOGROB:    // Graphic
	case DOLIB:     // Library
	case DOBAK:     // Backup
	case DOEXT0:    // Library Data
	case DOEXT2:    // Reserved 1, Font (HP49G)
	case DOEXT3:    // Reserved 2
	case DOEXT4:    // Reserved 3
	case DOCODE:    // Code
		l = 5 + Npack(o+5,5);
		break;
	default: l=5;
	}
	return l;
}

DWORD 
Read5(DWORD d)
{
	word_20 p = 0;
	load_addr(&p,d,5);
	return p;
}

static void
Nunpack(BYTE *a, DWORD b, UINT s)
{
	UINT i;
	for (i=0; i<s; i++) { a[i] = (BYTE)(b&0xf); b>>=4; }
}

void 
Nwrite(BYTE *a, DWORD d, UINT s)
{
 store_n(d,a,s);
}

void 
Write5(DWORD d, DWORD n)
{
	BYTE p[5];

	Nunpack(p,n,5);
	Nwrite(p,d,5);
	return;
}




DWORD 
RPL_CreateTemp(DWORD l)
{
	DWORD a, b, c;
	BYTE *p;

	l += 6;									// memory for link field (5) + marker (1) and end
	a = Read5(TEMPTOP);						// tail address of top object
	b = Read5(RSKTOP);						// tail address of rtn stack
	c = Read5(DSKTOP);						// top of data stack
	if ((b+l)>c) return 0;					// check if there's enough memory to move DSKTOP
	Write5(TEMPTOP, a+l);					// adjust new end of top object
	Write5(RSKTOP,  b+l);					// adjust new end of rtn stack
	Write5(AVMEM, (c-b-l)/5);				// calculate free memory (*5 nibbles)
	p = (BYTE *)malloc(b-a);				
  memset(p,0,b-a);
	Npeek(p,a,b-a);
	Nwrite(p,a+l,b-a);
	free(p);
	Write5(a+l-5,l);						// set object length field
	return (a+1);							// return base address of new object
}

void
RPL_Push(DWORD n)
{
	DWORD stkp, avmem;

	avmem = Read5(AVMEM);					// amount of free memory
	if (avmem==0) return;					// no memory free
	avmem--;
	// fetch memory
	Write5(AVMEM,avmem);					// save new amount of free memory
	stkp = Read5(DSKTOP);					// get pointer to stack level 1
# if 0
	if (METAKERNEL)							// Metakernel running ?
	{
		
		Write5(stkp-5,Read5(stkp));			// copy object pointer of stack level 1 to new stack level 1 entry
		Write5(stkp,n);						// save pointer to new object on stack level 2
		stkp-=5;							// fetch new stack entry
	}
	else
# else
	{
		stkp-=5;							// fetch new stack entry
		Write5(stkp,n);						// save pointer to new object on stack level 1
	}
# endif
	Write5(DSKTOP,stkp);					// save new pointer to stack level 1
	return;
}

int
read_bin_file(char *filename)
{
  struct stat st;
  FILE       *fp;
  DWORD       bin_size = 0;
  BYTE       *bin_buffer = (BYTE *)0;
  int         bBinary;
  long        dwAddress;
  long        i;
LOGI("Loading filename: %s", filename);
  if (NULL == (fp = fopen(filename, "r")))
  {
    return 0;
  }

  if (stat(filename, &st) < 0)
  {
    fclose(fp);
    return 0;
  }

  bin_size   = st.st_size;
  bin_buffer = (BYTE *)malloc(bin_size * 2);

  if (fread(bin_buffer + bin_size, 1, (size_t)bin_size, fp) != bin_size)
  {
    free(bin_buffer);
    fclose(fp);
    return 0;
  }
  fclose(fp);

  bBinary =  ((bin_buffer[bin_size+0]=='H')
          &&  (bin_buffer[bin_size+1]=='P')
          &&  (bin_buffer[bin_size+2]=='H')
          &&  (bin_buffer[bin_size+3]=='P')
          &&  (bin_buffer[bin_size+4]=='4')
          &&  (bin_buffer[bin_size+5]=='8')
          &&  (bin_buffer[bin_size+6]=='-'));

  for (i = 0; i < bin_size; i++)
  {
    BYTE byTwoNibs = bin_buffer[i+bin_size];
    bin_buffer[i*2  ] = (BYTE)(byTwoNibs&0xF);
    bin_buffer[i*2+1] = (BYTE)(byTwoNibs>>4);
  }

  if (bBinary)
  { // load as binary
    bin_size    = RPL_ObjectSize(bin_buffer+16);
    dwAddress   = RPL_CreateTemp(bin_size);
    if (dwAddress == 0) return 0;

    Nwrite(bin_buffer+16,dwAddress,bin_size);
  }
  else
  { // load as string
    bin_size *= 2;
    dwAddress = RPL_CreateTemp(bin_size+10);
    if (dwAddress == 0) return 0;
    Write5(dwAddress,0x02A2C);      // String
    Write5(dwAddress+5,bin_size+5);   // length of String
    Nwrite(bin_buffer,dwAddress+10,bin_size);  // data
  }
  RPL_Push(dwAddress);
LOGI("Done loading filename: %s", filename);

  return 1;
}
