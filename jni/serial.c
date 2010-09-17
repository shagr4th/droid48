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

/* $Log: serial.c,v $
 * Revision 1.11  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.10  1994/12/07  20:20:50  ecd
 * complete change in handling of serial line,
 * lines can be turned off now
 *
 * Revision 1.10  1994/12/07  20:20:50  ecd
 * complete change in handling of serial line,
 * lines can be turned off now
 *
 * Revision 1.9  1994/11/28  02:00:51  ecd
 * added support for drawing the connections in the window title
 *
 * Revision 1.8  1994/11/02  14:44:28  ecd
 * support for HPUX added
 *
 * Revision 1.7  1994/10/06  16:30:05  ecd
 * new init for IRIX
 * added CREAD for serial line
 *
 * Revision 1.6  1994/10/05  08:49:59  ecd
 * changed printf() to print the correct /dev/ttyp?
 *
 * Revision 1.5  1994/09/30  12:37:09  ecd
 * check if serial device is opened by OPENIO
 *
 * Revision 1.4  1994/09/18  15:29:22  ecd
 * turned off unused rcsid message
 *
 * Revision 1.3  1994/09/13  16:57:00  ecd
 * changed to plain X11
 *
 * Revision 1.2  1994/08/31  18:23:21  ecd
 * changed IR and wire definitions.
 *
 * Revision 1.1  1994/08/26  11:09:02  ecd
 * Initial revision
 *
 * $Id: serial.c,v 1.11 1995/01/11 18:20:01 ecd Exp ecd $
 */


#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#if defined(HPUX) || defined(CSRG_BASED)
#  include <sys/ioctl.h>
#endif
#include <unistd.h>
#include <termios.h>
#ifdef SOLARIS
#  include <sys/stream.h>
#  include <sys/stropts.h>
#  include <sys/termios.h>
#endif

#include "hp48.h"
#include "device.h"
#include "hp48_emu.h"
#include "resources.h"

static int wire_fd;
static int ir_fd;
static int ttyp;

extern int rece_instr;

static char *wire_name = (char *)0;
static char *ir_name = (char *)0;

/* #define DEBUG_SERIAL */

void
#ifdef __FunctionProto__
update_connection_display(void)
#else
update_connection_display()
#endif
{
  if (wire_fd == -1)
    {
      if (wire_name) free(wire_name);
      wire_name = (char *)0;
    }
  if (ir_fd == -1)
    {
      if (ir_name) free(ir_name);
      ir_name = (char *)0;
    }
  ShowConnections(wire_name, ir_name);
}

int
#ifdef __FunctionProto__
serial_init(void)
#else
serial_init()
#endif
{
  char *p;
  int   c;
  int   n;
  char  tty_dev_name[128];
  struct termios ttybuf;

  wire_fd = -1;
  ttyp = -1;
  if (useTerminal)
    {
#if defined(IRIX)
      if ((p = _getpty(&wire_fd, O_RDWR | O_EXCL | O_NDELAY, 0666, 0)) == NULL)
        {
          wire_fd = -1;
          ttyp = -1;
        }
      else
        {
          if ((ttyp = open(p, O_RDWR | O_NDELAY, 0666)) < 0)
            {
              close(wire_fd);
              wire_fd = -1;
              ttyp = -1;
            }
          else
            {
              if (verbose)
                printf("%s: wire connection on %s\n", progname, p);
              wire_name = strdup(p);
            }
        }
#elif defined(SOLARIS)
      if ((wire_fd = open("/dev/ptmx", O_RDWR | O_NONBLOCK, 0666)) >= 0)
        {
          grantpt(wire_fd);
          unlockpt(wire_fd);
          p = ptsname(wire_fd);
          strcpy(tty_dev_name, p);
          if ((ttyp = open(tty_dev_name, O_RDWR | O_NDELAY, 0666)) >= 0)
            {
              ioctl(ttyp, I_PUSH, "ptem");
              ioctl(ttyp, I_PUSH, "ldterm");
              if (verbose)
                printf("%s: wire connection on %s\n", progname,
                      tty_dev_name);
              wire_name = strdup(tty_dev_name);
            }
        }
#elif defined(LINUX)
      /* Unix98 PTY (Preferred) */
      if ((wire_fd = open("/dev/ptmx", O_RDWR | O_NONBLOCK, 0666)) >= 0)
        {
          grantpt(wire_fd);
          unlockpt(wire_fd);
          if (ptsname_r(wire_fd, tty_dev_name, 128)) {
	      perror("Could not get the name of the wire device.");
	      exit(-1);
	  }
          if ((ttyp = open(tty_dev_name, O_RDWR | O_NDELAY, 0666)) >= 0)
            {
              if (verbose)
                printf("%s: wire connection on %s\n", progname,
                      tty_dev_name);
              wire_name = strdup(tty_dev_name);
            }
        }
      /* BSD PTY (Legacy) */
      else
	{
          c = 'p';
          do
            {
              for (n = 0; n < 16; n++)
                {
                  sprintf(tty_dev_name, "/dev/pty%c%x", c, n);
                  if ((wire_fd = open(tty_dev_name,
                                      O_RDWR | O_EXCL | O_NDELAY, 0666)) >= 0)
                    {
                      ttyp = wire_fd;
                      sprintf(tty_dev_name, "/dev/tty%c%x", c, n);
                      if (verbose)
                        printf("%s: wire connection on %s\n", progname,
                               tty_dev_name);
                      wire_name = strdup(tty_dev_name);
                      break;
                    }
                }
              c++;
            }
          while ((wire_fd < 0) && (errno != ENOENT));
	}
#else
      /*
       * Here we go for SUNOS, HPUX
       */
      c = 'p';
      do
        {
          for (n = 0; n < 16; n++)
            {
              sprintf(tty_dev_name, "/dev/ptyp%x", n);
              if ((wire_fd = open(tty_dev_name,
                                  O_RDWR | O_EXCL | O_NDELAY, 0666)) >= 0)
                {
                  sprintf(tty_dev_name, "/dev/tty%c%x", c, n);
                  if ((ttyp = open(tty_dev_name, O_RDWR | O_NDELAY, 0666)) < 0)
                    {
                      wire_fd = -1;
                      ttyp = -1;
                    }
                  else
                    {
                      if (verbose)
                        printf("%s: wire connection on %s\n", progname,
                               tty_dev_name);
                      wire_name = strdup(tty_dev_name);
                      break;
                    }
                }
            }
          c++;
        }
      while ((wire_fd < 0) && (errno != ENOENT));
#endif
    }

  if (ttyp >= 0)
    {
#if defined(TCSANOW)
      if (tcgetattr(ttyp, &ttybuf) < 0)
#else
      if (ioctl(ttyp, TCGETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(wire, TCGETS) failed, errno = %d\n",
                    progname, errno);
          wire_fd = -1;
          ttyp = -1;
        }
    }

  ttybuf.c_lflag = 0;
  ttybuf.c_iflag = 0;
  ttybuf.c_oflag = 0;
  ttybuf.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  for (n = 0; n < NCCS; n++)
    ttybuf.c_cc[n] = 0;
  ttybuf.c_cc[VTIME] = 0;
  ttybuf.c_cc[VMIN] = 1;

  if (ttyp >= 0)
    {
#if defined(TCSANOW)
      if (tcsetattr(ttyp, TCSANOW, &ttybuf) < 0)
#else
      if (ioctl(ttyp, TCSETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(wire, TCSETS) failed, errno = %d\n",
                    progname, errno);
          wire_fd = -1;
          ttyp = -1;
        }
    }

  ir_fd = -1;
  if (useSerial)
    {
      sprintf(tty_dev_name, serialLine);
      if ((ir_fd = open(tty_dev_name, O_RDWR | O_NDELAY)) >= 0)
        {
          if (verbose)
            printf("%s: IR connection on %s\n", progname, tty_dev_name);
          ir_name = strdup(tty_dev_name);
	}
    }

  if (ir_fd >= 0)
    {
#if defined(TCSANOW)
      if (tcgetattr(ir_fd, &ttybuf) < 0)
#else
      if (ioctl(ir_fd, TCGETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(IR, TCGETS) failed, errno = %d\n",
                    progname, errno);
          ir_fd = -1;
        }
    }

  ttybuf.c_lflag = 0;
  ttybuf.c_iflag = 0;
  ttybuf.c_oflag = 0;
  ttybuf.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  for (n = 0; n < NCCS; n++)
    ttybuf.c_cc[n] = 0;
  ttybuf.c_cc[VTIME] = 0;
  ttybuf.c_cc[VMIN] = 1;

  if (ir_fd >= 0)
    {
#if defined(TCSANOW)
      if (tcsetattr(ir_fd, TCSANOW, &ttybuf) < 0)
#else
      if (ioctl(ir_fd, TCSETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(IR, TCSETS) failed, errno = %d\n",
                    progname, errno);
          ir_fd = -1;
        }
    }
  update_connection_display();
  return 1;
}

void
#ifdef __FunctionProto__
serial_baud(int baud)
#else
serial_baud(baud)
int baud;
#endif
{
  int error = 0;
  struct termios ttybuf;

  if (ir_fd >= 0)
    {
#if defined(TCSANOW)
      if (tcgetattr(ir_fd, &ttybuf) < 0)
#else
      if (ioctl(ir_fd, TCGETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(IR,  TCGETS) failed, errno = %d\n",
                    progname, errno);
          ir_fd = -1;
          error = 1;
        }
    }

#if defined(__APPLE__)
  baud &= 0x7;
  switch (baud)
    {
      case 0:	/* 1200 */
        ttybuf.c_cflag |= B1200;
        break;
      case 1:	/* 1920 */
#  ifdef B1920
        ttybuf.c_cflag |= B1920;
#  endif
        break;
      case 2:	/* 2400 */
        ttybuf.c_cflag |= B2400;
        break;
      case 3:	/* 3840 */
#  ifdef B3840
        ttybuf.c_cflag |= B3840;
#  endif
        break;
      case 4:	/* 4800 */
        ttybuf.c_cflag |= B4800;
        break;
      case 5:	/* 7680 */
#  ifdef B7680
        ttybuf.c_cflag |= B7680;
#  endif
        break;
      case 6:	/* 9600 */
        ttybuf.c_cflag |= B9600;
        break;
      case 7:	/* 15360 */
#  ifdef B15360
        ttybuf.c_cflag |= B15360;
#  endif
        break;
    }

  if ((ir_fd >= 0) && ((ttybuf.c_ospeed) == 0))
    {
      if (!quiet)
        LOGE( "%s: can\'t set baud rate, using 9600\n", progname);
      ttybuf.c_cflag |= B9600;
    }
#else
  ttybuf.c_cflag &= ~CBAUD;

  baud &= 0x7;
  switch (baud)
    {
      case 0:	/* 1200 */
        ttybuf.c_cflag |= B1200;
        break;
      case 1:	/* 1920 */
#  ifdef B1920
        ttybuf.c_cflag |= B1920;
#  endif
        break;
      case 2:	/* 2400 */
        ttybuf.c_cflag |= B2400;
        break;
      case 3:	/* 3840 */
#  ifdef B3840
        ttybuf.c_cflag |= B3840;
#  endif
        break;
      case 4:	/* 4800 */
        ttybuf.c_cflag |= B4800;
        break;
      case 5:	/* 7680 */
#  ifdef B7680
        ttybuf.c_cflag |= B7680;
#  endif
        break;
      case 6:	/* 9600 */
        ttybuf.c_cflag |= B9600;
        break;
      case 7:	/* 15360 */
#  ifdef B15360
        ttybuf.c_cflag |= B15360;
#  endif
        break;
    }

  if ((ir_fd >= 0) && ((ttybuf.c_cflag & CBAUD) == 0))
    {
      if (!quiet)
        LOGE( "%s: can\'t set baud rate, using 9600\n", progname);
      ttybuf.c_cflag |= B9600;
    }
#endif
  if (ir_fd >= 0)
    {
#if defined(TCSANOW)
      if (tcsetattr(ir_fd, TCSANOW, &ttybuf) < 0)
#else
      if (ioctl(ir_fd, TCSETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(IR,  TCSETS) failed, errno = %d\n",
                    progname, errno);
          ir_fd = -1;
          error = 1;
        }
    }

  if (ttyp >= 0)
    {
#if defined(TCSANOW)
      if (tcgetattr(ttyp, &ttybuf) < 0)
#else
      if (ioctl(ttyp, TCGETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(wire, TCGETS) failed, errno = %d\n",
                    progname, errno);
          wire_fd = -1;
          ttyp = -1;
          error = 1;
        }
    }

#if defined(__APPLE__)
#else
  ttybuf.c_cflag &= ~CBAUD;

  baud &= 0x7;
  switch (baud)
    {
      case 0:	/* 1200 */
        ttybuf.c_cflag |= B1200;
        break;
      case 1:	/* 1920 */
#  ifdef B1920
        ttybuf.c_cflag |= B1920;
#  endif
        break;
      case 2:	/* 2400 */
        ttybuf.c_cflag |= B2400;
        break;
      case 3:	/* 3840 */
#  ifdef B3840
        ttybuf.c_cflag |= B3840;
#  endif
        break;
      case 4:	/* 4800 */
        ttybuf.c_cflag |= B4800;
        break;
      case 5:	/* 7680 */
#  ifdef B7680
        ttybuf.c_cflag |= B7680;
#  endif
        break;
      case 6:	/* 9600 */
        ttybuf.c_cflag |= B9600;
        break;
      case 7:	/* 15360 */
#  ifdef B15360
        ttybuf.c_cflag |= B15360;
#  endif
        break;
    }

  if ((ttyp >= 0) && ((ttybuf.c_cflag & CBAUD) == 0))
    {
      if (!quiet)
        LOGE( "%s: can\'t set baud rate, using 9600\n", progname);
      ttybuf.c_cflag |= B9600;
    }
#endif
  if (ttyp >= 0)
    {
#if defined(TCSANOW)
      if (tcsetattr(ttyp, TCSANOW, &ttybuf) < 0)
#else
      if (ioctl(ttyp, TCSETS, (char *)&ttybuf) < 0)
#endif
        {
          if (!quiet)
            LOGE( "%s: ioctl(wire, TCSETS) failed, errno = %d\n",
                    progname, errno);
          wire_fd = -1;
          ttyp = -1;
          error = 1;
        }
    }
  if (error)
    update_connection_display();
}


void
#ifdef __FunctionProto__
transmit_char(void)
#else
transmit_char()
#endif
{
#ifdef DEBUG_SERIALx
  LOGE( "XMT %s\n", (saturn.ir_ctrl & 0x04) ? "IR" : "wire");
#endif

  if (saturn.ir_ctrl & 0x04) {
    if (ir_fd == -1) {
      saturn.tcs &= 0x0e;
      if (saturn.io_ctrl & 0x04) {
        do_interupt();
      }
      return;
    }
  } else {
    if (wire_fd == -1) {
      saturn.tcs &= 0x0e;
      if (saturn.io_ctrl & 0x04) {
        do_interupt();
      }
      return;
    }
  }

#ifdef DEBUG_SERIAL
  if (isprint(saturn.tbr)) {
    LOGE( "-> \'%c\'\n", saturn.tbr);
  } else {
    LOGE( "-> %x\n", saturn.tbr);
  }
#endif
  if (saturn.ir_ctrl & 0x04) {
    if (write(ir_fd, &saturn.tbr, 1) == 1) {
      saturn.tcs &= 0x0e;
      if (saturn.io_ctrl & 0x04) {
        do_interupt();
      }
    } else {
      if (errno != EAGAIN) {
        LOGE( "%s: serial write error: %d\n", progname, errno);
      }
      saturn.tcs &= 0x0e;
      if (saturn.io_ctrl & 0x04) {
        do_interupt();
      }
    }
  } else {
    if (write(wire_fd, &saturn.tbr, 1) == 1) {
      saturn.tcs &= 0x0e;
      if (saturn.io_ctrl & 0x04) {
        do_interupt();
      }
    } else {
      if (errno != EAGAIN) {
        if (!quiet)
          LOGE( "%s: serial write error: %d\n", progname, errno);
      }
      saturn.tcs &= 0x0e;
      if (saturn.io_ctrl & 0x04) {
        do_interupt();
      }
    }
  }
}

#define NR_BUFFER 256

void
#ifdef __FunctionProto__
receive_char()
#else
receive_char()
#endif
{
  struct timeval tout;
  fd_set rfds;
  int nfd;
  static unsigned char buf[NR_BUFFER + 1];
  static int nrd = 0, bp = 0;

#ifdef DEBUG_SERIALx
  LOGE( "RCV %s\n", (saturn.ir_ctrl & 0x04) ? "IR" : "wire");
#endif

  rece_instr = 0;

  if (saturn.ir_ctrl & 0x04) {
    if (ir_fd == -1)
      return;
  } else {
    if (wire_fd == -1)
      return;
  }

  if (saturn.rcs & 0x01) {
    return;
  }

  if (nrd == 0) {
    tout.tv_sec = 0;
    tout.tv_usec = 0;
    FD_ZERO(&rfds);
    if (saturn.ir_ctrl & 0x04) {
      FD_SET(ir_fd, &rfds);
      nfd = ir_fd + 1;
    } else {
      FD_SET(wire_fd, &rfds);
      nfd = wire_fd + 1;
    }
    if ((nfd = select(nfd, &rfds, (fd_set *)0, (fd_set *)0, &tout)) > 0) {
#ifdef DEBUG_SERIAL
      LOGE( "select = %d\n", nfd);
#endif
      if (saturn.ir_ctrl & 0x04) {
        if (FD_ISSET(ir_fd, &rfds)) {
          nrd = read(ir_fd, buf, NR_BUFFER);
          if (nrd < 0) {
            nrd = 0;
            return;
          }
          bp = 0;
        } else {
          return;
        }
      } else {
        if (FD_ISSET(wire_fd, &rfds)) {
          nrd = read(wire_fd, buf, NR_BUFFER);
          if (nrd < 0) {
            nrd = 0;
            return;
          }
          bp = 0;
        } else {
          return;
        }
      }
    } else {
      return;
    }
  }
  if (nrd == 0) {
    return;
  }
  if (!(saturn.io_ctrl & 0x08)) {
    nrd = 0;
    return;
  }
  saturn.rbr = buf[bp++];
  nrd--;
  saturn.rcs |= 0x01;
  if (saturn.io_ctrl & 0x02) {
    do_interupt();
  }
}

