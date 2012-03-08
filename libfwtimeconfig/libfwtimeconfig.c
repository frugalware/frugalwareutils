/*
 *  libfwtimeconfig.c for frugalwareutils
 *
 *  Copyright (c) 2006, 2010 by Miklos Vajna <vmiklos@frugalware.org>
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
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

#define ZONEDIR "/usr/share/zoneinfo"
#define ZONEFILE "/etc/localtime"

GList *zones=NULL;

/** @defgroup libfwtimeconfig Frugalware Time Configuration library
 * @brief Functions to make time configuration easier
 * @{
 */

/** Creates the hardware clock configuration file.
 * @param mode clock mode
 * @return 1 on failure, 0 on success
 */
int fwtime_hwclockconf(char *mode)
{
	char cmd[_POSIX_ARG_MAX];

	strcpy(cmd,"hwclock --adjust");

	if(!strcmp(mode,"UTC"))
		strcat(cmd," --utc");
	else if(!strcmp(mode,"localtime"))
		strcat(cmd," --localtime");

	return system(cmd) ? 1 : 0;
}

/** Helper function for finding timezones available. The result will be written
 * to the global "zones" GList.
 * @param dirname path of the dir which contains the timezones
 * @return 1 on failure, 0 on success
 */
int fwtime_find(char *dirname)
{
	DIR *dir;
	struct dirent *ent;
	struct stat statbuf;
	char *fn;

	dir = opendir(dirname);
	if (!dir)
	{
		perror(dirname);
		return(1);
	}
	while ((ent = readdir(dir)) != NULL)
	{
		fn = g_strdup_printf("%s/%s", dirname, ent->d_name);
		if(!stat(fn, &statbuf) && S_ISDIR(statbuf.st_mode))
			if(strcmp(ent->d_name, ".") &&
				strcmp(ent->d_name, ".."))
				fwtime_find(fn);
		if(!stat(fn, &statbuf) && S_ISREG(statbuf.st_mode) &&
			!strrchr(ent->d_name, '.'))
			zones = g_list_append(zones, strdup(fn+strlen(ZONEDIR)+1));
		free(fn);
	}
	closedir(dir);
	return(0);
}
/* @} */
