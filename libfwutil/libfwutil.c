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
#include <glib.h>

#include "libfwutil.h"

int f_util_dryrun = 0;

/** @defgroup libfwutil Frugalware General Utility library
 * @brief Functions to make writing Frugalware configuration tools easier
 * @{
 */

/** A wrapper to system() and printf(): the action depends on the global
 * f_util_dryrun variable (0 by default).
 * @param cmd the command to print/execute
 * @return 1 on failure, 0 on success
 */
int nc_system(const char *cmd)
{
	if(f_util_dryrun)
		return(printf("%s\n", cmd));
	else
		return(system(cmd) ? 1 : 0);
}

/** Initialize gettext, based on the filename.
 * @param namespace just use the __FILE__ constant
 */
void i18ninit(char *namespace)
{
	char *lang=NULL;
	char *ptr = strdup(namespace);

	// slice the .c suffix
	*(ptr + strlen(ptr)-2)='\0';

	lang=getenv("LC_ALL");
	if(lang==NULL || lang[0]=='\0')
		lang=getenv("LC_MESSAGES");
	if (lang==NULL || lang[0]=='\0')
		lang=getenv("LANG");

	setlocale(LC_ALL, lang);
	bindtextdomain(ptr, "/usr/share/locale");
	textdomain(ptr);
	free(ptr);
}

/** Trims whitespace from the start and end of the line.
 * @param str string to trim
 * @return pointer to the trim'ed string
 */
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

/** Converts a char* to uppercase using toupper()
 * @param str string to convert
 * @return pointer to the upper'ed sting
 */
char *strtoupper(char *str)
{
	char *ptr = str;

	while(*ptr)
		*ptr++ = toupper(*ptr);
	return str;
}

/** Build a string from a GList of char* using a given separator
 * @param list the GList
 * @param sep the separator
 */
char *g_list_display(GList *list, char *sep)
{
	int i, len=0;
	char *ret;

	for (i=1; i<g_list_length(list); i++)
	{
		len += strlen((char*)g_list_nth_data(list, i));
		len += strlen(sep)+1;
	}
	if(len==0)
		return(NULL);
	if((ret = (char*)malloc(len)) == NULL)
		return(NULL);
	*ret='\0';
	for (i=1; i<g_list_length(list); i++)
	{
		strcat(ret, (char*)g_list_nth_data(list, i));
		strcat(ret, sep);
	}
	return(ret);
}
/* @} */
