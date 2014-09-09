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

/* $Log: x48_x11.h,v $
 * Revision 1.11  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.10  1994/12/07  20:16:41  ecd
 * added "refresh_icon"
 *
 * Revision 1.10  1994/12/07  20:16:41  ecd
 * added "refresh_icon"
 *
 * Revision 1.9  1994/11/28  02:19:22  ecd
 * added support for contrast adjustment
 *
 * Revision 1.8  1994/11/04  03:44:47  ecd
 * added support for mono and gray displays
 *
 * Revision 1.7  1994/11/02  14:51:27  ecd
 * minor fix
 *
 * Revision 1.6  1994/10/09  20:26:35  ecd
 * changed disp_t
 *
 * Revision 1.5  1994/10/06  16:29:28  ecd
 * added XShm - Extension stuff
 *
 * Revision 1.4  1994/10/05  08:33:22  ecd
 * changed disp_t: removed Pixmap
 *
 * Revision 1.3  1994/09/30  12:32:49  ecd
 * changed display stuff, added detection of interrupts in GetEvent
 *
 * Revision 1.2  1994/09/18  15:31:58  ecd
 * started Real Time support
 *
 * Revision 1.1  1994/09/13  15:05:11  ecd
 * Initial revision
 *
 *
 * $Id: x48_x11.h,v 1.11 1995/01/11 18:20:01 ecd Exp ecd $
 */


#include "global.h"
#include <pthread.h>

#define WHITE		0
#define LEFT		1
#define RIGHT		2
#define BUT_TOP 	3
#define BUTTON  	4
#define BUT_BOT 	5
#define LCD		6
#define PIXEL		7
#define PAD_TOP 	8
#define PAD		9
#define PAD_BOT		10
#define DISP_PAD_TOP	11
#define DISP_PAD	12
#define DISP_PAD_BOT	13
#define LOGO		14
#define LOGO_BACK	15
#define LABEL		16
#define FRAME		17
#define UNDERLAY	18
#define BLACK		19

#include <android/log.h> 
#include <jni.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "x48",  __VA_ARGS__) 
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "x48",  __VA_ARGS__) 
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "x48",  __VA_ARGS__) 
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "x48",  __VA_ARGS__) 
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "x48",  __VA_ARGS__) 

/* TODO */
typedef struct XColor {
	int dummy;
} XColor;
typedef struct Window {
	int dummy;
} Window;
typedef struct GC {
	int dummy;
} GC;
typedef struct Display {
	int dummy;
} Display;
typedef struct Pixmap {
	char * data;
	int width;
	int height;
} Pixmap;
typedef struct Colormap {
	int dummy;
} Colormap;
typedef struct Atom {
	int dummy;
} Atom;
typedef struct Visual {
	int dummy;
} Visual;
/* */

typedef struct color_t {
  char *name;
  int r, g, b;
  int mono_rgb;
  int gray_rgb;
  XColor xcolor;
} color_t;

extern color_t *colors;

#define COLOR(c)        (colors[(c)].xcolor.pixel)

#define UPDATE_MENU	1
#define UPDATE_DISP	2

typedef struct disp_t {
  unsigned int     w, h;
  Window           win;
  GC               gc;
  short            mapped;
  int		   offset;
  int		   lines;
  int	display_update;
} disp_t;

extern disp_t   disp;

extern JNIEnv *android_env;
extern jobject android_callback;
extern jmethodID waitEvent;

extern Display *dpy;
extern int	screen;
extern int  exit_state;

extern int	InitDisplay	 __ProtoType__((int argc, char **argv));
extern int	CreateWindows    __ProtoType__((int argc, char **argv));
extern int	GetEvent	 __ProtoType__((void));

extern void	adjust_contrast  __ProtoType__((int contrast));
extern void	refresh_icon	 __ProtoType__((void));

extern void	blockConditionVariable	 __ProtoType__((void));

extern void	ShowConnections	 __ProtoType__((char *w, char *i));

// extern void	exit_x48	 __ProtoType__((int tell_x11));

extern Pixmap XCreateBitmapFromData __ProtoType__((Display *dpy, Window win, char* data, int a, int b));

extern void XClearArea __ProtoType__((Display *dpy, Window win, int x, int y, int width, int height, int boo));

extern void XCopyPlane __ProtoType__((Display *dpy, Pixmap map, Window win, GC gc, int a, int b, int x, int y, int width, int height, int boo));

extern void XClearWindow __ProtoType__((Display *dpy, Window win));

static pthread_cond_t  uiConditionVariable  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t uiConditionMutex     = PTHREAD_MUTEX_INITIALIZER;