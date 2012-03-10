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

#define FWGRUB_LOGDEV "/dev/tty4"

/** @defgroup libfwgrubconfig Frugalware GRUB configuration and installation library
 * @brief Functions to make GRUB configuration and installation easier
 * @{
 */

static
char *get_first_device(char *s,size_t n)
{
	FILE *file;
	regex_t re;
	int i = 1;
	char line[LINE_MAX], *p, *dev;

	file = fopen("/proc/partitions","rb");

	if(!file)
		return 0;

	if(regcomp(&re,"^[hsv]d[a-z]$",REG_EXTENDED | REG_NOSUB))
	{
		fclose(file);

		return 0;
	}

	*s = 0;

	while(fgets(line,sizeof line,file))
	{
		if(i < 3)
		{
			++i;

			continue;
		}

		if(!strtok_r(line," \n",&p))
			continue;

		if(!strtok_r(0," \n",&p))
			continue;

		if(!strtok_r(0," \n",&p))
			continue;

		dev = strtok_r(0," \n",&p);

		if(!dev)
			continue;

		if(regexec(&re,dev,0,0,0))
			continue;

		snprintf(s,n,"/dev/%s",dev);

		break;
	}

	fclose(file);

	regfree(&re);

	return *s ? s : 0;
}

static
char *get_root_device(char *s,size_t n)
{
	FILE *file;
	regex_t re;
	char line[LINE_MAX], *dev, *dir, *p;
	regmatch_t mat;

	file = fopen("/proc/mounts","rb");

	if(!file)
		return 0;

	if(regcomp(&re,"^/dev/[hsv]d[a-z]",REG_EXTENDED))
	{
		fclose(file);

		return 0;
	}

	*s = 0;

	while(fgets(line,sizeof line,file))
	{
		dev = strtok_r(line," ",&p);

		if(!dev)
	        continue;

		dir = strtok_r(0," ",&p);

		if(!dir)
			continue;

		if(strcmp(dir,"/"))
			continue;

		if(regexec(&re,dev,1,&mat,0))
			continue;

		snprintf(s,n,"%.*s",mat.rm_eo - mat.rm_so,dev);

		break;
	}

	fclose(file);

	regfree(&re);

	return *s ? s : 0;
}

/** Installs grub to a given target
 * @param mode FWGRUB_INSTALL_MBR_FIRST, FWGRUB_INSTALL_MBR_ROOT, FWGRUB_INSTALL_EFI
 * @return 0 on succcess, 1 on error
 */

int fwgrub_install(enum fwgrub_install_mode mode)
{
	char cmd[_POSIX_ARG_MAX], device[PATH_MAX];
	struct stat st;

	/* First, define the common parts of the install command. */
	strcpy(cmd,"grub-install --recheck --no-floppy --boot-directory=/boot ");

	/* Now, define additional arguments based on installation mode. */
	switch(mode)
	{
		case FWGRUB_INSTALL_MBR_FIRST:
			if(!get_first_device(device,sizeof device))
				return 1;
			strcat(cmd,device);
			break;

		case FWGRUB_INSTALL_MBR_ROOT:
			if(!get_root_device(device,sizeof device))
				return 1;
			strcat(cmd,device);
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

	return system(cmd) ? 1 : 0;
}

/** Make a grub2 configuration file
 * @return 0 on succcess, 1 on error
 */
int fwgrub_make_config(void)
{
	return system("grub-mkconfig -o /boot/grub/grub.cfg > " FWGRUB_LOGDEV " 2>&1") ? 1 : 0;
}

/** @} */
