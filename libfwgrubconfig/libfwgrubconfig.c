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
#include <libfwutil.h>
#include <libfwgrubconfig.h>

#define FWGRUB_LOGDEV "/dev/tty4"

/** @defgroup libfwgrubconfig Frugalware GRUB configuration and installation library
 * @brief Functions to make GRUB configuration and installation easier
 * @{
 */

static
char *get_root_device(const char *root,char *s,size_t n)
{
	FILE *file;
	regex_t re;
	char line[LINE_MAX], *dev, *dir, *p;
	regmatch_t mat;

	file = fopen("/proc/mounts","rb");

	if(!file)
		return 0;

	if(regcomp(&re,"^/dev/(sd[a-z]|hd[a-z]|vd[a-z]|md[0-9]+)",REG_EXTENDED))
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

		if(strcmp(dir,root))
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

static
char *get_sysfs_contents(const char *path)
{
	FILE *file;
	char line[LINE_MAX], *s;

	file = fopen(path,"rb");

	if(!file)
		return 0;

	if(!fgets(line,sizeof line,file))
	{
		fclose(file);

		return 0;
	}

	s = strchr(line,'\n');

	if(s)
		*s = 0;

	s = strdup(line);

	fclose(file);

	return s;
}

static
void free_device_list(char **devices)
{
	char **p = devices;

	while(*p)
		free(*p++);

	free(devices);
}

static
char **get_device_list(const char *root)
{
	char **devices = 0;

	if(!strncmp(root,"/dev/md",7))
	{
		int disks_count, i;
		char path[PATH_MAX], *level = 0, *disks = 0;
		struct stat st;

		snprintf(path,sizeof path,"/sys/block/%s/md/level",root+5);

		level = get_sysfs_contents(path);

		snprintf(path,sizeof path,"/sys/block/%s/md/raid_disks",root+5);

		disks = get_sysfs_contents(path);

		if(!level || !disks || strcmp(level,"raid1") || atoi(disks) < 2)
		{
			free(level);

			free(disks);

			return 0;
		}

		disks_count = atoi(disks);

		devices = malloc(sizeof(char *) * (disks_count + 1));

		for( i = 0 ; i < disks_count ; ++i )
		{
			char buf[PATH_MAX], dev[PATH_MAX];
			ssize_t n;

			snprintf(path,sizeof path,"/sys/block/%s/md/rd%d",root+5,i);

			n = readlink(path,buf,sizeof buf);

			if(n >= 0 && n < (ssize_t) sizeof buf)
				buf[n] = 0;

			if(
				n == -1                                       ||
				n == (ssize_t) sizeof buf                     ||
				strncmp(buf,"dev-",4)                         ||
				snprintf(dev,sizeof dev,"/dev/%3s",buf+4) < 0 ||
				stat(dev,&st)
			)
			{
				free(level);

				free(disks);

				devices[i] = 0;

				free_device_list(devices);

				return 0;
			}

			devices[i] = strdup(dev);
		}

		devices[i] = 0;

		free(level);

		free(disks);
	}
	else
	{
		devices = malloc(sizeof(char *) * 2);

		devices[0] = strdup(root);

		devices[1] = 0;
	}

	return devices;
}

/** Installs grub to a given target
 * @param root directory to chroot to before installing grub
 * @param mode FWGRUB_INSTALL_MBR, FWGRUB_INSTALL_EFI
 * @return 0 on succcess, 1 on error
 */

int fwgrub_install(const char *root,enum fwgrub_install_mode mode)
{
	char cmd[_POSIX_ARG_MAX], dev[PATH_MAX], **devices, **p;

	if(mode == FWGRUB_INSTALL_MBR)
	{
		if(!get_root_device(root,dev,sizeof dev))
			return 1;

		devices = get_device_list(dev);

		if(!devices)
			return 1;

		for( p = devices ; *p ; ++p )
		{
			snprintf(cmd,sizeof cmd,"grub-install --recheck --no-floppy --boot-directory=/boot %s > " FWGRUB_LOGDEV " 2>&1",*p);

			if(fwutil_system_chroot(root,cmd))
			{
				free_device_list(devices);

				return 1;
			}
		}

		free_device_list(devices);

		return 0;
	}
#if 0
		case FWGRUB_INSTALL_EFI:
			strcat(cmd,"--root-directory=/boot/efi --bootloader-id=frugalware");
			if(!stat("/boot/efi",&st) && !S_ISDIR(st.st_mode))
				return 1;
			else if(mkdir("/boot/efi",0755))
				return 1;
			break;
#endif

	return 1;
}

/** Make a grub2 configuration file
 * @return 0 on succcess, 1 on error
 */
int fwgrub_make_config(void)
{
	return system("grub-mkconfig -o /boot/grub/grub.cfg > " FWGRUB_LOGDEV " 2>&1") ? 1 : 0;
}

/** @} */
