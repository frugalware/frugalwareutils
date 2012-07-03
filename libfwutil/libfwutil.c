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
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <glib.h>

#include "libfwutil.h"

int fwutil_dryrun = 0;

/** @defgroup libfwutil Frugalware General Utility library
 * @brief Functions to make writing Frugalware configuration tools easier
 * @{
 */

/** A wrapper to execl() and printf(): the action depends on the global
 * f_util_dryrun variable (0 by default).
 * @param cmd the command to print/execute
 * @return 1 on failure, 0 on success
 */
int fwutil_system(const char *cmd)
{
	pid_t id;
	int status;

	if(fwutil_dryrun)
		return(printf("%s\n", cmd) < 0 ? 1 : 0);

	id = fork();

	if(!id)
	{
		execl("/bin/sh","/bin/sh","-c",cmd,(void *) 0);

		_exit(EXIT_FAILURE);
	}
	else if(id == -1)
		return 1;

	if(waitpid(id,&status,0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status))
		return 1;

	return 0;
}

/** A wrapper to execl() and printf(): the action depends on the global
 * f_util_dryrun variable (0 by default).
 * @param root the directory to chroot to before execute
 * @param cmd the command to print/execute
 * @return 1 on failure, 0 on success
 */
int fwutil_system_chroot(const char *root,const char *cmd)
{
	pid_t id;
	int status;

	if(fwutil_dryrun)
		return(printf("%s: %s\n", root, cmd) < 0 ? 1 : 0);

	id = fork();

	if(!id)
	{
		if(chroot(root) || chdir("/"))
			_exit(EXIT_FAILURE);

		execl("/bin/sh","/bin/sh","-c",cmd,(void *) 0);

		_exit(EXIT_FAILURE);
	}
	else if(id == -1)
		return 1;

	if(waitpid(id,&status,0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status))
		return 1;

	return 0;
}

/** Initialize gettext, based on the filename.
 * @param namespace just use the __FILE__ constant
 */
void fwutil_i18ninit(char *namespace)
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
char *fwutil_trim(char *str)
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
 * @return pointer to the converted string
 */
char *fwutil_strtoupper(char *str)
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
char *fwutil_glist_display(GList *list, char *sep)
{
	int i, len=0;
	char *ret;

	for (i=0; i<g_list_length(list); i++)
	{
		len += strlen((char*)g_list_nth_data(list, i));
		len += strlen(sep)+1;
	}
	if(len==0)
		return(NULL);
	if((ret = (char*)malloc(len)) == NULL)
		return(NULL);
	*ret='\0';
	for (i=0; i<g_list_length(list); i++)
	{
		strcat(ret, (char*)g_list_nth_data(list, i));
		strcat(ret, sep);
	}
	return(ret);
}

/** Initialize the environment if necessary
 * @return true if fwx_release() call is needed later, false if not
 */
int fwutil_init()
{
	struct stat buf;
	FILE *fi, *fo;
	int ret=0;

	if(stat("/proc/1", &buf))
	{
		system("mount /proc");
		ret++;
	}
	if(stat("/dev/zero", &buf))
	{
		system("/etc/rc.d/rc.udev");
		system("mount / -o rw,remount");


		system("mount /dev/pts");
		ret++;
	}
	if((fi = fopen("/proc/mounts", "r")))
	{
		if((fo = fopen("/etc/mtab", "w")))
		{
			char line[256];

			while(!feof(fi))
			{
				if(!fgets(line, 255, fi))
					break;
				if(!strstr(line, "root"))
					fprintf(fo, "%s", line);
			}
			fclose(fo);
		}
		fclose(fi);
	}
	return(ret);
}

/** Release the environment
 */
void fwutil_release()
{
	umount("/dev/pts");
	umount("/dev");
	umount("/sys");
	umount("/proc");
}

/** does the same as cp
 * @param src source
 * @param dest destination
 * @return 1 on error, 0 on success
 */
int fwutil_cp(char *src, char *dest)
{
	FILE *in, *out;
	size_t len;
	char buf[4097];

	in = fopen(src, "r");
	if(in == NULL)
		return(1);
	out = fopen(dest, "w");
	if(out == NULL)
	{
		fclose(in);
		return(1);
	}
	while((len = fread(buf, 1, 4096, in)))
		fwrite(buf, 1, len, out);
	fclose(in);
	fclose(out);
	return(0);
}

/* @} */
