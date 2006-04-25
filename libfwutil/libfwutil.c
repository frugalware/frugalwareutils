/*
 *  libfwutil.c for frugalwareutils
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
#include <unistd.h>
#include <libintl.h>
#include <string.h>
#include <ctype.h>

#include "libfwutil.h"

int f_util_dryrun = 0;

int nc_system(const char *cmd)
{
	if(f_util_dryrun)
		return(printf("%s\n", cmd));
	else
		return(system(cmd) ? 1 : 0);
}

void i18ninit(void)
{
	char *lang=NULL;

	lang=getenv("LC_ALL");
	if(lang==NULL || lang[0]=='\0')
		lang=getenv("LC_MESSAGES");
	if (lang==NULL || lang[0]=='\0')
		lang=getenv("LANG");

	setlocale(LC_ALL, lang);
	bindtextdomain("netconfig", "/usr/share/locale");
	textdomain("netconfig");
}

char *trim(char *str)
{
	char *ptr = str;

	while(isspace(*ptr++))
		if(ptr != str)
			memmove(str, ptr, (strlen(ptr) + 1));
	ptr = (char *)(str + (strlen(str) - 1));
	while(isspace(*ptr--))
		*++ptr = '\0';
	return str;
}

char *strtoupper(char *str)
{
	char *ptr = str;

	while(*ptr)
		*ptr++ = toupper(*ptr);
	return str;
}
