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

/* $Log: options.c,v $
 * Revision 1.5  1995/01/11  18:20:01  ecd
 * major update to support HP48 G/GX
 *
 * Revision 1.4  1994/12/07  20:20:50  ecd
 * more options
 *
 * Revision 1.4  1994/12/07  20:20:50  ecd
 * more options
 *
 * Revision 1.3  1994/11/28  02:00:51  ecd
 * complete rewrite
 *
 * Revision 1.2  1994/11/04  03:42:34  ecd
 * finally implemented the first options
 *
 * Revision 1.1  1994/11/02  14:44:28  ecd
 * Initial revision
 *
 *
 * $Id: options.c,v 1.5 1995/01/11 18:20:01 ecd Exp ecd $
 */

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "constants.h"
#include "resources.h"

void
#ifdef __FunctionProto__
usage(void)
#else
usage()
#endif
{
  fprintf(stdout, "\n\
x48 Version %d.%d.%d, Copyright (c) 1994-2005 by Eddie C. Dost <ecd@dressler.de>.\n\
\n\
usage:\n\t%s [-options ...]\n\
\n\
where options include:\n\
    -help                        print out this message\n\
    -display    <displayname>    X server to contact\n\
    -name	<string>         set application name to <string>\n\
    -title      <string>         set window title to <string>\n\
    -geometry   <geometry>       position of window\n\
    -iconGeom   <geometry>       position of icon window\n\
    -iconic                      start iconic\n\
    -visual     <visualname>     use visual <visualname>\n\
    -mono                        force monochrome\n\
    -gray                        force grayscale\n\
    -monoIcon                    force monochrome icon\n\
    -smallFont  <fontname>       <fontname> to draw small labels (MTH - DEL)\n\
    -mediumFont <fontname>       <fontname> to draw medium label (ENTER)\n\
    -largeFont  <fontname>       <fontname> to draw large labels (Numbers)\n\
    -connFont   <fontname>       <fontname> to display wire & IR connections\n\
    -/+xshm                      turn on/off XShm extension\n\
    -version                     print out version information\n\
    -copyright                   print out copyright information\n\
    -warranty                    print out warranty information\n\
    -verbose                     run verbosive\n\
    -quiet                       run quietly\n\
    -/+terminal                  turn on/off pseudo terminal interface\n\
    -/+serial                    turn on/off serial interface\n\
    -line       <devicename>     use serial line <devicename> for IR connection\n\
    -/+debug                     turn on/off debugger\n\
    -disasm     <string>         use <string> (\'HP\' or \'class\') mnemonics\n\
    -reset                       perform a reset (PC = 0) on startup\n\
    -initialize                  force initialization x48 from ROM-dump\n\
    -rom        <filename>       if initializing, read ROM from <filename>\n\
    -home       <directory>      use directory ~/<directory> to save x48 files\n\
    -xrm        <resource>       set Xresource <resource>\n\
\n", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, progname);

  fflush(stdout);
  exit (1);
}

void
#ifdef __FunctionProto__
show_version(void)
#else
show_version()
#endif
{
  fprintf(stdout, "\n\
%s Version %d.%d.%d, x48 is Copyright (c) 1994-2005 by Eddie C. Dost <ecd@dressler.de>.\n\
Compiled on %s by <%s> #%d\n\n",
	 progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL,
         COMPILE_TIME, COMPILE_BY, COMPILE_VERSION);
}

void
#ifdef __FunctionProto__
show_copyright(void)
#else
show_copyright()
#endif
{
  fprintf(stdout, "\n\
                               COPYRIGHT\n\
\n\
X48 is an Emulator/Debugger for the HP-48 Handheld Calculator.\n\
Copyright (C) 1994 by Eddie C. Dost <ecd@dressler.de>.\n\
\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n");
}

void
#ifdef __FunctionProto__
show_warranty(void)
#else
show_warranty()
#endif
{
  fprintf(stdout, "\n\
                              NO WARRANTY\n\
\n\
      BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n\
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n\
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n\
PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n\
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n\
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n\
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n\
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n\
REPAIR OR CORRECTION.\n\
\n\
      IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n\
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n\
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n\
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n\
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n\
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n\
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n\
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n\
POSSIBILITY OF SUCH DAMAGES.\n\n");
}


