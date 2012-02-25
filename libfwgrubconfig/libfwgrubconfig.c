/*
 *  libfwgrubconfig.c for frugalwareutils
 *
 *  Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 by Miklos Vajna <vmiklos@frugalware.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include "libfwgrubconfig.h"

#define SPACE         " \t\r\n\v\f"
#define DIGIT         "0123456789"
#define FWGRUB_LOGDEV "/dev/tty4"

/** @defgroup libfwgrubconfig Frugalware GRUB configuration and installation library
 * @brief Functions to make GRUB configuration and installation easier
 * @{
 */

static
char *guess_mbr_device(void)
{
	FILE *f;
	char line[LINE_MAX], *p;
	int i, j;
	regex_t re;
	static char root[PATH_MAX];

	f = fopen("/proc/partitions","rb");

	if(!f)
		return 0;

	if(regcomp(&re,"^[shv]d[a-z]$",REG_EXTENDED | REG_NOSUB | REG_NEWLINE))
	{
		fclose(f);

		return 0;
	}

	for( i = 1, *root = 0 ; fgets(line,sizeof line,f) ; ++i )
	{
		if(i < 3)
			continue;

		p = line + strspn(line,SPACE);

		for( j = 0 ; j < 3 ; ++j )
		{
			p += strspn(p,DIGIT);

			p += strspn(p,SPACE);
		}

		if(!regexec(&re,p,0,0,0))
		{
			strcpy(root,"/dev/");
			strcpy(root,p);
			p = strchr(root,'\n');
			if(p)
				*p = 0;
			break;
		}
	}

	fclose(f);

	regfree(&re);

	return (*root) ? root : 0;
}

static
int execute_command(const char *cmd)
{
	pid_t pid;
	int status;

	/* Let's do the splits! :D */
	pid = fork();

	if(!pid)
	{
		/* Execute the command in the child process. */
		execl("/bin/sh","/bin/sh","-c",cmd,(void *) 0);

		_exit(EXIT_FAILURE);
	}
	else if(pid == -1)
		return 1;

	/* Did the process exit normally and return a zero exit code? */
	if(waitpid(pid,&status,0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status))
		return 1;

	return 0;
}

/** Installs grub to a given target
 * @param mode FWGRUB_INSTALL_MBR, FWGRUB_INSTALL_EFI
 * @return 0 on succcess, 1 on error
 */

int fwgrub_install(enum fwgrub_install_mode mode)
{
	char cmd[_POSIX_ARG_MAX], *mbr;
	struct stat st;

	/* First, define the common parts of the install command. */
	strcpy(cmd,"grub-install --recheck --no-floppy --boot-directory=/boot ");

	/* Now, define additional arguments based on installation mode. */
	switch(mode)
	{
		case FWGRUB_INSTALL_MBR:
			mbr = guess_mbr_device();
			if(!mbr)
				return 1;
			strcat(cmd,mbr);
			break;

		case FWGRUB_INSTALL_EFI:
			strcat(cmd,"--root-directory=/boot/efi --bootloader-id=frugalware");
			if(!stat("/boot/efi",&st) && !S_ISDIR(st.st_mode))
				return 1;
			else if(mkdir("/boot/efi",0755))
				return 1;
			break;
	}

	/* Setup logging. */
	strcat(cmd," > " FWGRUB_LOGDEV " 2>&1");

	return execute_command(cmd);
}

/** Make a grub2 configuration file
 * @return 0 on succcess, 1 on error
 */
int fwgrub2_make_config(void)
{
	return execute_command("grub-mkconfig -o /boot/grub/grub.cfg > " FWGRUB_LOGDEV " 2>&1");
}

/** @} */
