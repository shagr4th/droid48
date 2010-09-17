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

/* $Log: device.c,v $
 * Revision 1.8  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.7  1994/11/28  02:00:51  ecd
 * played around with sound stuff
 *
 * Revision 1.7  1994/11/28  02:00:51  ecd
 * played around with sound stuff
 *
 * Revision 1.6  1994/11/02  14:40:38  ecd
 * adapted to new timer 2 stuff from timer.c
 *
 * Revision 1.5  1994/10/05  08:36:44  ecd
 * removed addr queue
 *
 * Revision 1.4  1994/09/30  12:37:09  ecd
 * new and FASTER display handling
 *
 * Revision 1.3  1994/09/18  15:29:22  ecd
 * started Real Time support
 *
 * Revision 1.2  1994/09/13  16:57:00  ecd
 * changed to plain X11
 *
 * Revision 1.1  1994/08/26  11:09:02  ecd
 * Initial revision
 *
 *
 * $Id: device.c,v 1.8 1995/01/11 18:20:01 ecd Exp ecd $
 */

#include "global.h"


#include <stdlib.h>

#include "hp48.h"
#include "hp48_emu.h"
#include "device.h"
#include "timer.h"

extern int device_check;

device_t device;

void
#ifdef __FunctionProto__
check_devices(void)
#else
check_devices()
#endif
{
  if (device.display_touched > 0 && device.display_touched-- == 1) {
    device.display_touched = 0;
    update_display();
  }
  if (device.display_touched > 0) {
    device_check = 1;
  }
  if (device.contrast_touched) {
    device.contrast_touched = 0;
    adjust_contrast(display.contrast);
  }
  if (device.ann_touched) {
    device.ann_touched = 0;
    draw_annunc();
  }
  if (device.baud_touched) {
    device.baud_touched = 0;
    serial_baud(saturn.baud);
  }
  if (device.ioc_touched) {
    device.ioc_touched = 0;
    if ((saturn.io_ctrl & 0x02) && (saturn.rcs & 0x01)) {
      do_interupt();
    }
  }
  if (device.rbr_touched) {
    device.rbr_touched = 0;
    receive_char();
  }
  if (device.tbr_touched) {
    device.tbr_touched = 0;
    transmit_char();
  }
  if (device.t1_touched) {
    saturn.t1_instr = 0;
    sched_timer1 = saturn.t1_tick;
    restart_timer(T1_TIMER);
    set_t1 = saturn.timer1;
    device.t1_touched = 0;
  }
  if (device.t2_touched) {
    saturn.t2_instr = 0;
    sched_timer2 = saturn.t2_tick;
    device.t2_touched = 0;
  }
#if 0
  if (device.disp_test_touched) {
    device.disp_test_touched = 0;
  }
  if (device.crc_touched) {
    device.crc_touched = 0;
  }
  if (device.power_status_touched) {
    device.power_status_touched = 0;
  }
  if (device.power_ctrl_touched) {
    device.power_ctrl_touched = 0;
  }
  if (device.mode_touched) {
    device.mode_touched = 0;
  }
  if (device.card_ctrl_touched) {
    device.card_ctrl_touched = 0;
  }
  if (device.card_status_touched) {
    device.card_status_touched = 0;
  }
  if (device.tcs_touched) {
    device.tcs_touched = 0;
  }
  if (device.rcs_touched) {
    device.rcs_touched = 0;
  }
  if (device.sreq_touched) {
    device.sreq_touched = 0;
  }
  if (device.ir_ctrl_touched) {
    device.ir_ctrl_touched = 0;
  }
  if (device.base_off_touched) {
    device.base_off_touched = 0;
  }
  if (device.lcr_touched) {
    device.lcr_touched = 0;
  }
  if (device.lbr_touched) {
    device.lbr_touched = 0;
  }
  if (device.scratch_touched) {
    device.scratch_touched = 0;
  }
  if (device.base_nibble_touched) {
    device.base_nibble_touched = 0;
  }
  if (device.unknown_touched) {
    device.unknown_touched = 0;
  }
  if (device.t1_ctrl_touched) {
    device.t1_ctrl_touched = 0;
  }
  if (device.t2_ctrl_touched) {
    device.t2_ctrl_touched = 0;
  }
  if (device.unknown2_touched) {
    device.unknown2_touched = 0;
  }
#endif
}

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void
#ifdef __FunctionProto__
check_out_register(void)
#else
check_out_register()
#endif
{
  /*static int au = -2;
  unsigned char c[] = { 0xff, 0x00 };

  if (au == -2)
    if ((au = open("/dev/audio", O_WRONLY)) < 0)
  if (au < 0)
    return;
  if (saturn.OUT[2] & 0x8)
    write(au, c, 1);
  else
    write(au, &c[1], 1);*/
}

#endif
