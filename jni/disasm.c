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

/* $Log: disasm.c,v $
 * Revision 1.6  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.5  1994/12/07  20:20:50  ecd
 * some minor fixes
 *
 * Revision 1.5  1994/12/07  20:20:50  ecd
 * some minor fixes
 *
 * Revision 1.4  1994/11/28  02:00:51  ecd
 * enlarged field_tbl further
 *
 * Revision 1.3  1994/11/04  03:42:34  ecd
 * fixed bug in field_tbl
 *
 * Revision 1.2  1994/11/02  14:40:38  ecd
 * completed disassembler
 *
 * Revision 1.1  1994/10/09  20:29:47  ecd
 * Initial revision
 *
 *
 * $Id: disasm.c,v 1.6 1995/01/11 18:20:01 ecd Exp ecd $
 */

#include "global.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hp48.h"
#include "disasm.h"

#define TAB_SKIP	8

int disassembler_mode = CLASS_MNEMONICS;

char *mode_name[] =
{
  "HP",
  "class"
};

static char *hex[] =
{
  "0123456789ABCDEF",
  "0123456789abcdef",
};

static char *opcode_0_tbl[32] =
{
/*
   * HP Mnemonics
   */
  "RTNSXM", "RTN", "RTNSC", "RTNCC",
  "SETHEX", "SETDEC", "RSTK=C", "C=RSTK",
  "CLRST", "C=ST", "ST=C", "CSTEX",
  "P=P+1", "P=P-1", "(NULL)", "RTI",
/*
   * Class Mnemonics
   */
  "rtnsxm", "rtn", "rtnsc", "rtncc",
  "sethex", "setdec", "push", "pop",
  "clr.3   st", "move.3  st, c", "move.3  c, st", "exg.3   c, st",
  "inc.1   p", "dec.1   p", "(null)", "rti"
};

static char *op_str_0[16] =
{
/*
   * HP Mnemonics
   */
  "A=A%cB", "B=B%cC", "C=C%cA", "D=D%cC",
  "B=B%cA", "C=C%cB", "A=A%cC", "C=C%cD",
/*
   * Class Mnemonics
   */
  "b, a", "c, b", "a, c", "c, d",
  "a, b", "b, c", "c, a", "d, c"
};

static char *op_str_1[16] =
{
/*
   * HP Mnemonics
   */
  "DAT0=A", "DAT1=A", "A=DAT0", "A=DAT1",
  "DAT0=C", "DAT1=C", "C=DAT0", "C=DAT1",
/*
   * Class Mnemonics
   */
  "a, (d0)", "a, (d1)", "(d0), a", "(d1), a",
  "c, (d0)", "c, (d1)", "(d0), c", "(d1), c"
};

static char *in_str_80[32] =
{
/*
   * HP Mnemonics
   */
  "OUT=CS", "OUT=C", "A=IN", "C=IN",
  "UNCNFG", "CONFIG", "C=ID", "SHUTDN",
  NULL, "C+P+1", "RESET", "BUSCC",
  NULL, NULL, "SREQ?", NULL,
/*
   * Class Mnemonics
   */
  "move.s  c, out", "move.3  c, out", "move.4  in, a", "move.4  in, c",
  "uncnfg", "config", "c=id", "shutdn",
  NULL, "add.a   p+1, c", "reset", "buscc",
  NULL, NULL, "sreq?", NULL
};

static char *in_str_808[32] =
{
/*
   * HP Mnemonics
   */
  "INTON", NULL, NULL, "BUSCB",
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  "PC=(A)", "BUSCD", "PC=(C)", "INTOFF",
/*
   * Class Mnemonics
   */
  "inton", NULL, NULL, "buscb",
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  "jmp     (a)", "buscd", "jmp     (c)", "intoff"
};

static char *op_str_81[8] =
{
/*
   * HP Mnemonics
   */
  "A", "B", "C", "D",
/*
   * Class Mnemonics
   */
  "a", "b", "c", "d",
};

static char *in_str_81b[32] =
{
/*
   * HP Mnemonics
   */
  NULL, NULL, "PC=A", "PC=C",
  "A=PC", "C=PC", "APCEX", "CPCEX",
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
/*
   * Class Mnemonics
   */
  NULL, NULL, "jmp     a", "jmp     c",
  "move.a  pc, a", "move.a  pc, c", "exg.a   a, pc", "exg.a   c, pc",
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
};

static char *in_str_9[16] =
{
/*
   * HP Mnemonics
   */
  "=", "#", "=", "#",
  ">", "<", ">=", "<=",
/*
   * Class Mnemonics
   */
  "eq", "ne", "eq", "ne",
  "gt", "lt", "ge", "le"
};

static char *op_str_9[16] =
{
/*
   * HP Mnemonics
   */
  "?A%sB", "?B%sC", "?C%sA", "?D%sC",
  "?A%s0", "?B%s0", "?C%s0", "?D%s0",
/*
   * Class Mnemonics
   */
  "a, b", "b, c", "c, a", "d, c",
  "a, 0", "b, 0", "c, 0", "d, 0"
};

static char *op_str_af[32] =
{
/*
   * HP Mnemonics
   */
  "A=A%sB", "B=B%sC", "C=C%sA", "D=D%sC",
  "A=A%sA", "B=B%sB", "C=C%sC", "D=D%sD",
  "B=B%sA", "C=C%sB", "A=A%sC", "C=C%sD",
  "A=B%sA", "B=C%sB", "C=A%sC", "D=C%sD",
/*
   * Class Mnemonics
   */
  "b, a", "c, b", "a, c", "c, d",
  "a, a", "b, b", "c, c", "d, d",
  "a, b", "b, c", "c, a", "d, c",
  "b, a", "c, b", "a, c", "c, d"
};

static char hp_reg_1_af[] = "ABCDABCDBCACABAC";
static char hp_reg_2_af[] = "0000BCACABCDBCCD";

static char *field_tbl[32] =
{
/*
   * HP Mnemonics
   */
  "P", "WP", "XS", "X",
  "S", "M", "B", "W",
  "P", "WP", "XS", "X",
  "S", "M", "B", "A",
/*
   * Class Mnemonics
   */
  ".p", ".wp", ".xs", ".x",
  ".s", ".m", ".b", ".w",
  ".p", ".wp", ".xs", ".x",
  ".s", ".m", ".b", ".a",
};

static char *hst_bits[8] =
{
/*
   * HP Mnemonics
   */
  "XM", "SB", "SR", "MP",
/*
   * Class Mnemonics
   */
  "xm", "sb", "sr", "mp",
};

int
#ifdef __FunctionProto__
read_int (word_20 * addr, int n)
#else
read_int (addr, n)
     word_20 *addr;
     int n;
#endif
{
  int i, t;

  for (i = 0, t = 0; i < n; i++)
    t |= read_nibble ((*addr)++) << (i * 4);
  return t;
}

char *
#ifdef __FunctionProto__
append_str (char *buf, char *str)
#else
append_str (buf, str)
     char *buf;
     char *str;
#endif
{
  while ((*buf = *str++))
    buf++;
  return buf;
}

char *
#ifdef __FunctionProto__
append_tab_16 (char *buf)
#else
append_tab_16 (buf)
     char *buf;
#endif
{
  int n;
  char *p;

  n = 16 - (strlen (buf) % 16);
  p = &buf[strlen (buf)];
  while (n--)
    *p++ = ' ';
  *p = '\0';
  return p;
}

char *
#ifdef __FunctionProto__
append_tab (char *buf)
#else
append_tab (buf)
     char *buf;
#endif
{
  int n;
  char *p;

  n = TAB_SKIP - (strlen (buf) % TAB_SKIP);
  p = &buf[strlen (buf)];
  while (n--)
    *p++ = ' ';
  *p = '\0';
  return p;
}

char *
#ifdef __FunctionProto__
append_field (char *buf, word_4 fn)
#else
append_field (buf, fn)
     char *buf;
     word_4 fn;
#endif
{
  buf = append_str (buf, field_tbl[fn + 16 * disassembler_mode]);
  return buf;
}

char *
#ifdef __FunctionProto__
append_imm_nibble (char *buf, word_20 * addr, int n)
#else
append_imm_nibble (buf, addr, n)
     char *buf;
     word_20 *addr;
     int n;
#endif
{
  int i;
  char t[16];

  if (disassembler_mode == CLASS_MNEMONICS)
    {
      *buf++ = '#';
      if (n > 1)
	*buf++ = '$';
    }
  if (n > 1)
    {
      for (i = 0; i < n; i++)
	t[i] = hex[disassembler_mode][read_nibble ((*addr)++)];
      for (i = n - 1; i >= 0; i--)
	{
	  *buf++ = t[i];
	}
      *buf = '\0';
    }
  else
    {
      sprintf (t, "%d", read_nibble ((*addr)++));
      buf = append_str (buf, t);
    }
  return buf;
}

char *
#ifdef __FunctionProto__
append_addr (char *buf, word_20 addr)
#else
append_addr (buf, addr)
     char *buf;
     word_20 addr;
#endif
{
  int shift;
  long mask;

  if (disassembler_mode == CLASS_MNEMONICS)
    {
      *buf++ = '$';
    }
  for (mask = 0xf0000, shift = 16; mask != 0; mask >>= 4, shift -= 4)
    *buf++ = hex[disassembler_mode][(addr & mask) >> shift];
  *buf = '\0';
  return buf;
}

char *
#ifdef __FunctionProto__
append_r_addr (char *buf, word_20 * pc, long disp, int n, int offset)
#else
append_r_addr (buf, pc, disp, n, offset)
     char *buf;
     word_20 *pc;
     long disp;
     int n;
     int offset;
#endif
{
  long sign;

  sign = 1 << (n * 4 - 1);
  if (disp & sign)
    disp |= ~(sign - 1);
  *pc += disp;

  switch (disassembler_mode)
    {
    case HP_MNEMONICS:
      if (disp < 0)
	{
	  buf = append_str (buf, "-");
	  disp = -disp - offset;
	}
      else
	{
	  buf = append_str (buf, "+");
	  disp += offset;
	}
      buf = append_addr (buf, disp);
      break;
    case CLASS_MNEMONICS:
      if (disp < 0)
	{
	  buf = append_str (buf, "-");
	  disp = -disp - offset;
	}
      else
	{
	  buf = append_str (buf, "+");
	  disp += offset;
	}
      buf = append_addr (buf, disp);
      break;
    default:
      buf = append_str (buf, "Unknown disassembler mode");
      break;
    }
  return buf;
}

char *
#ifdef __FunctionProto__
append_pc_comment (char *buf, word_20 pc)
#else
append_pc_comment (buf, pc)
     char *buf;
     word_20 pc;
#endif
{
  char *p = buf;

  while (strlen (buf) < 4 * TAB_SKIP)
    p = append_tab (buf);

  switch (disassembler_mode)
    {
    case HP_MNEMONICS:
      p = append_str (p, "# Address: ");
      p = append_addr (p, pc);
      break;
    case CLASS_MNEMONICS:
      p = append_str (p, "; address: ");
      p = append_addr (p, pc);
      break;
    default:
      p = append_str (p, "Unknown disassembler mode");
      break;
    }
  return p;
}


char *
#ifdef __FunctionProto__
append_hst_bits (char *buf, int n)
#else
append_hst_bits (buf, n)
     char *buf;
     int n;
#endif
{
  int i;
  char *p = buf;

  switch (disassembler_mode)
    {
    case HP_MNEMONICS:
      for (i = 0; i < 4; i++)
	if (n & (1 << i))
	  {
	    if (p != buf)
	      p = append_str (p, "=");
	    p = append_str (p, hst_bits[i + 4 * disassembler_mode]);
	  }
      break;

    case CLASS_MNEMONICS:
      while (strlen (buf) < 4 * TAB_SKIP)
	p = append_tab (buf);
      p = &buf[strlen (buf)];
      p = append_str (p, "; hst bits: ");

      for (buf = p, i = 0; i < 4; i++)
	if (n & (1 << i))
	  {
	    if (p != buf)
	      p = append_str (p, ", ");
	    p = append_str (p, hst_bits[i + 4 * disassembler_mode]);
	  }
      break;

    default:
      p = append_str (p, "Unknown disassembler mode");
      break;
    }

  return p;
}

char *
#ifdef __FunctionProto__
disasm_1 (word_20 * addr, char *out)
#else
disasm_1 (addr, out)
     word_20 *addr;
     char *out;
#endif
{
  word_4 n;
  word_4 fn;
  char *p;
  char buf[20];
  char c;

  p = out;
  switch ((n = read_nibble ((*addr)++)))
    {
    case 0:
    case 1:
      fn = read_nibble ((*addr)++);
      fn = (fn & 7);
      if (fn > 4)
	fn -= 4;
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  c = (char) ((fn < 8) ? 'A' : 'C');
	  if (n == 0)
	    sprintf (buf, "R%d=%c", fn, c);
	  else
	    sprintf (buf, "%c=R%d", c, fn);
	  p = append_str (out, buf);
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, "move.w");
	  p = append_tab (out);
	  c = (char) ((fn < 8) ? 'a' : 'c');
	  if (n == 0)
	    sprintf (buf, "%c, r%d", c, fn);
	  else
	    sprintf (buf, "r%d, %c", fn, c);
	  p = append_str (p, buf);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 2:
      fn = read_nibble ((*addr)++);
      fn = (fn & 7);
      if (fn > 4)
	fn -= 4;
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  c = (char) ((fn < 8) ? 'A' : 'C');
	  sprintf (buf, "%cR%dEX", c, fn);
	  p = append_str (out, buf);
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, "exg.w");
	  p = append_tab (out);
	  c = (char) ((fn < 8) ? 'a' : 'c');
	  sprintf (buf, "%c, r%d", c, fn);
	  p = append_str (p, buf);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 3:
      n = read_nibble ((*addr)++);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  c = (n & 4) ? 'C' : 'A';
	  if (n & 2)
	    {
	      if (n < 8)
		{
		  sprintf (buf, "%cD%dEX", c, (n & 1));
		}
	      else
		{
		  sprintf (buf, "%cD%dXS", c, (n & 1));
		}
	    }
	  else
	    {
	      if (n < 8)
		{
		  sprintf (buf, "D%d=%c", (n & 1), c);
		}
	      else
		{
		  sprintf (buf, "D%d=%cS", (n & 1), c);
		}
	    }
	  p = append_str (out, buf);
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (n & 2) ? "exg." : "move.");
	  p = append_str (p, (n < 8) ? "a" : "4");
	  p = append_tab (out);
	  c = (n & 4) ? 'c' : 'a';
	  sprintf (buf, "%c, d%d", c, (n & 1));
	  p = append_str (p, buf);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 4:
    case 5:
      fn = read_nibble ((*addr)++);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  p = append_str (out, op_str_1[(fn & 7) + 8 * disassembler_mode]);
	  p = append_tab (out);
	  if (n == 4)
	    {
	      p = append_str (p, (fn < 8) ? "A" : "B");
	    }
	  else
	    {
	      n = read_nibble ((*addr)++);
	      if (fn < 8)
		{
		  p = append_field (p, n);
		}
	      else
		{
		  sprintf (buf, "%d", n + 1);
		  p = append_str (p, buf);
		}
	    }
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, "move");
	  if (n == 4)
	    {
	      p = append_str (p, ".");
	      p = append_str (p, (fn < 8) ? "a" : "b");
	    }
	  else
	    {
	      n = read_nibble ((*addr)++);
	      if (fn < 8)
		{
		  p = append_field (p, n);
		}
	      else
		{
		  sprintf (buf, ".%d", n + 1);
		  p = append_str (p, buf);
		}
	    }
	  p = append_tab (out);
	  p = append_str (p, op_str_1[(fn & 7) + 8 * disassembler_mode]);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 6:
    case 7:
    case 8:
    case 0xc:
      fn = read_nibble (*addr++);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  if (n == 6 || n == 8)
	    p = append_str (out, "D0=D0");
	  else
	    p = append_str (out, "D1=D1");
	  if (n < 8)
	    p = append_str (p, "+");
	  else
	    p = append_str (p, "-");
	  p = append_tab (out);
	  sprintf (buf, "%d", fn + 1);
	  p = append_str (p, buf);
	  break;
	case CLASS_MNEMONICS:
	  if (n < 8)
	    p = append_str (out, "add.a");
	  else
	    p = append_str (out, "sub.a");
	  p = append_tab (out);
	  sprintf (buf, "#%d, ", fn + 1);
	  p = append_str (p, buf);
	  if (n == 6 || n == 8)
	    p = append_str (p, "d0");
	  else
	    p = append_str (p, "d1");
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 9:
    case 0xa:
    case 0xb:
    case 0xd:
    case 0xe:
    case 0xf:
      c = (char) ((n < 0xd) ? '0' : '1');
      switch (n & 3)
	{
	case 1:
	  n = 2;
	  break;
	case 2:
	  n = 4;
	  break;
	case 3:
	  n = 5;
	  break;
	}
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  sprintf (buf, "D%c=(%d)", c, n);
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_imm_nibble (p, addr, n);
	  break;
	case CLASS_MNEMONICS:
	  if (n == 5)
	    {
	      sprintf (buf, "move.a");
	    }
	  else if (n == 4)
	    {
	      sprintf (buf, "move.as");
	    }
	  else
	    {
	      sprintf (buf, "move.b");
	    }
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_imm_nibble (p, addr, n);
	  sprintf (buf, ", d%c", c);
	  p = append_str (p, buf);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    default:
      break;
    }
  return p;
}


char *
#ifdef __FunctionProto__
disasm_8 (word_20 * addr, char *out)
#else
disasm_8 (addr, out)
     word_20 *addr;
     char *out;
#endif
{
  word_4 n;
  word_4 fn;
  char *p = out;
  char c;
  char buf[20];
  word_20 disp, pc;

  fn = read_nibble ((*addr)++);
  switch (fn)
    {
    case 0:
      n = read_nibble ((*addr)++);
      if (NULL != (p = in_str_80[n + 16 * disassembler_mode]))
	{
	  p = append_str (out, p);
	  return p;
	}
      switch (n)
	{
	case 8:
	  fn = read_nibble ((*addr)++);
	  if (NULL != (p = in_str_808[fn + 16 * disassembler_mode]))
	    {
	      p = append_str (out, p);
	      return p;
	    }
	  switch (fn)
	    {
	    case 1:
	      n = read_nibble ((*addr)++);
	      if (n == 0)
		{
		  switch (disassembler_mode)
		    {
		    case HP_MNEMONICS:
		      p = append_str (out, "RSI");
		      break;
		    case CLASS_MNEMONICS:
		      p = append_str (out, "rsi");
		      break;
		    default:
		      p = append_str (out, "Unknown disassembler mode");
		      break;
		    }
		}
	      break;
	    case 2:
	      n = read_nibble ((*addr)++);
	      switch (disassembler_mode)
		{
		case HP_MNEMONICS:
		  if (n < 5)
		    {
		      sprintf (buf, "LA(%d)", n + 1);
		    }
		  else
		    {
		      sprintf (buf, "LAHEX");
		    }
		  p = append_str (out, buf);
		  p = append_tab (out);
		  p = append_imm_nibble (p, addr, n + 1);
		  break;
		case CLASS_MNEMONICS:
		  sprintf (buf, "move.%d", n + 1);
		  p = append_str (out, buf);
		  p = append_tab (out);
		  p = append_imm_nibble (p, addr, n + 1);
		  sprintf (buf, ", a.p");
		  p = append_str (p, buf);
		  break;
		default:
		  p = append_str (out, "Unknown disassembler mode");
		  break;
		}
	      break;

	    case 4:
	    case 5:
	    case 8:
	    case 9:

	      switch (disassembler_mode)
		{
		case HP_MNEMONICS:
		  sprintf (buf, "%cBIT=%d", (fn & 8) ? 'C' : 'A',
			   (fn & 1) ? 1 : 0);
		  p = append_str (out, buf);
		  p = append_tab (out);
		  p = append_imm_nibble (p, addr, 1);
		  break;
		case CLASS_MNEMONICS:
		  p = append_str (out, (fn & 1) ? "bset" : "bclr");
		  p = append_tab (out);
		  p = append_imm_nibble (p, addr, 1);
		  p = append_str (p, (fn & 8) ? ", c" : ", a");
		  break;
		default:
		  p = append_str (out, "Unknown disassembler mode");
		  break;
		}
	      break;

	    case 6:
	    case 7:
	    case 0xa:
	    case 0xb:

	      n = read_nibble ((*addr)++);
	      pc = *addr;
	      disp = read_int (addr, 2);

	      switch (disassembler_mode)
		{
		case HP_MNEMONICS:
		  c = (char) ((fn < 0xa) ? 'A' : 'C');
		  sprintf (buf, "?%cBIT=%d", c, (fn & 1) ? 1 : 0);
		  p = append_str (out, buf);
		  p = append_tab (out);
		  sprintf (buf, "%d", n);
		  p = append_str (p, buf);
		  if (disp != 0)
		    {
		      p = append_str (p, ", GOYES ");
		      p = append_r_addr (p, &pc, disp, 2, 5);
		      p = append_pc_comment (out, pc);
		    }
		  else
		    p = append_str (p, ", RTNYES");
		  break;
		case CLASS_MNEMONICS:
		  c = (char) ((fn < 0xa) ? 'a' : 'c');
		  p = append_str (out, (disp == 0) ? "rt" : "b");
		  p = append_str (p, (fn & 1) ? "bs" : "bc");
		  p = append_tab (out);
		  sprintf (buf, "#%d, %c", n, c);
		  p = append_str (p, buf);
		  if (disp != 0)
		    {
		      p = append_str (p, ", ");
		      p = append_r_addr (p, &pc, disp, 2, 5);
		      p = append_pc_comment (out, pc);
		    }
		  break;
		default:
		  p = append_str (out, "Unknown disassembler mode");
		  break;
		}
	      break;

	    default:
	      break;
	    }
	  break;

	case 0xc:
	case 0xd:
	case 0xf:
	  fn = read_nibble ((*addr)++);
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      sprintf (buf, (n == 0xf) ? "%c%cEX" : "%c=%c",
		       (n == 0xd) ? 'P' : 'C', (n == 0xd) ? 'C' : 'P');
	      p = append_str (out, buf);
	      p = append_tab (out);
	      sprintf (buf, "%d", fn);
	      p = append_str (p, buf);
	      break;
	    case CLASS_MNEMONICS:
	      p = append_str (out, (n == 0xf) ? "exg.1" : "move.1");
	      p = append_tab (out);
	      sprintf (buf, (n == 0xd) ? "p, c.%d" : "c.%d, p", fn);
	      p = append_str (p, buf);
	      break;
	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      break;
	    }
	  break;

	default:
	  break;

	}
      break;

    case 1:
      switch (n = read_nibble ((*addr)++))
	{
	case 0:
	case 1:
	case 2:
	case 3:
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      sprintf (buf, "%sSLC", op_str_81[(n & 3) + 4 * disassembler_mode]);
	      p = append_str (out, buf);
	      break;
	    case CLASS_MNEMONICS:
	      p = append_str (out, "rol.w");
	      p = append_tab (out);
	      p = append_str (p, "#4, ");
	      p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
	      break;
	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      break;
	    }
	  break;

	case 4:
	case 5:
	case 6:
	case 7:
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      sprintf (buf, "%sSRC", op_str_81[(n & 3) + 4 * disassembler_mode]);
	      p = append_str (out, buf);
	      break;
	    case CLASS_MNEMONICS:
	      p = append_str (out, "ror.w");
	      p = append_tab (out);
	      p = append_str (p, "#4, ");
	      p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
	      break;
	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      break;
	    }
	  break;

	case 8:
	  fn = read_nibble ((*addr)++);
	  n = read_nibble ((*addr)++);
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      sprintf (buf, "%s=%s%cCON",
		       op_str_81[(n & 3) + 4 * disassembler_mode],
		       op_str_81[(n & 3) + 4 * disassembler_mode],
		       (n < 8) ? '+' : '-');
	      p = append_str (out, buf);
	      p = append_tab (out);
	      p = append_field (p, fn);
	      fn = read_nibble ((*addr)++);
	      sprintf (buf, ", %d", fn + 1);
	      p = append_str (p, buf);
	      break;
	    case CLASS_MNEMONICS:
	      p = append_str (out, (n < 8) ? "add" : "sub");
	      p = append_field (p, fn);
	      p = append_tab (out);
	      fn = read_nibble ((*addr)++);
	      sprintf (buf, "#%d, ", fn + 1);
	      p = append_str (p, buf);
	      p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
	      break;
	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      break;
	    }
	  break;

	case 9:
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      sprintf (buf, "%sSRB.F",
		       op_str_81[(n & 3) + 4 * disassembler_mode]);
	      p = append_str (out, buf);
	      p = append_tab (out);
	      p = append_field (p, read_nibble ((*addr)++));
	      break;
	    case CLASS_MNEMONICS:
	      p = append_str (out, "lsr");
	      p = append_field (p, read_nibble ((*addr)++));
	      p = append_tab (out);
	      p = append_str (p, "#1, ");
	      p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
	      break;
	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      break;
	    }
	  break;

	case 0xa:
	  fn = read_nibble ((*addr)++);
	  n = read_nibble ((*addr)++);
	  if (n > 2)
	    break;
	  c = (char) read_nibble ((*addr)++);
	  if (((int) c & 7) > 4)
	    break;
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      if (n == 2)
		{
		  sprintf (buf, "%cR%dEX.F", ((int) c < 8) ? 'A' : 'C',
			   (int) c & 7);
		}
	      else if (n == 1)
		{
		  sprintf (buf, "%c=R%d.F", ((int) c < 8) ? 'A' : 'C',
			   (int) c & 7);
		}
	      else
		{
		  sprintf (buf, "R%d=%c.F", (int) c & 7,
			   ((int) c < 8) ? 'A' : 'C');
		}
	      p = append_str (out, buf);
	      p = append_tab (out);
	      p = append_field (p, fn);
	      break;
	    case CLASS_MNEMONICS:
	      p = append_str (out, (n == 2) ? "exg" : "move");
	      p = append_field (p, fn);
	      p = append_tab (out);
	      if (n == 1)
		{
		  sprintf (buf, "r%d", (int) c & 7);
		  p = append_str (p, buf);
		}
	      else
		p = append_str (p, ((int) c < 8) ? "a" : "c");
	      p = append_str (p, ", ");
	      if (n == 1)
		p = append_str (p, ((int) c < 8) ? "a" : "c");
	      else
		{
		  sprintf (buf, "r%d", (int) c & 7);
		  p = append_str (p, buf);
		}
	      break;
	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      break;
	    }
	  break;

	case 0xb:
	  n = read_nibble ((*addr)++);
	  if ((n < 2) || (n > 7))
	    break;

	  p = append_str (out, in_str_81b[n + 16 * disassembler_mode]);
	  break;

	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      sprintf (buf, "%sSRB", op_str_81[(n & 3) + 4 * disassembler_mode]);
	      p = append_str (out, buf);
	      break;
	    case CLASS_MNEMONICS:
	      p = append_str (out, "lsr.w");
	      p = append_tab (out);
	      p = append_str (p, "#1, ");
	      p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
	      break;
	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      break;
	    }
	  break;

	default:
	  break;
	}
      break;

    case 2:
      n = read_nibble ((*addr)++);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  if (n == 0xf)
	    {
	      p = append_str (out, "CLRHST");
	    }
	  else
	    {
	      p = append_hst_bits (out, n);
	      p = append_str (p, "=0");
	    }
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, "clr.1");
	  p = append_tab (out);
	  sprintf (buf, "#%d, hst", n);
	  p = append_str (p, buf);
	  p = append_hst_bits (out, n);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 3:
      n = read_nibble ((*addr)++);
      pc = *addr;
      disp = read_int (addr, 2);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  p = append_str (out, "?");
	  p = append_hst_bits (p, n);
	  p = append_str (p, "=0");
	  p = append_tab (out);
	  if (disp != 0)
	    {
	      p = append_str (p, "GOYES ");
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  else
	    p = append_str (p, "RTNYES");
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (disp == 0) ? "rt" : "b");
	  p = append_str (p, "eq.1");
	  p = append_tab (out);
	  sprintf (buf, "#%d, hst", n);
	  p = append_str (p, buf);
	  if (disp != 0)
	    {
	      p = append_str (p, ", ");
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  p = append_hst_bits (out, n);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 4:
    case 5:
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  sprintf (buf, "ST=%d", (fn == 4) ? 0 : 1);
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_imm_nibble (p, addr, 1);
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (fn == 4) ? "bclr" : "bset");
	  p = append_tab (out);
	  p = append_imm_nibble (p, addr, 1);
	  p = append_str (p, ", st");
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 6:
    case 7:
      n = read_nibble ((*addr)++);
      pc = *addr;
      disp = read_int (addr, 2);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  sprintf (buf, "?ST=%d", (fn == 6) ? 0 : 1);
	  p = append_str (out, buf);
	  p = append_tab (out);
	  sprintf (buf, "%d", n);
	  p = append_str (p, buf);
	  if (disp != 0)
	    {
	      p = append_str (p, ", GOYES ");
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  else
	    p = append_str (p, ", RTNYES");
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (disp == 0) ? "rt" : "b");
	  p = append_str (p, (fn == 6) ? "bc" : "bs");
	  p = append_tab (out);
	  sprintf (buf, "#%d, st", n);
	  p = append_str (p, buf);
	  if (disp != 0)
	    {
	      p = append_str (p, ", ");
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 8:
    case 9:
      n = read_nibble ((*addr)++);
      pc = *addr;
      disp = read_int (addr, 2);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  sprintf (buf, "?P%c", (fn == 8) ? '#' : '=');
	  p = append_str (out, buf);
	  p = append_tab (out);
	  sprintf (buf, "%d", n);
	  p = append_str (p, buf);
	  if (disp != 0)
	    {
	      p = append_str (p, ", GOYES ");
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  else
	    p = append_str (p, ", RTNYES");
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (disp == 0) ? "rt" : "b");
	  p = append_str (p, (fn == 8) ? "ne.1" : "eq.1");
	  p = append_tab (out);
	  sprintf (buf, "#%d, p", n);
	  p = append_str (p, buf);
	  if (disp != 0)
	    {
	      p = append_str (p, ", ");
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 0xc:
    case 0xe:
      pc = *addr;
      if (fn == 0xe)
	pc += 4;
      disp = read_int (addr, 4);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  p = append_str (out, (fn == 0xc) ? "GOLONG" : "GOSUBL");
	  p = append_tab (out);
	  p = append_r_addr (p, &pc, disp, 4, (fn == 0xc) ? 2 : 6);
	  p = append_pc_comment (out, pc);
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (fn == 0xc) ? "bra.4" : "bsr.4");
	  p = append_tab (out);
	  p = append_r_addr (p, &pc, disp, 4, (fn == 0xc) ? 2 : 6);
	  p = append_pc_comment (out, pc);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 0xd:
    case 0xf:
      pc = read_int (addr, 5);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  p = append_str (out, (fn == 0xc) ? "GOVLNG" : "GOSBVL");
	  p = append_tab (out);
	  p = append_addr (p, pc);
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (fn == 0xc) ? "jmp" : "jsr");
	  p = append_tab (out);
	  p = append_addr (p, pc);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    default:
      break;

    }
  return p;
}


word_20
#ifdef __FunctionProto__
disassemble (word_20 addr, char *out)
#else
disassemble (addr, out)
     word_20 addr;
     char *out;
#endif
{
  word_4 n;
  word_4 fn;
  char *p = out;
  char c;
  char buf[20];
  word_20 disp, pc;

  switch (n = read_nibble (addr++))
    {
    case 0:
      if ((n = read_nibble (addr++)) != 0xe)
	{
	  p = append_str (out, opcode_0_tbl[n + 16 * disassembler_mode]);
	  break;
	}
      fn = read_nibble (addr++);
      n = read_nibble (addr++);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  sprintf (buf, op_str_0[(n & 7) + 8 * HP_MNEMONICS],
		   (n < 8) ? '&' : '!');
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_field (p, fn);
	  break;
	case CLASS_MNEMONICS:
	  p = append_str (out, (n < 8) ? "and" : "or");
	  p = append_field (p, fn);
	  p = append_tab (out);
	  p = append_str (p, op_str_0[(n & 7) + 8 * CLASS_MNEMONICS]);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 1:
      p = disasm_1 (&addr, out);
      break;

    case 2:
      n = read_nibble (addr++);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  sprintf (buf, "P=%d", n);
	  p = append_str (out, buf);
	  break;
	case CLASS_MNEMONICS:
	  sprintf (buf, "move.1  #%d, p", n);
	  p = append_str (out, buf);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 3:
      fn = read_nibble (addr++);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  if (fn < 5)
	    {
	      sprintf (buf, "LC(%d)", fn + 1);
	    }
	  else
	    {
	      sprintf (buf, "LCHEX");
	    }
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_imm_nibble (p, &addr, fn + 1);
	  break;
	case CLASS_MNEMONICS:
	  sprintf (buf, "move.%d", fn + 1);
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_imm_nibble (p, &addr, fn + 1);
	  sprintf (buf, ", c.p");
	  p = append_str (p, buf);
	  break;
	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 4:
    case 5:
      pc = addr;
      disp = read_int (&addr, 2);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  if (disp == 2)
	    {
	      p = append_str (out, "NOP3");
	      break;
	    }
	  sprintf (buf, (disp == 0) ? "RTN%sC" : "GO%sC", (n == 4) ? "" : "N");
	  p = append_str (out, buf);
	  if (disp != 0)
	    {
	      p = append_tab (out);
	      p = append_r_addr (p, &pc, disp, 2, 1);
	      p = append_pc_comment (out, pc);
	    }
	  break;

	case CLASS_MNEMONICS:
	  if (disp == 2)
	    {
	      p = append_str (out, "nop3");
	      break;
	    }
	  p = append_str (out, (disp == 0) ? "rtc" : "bc");
	  p = append_str (p, (n == 4) ? "s" : "c");
	  if (disp != 0)
	    {
	      p = append_tab (out);
	      p = append_r_addr (p, &pc, disp, 2, 1);
	      p = append_pc_comment (out, pc);
	    }
	  break;

	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 6:
      pc = addr;
      disp = read_int (&addr, 3);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  if (disp == 3)
	    {
	      p = append_str (out, "NOP4");
	      break;
	    }
	  if (disp == 4)
	    {
	      p = append_str (out, "NOP5");
	      break;
	    }
	  p = append_str (out, "GOTO");
	  p = append_tab (out);
	  p = append_r_addr (p, &pc, disp, 3, 1);
	  p = append_pc_comment (out, pc);
	  break;

	case CLASS_MNEMONICS:
	  if (disp == 3)
	    {
	      p = append_str (out, "nop4");
	      break;
	    }
	  if (disp == 4)
	    {
	      p = append_str (out, "nop5");
	      break;
	    }
	  p = append_str (out, "bra.3");
	  p = append_tab (out);
	  p = append_r_addr (p, &pc, disp, 3, 1);
	  p = append_pc_comment (out, pc);
	  break;

	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 7:
      pc = addr + 3;
      disp = read_int (&addr, 3);
      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  p = append_str (out, "GOSUB");
	  p = append_tab (out);
	  p = append_r_addr (p, &pc, disp, 3, 4);
	  p = append_pc_comment (out, pc);
	  break;

	case CLASS_MNEMONICS:
	  p = append_str (out, "bsr.3");
	  p = append_tab (out);
	  p = append_r_addr (p, &pc, disp, 3, 4);
	  p = append_pc_comment (out, pc);
	  break;

	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    case 8:
      fn = read_nibble (addr);	/* PEEK */
      if (fn != 0xa && fn != 0xb)
	{
	  p = disasm_8 (&addr, out);
	  break;
	}
      /* Fall through */

    case 9:
      fn = read_nibble (addr++);
      if (n == 8)
	{
	  c = (char) ((fn == 0xa) ? 0 : 1);
	  fn = 0xf;
	}
      else
	{
	  c = (char) ((fn < 8) ? 0 : 1);
	  fn &= 7;
	}

      n = read_nibble (addr++);
      pc = addr;
      disp = read_int (&addr, 2);

      switch (disassembler_mode)
	{
	case HP_MNEMONICS:
	  if ((c == 0) && (n >= 8))
	    sprintf (buf, op_str_9[(n & 3) + 8 * HP_MNEMONICS + 4],
		     in_str_9[((n >> 2) & 3) + 4 * c + 8 * HP_MNEMONICS]);
	  else
	    sprintf (buf, op_str_9[(n & 3) + 8 * HP_MNEMONICS],
		     in_str_9[((n >> 2) & 3) + 4 * c + 8 * HP_MNEMONICS]);
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_field (p, fn);
	  p = append_str (p, ", ");
	  p = append_str (p, (disp == 0) ? "RTNYES" : "GOYES ");
	  if (disp != 0)
	    {
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  break;

	case CLASS_MNEMONICS:
	  p = append_str (out, (disp == 0) ? "rt" : "b");
	  p = append_str (p, in_str_9[((n >> 2) & 3) + 4 * c + 8 * CLASS_MNEMONICS]);
	  p = append_field (p, fn);
	  p = append_tab (out);
	  if ((c == 0) && (n >= 8))
	    p = append_str (p, op_str_9[(n & 3) + 8 * CLASS_MNEMONICS + 4]);
	  else
	    p = append_str (p, op_str_9[(n & 3) + 8 * CLASS_MNEMONICS]);
	  if (disp != 0)
	    {
	      p = append_str (p, ", ");
	      p = append_r_addr (p, &pc, disp, 2, 3);
	      p = append_pc_comment (out, pc);
	    }
	  break;

	default:
	  p = append_str (out, "Unknown disassembler mode");
	  break;
	}
      break;

    default:
      switch (n)
	{
	case 0xa:
	  fn = read_nibble (addr++);
	  c = (char) ((fn < 8) ? 0 : 1);
	  fn &= 7;
	  disp = 0xa;
	  break;
	case 0xb:
	  fn = read_nibble (addr++);
	  c = (char) ((fn < 8) ? 0 : 1);
	  fn &= 7;
	  disp = 0xb;
	  break;
	case 0xc:
	case 0xd:
	  fn = 0xf;
	  c = (char) (n & 1);
	  disp = 0xa;
	  break;
	case 0xe:
	case 0xf:
	  fn = 0xf;
	  c = (char) (n & 1);
	  disp = 0xb;
	  break;
	default:
	  fn = 0;
	  disp = 0;
	  c = 0;
	  break;
	}

      n = read_nibble (addr++);
      pc = 0;

      switch (disp)
	{
	case 0xa:
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      if (c == 0)
		{
		  if (n < 0xc)
		    {
		      p = "+";
		    }
		  else
		    {
		      p = "%c=%c-1";
		      pc = 2;
		    }
		}
	      else
		{
		  if (n < 4)
		    {
		      p = "%c=0";
		      pc = 1;
		    }
		  else if (n >= 0xc)
		    {
		      p = "%c%cEX";
		      pc = 3;
		    }
		  else
		    {
		      p = "%c=%c";
		      pc = 3;
		    }
		}
	      break;

	    case CLASS_MNEMONICS:
	      if (c == 0)
		{
		  if (n < 0xc)
		    {
		      p = "add";
		    }
		  else
		    {
		      p = "dec";
		      pc = 1;
		    }
		}
	      else
		{
		  if (n < 4)
		    {
		      p = "clr";
		      pc = 1;
		    }
		  else if (n >= 0xc)
		    {
		      p = "exg";
		    }
		  else
		    {
		      p = "move";
		      if (n < 8)
			n -= 4;
		    }
		}
	      break;

	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      return addr;
	    }
	  break;

	case 0xb:
	  switch (disassembler_mode)
	    {
	    case HP_MNEMONICS:
	      if (c == 0)
		{
		  if (n >= 0xc)
		    {
		      p = "-";
		    }
		  else if ((n >= 4) && (n <= 7))
		    {
		      p = "%c=%c+1";
		      pc = 2;
		      n -= 4;
		    }
		  else
		    {
		      p = "-";
		    }
		}
	      else
		{
		  if (n < 4)
		    {
		      p = "%cSL";
		      pc = 1;
		    }
		  else if (n < 8)
		    {
		      p = "%cSR";
		      pc = 1;
		    }
		  else if (n < 0xc)
		    {
		      p = "%c=%c-1";
		      pc = 2;
		    }
		  else
		    {
		      p = "%c=-%c-1";
		      pc = 2;
		    }
		}
	      break;

	    case CLASS_MNEMONICS:
	      if (c == 0)
		{
		  if (n >= 0xc)
		    {
		      p = "subr";
		    }
		  else if ((n >= 4) && (n <= 7))
		    {
		      p = "inc";
		      pc = 1;
		      n -= 4;
		    }
		  else
		    {
		      p = "sub";
		    }
		}
	      else
		{
		  pc = 1;
		  if (n < 4)
		    {
		      p = "lsl";
		    }
		  else if (n < 8)
		    {
		      p = "lsr";
		    }
		  else if (n < 0xc)
		    {
		      p = "neg";
		    }
		  else
		    {
		      p = "not";
		    }
		}
	      break;

	    default:
	      p = append_str (out, "Unknown disassembler mode");
	      return addr;
	    }
	  break;

	}

      switch (disassembler_mode)
	{
	case HP_MNEMONICS:

	  if (pc == 0)
	    {
	      sprintf (buf, op_str_af[n + 16 * HP_MNEMONICS], p);
	    }
	  else if (pc == 1)
	    {
	      sprintf (buf, p, (n & 3) + 'A');
	    }
	  else if (pc == 2)
	    {
	      sprintf (buf, p, (n & 3) + 'A', (n & 3) + 'A');
	    }
	  else
	    {
	      sprintf (buf, p, hp_reg_1_af[n], hp_reg_2_af[n]);
	    }
	  p = append_str (out, buf);
	  p = append_tab (out);
	  p = append_field (p, fn);
	  break;

	case CLASS_MNEMONICS:

	  p = append_str (out, p);
	  p = append_field (p, fn);
	  p = append_tab (out);
	  if (pc == 1)
	    {
	      sprintf (buf, "%c", (n & 3) + 'a');
	      p = append_str (p, buf);
	    }
	  else
	    {
	      p = append_str (p, op_str_af[n + 16 * CLASS_MNEMONICS]);
	    }
	  break;

	default:
	  p = append_str (p, "Unknown disassembler mode");
	  break;
	}

      break;
    }
  *p = '\0';
  return addr;
}
