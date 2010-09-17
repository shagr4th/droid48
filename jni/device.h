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

/* $Log: device.h,v $
 * Revision 1.7  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.6  1994/11/28  02:19:22  ecd
 * played with the out register
 *
 * Revision 1.6  1994/11/28  02:19:22  ecd
 * played with the out register
 *
 * Revision 1.5  1994/11/02  14:51:27  ecd
 * added some function declarations
 *
 * Revision 1.4  1994/10/05  08:33:22  ecd
 * remove addr queue
 *
 * Revision 1.3  1994/09/30  12:32:49  ecd
 * added DISP_INSTR_OFF for faster and better display
 *
 * Revision 1.2  1994/09/13  16:58:42  ecd
 * changed to plain X11
 *
 * Revision 1.1  1994/08/26  11:09:18  ecd
 * Initial revision
 *
 *
 *
 * $Id: device.h,v 1.7 1995/01/11 18:20:01 ecd Exp ecd $
 */

#ifndef _DEVICE_H
#define _DEVICE_H 1

#include "global.h"

#define DISP_INSTR_OFF   0x10

#define ANN_LEFT         0x81
#define ANN_RIGHT        0x82
#define ANN_ALPHA        0x84
#define ANN_BATTERY      0x88
#define ANN_BUSY         0x90
#define ANN_IO           0xa0

typedef struct device_t {

  int  display_touched;

  char contrast_touched;

  char disp_test_touched;
 
  char crc_touched;

  char power_status_touched;
  char power_ctrl_touched;

  char mode_touched;

  char ann_touched;
 
  char baud_touched;

  char card_ctrl_touched;
  char card_status_touched;

  char ioc_touched;

  char tcs_touched;
  char rcs_touched;

  char rbr_touched;
  char tbr_touched;

  char sreq_touched;

  char ir_ctrl_touched;

  char base_off_touched;

  char lcr_touched;
  char lbr_touched;

  char scratch_touched;
  char base_nibble_touched;
  
  char unknown_touched;

  char t1_ctrl_touched;
  char t2_ctrl_touched;

  char unknown2_touched;

  char t1_touched;
  char t2_touched;

} device_t;

extern device_t device;
extern void	check_devices      __ProtoType__((void));
#if 0
extern void	check_out_register __ProtoType__((void));
#endif

extern void     update_display     __ProtoType__((void));
extern void     redraw_display     __ProtoType__((void));
extern void 	disp_draw_nibble   __ProtoType__((word_20 addr, word_4 val));
extern void 	menu_draw_nibble   __ProtoType__((word_20 addr, word_4 val));
extern void     draw_annunc	   __ProtoType__((void));
extern void     redraw_annunc	   __ProtoType__((void));

#endif /* !_DEVICE_H */
