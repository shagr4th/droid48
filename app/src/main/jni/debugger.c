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

/* $Log: debugger.c,v $
 * Revision 1.8  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.7  1994/12/07  20:20:50  ecd
 * more functions
 *
 * Revision 1.7  1994/12/07  20:20:50  ecd
 * more functions
 *
 * Revision 1.6  1994/11/28  02:00:51  ecd
 * new functions: do_ram, do_stack
 *
 * Revision 1.5  1994/11/02  14:40:38  ecd
 * more functions
 *
 * Revision 1.4  1994/10/09  20:29:47  ecd
 * start of disassembler implementation.
 *
 * Revision 1.3  1994/10/06  16:30:05  ecd
 * added refresh_display()
 *
 * Revision 1.2  1994/10/05  08:36:44  ecd
 * more functions
 *
 * Revision 1.1  1994/10/04  15:12:21  ecd
 * Initial revision
 *
 *
 * $Id: debugger.c,v 1.8 1995/01/11 18:20:01 ecd Exp ecd $
 */

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#ifdef SUNOS
#include <memory.h>
#endif

#include "hp48.h"
#include "device.h"
#include "timer.h"
#include "x48.h"
#include "debugger.h"
#include "disasm.h"
#include "rpl.h"
#include "romio.h"
#include "resources.h"

extern char *readline __ProtoType__ ((const char *));
extern void add_history __ProtoType__ ((char *));

#define MAX_ARGS 16

int enter_debugger = 0;
int in_debugger = 0;
int exec_flags = 0;

static int continue_flag;
static char instr[100];

/*
 * Pointers in the HP48sx ROM
 */
#define DSKTOP_SX	0x70579
#define DSKBOT_SX	0x7057e
#define DSKTOP_GX	0x806f8
#define DSKBOT_GX	0x806fd

/*
 * Breakpoint related stuff
 */
#define BP_EXEC		1
#define BP_READ		2
#define BP_WRITE	4
#define BP_RANGE	8

#define MAX_BREAKPOINTS	32
int num_bkpts;

struct breakpoint
  {
    word_20 addr;
    word_20 end_addr;
    int flags;
  }

bkpt_tbl[MAX_BREAKPOINTS + 1];

/*
 * command functions
 */
static void do_break __ProtoType__ ((int, char **));
static void do_continue __ProtoType__ ((int, char **));
static void do_delete __ProtoType__ ((int, char **));
static void do_exit __ProtoType__ ((int, char **));
static void do_go __ProtoType__ ((int, char **));
static void do_help __ProtoType__ ((int, char **));
static void do_load __ProtoType__ ((int, char **));
static void do_mode __ProtoType__ ((int, char **));
static void do_quit __ProtoType__ ((int, char **));
static void do_regs __ProtoType__ ((int, char **));
static void do_save __ProtoType__ ((int, char **));
static void do_stack __ProtoType__ ((int, char **));
static void do_stat __ProtoType__ ((int, char **));
static void do_step __ProtoType__ ((int, char **));
static void do_ram __ProtoType__ ((int, char **));
static void do_reset __ProtoType__ ((int, char **));
static void do_rstk __ProtoType__ ((int, char **));

struct cmd
  {
    char *name;
    void (*func) __ProtoType__ ((int, char **));
    char *help;
  }

cmd_tbl[] =
{
  {
    "break", do_break,
    "break [address]            Set breakpoint at `address\' or show breakpoints"
  }
  ,
  {
    "b", do_break, 0
  }
  ,

  {
    "cont", do_continue,
    "cont                       Continue execution"
  }
  ,
  {
    "c", do_continue, 0
  }
  ,

  {
    "delete", do_delete,
    "delete [all | n]           Delete breakpoint or watchpoint number `n\',\n                           all breakpoints, or current breakpoint"
  }
  ,
  {
    "d", do_delete, 0
  }
  ,

  {
    "exit", do_exit,
    "exit                       Exit the emulator without saving"
  }
  ,

  {
    "go", do_go,
    "go address                 Set PC to `address\'"
  }
  ,

  {
    "help", do_help,
    "help                       Display this information"
  }
  ,
  {
    "h", do_help, 0
  }
  ,
  {
    "?", do_help, 0
  }
  ,

  {
    "load", do_load,
    "load                       Load emulator-state from files"
  }
  ,

  {
    "mode", do_mode,
    "mode [hp | class]          Show or set disassembler mode"
  }
  ,

  {
    "quit", do_quit,
    "quit                       Exit the emulator after saving its state"
  }
  ,
  {
    "q", do_quit, 0
  }
  ,

  {
    "ram", do_ram,
    "ram                        Show RAM layout"
  }
  ,

  {
    "reg", do_regs,
    "reg [register [hexvalue]]  Display or set register value"
  }
  ,
  {
    "r", do_regs, 0
  }
  ,

  {
    "reset", do_reset,
    "reset                      Set the HP48\'s PC to ZERO"
  }
  ,

  {
    "save", do_save,
    "save                       Save emulator-state to files"
  }
  ,

  {
    "stack", do_stack,
    "stack                      Display RPL stack"
  }
  ,

  {
    "stat", do_stat,
    "stat                       Display statistics for the emulator"
  }
  ,

  {
    "step", do_step,
    "step [n]                   Step one or n Instruction(s)"
  }
  ,
  {
    "s", do_step, 0
  }
  ,

  {
    "where", do_rstk,
    "where                      Show ML return stack"
  }
  ,

  {
    0, 0, 0
  }
};

void
#ifdef __FunctionProto__
init_debugger (void)
#else
init_debugger ()
#endif
{
  int i;

  num_bkpts = 0;
  for (i = 0; i < MAX_BREAKPOINTS; i++)
    bkpt_tbl[i].flags = 0;
  exec_flags = 0;
}

int
#ifdef __FunctionProto__
check_breakpoint (int type, word_20 addr)
#else
check_breakpoint (type, addr)
     int type;
     word_20 addr;
#endif
{
  struct breakpoint *bp;
  int i, n;

  bp = bkpt_tbl;
  n = num_bkpts;
  i = 0;
  for (; n > 0; bp++)
    {
      i++;
      if (bp->flags == 0)
	continue;
      n--;
      if (bp->flags & BP_RANGE && addr >= bp->addr && addr <= bp->end_addr)
	{
	  goto hit_it;
	}
      if (bp->flags & type && addr == bp->addr)
	{
	hit_it:
	  if (type == BP_READ)
	    {
	      printf ("%.5lX: Read watchpoint %d hit at %.5lX\n", saturn.PC,
		      i, addr);
	    }
	  else if (type == BP_WRITE)
	    {
	      printf ("%.5lX: Write watchpoint %d hit at %.5lX\n", saturn.PC,
		      i, addr);
	    }
	  else
	    {
	      printf ("Breakpoint %d hit at %.5lX\n", i, addr);
	    }
	  return 1;
	}
    }
  return 0;
}


char *
#ifdef __FunctionProto__
read_str(char *str, int n, int fp)
#else
read_str(str, n, fp)
	char *str;
	int n;
	int fp;
#endif
{
  int cc;
  int flags;

  while (1)
    {
      cc = read(fp, str, n);
      if (cc > 0)
        return str;
      if (cc == 0)
        return NULL;

      if (errno == EINTR)
        continue;

      if (errno == EAGAIN)
        {
          flags = fcntl(fp, F_GETFL, 0);
          flags &= ~ O_NONBLOCK;
          fcntl(fp, F_SETFL, flags);
          continue;
        }
 
      return NULL;
    }
}


static inline void
#ifdef __FunctionProto__
str_to_upper (char *arg)
#else
str_to_upper (arg)
     char *arg;
#endif
{
  int i;

  for (i = 0; i < strlen (arg); i++)
    {
      if ('a' <= arg[i] && arg[i] <= 'z')
	{
	  arg[i] = (char) ((int) arg[i] - (int) 'a' + (int) 'A');
	}
    }
}

static int
#ifdef __FunctionProto__
decode_dec (int *num, char *arg)
#else
decode_dec (num, arg)
     int *num;
     char *arg;
#endif
{
  int i;

  if (arg == (char *) 0)
    {
      printf ("Command requires an argument.\n");
      return 0;
    }

  *num = 0;
  for (i = 0; i < strlen (arg); i++)
    {
      *num *= 10;
      if ('0' <= arg[i] && arg[i] <= '9')
	{
	  *num += ((int) arg[i] - (int) '0');
	}
      else
	{
	  *num = 0;
	  printf ("Not a number: %s.\n", arg);
	  return 0;
	}
    }
  return 1;
}

static int
#ifdef __FunctionProto__
decode_20 (word_20 * addr, char *arg)
#else
decode_20 (addr, arg)
     word_20 *addr;
     char *arg;
#endif
{
  int i;

  if (arg == (char *) 0)
    {
      printf ("Command requires an argument.\n");
      return 0;
    }

  *addr = 0;
  for (i = 0; i < strlen (arg); i++)
    {
      *addr <<= 4;
      if ('0' <= arg[i] && arg[i] <= '9')
	{
	  *addr |= ((int) arg[i] - (int) '0');
	}
      else if ('A' <= arg[i] && arg[i] <= 'F')
	{
	  *addr |= ((int) arg[i] - (int) 'A' + 10);
	}
      else
	{
	  *addr = 0;
	  printf ("Not a number: %s.\n", arg);
	  return 0;
	}
      *addr &= 0xfffff;
    }
  return 1;
}

static int
#ifdef __FunctionProto__
decode_32 (word_32 * addr, char *arg)
#else
decode_32 (addr, arg)
     word_32 *addr;
     char *arg;
#endif
{
  int i;

  if (arg == (char *) 0)
    {
      printf ("Command requires an argument.\n");
      return 0;
    }

  *addr = 0;
  for (i = 0; i < strlen (arg); i++)
    {
      *addr <<= 4;
      if ('0' <= arg[i] && arg[i] <= '9')
	{
	  *addr |= ((int) arg[i] - (int) '0');
	}
      else if ('A' <= arg[i] && arg[i] <= 'F')
	{
	  *addr |= ((int) arg[i] - (int) 'A' + 10);
	}
      else
	{
	  *addr = 0;
	  printf ("Not a number: %s.\n", arg);
	  return 0;
	}
    }
  return 1;
}

static int
#ifdef __FunctionProto__
decode_64 (word_64 * addr, char *arg)
#else
decode_64 (addr, arg)
     word_64 *addr;
     char *arg;
#endif
{
  int i;

  if (arg == (char *) 0)
    {
      printf ("Command requires an argument.\n");
      return 0;
    }

  addr->lo = addr->hi = 0;
  for (i = 0; i < strlen (arg); i++)
    {
      addr->hi <<= 4;
      addr->hi |= ((addr->lo >> 28) & 0x0f);
      addr->lo <<= 4;
      if ('0' <= arg[i] && arg[i] <= '9')
	{
	  addr->lo |= ((int) arg[i] - (int) '0');
	}
      else if ('A' <= arg[i] && arg[i] <= 'F')
	{
	  addr->lo |= ((int) arg[i] - (int) 'A' + 10);
	}
      else
	{
	  addr->hi = addr->lo = 0;
	  printf ("Not a number: %s.\n", arg);
	  return 0;
	}
    }
  return 1;
}

char *
#ifdef __FunctionProto__
str_nibbles (word_20 addr, int n)
#else
str_nibbles (addr, n)
     word_20 addr;
     int n;
#endif
{
  static char str[1025];
  char *cp;
  int i;

  if (n > 1024)
    {
      str[0] = '\0';
      return str;
    }

  for (cp = str, i = 0; i < n; i++)
    {
      sprintf (cp, "%.1X", read_nibble (addr + i));
      cp++;
    }
  *cp = '\0';

  return str;
}

static int
#ifdef __FunctionProto__
confirm (const char *prompt)
#else
confirm (prompt)
     const char *prompt;
#endif
{
  char ans[80];

  printf ("%s (y or n) ", prompt);
  fflush (stdout);
  read_str (ans, sizeof (ans), 0);
  while (ans[0] != 'y' && ans[0] != 'Y' && ans[0] != 'n' && ans[0] != 'N')
    {
      printf ("Please answer y or n.\n");
      printf ("%s (y or n) ", prompt);
      fflush (stdout);
      read_str (ans, sizeof (ans), 0);
    }
  if (ans[0] == 'y' || ans[0] == 'Y')
    {
      return 1;
    }
  else
    {
      printf ("Not confirmed.\n");
      return 0;
    }
}

static void
#ifdef __FunctionProto__
do_break (int argc, char **argv)
#else
do_break (argc, argv)
     int argc;
     char *argv;
#endif
{
  int i;
  word_20 addr;

  if (argc == 1)
    {
      for (i = 0; i < MAX_BREAKPOINTS; i++)
	{
	  if (bkpt_tbl[i].flags == 0)
	    continue;
	  if (bkpt_tbl[i].flags == BP_EXEC)
	    {
	      printf ("Breakpoint %d at 0x%.5lX\n", i + 1, bkpt_tbl[i].addr);
	    }
	  else if (bkpt_tbl[i].flags == BP_RANGE)
	    {
	      printf ("Range watchpoint %d at 0x%.5lX - 0x%.5lX\n", i + 1,
		      bkpt_tbl[i].addr, bkpt_tbl[i].end_addr);
	    }
	  else
	    {
	      printf ("Watchpoint %d at 0x%.5lX\n", i + 1, bkpt_tbl[i].addr);
	    }
	}
    }
  else
    {
      str_to_upper (argv[1]);
      if (!decode_20 (&addr, argv[1]))
	{
	  return;
	}
      for (i = 0; i < MAX_BREAKPOINTS; i++)
	{
	  if (bkpt_tbl[i].flags == 0)
	    {
	      bkpt_tbl[i].flags = BP_EXEC;
	      bkpt_tbl[i].addr = addr;
	      printf ("Breakpoint %d at 0x%.5lX\n", i + 1, bkpt_tbl[i].addr);
	      num_bkpts++;
	      return;
	    }
	}
      printf ("Breakpoint table full\n");
    }
}

static void
#ifdef __FunctionProto__
do_continue (int argc, char **argv)
#else
do_continue (argc, argv)
     int argc;
     char *argv;
#endif
{
  continue_flag = 1;
}

static void
#ifdef __FunctionProto__
do_delete (int argc, char **argv)
#else
do_delete (argc, argv)
     int argc;
     char *argv;
#endif
{
  int num;

  if (argc == 1)
    {
      for (num = 0; num < MAX_BREAKPOINTS; num++)
	{
	  if (bkpt_tbl[num].addr == saturn.PC)
            {
	      if (bkpt_tbl[num].flags == BP_EXEC)
		{
		  printf ("Breakpoint %d at 0x%.5lX deleted.\n",
			  num + 1, bkpt_tbl[num].addr);
		}
	      else if (bkpt_tbl[num].flags == BP_RANGE)
		{
		  printf ("Range watchpoint %d at 0x%.5lX - 0x%.5lX deleted.\n",
		       num + 1, bkpt_tbl[num].addr, bkpt_tbl[num].end_addr);
		}
	      else if (bkpt_tbl[num].flags)
		{
		  printf ("Watchpoint %d at 0x%.5lX deleted.\n",
			  num + 1, bkpt_tbl[num].addr);
		}
	      num_bkpts--;
	      bkpt_tbl[num].addr = 0;
	      bkpt_tbl[num].flags = 0;
            }
	}
    }
  else
    {
      str_to_upper (argv[1]);
      if (!strcmp ("ALL", argv[1]))
	{
	  for (num = 0; num < MAX_BREAKPOINTS; num++)
	    {
	      bkpt_tbl[num].addr = 0;
	      bkpt_tbl[num].flags = 0;
	    }
	  num_bkpts = 0;
	  printf ("All breakpoints deleted.\n");
	}
      else
	{
	  if (decode_dec (&num, argv[1]))
	    {
	      if (num < 1 || num > MAX_BREAKPOINTS)
		{
		  printf ("Breakpoint %d out of range.\n", num);
		  return;
		}
	      num -= 1;
	      if (bkpt_tbl[num].flags == BP_EXEC)
		{
		  printf ("Breakpoint %d at 0x%.5lX deleted.\n",
			  num + 1, bkpt_tbl[num].addr);
		}
	      else if (bkpt_tbl[num].flags == BP_RANGE)
		{
		  printf ("Range watchpoint %d at 0x%.5lX - 0x%.5lX deleted.\n",
		       num + 1, bkpt_tbl[num].addr, bkpt_tbl[num].end_addr);
		}
	      else if (bkpt_tbl[num].flags)
		{
		  printf ("Watchpoint %d at 0x%.5lX deleted.\n",
			  num + 1, bkpt_tbl[num].addr);
		}
	      num_bkpts--;
	      bkpt_tbl[num].addr = 0;
	      bkpt_tbl[num].flags = 0;
	    }
	}
    }
}

static void
#ifdef __FunctionProto__
do_exit (int argc, char **argv)
#else
do_exit (argc, argv)
     int argc;
     char *argv;
#endif
{
  /*if (confirm ("Exit the emulator WITHOUT saving its state?"))
    {
      printf ("Exit.\n");
      XCloseDisplay(dpy);
      exit (0);
    }*/
}

static void
#ifdef __FunctionProto__
do_go (int argc, char **argv)
#else
do_go (argc, argv)
     int argc;
     char *argv;
#endif
{
  word_20 addr;

  str_to_upper (argv[1]);
  if (decode_20 (&addr, argv[1]))
    {
      saturn.PC = addr;
      enter_debugger &= ~ILLEGAL_INSTRUCTION;
    }
}

static void
#ifdef __FunctionProto__
do_help (int argc, char **argv)
#else
do_help (argc, argv)
     int argc;
     char *argv;
#endif
{
  int i;

  for (i = 0; cmd_tbl[i].name; i++)
    {
      if (cmd_tbl[i].help)
	{
	  printf ("%s.\n", cmd_tbl[i].help);
	}
    }
}

static void
#ifdef __FunctionProto__
do_load (int argc, char **argv)
#else
do_load (argc, argv)
     int argc;
     char *argv;
#endif
{
  saturn_t tmp_saturn;
  device_t tmp_device;

  if (confirm ("Load emulator-state from files?"))
    {
      memcpy (&tmp_saturn, &saturn, sizeof (saturn));
      memcpy (&tmp_device, &device, sizeof (device));
      memset (&saturn, 0, sizeof (saturn));
      if (read_files ())
	{
	  printf ("Loading done.\n");
	  enter_debugger &= ~ILLEGAL_INSTRUCTION;
	  if (tmp_saturn.rom)
	    {
	      free (tmp_saturn.rom);
	    }
	  if (tmp_saturn.ram)
	    {
	      free (tmp_saturn.ram);
	    }
	  if (tmp_saturn.port1)
	    {
	      free (tmp_saturn.port1);
	    }
	  if (tmp_saturn.port2)
	    {
	      free (tmp_saturn.port2);
	    }
	  init_display ();
	  update_display ();
#ifdef HAVE_XSHM
	  if (disp.display_update)
	    refresh_display ();
#endif
	}
      else
	{
	  printf ("Loading emulator-state from files failed.\n");
	  if (saturn.rom)
	    {
	      free (saturn.rom);
	    }
	  if (saturn.ram)
	    {
	      free (saturn.ram);
	    }
	  if (saturn.port1)
	    {
	      free (saturn.port1);
	    }
	  if (saturn.port2)
	    {
	      free (saturn.port2);
	    }
	  memcpy (&saturn, &tmp_saturn, sizeof (saturn));
	  memcpy (&device, &tmp_device, sizeof (device));
	}
    }
}

static void
#ifdef __FunctionProto__
do_mode (int argc, char **argv)
#else
do_mode (argc, argv)
     int argc;
     char *argv;
#endif
{
  if (argc < 2)
    {
      printf ("Disassembler uses %s mnemonics.\n", mode_name[disassembler_mode]);
    }
  else
    {
      str_to_upper (argv[1]);
      if (!strcmp ("HP", argv[1]))
	{
	  disassembler_mode = HP_MNEMONICS;
	}
      else if (!strcmp ("CLASS", argv[1]))
	{
	  disassembler_mode = CLASS_MNEMONICS;
	}
      else
	{
	  printf ("Unknown disassembler mode %s. Try \"help\".\n", argv[1]);
	}
    }
}

static void
#ifdef __FunctionProto__
do_quit (int argc, char **argv)
#else
do_quit (argc, argv)
     int argc;
     char *argv;
#endif
{
 /* if (confirm ("Quit the emulator and save its state?"))
    {
      printf ("Exit.\n");
      exit_emulator ();
      XCloseDisplay(dpy);
      exit (0);
    }*/
}

static void
#ifdef __FunctionProto__
set_reg (word_64 val, int n, unsigned char *r)
#else
set_reg (val, n, r)
     word_64 val;
     int n;
     unsigned char *r;
#endif
{
  int i;

  for (i = 0; i < n; i++)
    {
      if (i < 8)
	r[i] = (unsigned char) ((val.lo & (0xf << (4 * i))) >> (4 * i));
      else
	r[i] = (unsigned char) ((val.hi & (0xf << (4 * (i - 8)))) >> (4 * (i - 8)));
    }
}

static void
#ifdef __FunctionProto__
dump_reg (const char *reg, int n, unsigned char *r)
#else
dump_reg (reg, n, r)
     const char *reg;
     int n;
     unsigned char *r;
#endif
{
  int i;

  printf ("%s:\t", reg);
  for (i = n - 1; i >= 0; i--)
    {
      printf ("%.1X", r[i] & 0xf);
    }
  printf ("\n");
}


static void
#ifdef __FunctionProto__
set_st (word_64 val)
#else
set_st (val)
     word_64 val;
#endif
{
  int i;

  for (i = 0; i < 16; i++)
    saturn.PSTAT[i] = (val.lo & (1 << i)) ? 1 : 0;
}

static void
#ifdef __FunctionProto__
dump_st (void)
#else
dump_st ()
#endif
{
  int i;
  int val;

  val = 0;
  for (i = NR_PSTAT - 1; i >= 0; i--)
    {
      val <<= 1;
      val |= saturn.PSTAT[i] ? 1 : 0;
    }
  printf ("    ST:\t%.4X (", val);
  for (i = NR_PSTAT - 1; i > 0; i--)
    {
      if (saturn.PSTAT[i])
	{
	  printf ("%.1X ", i);
	}
      else
	{
	  printf ("- ");
	}
    }
  if (saturn.PSTAT[0])
    {
      printf ("%.1X)\n", 0);
    }
  else
    {
      printf ("-)\n");
    }
}

static void
#ifdef __FunctionProto__
set_hst (word_64 val)
#else
set_hst (val)
     word_64 val;
#endif
{
  saturn.XM = 0;
  saturn.SB = 0;
  saturn.SR = 0;
  saturn.MP = 0;
  if (val.lo & 1)
    saturn.XM = 1;
  if (val.lo & 2)
    saturn.SB = 1;
  if (val.lo & 4)
    saturn.SR = 1;
  if (val.lo & 8)
    saturn.MP = 1;
}

static void
#ifdef __FunctionProto__
dump_hst (void)
#else
dump_hst ()
#endif
{
  short hst = 0;
  if (saturn.XM != 0)
    hst |= 1;
  if (saturn.SB != 0)
    hst |= 2;
  if (saturn.SR != 0)
    hst |= 3;
  if (saturn.MP != 0)
    hst |= 4;
  printf ("   HST:\t%.1X    (%s%s%s%s)\n", hst,
          saturn.MP ? "MP " : "-- ", saturn.SR ? "SR " : "-- ",
          saturn.SB ? "SB " : "-- ", saturn.XM ? "XM" : "--");
}

static char *mctl_str_gx[] = {
  "MMIO       ", 
  "SysRAM     ",
  "Bank Switch",
  "Port 1     ",
  "Port 2     ",
  "SysROM     "
};

static char *mctl_str_sx[] = {
  "MMIO  ", 
  "SysRAM",
  "Port 1",
  "Port 2",
  "Extra ",
  "SysROM"
};

static void
#ifdef __FunctionProto__
do_ram (int argc, char **argv)
#else
do_ram (argc, argv)
     int argc;
     char *argv;
#endif
{
  int i;

  for (i = 0; i < 5; i++)
    {
      printf("%s ", opt_gx ? mctl_str_gx[i] : mctl_str_sx[i]);
      if (saturn.mem_cntl[i].unconfigured)
        printf("unconfigured\n");
      else
        if (i == 0)
          printf("configured to 0x%.5lx\n", saturn.mem_cntl[i].config[0]);
        else
          printf("configured to 0x%.5lX - 0x%.5lX\n",
                 saturn.mem_cntl[i].config[0],
                 (saturn.mem_cntl[i].config[0] | ~saturn.mem_cntl[i].config[1])
                 & 0xfffff);
    }
  if (opt_gx)
    printf("Port 2      switched to bank %d\n", saturn.bank_switch);
}

static void
#ifdef __FunctionProto__
do_regs (int argc, char **argv)
#else
do_regs (argc, argv)
     int argc;
     char *argv;
#endif
{
  int i;
  word_64 val;

  if (argc < 2)
    {
      /*
     * dump all registers
     */
      printf ("CPU is in %s mode. Registers:\n",
	      saturn.hexmode == HEX ? "HEX" : "DEC");
      dump_reg ("     A", 16, saturn.A);
      dump_reg ("     B", 16, saturn.B);
      dump_reg ("     C", 16, saturn.C);
      dump_reg ("     D", 16, saturn.D);
      printf ("    D0:\t%.5lX ->", saturn.D0);
      for (i = 0; i < 20; i += 5)
	{
	  printf (" %s", str_nibbles (saturn.D0 + i, 5));
	}
      printf ("\n");
      printf ("    D1:\t%.5lX ->", saturn.D1);
      for (i = 0; i < 20; i += 5)
	{
	  printf (" %s", str_nibbles (saturn.D1 + i, 5));
	}
      printf ("\n");
      printf ("     P:\t%.1X\n", saturn.P);
      disassemble (saturn.PC, instr);
      printf ("    PC:\t%.5lX -> %s\n", saturn.PC, instr);
      dump_reg ("    R0", 16, saturn.R0);
      dump_reg ("    R1", 16, saturn.R1);
      dump_reg ("    R2", 16, saturn.R2);
      dump_reg ("    R3", 16, saturn.R3);
      dump_reg ("    R4", 16, saturn.R4);
      dump_reg ("    IN", 4, saturn.IN);
      dump_reg ("   OUT", 3, saturn.OUT);
      printf (" CARRY:\t%.1d\n", saturn.CARRY);
      dump_st ();
      dump_hst ();
    }
  else if (argc == 2)
    {
      /*
     * dump specified register
     */
      str_to_upper (argv[1]);
      if (!strcmp ("A", argv[1]))
	{
          dump_reg ("     A", 16, saturn.A);
        }
      else if (!strcmp ("B", argv[1]))
        {
          dump_reg ("     B", 16, saturn.B);
        }
      else if (!strcmp ("C", argv[1]))
        {
          dump_reg ("     C", 16, saturn.C);
        }
      else if (!strcmp ("D", argv[1]))
        {
          dump_reg ("     D", 16, saturn.D);
	}
      else if (!strcmp ("D0", argv[1]))
	{
	  printf ("    D0:\t%.5lX ->", saturn.D0);
	  for (i = 0; i < 20; i += 5)
	    {
	      printf (" %s", str_nibbles (saturn.D0 + i, 5));
	    }
	  printf ("\n");
	}
      else if (!strcmp ("D1", argv[1]))
	{
	  printf ("    D1:\t%.5lX ->", saturn.D1);
	  for (i = 0; i < 20; i += 5)
	    {
	      printf (" %s", str_nibbles (saturn.D1 + i, 5));
	    }
	  printf ("\n");
	}
      else if (!strcmp ("P", argv[1]))
	{
	  printf ("     P:\t%.1X\n", saturn.P);
	}
      else if (!strcmp ("PC", argv[1]))
	{
	  disassemble (saturn.PC, instr);
	  printf ("    PC:\t%.5lX -> %s\n", saturn.PC, instr);
	}
      else if (!strcmp ("R0", argv[1]))
	{
          dump_reg ("    R0", 16, saturn.R0);
        }
      else if (!strcmp ("R1", argv[1]))
        {
          dump_reg ("    R1", 16, saturn.R1);
        }
      else if (!strcmp ("R2", argv[1]))
        {
          dump_reg ("    R2", 16, saturn.R2);
        }
      else if (!strcmp ("R3", argv[1]))
        {
          dump_reg ("    R3", 16, saturn.R3);
        }
      else if (!strcmp ("R4", argv[1]))
        {
          dump_reg ("    R4", 16, saturn.R4);
        }
      else if (!strcmp ("IN", argv[1]))
        {
          dump_reg ("    IN", 4, saturn.IN);
        }
      else if (!strcmp ("OUT", argv[1]))
        {
          dump_reg ("   OUT", 3, saturn.OUT);
	}
      else if (!strcmp ("CARRY", argv[1]))
	{
	  printf (" CARRY:\t%.1d\n", saturn.CARRY);
	}
      else if (!strcmp ("CY", argv[1]))
	{
	  printf (" CARRY:\t%.1d\n", saturn.CARRY);
	}
      else if (!strcmp ("ST", argv[1]))
	{
	  dump_st ();
	}
      else if (!strcmp ("HST", argv[1]))
	{
	  dump_hst ();
	}
      else
	{
	  printf ("No Register %s in CPU.\n", argv[1]);
	}
    }
  else
    {
      /*
     * set specified register
     */
      str_to_upper (argv[1]);
      str_to_upper (argv[2]);
      if (decode_64 (&val, argv[2]))
	{
	  if (!strcmp ("A", argv[1]))
	    {
              set_reg (val, 16, saturn.A);
              dump_reg ("     A", 16, saturn.A);
            }
          else if (!strcmp ("B", argv[1]))
            {
              set_reg (val, 16, saturn.B);
              dump_reg ("     B", 16, saturn.B);
            }
          else if (!strcmp ("C", argv[1]))
            {
              set_reg (val, 16, saturn.C);
              dump_reg ("     C", 16, saturn.C);
            }
          else if (!strcmp ("D", argv[1]))
            {
              set_reg (val, 16, saturn.D);
              dump_reg ("     D", 16, saturn.D);
	    }
	  else if (!strcmp ("D0", argv[1]))
	    {
	      saturn.D0 = (word_20)(val.lo & 0xfffff);
	      printf ("    D0:\t%.5lX ->", saturn.D0);
	      for (i = 0; i < 20; i += 5)
		{
		  printf (" %s", str_nibbles (saturn.D0 + i, 5));
		}
	      printf ("\n");
	    }
	  else if (!strcmp ("D1", argv[1]))
	    {
	      saturn.D1 = (word_20)(val.lo & 0xfffff);
	      printf ("    D1:\t%.5lX ->", saturn.D1);
	      for (i = 0; i < 20; i += 5)
		{
		  printf (" %s", str_nibbles (saturn.D1 + i, 5));
		}
	      printf ("\n");
	    }
	  else if (!strcmp ("P", argv[1]))
	    {
	      saturn.P = (word_4)(val.lo & 0xf);
	      printf ("     P:\t%.1X\n", saturn.P);
	    }
	  else if (!strcmp ("PC", argv[1]))
	    {
	      saturn.PC = (word_20)(val.lo & 0xfffff);
	      disassemble (saturn.PC, instr);
	      printf ("    PC:\t%.5lX -> %s\n", saturn.PC, instr);
	    }
	  else if (!strcmp ("R0", argv[1]))
	    {
              set_reg (val, 16, saturn.R0);
              dump_reg ("    R0", 16, saturn.R0);
            }
          else if (!strcmp ("R1", argv[1]))
            {
              set_reg (val, 16, saturn.R1);
              dump_reg ("    R1", 16, saturn.R1);
            }
          else if (!strcmp ("R2", argv[1]))
            {
              set_reg (val, 16, saturn.R2);
              dump_reg ("    R2", 16, saturn.R2);
            }
          else if (!strcmp ("R3", argv[1]))
            {
              set_reg (val, 16, saturn.R3);
              dump_reg ("    R3", 16, saturn.R3);
            }
          else if (!strcmp ("R4", argv[1]))
            {
              set_reg (val, 16, saturn.R4);
              dump_reg ("    R4", 16, saturn.R4);
            }
          else if (!strcmp ("IN", argv[1]))
            {
              set_reg (val, 4, saturn.IN);
              dump_reg ("    IN", 4, saturn.IN);
            }
          else if (!strcmp ("OUT", argv[1]))
            {
              set_reg (val, 3, saturn.OUT);
              dump_reg ("   OUT", 3, saturn.OUT);
	    }
	  else if (!strcmp ("CARRY", argv[1]))
	    {
	      saturn.CARRY = (word_1)(val.lo & 0x1);
	      printf (" CARRY:\t%.1d\n", saturn.CARRY);
	    }
	  else if (!strcmp ("CY", argv[1]))
	    {
	      saturn.CARRY = (word_1)(val.lo & 0x1);
	      printf (" CARRY:\t%.1d\n", saturn.CARRY);
	    }
	  else if (!strcmp ("ST", argv[1]))
	    {
	      set_st (val);
	      dump_st ();
	    }
	  else if (!strcmp ("HST", argv[1]))
	    {
	      set_hst (val);
	      dump_hst ();
	    }
	  else
	    {
	      printf ("No Register %s in CPU.\n", argv[1]);
	    }
	}
    }
}

static void
#ifdef __FunctionProto__
do_save (int argc, char **argv)
#else
do_save (argc, argv)
     int argc;
     char *argv;
#endif
{
  if (write_files ())
    {
      printf ("Saving done.\n");
    }
  else
    {
      printf ("Saving emulator-state failed.\n");
    }
}

struct se {
  int        se_n;
  word_20    se_p;
  struct se *se_next;
};

static void
#ifdef __FunctionProto__
do_stack (int argc, char **argv)
#else
do_stack (argc, argv)
     int argc;
     char *argv;
#endif
{
  word_20 dsktop, dskbot;
  word_20 sp = 0, end = 0, ent = 0;
  word_20 ram_base, ram_mask;
  char    buf[65536];
  struct se *stack, *se;
  int n;

  ram_base = saturn.mem_cntl[1].config[0];
  ram_mask = saturn.mem_cntl[1].config[1];
  if (opt_gx)
    {
      saturn.mem_cntl[1].config[0] = 0x80000;
      saturn.mem_cntl[1].config[1] = 0xc0000;
      dsktop = DSKTOP_GX;
      dskbot = DSKBOT_GX;
    }
  else
    {
      saturn.mem_cntl[1].config[0] = 0x70000;
      saturn.mem_cntl[1].config[1] = 0xf0000;
      dsktop = DSKTOP_SX;
      dskbot = DSKBOT_SX;
    }

  load_addr(&sp, dsktop, 5);
  load_addr(&end, dskbot, 5);

  stack = (struct se *)0;
  n = 0;
  do
    {
      load_addr(&ent, sp, 5);
      if (ent == 0)
        break;
      n++;
      sp += 5;
      se = (struct se *)malloc(sizeof(struct se));
      if (se == 0)
        {
          fprintf(stderr, "Out off memory.\n");
          break;
        }
      se->se_n = n;
      se->se_p = ent;
      se->se_next = stack;
      stack = se;
    }
  while (sp <= end);

  if (n == 0)
    printf("Empty stack.\n");

  se = stack;
  while (se)
    {
      decode_rpl_obj(se->se_p, buf);
      if (se->se_n != 1)
        if (strlen(buf) > 63)
          {
            sprintf(&buf[60], "...");
            buf[63] = '\0';
          }
      printf("%5d: %.5lX -> %s\n", se->se_n, se->se_p, buf);
      se = se->se_next;
    } 

  se = stack;
  while (se)
    {
      stack = se;
      se = se->se_next;
      free(stack);
    }

  saturn.mem_cntl[1].config[0] = ram_base;
  saturn.mem_cntl[1].config[1] = ram_mask;
}

static void
#ifdef __FunctionProto__
do_stat (int argc, char **argv)
#else
do_stat (argc, argv)
     int argc;
     char *argv;
#endif
{
  printf ("Instructions/s: %ld\n", saturn.i_per_s);
  printf ("Timer 1 I/TICK: %d\n", saturn.t1_tick);
  printf ("Timer 2 I/TICK: %d\n", saturn.t2_tick);
}

static void
#ifdef __FunctionProto__
do_step (int argc, char **argv)
#else
do_step (argc, argv)
     int argc;
     char *argv;
#endif
{
  word_20 next_instr;
  word_32 n;
  int leave;

  if (enter_debugger & ILLEGAL_INSTRUCTION)
    {
      printf ("Can\'t step into an illegal instruction.");
      return;
    }

  n = 1;
  if (argc > 1)
    if (!decode_32 (&n, argv[1]))
      return;

  if (n <= 0)
    return;

  in_debugger = 1;
  step_instruction ();

  if (exec_flags & EXEC_BKPT)
    {
      if (check_breakpoint (BP_EXEC, saturn.PC))
	{
	  enter_debugger |= BREAKPOINT_HIT;
	  return;
	}
    }

  next_instr = saturn.PC;

  sched_adjtime = 0;
  schedule ();

  enter_debugger = 0;
  while (1)
    {
      if (enter_debugger)
	break;

      leave = 0;

      if (saturn.PC == next_instr)
	{
	  n--;
	  leave = 1;
	  if (n == 0)
	    break;
	}

      step_instruction ();

      if (exec_flags & EXEC_BKPT)
	{
	  if (check_breakpoint (BP_EXEC, saturn.PC))
	    {
	      enter_debugger |= BREAKPOINT_HIT;
	      break;
	    }
	}

      if (leave)
	next_instr = saturn.PC;

      schedule ();
    }
}

static void
#ifdef __FunctionProto__
do_reset (int argc, char **argv)
#else
do_reset (argc, argv)
     int argc;
     char *argv;
#endif
{
  if (confirm ("Do a RESET (PC = 00000)?"))
    {
      saturn.PC = 0;
      enter_debugger &= ~ILLEGAL_INSTRUCTION;
    }
}

static void
#ifdef __FunctionProto__
do_rstk (int argc, char **argv)
#else
do_rstk (argc, argv)
     int argc;
     char *argv;
#endif
{
  int i, j;

  disassemble (saturn.PC, instr);
  printf ("PC: %.5lX: %s\n", saturn.PC, instr);
  if (saturn.rstkp < 0)
    {
      printf ("Empty return stack.\n");
    }
  else
    {
      j = 0;
      for (i = saturn.rstkp; i >= 0; i--)
	{
	  disassemble (saturn.rstk[i], instr);
	  printf ("%2d: %.5lX: %s\n", j, saturn.rstk[i], instr);
	  j++;
	}
    }
}

int
#ifdef __FunctionProto__
debug (void)
#else
debug ()
#endif
{
  t1_t2_ticks ticks;
  struct cmd *cmdp;
  char *cp;
  int argc;
  char *argv[MAX_ARGS];
  char *rl = NULL;
  static char *cl = (char *) 0;
  static char *old_line = (char *) 0;
  int i;

  /*
   * do we want to debug ???
   */
  if (!useDebugger)
    {
      if (enter_debugger & ILLEGAL_INSTRUCTION)
        {
          if (!quiet)
            fprintf (stderr, "%s: reset (illegal instruction at 0x%.5lX)\n",
                     progname, saturn.PC);
          saturn.PC = 0;
        }
      if (enter_debugger & USER_INTERRUPT)
        if (verbose)
          printf ("%s: user interrupt (SIGINT) ignored\n", progname);
      if (enter_debugger & BREAKPOINT_HIT)
        if (verbose)
          printf ("%s: breakpoint hit at 0x%.5lX ignored\n",
                  progname, saturn.PC);
      if (enter_debugger & TRAP_INSTRUCTION)
        if (verbose)
          printf ("%s: trap instruction at 0x%.5lX ignored\n",
                  progname, saturn.PC);
      enter_debugger = 0;
      return 0;
    }

  /*
   * update the lcd if necessary
   */
  if (device.display_touched)
    {
      device.display_touched = 0;
      update_display ();
#ifdef HAVE_XSHM
      if (disp.display_update)
	refresh_display ();
#endif
    }

  /*
   * debugging is counted as idle time
   */
  
  stop_timer (RUN_TIMER);
  start_timer (IDLE_TIMER);

  continue_flag = 0;

  if (enter_debugger & ILLEGAL_INSTRUCTION)
    {
      printf ("ILLEGAL INSTRUCTION at %.5lX : %s\n",
	      saturn.PC, str_nibbles (saturn.PC, 16));
    }

  if (enter_debugger & TRAP_INSTRUCTION)
    {
      printf ("TRAP at %.5lX : %s\n",
	      saturn.PC - 5, str_nibbles (saturn.PC - 5, 16));
      enter_debugger &= ~TRAP_INSTRUCTION;
    }

  do
    {

      /*
       * print current instruction
       */
      disassemble (saturn.PC, instr);
      printf ("%.5lX: %s\n", saturn.PC, instr);

      /*
       * read a command
       */
#ifdef HAVE_READLINE
      rl = readline ("x48-debug> ");
#else
      if (rl == (char *) 0)
        rl = (char *)malloc((size_t)80);
      printf("x48-debug> ");
      fflush(stdout);
      rl = read_str(rl, 80, 0);
#endif

      if (rl == (char *) 0)
	{
	  continue_flag = 1;
	  continue;
	}
      if (*rl == '\0')
	{
	  free (rl);
          rl = (char *) 0;
	  if (cl)
            {
	      free (cl);
              cl = (char *) 0;
            }
	  if (old_line)
	    cl = strcpy ((char *) malloc (strlen (old_line)), old_line);
	  else
	    cl = strcpy ((char *) malloc (strlen ("(null)")), "(null)");
	}
      else
	{
#ifndef HAVE_READLINE
          if (rl[strlen(rl) - 1] == '\n')
            rl[strlen(rl) - 1] = '\0';
#endif
	  if (cl)
	    {
	      free (cl);
              cl = (char *) 0;
	    }
	  if (old_line)
	    {
	      free (old_line);
              old_line = (char *) 0;
	    }
	  cl = strcpy ((char *) malloc (strlen (rl)), rl);
	  old_line = strcpy ((char *) malloc (strlen (rl)), rl);
#ifdef HAVE_READLINE
	  add_history (rl);
#endif
	  free (rl);
          rl = (char *) 0;
	}

      /*
       * decode the commandline
       */
      cp = strtok (cl, " \t");
      for (cmdp = cmd_tbl; cmdp->name; cmdp++)
	{
	  if (strcmp (cp, cmdp->name) == 0)
	    {
	      break;
	    }
	}

      argc = 0;
      argv[argc++] = cp;
      while ((cp = strtok ((char *) 0, " \t")) != (char *) 0)
	{
	  argv[argc++] = cp;
	  if (argc == MAX_ARGS)
	    break;
	}
      for (i = argc; i < MAX_ARGS; i++)
	argv[i] = (char *) NULL;

      /*
       * execute the command, if valid
       */
      if (cmdp->func)
	{
	  (*cmdp->func) (argc, argv);
	}
      else
	{
	  printf ("Undefined command \"%s\". Try \"help\".\n", argv[0]);
	}
      in_debugger = 0;

    }
  while (!continue_flag);

  /*
   * adjust the hp48's timers
   */
  in_debugger = 1;
  ticks = get_t1_t2 ();
  in_debugger = 0;
  
  if (saturn.t2_ctrl & 0x01)
    {
      saturn.timer2 = ticks.t2_ticks;
    }

  saturn.timer1 = (set_t1 - ticks.t1_ticks) & 0xf;

  sched_adjtime = 0;

  /*
   * restart timers
   */
  stop_timer (IDLE_TIMER);
  start_timer (RUN_TIMER);

  set_accesstime();

  if (enter_debugger & ILLEGAL_INSTRUCTION)
    {
      printf ("Reset (ILLEGAL INSTRUCTION)\n");
      saturn.PC = 0;
    }
  else
    {
      printf ("Continue.\n");
    }

  enter_debugger = 0;

  /*
   * Set exec_flags according to breakpoints, etc.
   */
  exec_flags = 0;
  if (num_bkpts)
    exec_flags |= EXEC_BKPT;

  return 0;
}

int
#ifdef __FunctionProto__
emulate_debug (void)
#else
emulate_debug ()
#endif
{
  do
    {

      step_instruction ();

      if (exec_flags & EXEC_BKPT)
	{
	  if (check_breakpoint (BP_EXEC, saturn.PC))
	    {
	      enter_debugger |= BREAKPOINT_HIT;
	      break;
	    }
	}

      if (schedule_event-- == 0)
	{
	  schedule ();
	}

    }
  while (!enter_debugger);

  return 0;
}
