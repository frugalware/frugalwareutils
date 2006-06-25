/*
 *  libfwxwmconfig.c for frugalwareutils
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
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#define FWGETTEXT_LIB "libfwxwmconfig"
#include <libfwutil.h>
#include <libintl.h>
#include <alpm.h>
#include <glib.h>

#include "libfwxwmconfig.h"

#define _FWXWM_XINITDIR "/etc/X11/xinit"

/** @defgroup libfwxwmconfig Frugalware XDM Configuration library
 * @brief Functions to make XDM configuration easier
 * @{
 */

/** Initialize libalpm and the local database
 * @return the local database object on success, NULL on error
 */
PM_DB *fwxwm_init()
{
	PM_DB *db;

	if(alpm_initialize("/")==-1)
		return(NULL);
	if(!(db = alpm_db_register("local")))
	{
		alpm_release();
		return(NULL);
	}
	return(db);
}

/** Release libalpm and the local database
 * @param db the local database object
 * @return 0 on success, < 0 on error
 */
int fwxwm_release(PM_DB *db)
{
	int ret=0;

	ret += alpm_db_unregister(db);
	ret += alpm_release();
	return(ret);
}

/** Check for desktop managers other than XDM
 * @param db the local database object
 * @return 0 if no desktop manager found, > 0 if found at least one
 */
unsigned int fwxwm_checkdms(PM_DB *db)
{
	unsigned int ret=0;

	if(alpm_db_readpkg(db, "kdebase"))
		ret |= FWXWM_KDM;
	if(alpm_db_readpkg(db, "gdm"))
		ret |= FWXWM_GDM;
	return(ret);
}

/** List available window managers
 * @param db the local database object
 * @return a list of names and descriptions, in the "name1 desc1 name2 desc2"
 * format on success, NULL on error
 */
GList *fwx_listwms(PM_DB *db)
{
	GList *list=NULL;
	PM_PKG *pkg;
	DIR *dir;
	struct dirent *ent;
	struct stat buf;
	char path[PATH_MAX+1], *ptr;

	dir = opendir(_FWXWM_XINITDIR);
	if (!dir)
		return(NULL);
	while ((ent = readdir(dir)) != NULL)
	{
		if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		snprintf(path, PATH_MAX, "%s/%s", _FWXWM_XINITDIR, ent->d_name);
		if(lstat(path, &buf) || S_ISLNK(buf.st_mode))
			continue;
		if(strlen(ent->d_name)<=strlen("xinitrc."))
			continue;
		ptr = ent->d_name+strlen("xinitrc.");
		if((pkg = alpm_db_readpkg(db, ptr)))
		{
			list = g_list_append(list, strdup(ptr));
			list = g_list_append(list, strdup(alpm_pkg_getinfo(pkg, PM_PKG_DESC)));
			/*printf("pkg: %s\n", ptr);
			printf("desc: %s\n", (char*)alpm_pkg_getinfo(pkg, PM_PKG_DESC));*/
		}
		else
		{
			char *desc;

			if(!strcmp(ptr, "kde"))
				desc=strdup(_("K Desktop Environment"));
			else if(!strcmp(ptr, "xfce4"))
				desc=strdup(_("A lightweight desktop environment."));
			else
				desc=strdup(_("--- No description, please report! ---"));
			list = g_list_append(list, strdup(ptr));
			list = g_list_append(list, desc);
		}
	}

	return(list);
}

/** Set the window manager used by XDM
 * @param name the name of the selected window manager
 * @return 1 on success, 0 on error
 */
int fwxwm_set(char *name)
{
	char oldpath[PATH_MAX+1], newpath[PATH_MAX+1];

	snprintf(oldpath, PATH_MAX, "xinitrc.%s", name);
	snprintf(newpath, PATH_MAX, "%s/xinitrc", _FWXWM_XINITDIR);

	if(unlink(newpath)==-1)
		return(0);
	if(symlink(oldpath, newpath)==-1)
		return(0);
	return(1);
}
/* @} */
