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

/* $Log: lcd.c,v $
 * Revision 1.13  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.12  1994/12/08  22:14:50  ecd
 * fixed bug with XShmPutImage causing errors in init_display
 *
 * Revision 1.11  1994/12/07  20:20:50  ecd
 * added support for icon colors
 *
 * Revision 1.10  1994/11/28  02:00:51  ecd
 * added support for colors on icon
 *
 * Revision 1.9  1994/11/02  14:44:28  ecd
 * minor fixes
 *
 * Revision 1.8  1994/10/09  20:32:02  ecd
 * implemented bit offset stuff.
 *
 * Revision 1.7  1994/10/06  16:30:05  ecd
 * added Shared Memory stuff
 *
 * Revision 1.6  1994/10/05  08:36:44  ecd
 * pixmaps for nibble updates
 *
 * Revision 1.5  1994/09/30  12:37:09  ecd
 * new display code makes x48 a lot faster
 *
 * Revision 1.4  1994/09/18  15:29:22  ecd
 * turned off unused rcsid message
 *
 * Revision 1.3  1994/09/13  16:57:00  ecd
 * changed to plain X11
 *
 * Revision 1.2  1994/08/31  18:23:21  ecd
 * changed display initialization.
 *
 * Revision 1.1  1994/08/26  11:09:02  ecd
 * Initial revision
 *
 * $Id: lcd.c,v 1.13 1995/01/11 18:20:01 ecd Exp ecd $
 */


#include "global.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#ifdef SUNOS
#include <memory.h>
#endif


#include "hp48.h"
#include "hp48_emu.h"
#include "annunc.h"
#include "device.h"

static int last_annunc_state = -1;

display_t display;

#define DISP_ROWS           64

#define NIBS_PER_BUFFER_ROW    (NIBBLES_PER_ROW + 2)

#define DISP_ROWS_AS_SHORT  (DISP_ROWS*2)

#define NIBS_PER_BUFFER_ROW_AS_SHORT 262 // (NIBBLES_PER_ROW * 8 )

#define BLANK_COLOR 0x842d

#define ANDROID_BUF_LENGTH (DISP_ROWS_AS_SHORT * NIBS_PER_BUFFER_ROW_AS_SHORT)

#define ANDROID_BUF_HEADER_LENGTH (14 * NIBS_PER_BUFFER_ROW_AS_SHORT)

unsigned char disp_buf[DISP_ROWS][NIBS_PER_BUFFER_ROW];
unsigned char lcd_buffer[DISP_ROWS][NIBS_PER_BUFFER_ROW];
unsigned short disp_buf_short [ANDROID_BUF_LENGTH]; // 64 * 2 * 262
unsigned short disp_buf_header_short [ANDROID_BUF_HEADER_LENGTH];
uint8_t ann_boolean [6];

Pixmap nibble_maps[16];

unsigned char nibbles[16][2] =
{
  { 0x00, 0x00 },	/* ---- */
  { 0x03, 0x03 },	/* *--- */
  { 0x0c, 0x0c },	/* -*-- */
  { 0x0f, 0x0f },	/* **-- */
  { 0x30, 0x30 },	/* --*- */
  { 0x33, 0x33 },	/* *-*- */
  { 0x3c, 0x3c },	/* -**- */
  { 0x3f, 0x3f },	/* ***- */
  { 0xc0, 0xc0 },	/* ---* */
  { 0xc3, 0xc3 },	/* *--* */
  { 0xcc, 0xcc },	/* -*-* */
  { 0xcf, 0xcf },	/* **-* */
  { 0xf0, 0xf0 },	/* --** */
  { 0xf3, 0xf3 },	/* *-** */
  { 0xfc, 0xfc },	/* -*** */
  { 0xff, 0xff }	/* **** */
};

/*{ 0xffff, 0xffff, 0xffff, 0xffff },    
  { 0x0000, 0xffff, 0xffff, 0xffff },    
  { 0xffff, 0x0000, 0xffff, 0xffff },    
  { 0x0000, 0x0000, 0xffff, 0xffff },    
  { 0xffff, 0xffff, 0x0000, 0xffff },    
  { 0x0000, 0xffff, 0x0000, 0xffff },    
  { 0xffff, 0x0000, 0x0000, 0xffff },    
  { 0x0000, 0x0000, 0x0000, 0xffff },    
  { 0xffff, 0xffff, 0xffff, 0x0000 },    
  { 0x0000, 0xffff, 0xffff, 0x0000 },    
  { 0xffff, 0x0000, 0xffff, 0x0000 },    
  { 0x0000, 0x0000, 0xffff, 0x0000 },    
  { 0xffff, 0xffff, 0x0000, 0x0000 },    
  { 0x0000, 0xffff, 0x0000, 0x0000 },    
  { 0xffff, 0x0000, 0x0000, 0x0000 },    
  { 0x0000, 0x0000, 0x0000, 0x0000 }    
  */
unsigned short nibbles_short [16][8] =
{
  { BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR },    /* ---- */
  { 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR },    /* *--- */
  { BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR },    /* -*-- */
  { 0x0000, 0x0000, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR },    /* **-- */
  { BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR },    /* --*- */
  { 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR },    /* *-*- */
  { BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR },    /* -**- */
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR },    /* ***- */
  { BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000 },    /* ---* */
  { 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000 },    /* *--* */
  { BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000 },    /* -*-* */
  { 0x0000, 0x0000, 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000 },    /* **-* */
  { BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, 0x0000, 0x0000 },    /* --** */
  { 0x0000, 0x0000, BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, 0x0000, 0x0000 },    /* *-** */
  { BLANK_COLOR, BLANK_COLOR, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },    /* -*** */
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }    /* **** */
};

static unsigned char nibble_bits[16];

jshortArray array;
jbooleanArray ann_array;

int flipable = 0;

void
Java_org_ab_x48_X48_setBlankColor( JNIEnv*  env,
                                      jobject  this, jshort color) {
	int i, j;
	for(i=0;i<ANDROID_BUF_HEADER_LENGTH;i++)
		disp_buf_header_short[i] = color;
	for( i=0;i<16;i++)
		for( j=0;j<8;j++)
			if (nibbles_short[i][j] > 0)
			{
				nibbles_short[i][j] = color;
			}
	for(i=0;i<ANDROID_BUF_LENGTH;i++)
		if (disp_buf_short[i] > 0)
		{
				disp_buf_short[i] = color;
		}

	
	flipable = 1;
}

void
Java_org_ab_x48_X48_flipScreen( JNIEnv*  env,
                                      jobject  this) {

	flipable = 1;
}



jint
Java_org_ab_x48_X48_fillScreenData( JNIEnv*  env,
                                      jobject  this, jshortArray array) {
										 
	if (flipable == 1)
	{
		
		(*env)->SetShortArrayRegion(env, array, 0, ANDROID_BUF_HEADER_LENGTH, disp_buf_header_short);
		(*env)->SetShortArrayRegion(env, array, ANDROID_BUF_HEADER_LENGTH, ANDROID_BUF_LENGTH, disp_buf_short);
		flipable = 0;
		return 1;
	}
	return 0;
}
	
void
#ifdef __FunctionProto__
android_refresh_screen(void)
#else
android_refresh_screen()
#endif
{
	return;
	if (flipable == 1 && android_env)
	{

		if (!array)
		{
			array = (*android_env)->NewShortArray(android_env, ANDROID_BUF_LENGTH);
		}
		if(array) {
			(*android_env)->SetShortArrayRegion(android_env, array, 0, ANDROID_BUF_LENGTH, disp_buf_short);
		}
		(*android_env)->CallVoidMethod(android_env, android_callback, refreshMainScreen, array);
		flipable = 0;
	}
	return;
}

void
#ifdef __FunctionProto__
fill_disp_buf_short(int x, int y, int v)
#else
fill_disp_buf_short(x, y, v)
int x;
int y;
int v;
#endif
{
    int xx = x*8;
    int yy = y*2;
    if (xx >= NIBS_PER_BUFFER_ROW_AS_SHORT || yy >= DISP_ROWS_AS_SHORT)
    {
        //LOGE("error in location: %d:%d", x, y);
		return;
    }
	if (xx == NIBS_PER_BUFFER_ROW_AS_SHORT -6)
	{
		memcpy(&disp_buf_short[NIBS_PER_BUFFER_ROW_AS_SHORT * yy + xx], &nibbles_short[v][0], 12);
		memcpy(&disp_buf_short[NIBS_PER_BUFFER_ROW_AS_SHORT * (yy + 1) + xx], &nibbles_short[v][0], 12);
	} else {
	    memcpy(&disp_buf_short[NIBS_PER_BUFFER_ROW_AS_SHORT * yy + xx], &nibbles_short[v][0], 16);
		memcpy(&disp_buf_short[NIBS_PER_BUFFER_ROW_AS_SHORT * (yy + 1) + xx], &nibbles_short[v][0], 16);
	}
	flipable = 1;
}

void
#ifdef __FunctionProto__
init_nibble_maps(void)
#else
init_nibble_maps()
#endif
{
  int i;

  for (i = 0; i < 16; i++) {
    nibble_maps[i] = XCreateBitmapFromData(dpy, disp.win,
                                           (char *)nibbles[i], 8, 2);
  }
#ifdef HAVE_XSHM
  if (0) {
   
  }
#endif
}

void
#ifdef __FunctionProto__
init_display(void)
#else
init_display()
#endif
{
	disp.mapped = 1;
  display.on = (int)(saturn.disp_io & 0x8) >> 3;

  display.disp_start = (saturn.disp_addr & 0xffffe);
  display.offset = (saturn.disp_io & 0x7);
  disp.offset = 2 * display.offset;

  display.lines = (saturn.line_count & 0x3f);
  if (display.lines == 0)
    display.lines = 63;
  disp.lines = 2 * display.lines;
  if (disp.lines < 110)
    disp.lines = 110;

  if (display.offset > 3)
    display.nibs_per_line = (NIBBLES_PER_ROW+saturn.line_offset+2) & 0xfff;
  else
    display.nibs_per_line = (NIBBLES_PER_ROW+saturn.line_offset) & 0xfff;

  display.disp_end = display.disp_start +
	             (display.nibs_per_line * (display.lines + 1));

  display.menu_start = saturn.menu_addr;
  display.menu_end = saturn.menu_addr + 0x110;

  display.contrast = saturn.contrast_ctrl;
  display.contrast |= ((saturn.disp_test & 0x1) << 4);

  display.annunc = saturn.annunc;

  memset(disp_buf, 0xf0, sizeof(disp_buf));
  memset(lcd_buffer, 0xf0, sizeof(lcd_buffer));

	
  init_nibble_maps();
}

static inline void
#ifdef __FunctionProto__
draw_nibble(int c, int r, int val)
#else
draw_nibble(c, r, val)
int c;
int r;
int val;
#endif
{
  int x, y;

  x = (c * 8) + 5;
  if (r <= display.lines)
    x -= disp.offset;
  y = (r * 2) + 20;
  val &= 0x0f;
  if (val != lcd_buffer[r][c]) {
    //XCopyPlane(dpy, nibble_maps[val], disp.win, disp.gc, 0, 0, 8, 2, x, y, 1);
    lcd_buffer[r][c] = val;
	fill_disp_buf_short(c, r, val);
	
  }
}

static inline void
#ifdef __FunctionProto__
draw_row(long addr, int row)
#else
draw_row(addr, row)
long addr;
int row;
#endif
{
  int i, v;
  int line_length;

  line_length = NIBBLES_PER_ROW;
  if ((display.offset > 3) && (row <= display.lines))
    line_length += 2;
  for (i = 0; i < line_length; i++) {
    v = read_nibble(addr + i);
    if (v != disp_buf[row][i]) {
      disp_buf[row][i] = v;
      draw_nibble(i, row, v);
    }
  }
}

void
#ifdef __FunctionProto__
update_display(void)
#else
update_display()
#endif
{
  int i, j;
  long addr;
  static int old_offset = -1;
  static int old_lines = -1;
#ifdef HAVE_XSHM
  int addr_pad;
  int val, line_pad, line_length;
  word_20 data_addr, data_addr_2;
#endif

 /* if (!disp.mapped)
    {
      refresh_icon();
      return;
    }*/
  if (display.on) {
    addr = display.disp_start;
#ifdef HAVE_XSHM
    if (0) {
     
    } else {
#endif
      if (display.offset != old_offset) {
        memset(disp_buf, 0xf0,
               (size_t)((display.lines+1) * NIBS_PER_BUFFER_ROW));
        memset(lcd_buffer, 0xf0,
               (size_t)((display.lines+1) * NIBS_PER_BUFFER_ROW));
        old_offset = display.offset;
      }
      if (display.lines != old_lines) {
        memset(&disp_buf[56][0], 0xf0, (size_t)(8 * NIBS_PER_BUFFER_ROW));
        memset(&lcd_buffer[56][0], 0xf0, (size_t)(8 * NIBS_PER_BUFFER_ROW));
        old_lines = display.lines;
      }
      for (i = 0; i <= display.lines; i++) {
        draw_row(addr, i);
        addr += display.nibs_per_line;
      }
#ifdef HAVE_XSHM
    }
#endif
    if (i < DISP_ROWS) {
      addr = display.menu_start;
#ifdef HAVE_XSHM
      if (0) {
        
      } else {
#endif
        for (; i < DISP_ROWS; i++) {
          draw_row(addr, i);
          addr += NIBBLES_PER_ROW;
        }
#ifdef HAVE_XSHM
      }
#endif
    }
  } else {
#ifdef HAVE_XSHM
    if (0) {
     
    } else {
#endif
      memset(disp_buf, 0xf0, sizeof(disp_buf));
      for (i = 0; i < 64; i++) {
        for (j = 0; j < NIBBLES_PER_ROW; j++) {
          draw_nibble(j, i, 0x00);
        }
      }
#ifdef HAVE_XSHM
    }
#endif
  }
//android_refresh_screen();
}

void
#ifdef __FunctionProto__
redraw_display(void)
#else
redraw_display()
#endif
{
  XClearWindow(dpy, disp.win);
  memset(disp_buf, 0, sizeof(disp_buf));
  memset(lcd_buffer, 0, sizeof(lcd_buffer));
  update_display();
}

void
#ifdef __FunctionProto__
disp_draw_nibble(word_20 addr, word_4 val)
#else
disp_draw_nibble(addr, val)
word_20 addr;
word_4 val;
#endif
{
  long offset;
#ifdef HAVE_XSHM
  int shm_addr;
#endif
  int x, y;
int ok = 0;

  offset = (addr - display.disp_start);
  x = offset % display.nibs_per_line;
  if (x < 0 || x > 35)
    return;
  if (display.nibs_per_line != 0) {
    y = offset / display.nibs_per_line;
    if (y < 0 || y > 63)
      return;
#ifdef HAVE_XSHM
    if (0) {
      
    } else {
#endif
      if (val != disp_buf[y][x]) {
	ok = 1;
        disp_buf[y][x] = val;
        draw_nibble(x, y, val);
      }
#ifdef HAVE_XSHM
    }
#endif
  } else {
#ifdef HAVE_XSHM
    if (0) {
      
    } else {
#endif
      for (y = 0; y < display.lines; y++) {
        if (val != disp_buf[y][x]) {
			ok = 1;
          disp_buf[y][x] = val;
          draw_nibble(x, y, val);
        }
      }
#ifdef HAVE_XSHM
    }
#endif
  }


}

void
#ifdef __FunctionProto__
menu_draw_nibble(word_20 addr, word_4 val)
#else
menu_draw_nibble(addr, val)
word_20 addr;
word_4 val;
#endif
{
  long offset;
#ifdef HAVE_XSHM
  int shm_addr;
#endif
  int x, y;

  offset = (addr - display.menu_start);
#ifdef HAVE_XSHM
  if (0) {
  
  } else {
#endif
    x = offset % NIBBLES_PER_ROW;
    y = display.lines + (offset / NIBBLES_PER_ROW) + 1;
    if (val != disp_buf[y][x]) {
      disp_buf[y][x] = val;
      draw_nibble(x, y, val);
    }
#ifdef HAVE_XSHM
  }
#endif
	
}


struct ann_struct {
  int            bit;
  int            x;
  int            y;
  unsigned int   width;
  unsigned int   height;
  unsigned char *bits;
  Pixmap         pixmap;
} ann_tbl[] = {
  { ANN_LEFT, 16, 4, ann_left_width, ann_left_height, ann_left_bits },
  { ANN_RIGHT, 61, 4, ann_right_width, ann_right_height, ann_right_bits },
  { ANN_ALPHA, 106, 4, ann_alpha_width, ann_alpha_height, ann_alpha_bits },
  { ANN_BATTERY, 151, 4, ann_battery_width, ann_battery_height,
                         ann_battery_bits },
  { ANN_BUSY, 196, 4, ann_busy_width, ann_busy_height, ann_busy_bits },
  { ANN_IO, 241, 4, ann_io_width, ann_io_height, ann_io_bits },
  { 0 }
};

void
#ifdef __FunctionProto__
draw_annunc(void)
#else
draw_annunc()
#endif
{
  int val;
  int i;

  val = display.annunc;

  if (val == last_annunc_state)
    return;
  last_annunc_state = val;
  /*for (i = 0; ann_tbl[i].bit; i++)
    {
      if ((ann_tbl[i].bit & val) == ann_tbl[i].bit)
        {
          XCopyPlane(dpy, ann_tbl[i].pixmap, disp.win, disp.gc, 0, 0,
                     ann_tbl[i].width, ann_tbl[i].height,
                     ann_tbl[i].x, ann_tbl[i].y, 1);
        }
      else
        {
          XClearArea(dpy, disp.win, ann_tbl[i].x, ann_tbl[i].y,
                     ann_tbl[i].width, ann_tbl[i].height, 0);
        }
    }
  refresh_icon();*/


  if (!ann_array)
	{
		ann_array = (*android_env)->NewBooleanArray(android_env, 6);
	}
	if(ann_array) {
		for (i = 0; ann_tbl[i].bit; i++) {
			ann_boolean[i] = ((ann_tbl[i].bit & val) == ann_tbl[i].bit);
		}
			(*android_env)->SetBooleanArrayRegion(android_env, ann_array, 0, 6, ann_boolean);
			(*android_env)->CallVoidMethod(android_env, android_callback, refreshIcons, ann_array);
	}
	flipable = 1;
	
}

void
#ifdef __FunctionProto__
redraw_annunc(void)
#else
redraw_annunc()
#endif
{
  last_annunc_state = -1;
  draw_annunc();
}

void
#ifdef __FunctionProto__
init_annunc(void)
#else
init_annunc()
#endif
{
  int i;
 
  for (i = 0; ann_tbl[i].bit; i++) {
    ann_tbl[i].pixmap = XCreateBitmapFromData(dpy, disp.win,
                                              (char *)ann_tbl[i].bits,
                                	      ann_tbl[i].width,
                                              ann_tbl[i].height);
  }
}

