/*
 *  libfwmouseconfig.c for frugalwareutils
 * 
 *  Copyright (c) 2006 by Miklos Vajna <vmiklos@frugalware.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 */

#include <stdio.h>
#include <string.h>

/** @defgroup libfwmouseconfig Frugalware Mouse Configuration library
 * @brief Functions to make mouse configuration easier
 * @{
 */

/** Detect USB mouse
 * @return 1 if found 0 if not
 */
int fwmouse_detect_usb()
{
	FILE *fp;
	char buf[4097];

	if((fp = fopen("/proc/bus/usb/devices", "r"))==NULL)
		return(0);

	while((fread(buf, 1, 4096, fp)))
	{
		if(strstr(buf, "usb_mouse"))
		{
			fclose(fp);
			return(1);
		}
	}
	fclose(fp);
	return(0);
}

/** Write a gpm configuration file
 * @return 0 on success, 1 on failure
 */
int fwmouse_writeconfig(char* dev, char* type)
{
	FILE *fp;

	if((fp=fopen("/etc/sysconfig/gpm", "w"))==NULL)
		return(1);

	fprintf(fp, "# /etc/sysconfig/gpm\n");
	fprintf(fp, "# configuration file for gpm\n\n");
	fprintf(fp, "# the mouse file to open\n");
	fprintf(fp, "dev=%s\n\n", dev);
	fprintf(fp, "# type of your mouse device\n");
	fprintf(fp, "type=%s\n", type);

	fclose(fp);
	return(0);
}
/* @} */
