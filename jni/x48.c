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

/* $Log: x48_x11.c,v $
 * Revision 1.13  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.12  1994/12/08  22:17:24  ecd
 * display and menu images now correctly drawn according to disp.lines
 *
 * Revision 1.11  1994/12/07  20:20:50  ecd
 * better handling of resources
 *
 * Revision 1.10  1994/11/28  02:00:51  ecd
 * implemented WM_SAVE_YOURSELF protocol.
 * added support for mono and gray in color_t.
 * added support for all possible Visualclasses.
 * changed handling of KeyPress and KeyRelease.
 * added color icon stuff.
 * added support for contrast changes (ON_-, ON_+)
 * read in all those Xresources before running off.
 * use own icon window, no name-decor on icon.
 * show state of x48 in the icon's display
 * added support for setting the window title with the connections.
 *
 * Revision 1.9  1994/11/04  03:42:34  ecd
 * changed includes
 *
 * Revision 1.8  1994/11/02  14:44:28  ecd
 * works on machines that don't support backing store
 *
 * Revision 1.7  1994/10/09  20:32:02  ecd
 * changed refresh_display to support bit offset.
 *
 * Revision 1.6  1994/10/06  16:30:05  ecd
 * added XShm - Extension stuff
 *
 * Revision 1.5  1994/10/05  08:36:44  ecd
 * added backing_store = Always for subwindows
 *
 * Revision 1.4  1994/09/30  12:37:09  ecd
 * added support for interrupt detection in GetEvent,
 * faster display updates,
 * update display window only when mapped.
 *
 * Revision 1.3  1994/09/18  22:47:20  ecd
 * added version information
 *
 * Revision 1.2  1994/09/18  15:29:22  ecd
 * started Real Time support
 *
 * Revision 1.1  1994/09/13  15:05:05  ecd
 * Initial revision
 *
 * $Id: x48_x11.c,v 1.13 1995/01/11 18:20:01 ecd Exp ecd $
 */


#include "global.h"
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#ifdef SYSV
#include <sys/utsname.h>
#endif
#ifdef SUNOS
#include <memory.h>
#endif


#include "x48.h"
#include "icon.h"
#include "small.h"
#include "buttons.h"

#include "hp48.h"
#include "device.h"
#include "constants.h"
#include "romio.h"


extern int      saved_argc;
extern char   **saved_argv;

Display	       *dpy;
int		screen;
unsigned int	depth;
Colormap	cmap;
GC		gc;
Window		mainW;
Window		iconW;
disp_t		disp;
Atom		wm_delete_window, wm_save_yourself, wm_protocols;
Atom		ol_decor_del, ol_decor_icon_name;
Atom		atom_type;
Visual         *visual;
Pixmap		icon_pix;
Pixmap		icon_text_pix;
Pixmap		icon_disp_pix;
static int      last_icon_state = -1;

JNIEnv *android_env;
jobject android_callback;
jmethodID waitEvent;

int	 dynamic_color;
int	 direct_color;
int	 does_backing_store;
int	 color_mode;
int	 icon_color_mode;

#if 0
# define DEBUG_XEVENT 1
# define DEBUG_BUTTONS 1
# define DEBUG_FOCUS 1
# define DEBUG_BACKING_STORE 1
# define DEBUG_SHM 1
#endif



typedef struct button_t {

  char		*name;
  short		pressed;
  short		extra;

  int		code;
  int		x, y;
  unsigned int	w, h;

  int		lc;
  char		*label;
  short		font_size;
  unsigned int	lw, lh;
  unsigned char *lb;

  char		*letter;

  char		*left;
  short		is_menu;
  char		*right;
  char		*sub;

  Pixmap	map;
  Pixmap	down;
  Window	xwin;

} button_t;

#define BUTTON_A	0
#define BUTTON_B	1
#define BUTTON_C	2
#define BUTTON_D	3
#define BUTTON_E	4
#define BUTTON_F	5

#define BUTTON_MTH	6
#define BUTTON_PRG	7
#define BUTTON_CST	8
#define BUTTON_VAR	9
#define BUTTON_UP	10
#define BUTTON_NXT	11

#define BUTTON_COLON	12
#define BUTTON_STO	13
#define BUTTON_EVAL	14
#define BUTTON_LEFT	15
#define BUTTON_DOWN	16
#define BUTTON_RIGHT	17

#define BUTTON_SIN	18
#define BUTTON_COS	19
#define BUTTON_TAN	20
#define BUTTON_SQRT	21
#define BUTTON_POWER	22
#define BUTTON_INV	23

#define BUTTON_ENTER	24
#define BUTTON_NEG	25
#define BUTTON_EEX	26
#define BUTTON_DEL	27
#define BUTTON_BS	28

#define BUTTON_ALPHA	29
#define BUTTON_7	30
#define BUTTON_8	31
#define BUTTON_9	32
#define BUTTON_DIV	33

#define BUTTON_SHL	34
#define BUTTON_4	35
#define BUTTON_5	36
#define BUTTON_6	37
#define BUTTON_MUL	38

#define BUTTON_SHR	39
#define BUTTON_1	40
#define BUTTON_2	41
#define BUTTON_3	42
#define BUTTON_MINUS	43

#define BUTTON_ON	44
#define BUTTON_0	45
#define BUTTON_PERIOD	46
#define BUTTON_SPC	47
#define BUTTON_PLUS	48

#define LAST_BUTTON	48

button_t buttons[] = {
  { "A", 0, 0, 0x14,  0, 0, 36, 23, WHITE,
     0, 0, menu_label_width, menu_label_height, menu_label_bits,
    "A", 0, 0, 0, 0, 0 },
  { "B", 0, 0, 0x84,  50, 0, 36, 23, WHITE,
     0, 0, menu_label_width, menu_label_height, menu_label_bits,
    "B", 0, 0, 0, 0, 0 },
  { "C", 0, 0, 0x83, 100, 0, 36, 23, WHITE,
     0, 0, menu_label_width, menu_label_height, menu_label_bits,
    "C", 0, 0, 0, 0, 0 },
  { "D", 0, 0, 0x82, 150, 0, 36, 23, WHITE,
     0, 0, menu_label_width, menu_label_height, menu_label_bits,
    "D", 0, 0, 0, 0, 0 },
  { "E", 0, 0, 0x81, 200, 0, 36, 23, WHITE,
     0, 0, menu_label_width, menu_label_height, menu_label_bits,
    "E", 0, 0, 0, 0, 0 },
  { "F", 0, 0, 0x80, 250, 0, 36, 23, WHITE,
     0, 0, menu_label_width, menu_label_height, menu_label_bits,
    "F", 0, 0, 0, 0, 0 },

  { "MTH", 0, 0, 0x24,  0, 50, 36, 26, WHITE, "MTH", 0, 0, 0, 0,
    "G", "RAD", 0, "POLAR", 0, 0 },
  { "PRG", 0, 0, 0x74,  50, 50, 36, 26, WHITE, "PRG", 0, 0, 0, 0,
    "H", 0, 0, "CHARS", 0, 0 },
  { "CST", 0, 0, 0x73, 100, 50, 36, 26, WHITE, "CST", 0, 0, 0, 0,
    "I", 0, 0, "MODES", 0, 0 },
  { "VAR", 0, 0, 0x72, 150, 50, 36, 26, WHITE, "VAR", 0, 0, 0, 0,
    "J", 0, 0, "MEMORY", 0, 0 },
  { "UP", 0, 0, 0x71, 200, 50, 36, 26, WHITE,
     0, 0, up_width, up_height, up_bits,
    "K", 0, 0, "STACK", 0, 0 },
  { "NXT", 0, 0, 0x70, 250, 50, 36, 26, WHITE, "NXT", 0, 0, 0, 0,
    "L", "PREV", 0, "MENU", 0, 0 },

  { "COLON", 0, 0, 0x04,  0, 100, 36, 26, WHITE,
     0, 0, colon_width, colon_height, colon_bits,
    "M", "UP", 0, "HOME", 0, 0 },
  { "STO", 0, 0, 0x64,  50, 100, 36, 26, WHITE, "STO", 0, 0, 0, 0,
    "N", "DEF", 0, "RCL", 0, 0 },
  { "EVAL", 0, 0, 0x63, 100, 100, 36, 26, WHITE, "EVAL", 0, 0, 0, 0,
    "O", "aNUM", 0, "UNDO", 0, 0 },
  { "LEFT", 0, 0, 0x62, 150, 100, 36, 26, WHITE,
     0, 0, left_width, left_height, left_bits,
    "P", "PICTURE", 0, 0, 0, 0 },
  { "DOWN", 0, 0, 0x61, 200, 100, 36, 26, WHITE,
     0, 0, down_width, down_height, down_bits,
    "Q", "VIEW", 0, 0, 0, 0 },
  { "RIGHT", 0, 0, 0x60, 250, 100, 36, 26, WHITE,
     0, 0, right_width, right_height, right_bits,
    "R", "SWAP", 0, 0, 0, 0 },

  { "SIN", 0, 0, 0x34,  0, 150, 36, 26, WHITE, "SIN", 0, 0, 0, 0,
    "S", "ASIN", 0, "b", 0, 0 },
  { "COS", 0, 0, 0x54,  50, 150, 36, 26, WHITE, "COS", 0, 0, 0, 0,
    "T", "ACOS", 0, "c", 0, 0 },
  { "TAN", 0, 0, 0x53,  100, 150, 36, 26, WHITE, "TAN", 0, 0, 0, 0,
    "U", "ATAN", 0, "d", 0, 0 },
  { "SQRT", 0, 0, 0x52,  150, 150, 36, 26, WHITE,
     0, 0, sqrt_width, sqrt_height, sqrt_bits,
    "V", "n", 0, "o", 0, 0 },
  { "POWER", 0, 0, 0x51,  200, 150, 36, 26, WHITE,
     0, 0, power_width, power_height, power_bits,
    "W", "p", 0, "LOG", 0, 0 },
  { "INV", 0, 0, 0x50,  250, 150, 36, 26, WHITE,
     0, 0, inv_width, inv_height, inv_bits,
    "X", "q", 0, "LN", 0, 0 },

  { "ENTER", 0, 0, 0x44, 0, 200, 86, 26, WHITE, "ENTER", 2, 0, 0, 0,
     0, "EQUATION", 0, "MATRIX", 0, 0 },
  { "NEG", 0, 0, 0x43, 100, 200, 36, 26, WHITE,
     0, 0, neg_width, neg_height, neg_bits,
    "Y", "EDIT", 0, "CMD", 0, 0 },
  { "EEX", 0, 0, 0x42, 150, 200, 36, 26, WHITE, "EEX", 0, 0, 0, 0,
    "Z", "PURG", 0, "ARG", 0, 0 },
  { "DEL", 0, 0, 0x41, 200, 200, 36, 26, WHITE, "DEL", 0, 0, 0, 0,
     0, "CLEAR", 0, 0, 0, 0 },
  { "BS", 0, 0, 0x40, 250, 200, 36, 26, WHITE,
     0, 0, bs_width, bs_height, bs_bits,
     0, "DROP", 0, 0, 0, 0 },

  { "ALPHA", 0, 0, 0x35, 0, 250, 36, 26, WHITE,
     0, 0, alpha_width, alpha_height, alpha_bits,
     0, "USER", 0, "ENTRY", 0, 0 },
  { "7", 0, 0, 0x33, 60, 250, 46, 26, WHITE, "7", 1, 0, 0, 0,
     0, 0, 1, "SOLVE", 0, 0 },
  { "8", 0, 0, 0x32, 120, 250, 46, 26, WHITE, "8", 1, 0, 0, 0,
     0, 0, 1, "PLOT", 0, 0 },
  { "9", 0, 0, 0x31, 180, 250, 46, 26, WHITE, "9", 1, 0, 0, 0,
     0, 0, 1, "SYMBOLIC", 0, 0 },
  { "DIV", 0, 0, 0x30, 240, 250, 46, 26, WHITE,
     0, 0, div_width, div_height, div_bits,
     0, "r ", 0, "s", 0, 0 },

  { "SHL", 0, 0, 0x25, 0, 300, 36, 26, LEFT,
     0, 0, shl_width, shl_height, shl_bits,
     0, 0, 0, 0, 0, 0 },
  { "4", 0, 0, 0x23, 60, 300, 46, 26, WHITE, "4", 1, 0, 0, 0,
     0, 0, 1, "TIME", 0, 0 },
  { "5", 0, 0, 0x22, 120, 300, 46, 26, WHITE, "5", 1, 0, 0, 0,
     0, 0, 1, "STAT", 0, 0 },
  { "6", 0, 0, 0x21, 180, 300, 46, 26, WHITE, "6", 1, 0, 0, 0,
     0, 0, 1, "UNITS", 0, 0 },
  { "MUL", 0, 0, 0x20, 240, 300, 46, 26, WHITE,
     0, 0, mul_width, mul_height, mul_bits,
     0, "t ", 0, "u", 0, 0 },

  { "SHR", 0, 0, 0x15, 0, 350, 36, 26, RIGHT,
     0, 0, shr_width, shr_height, shr_bits,
     0, 0, 1, " ", 0, 0 },
  { "1", 0, 0, 0x13, 60, 350, 46, 26, WHITE, "1", 1, 0, 0, 0,
     0, 0, 1, "I/O", 0, 0 },
  { "2", 0, 0, 0x12, 120, 350, 46, 26, WHITE, "2", 1, 0, 0, 0,
     0, 0, 1, "LIBRARY", 0, 0 },
  { "3", 0, 0, 0x11, 180, 350, 46, 26, WHITE, "3", 1, 0, 0, 0,
     0, 0, 1, "EQ LIB", 0, 0 },
  { "MINUS", 0, 0, 0x10, 240, 350, 46, 26, WHITE,
     0, 0, minus_width, minus_height, minus_bits,
     0, "v ", 0, "w", 0, 0 },

  { "ON", 0, 0, 0x8000, 0, 400, 36, 26, WHITE, "ON", 0, 0, 0, 0,
     0, "CONT", 0, "OFF", "CANCEL", 0 },
  { "0", 0, 0, 0x03, 60, 400, 46, 26, WHITE, "0", 1, 0, 0, 0,
     0, "\004 ", 0, "\003", 0, 0 },
  { "PERIOD", 0, 0, 0x02, 120, 400, 46, 26, WHITE, ".", 1, 0, 0, 0,
     0, "\002 ", 0, "\001", 0, 0 },
  { "SPC", 0, 0, 0x01, 180, 400, 46, 26, WHITE, "SPC", 0, 0, 0, 0,
     0, "\005 ", 0, "z", 0, 0 },
  { "PLUS", 0, 0, 0x00, 240, 400, 46, 26, WHITE,
     0, 0, plus_width, plus_height, plus_bits,
     0, "x ", 0, "y", 0, 0 },

  { 0 }
};


int
#ifdef __FunctionProto__
button_pressed(int b)
#else
button_pressed(b)
int     b;
#endif
{
  int code;
  int i, r, c;

  code = buttons[b].code;
  buttons[b].pressed = 1;
  if (code == 0x8000) {
    for (i = 0; i < 9; i++)
      saturn.keybuf.rows[i] |= 0x8000;
    do_kbd_int();
  } else {
    r = code >> 4;
    c = 1 << (code & 0xf);
    if ((saturn.keybuf.rows[r] & c) == 0) {
      if (saturn.kbd_ien) {
        do_kbd_int();
      }
      if ((saturn.keybuf.rows[r] & c)) {
LOGE( "bug\n");
      }
      saturn.keybuf.rows[r] |= c;
    }
  }
#ifdef DEBUG_BUTTONS
  LOGE( "Button pressed  %d (%s)\n",
          buttons[b].code, buttons[b].name);
#endif
  return 0;
}

int
#ifdef __FunctionProto__
button_released(int b)
#else
button_released(b)
int     b;
#endif
{
  int code;

  code = buttons[b].code;
  buttons[b].pressed = 0;
  if (code == 0x8000) {
    int i;
    for (i = 0; i < 9; i++)
      saturn.keybuf.rows[i] &= ~0x8000;
  } else {
    int r, c;
    r = code >> 4;
    c = 1 << (code & 0xf);
    saturn.keybuf.rows[r] &= ~c;
  }
#ifdef DEBUG_BUTTONS
  LOGE( "Button released  %d (%s)\n",
          buttons[b].code, buttons[b].name);
#endif
  return 0;
}

static
int
#ifdef __FunctionProto__
button_release_all(void)
#else
button_release_all()
#endif
{
  int code;
  int b;

#ifdef DEBUG_BUTTONS
  LOGE( "Buttons released ");
#endif
  for (b = BUTTON_A; b <= LAST_BUTTON; b++)
    {
      if (buttons[b].pressed)
        {
#ifdef DEBUG_BUTTONS
  LOGE( "%d (%s) ",
          buttons[b].code, buttons[b].name);
#endif
	code = buttons[b].code;
        if (code == 0x8000) {
	  int i;
          for (i = 0; i < 9; i++)
            saturn.keybuf.rows[i] &= ~0x8000;
        } else {
	  int r, c;
          r = code >> 4;
          c = 1 << (code & 0xf);
          saturn.keybuf.rows[r] &= ~c;
        }
        buttons[b].pressed = 0;
      //  DrawButton(b);
      }
    }
#ifdef DEBUG_BUTTONS
  LOGE( "\n");
#endif
  return 0;
}

int
#ifdef __FunctionProto__
key_event(int b, int keypressed)
#else
key_event(b, keypressed)
int b;
int keypressed;
#endif
{
  int code;
  int i, r, c;
  int all_up;

  code = buttons[b].code;
  if (keypressed == 1) {
    buttons[b].pressed = 1;
 //   DrawButton(b);
    if (code == 0x8000) {
      for (i = 0; i < 9; i++)
        saturn.keybuf.rows[i] |= 0x8000;
      do_kbd_int();
    } else {
      r = code >> 4;
      c = 1 << (code & 0xf);
      if ((saturn.keybuf.rows[r] & c) == 0) {
        if (saturn.kbd_ien) {
          do_kbd_int();
        }
        saturn.keybuf.rows[r] |= c;
      }
    }
#ifdef DEBUG_BUTTONS
    LOGE( "Key pressed  %d (%s) %x\n",
            buttons[b].code, buttons[b].name), c;
#endif
  } else {
          if (code == 0x8000) {
            for (i = 0; i < 9; i++)
              saturn.keybuf.rows[i] &= ~0x8000;
	    memset (&saturn.keybuf, 0, sizeof (saturn.keybuf));
          } else {
            r = code >> 4;
            c = 1 << (code & 0xf);
            saturn.keybuf.rows[r] &= ~c;
          }
          buttons[b].pressed = 0;
        //  DrawButton(b);
#ifdef DEBUG_BUTTONS
    LOGE( "Key released %d (%s)\n",
            buttons[b].code, buttons[b].name);
#endif
  }
  return 0;
}


/* TODO */


Pixmap
#ifdef __FunctionProto__
XCreateBitmapFromData(Display *dpy, Window win, char* data, int width, int height)
#else
XCreateBitmapFromData(dpy, win, data, width, height)
Display *dpy;
Window win;
char* data;
int width;
int height;
#endif
{
	Pixmap p = { data, width, height };
	return p;
}

void
#ifdef __FunctionProto__
XClearArea(Display *dpy, Window win, int x, int y, int width, int height, int boo)
#else
XClearArea(dpy, win, x, y, width, height, boo)
Display *dpy;
Window win;
int x;
int y;
int width;
int height;
int boo;
#endif
{
	//LOGI("ClearArea: %d %d %d %d\n", x, y, width, height);
	return;
}

void
#ifdef __FunctionProto__
XCopyPlane(Display *dpy, Pixmap map, Window win, GC gc, int a, int b, int width, int height, int x, int y, int boo)
#else
XCopyPlane(dpy, map, win, gc, a, b, width, height,  x, y,boo)
Display *dpy;
Pixmap map;
Window win;
GC gc;
int a;
int b;
int width;
int height;
int x;
int y;
int boo;
#endif
{
	//LOGI("CopyArea: %d %d %d %d %s\n", x, y, width, height, map.data);
	return;
}

void
#ifdef __FunctionProto__
XClearWindow(Display *dpy, Window win)
#else
XClearWindow(dpy, win)
Display *dpy;
Window win;
#endif
{
	LOGI("XClearWindow");
	return;
}

void
#ifdef __FunctionProto__
refresh_display(void)
#else
refresh_display()
#endif
{
	LOGI("refresh_display");
	return;
}


int
#ifdef __FunctionProto__
GetEvent(void)
#else
GetEvent()
#endif
{

	int         wake = 0;
	/*wake = (*android_env)->CallIntMethod(android_env, android_callback, waitEvent);
	return wake;
	*/
	
	int code = (*android_env)->CallIntMethod(android_env, android_callback, waitEvent);

//LOGI("code: %d", code);
	if (code < 0)
	{
		code = -code;
		wake = 0;
	} else if (code > 0)
	{
		wake = 1;
	}

	if (code >= 100)
	{
		key_event(code - 100, 0);
	} else if (code > 0)
	{
		key_event(code - 1, 1);
	}

//	LOGI("wake: %d", wake);

	return wake;
}

int
exit_x48(int tell_x11)

{

	return 0;

}

void
#ifdef __FunctionProto__
refresh_icon(void)
#else
refresh_icon()
#endif
{
	return;

}

void
#ifdef __FunctionProto__
adjust_contrast(int contrast)
#else
adjust_contrast(contrast)
int contrast;
#endif
{

	return;

}

void
#ifdef __FunctionProto__
ShowConnections(char *wire, char *ir)
#else
ShowConnections(wire, ir)
char *wire;
char *ir;
#endif
{

	return;

}
