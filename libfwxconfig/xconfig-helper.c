/*
 *  xconfig-helper.c for frugalwareutils
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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libfwutil.h>
#include <libintl.h>

#include "xconfig-helper.h"

int main(int argc, char **argv)
{
	int i, ret=0, fd;
	char buf[256];

	i18ninit(__FILE__);

	system("xsetroot -solid SteelBlue");
	for(i=10;i>0;i--)
	{
		snprintf(buf, 255, _("xmessage -buttons OK:1 -center -timeout 1 \""
		"If you see this message, click OK within 10 seconds. "
		"Elapsed time: %d second(s).\""), i);
		if(system(buf))
		{
			ret=1;
			break;
		}
	}
	if(ret)
		fd = open(CLICKFILE, O_WRONLY | O_CREAT | O_EXCL, 0000);
	return(0);
}
