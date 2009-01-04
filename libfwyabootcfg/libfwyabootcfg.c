/*
 *  libfwyabootcfg.c for frugalwareutils
 * 
 *  Copyright (c) 2008 by Miklos Vajna <vmiklos@frugalware.org>
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

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <libfwutil.h>
#include <glib.h>
#include <parted/parted.h>

#include "libfwyabootcfg.h"

#define FWYABOOT_LOGDEV "/dev/tty4"

static char *gen_title()
{
	struct utsname name;
	FILE *fp;
	char line[PATH_MAX];

	fp = fopen("/etc/frugalware-release", "r");
	if(!fp)
		return(NULL);
	if(!fgets(line, PATH_MAX, fp))
		return(NULL);
	line[strlen(line)-1]='\0';
	uname(&name);
	return(g_strdup_printf("%s - %s", line, name.release));
}

static PedExceptionOption peh(PedException* ex)
{
	return(PED_EXCEPTION_IGNORE);
}

static char *find_boot_part(PedDisk *disk)
{
	PedPartition *part = NULL;

	if (!ped_disk_next_partition(disk, NULL))
		return NULL;
	for(part = ped_disk_next_partition(disk, NULL); part; part = part->next) {
		// - in path? avoid ped_partition_get_flag(), would segfault..
		if (!strchr(ped_partition_get_path(part), '-') && ped_partition_get_flag(part, PED_PARTITION_BOOT))
			return ped_partition_get_path(part);
	}
	return NULL;
}

static char *find_boot()
{
	PedDevice *dev=NULL;
	PedDisk *disk = NULL;
	char *ptr;
	
	ped_exception_set_handler(peh);
	ped_device_probe_all();

	if (!ped_device_get_next(NULL))
		return NULL;
	for (dev = ped_device_get_next(NULL); dev; dev = dev->next) {
		if (dev->read_only)
			continue;
		disk = ped_disk_new(dev);
		ptr = find_boot_part(disk);
		if (ptr)
			return ptr;
	}
	return NULL;
}

// 0: device, 1: partition
static char *of_path(char *dev, int mode)
{
	FILE *pp;
	char *cmdline, *ptr, *ret;
	char path[PATH_MAX];

	cmdline = g_strdup_printf("/usr/sbin/ofpath %s", dev);
	pp = popen(cmdline, "r");
	if (!pp)
		return NULL;
	fgets(path, PATH_MAX, pp);
	fclose(pp);
	FWUTIL_FREE(cmdline);
	ptr = strrchr(path, ':');
	ret = path;
	if (!mode) {
		if (++ptr)
			*ptr = '\0';
	} else {
		ret = ++ptr;
		if (ret[strlen(ret)-1] == '\n')
			ret[strlen(ret)-1] = '\0';
	}
	return g_strdup(ret);
}

/* find_dev_recursive() and find_dev() is based on Linus's original rdev */
static int find_dev_recursive(char *dirnamebuf, int number)
{
	DIR *dp;
	struct dirent *dir;
	struct stat s;
	int dirnamelen = 0;

	if ((dp = opendir(dirnamebuf)) == NULL)
		return(0);
	dirnamelen = strlen(dirnamebuf);
	while ((dir = readdir(dp)) != NULL)
	{
		if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
			continue;
		if (dirnamelen + 1 + strlen(dir->d_name) > PATH_MAX)
			continue;
		dirnamebuf[dirnamelen] = '/';
		strcpy(dirnamebuf+dirnamelen+1, dir->d_name);
		if (lstat(dirnamebuf, &s) < 0)
			continue;
		if ((s.st_mode & S_IFMT) == S_IFBLK && s.st_rdev == number)
			return(1);
		if ((s.st_mode & S_IFMT) == S_IFDIR &&
				find_dev_recursive(dirnamebuf, number))
			return(1);
	}
	dirnamebuf[dirnamelen] = 0;
	closedir(dp);
	return(0);
}

static char *find_dev(int n)
{
	char path[PATH_MAX+1];
	if (!n)
		return(NULL);
	strcpy(path, "/dev");
	if(find_dev_recursive(path, n))
		return(strdup(path));
	return(NULL);
}

static char *find_root()
{
	struct stat buf;

	if(!stat("/",&buf))
		return(find_dev(buf.st_dev));
	return(NULL);
}

static void os_prober(FILE *fp)
{
	struct stat buf;
	FILE *pp;
	char line[PATH_MAX], *ptr;

	if(stat("/usr/bin/os-prober", &buf))
		return;

	pp = popen("os-prober 2>/dev/null", "r");
	if (!pp)
		return;

	while(fgets(line, PATH_MAX, pp)) {
		ptr = strrchr(line, ':');
		if (!++ptr)
			continue;
		if (!strncmp(ptr, "macosx", 6)) {
			ptr = strchr(line, ':');
			if (ptr)
				*ptr = '\0';
			fprintf(fp, "macosx=%s\n\n", line);
		} else if (!strncmp(ptr, "linux", 5)) {
			char *label, *root;
			
			ptr = strchr(line, ':');
			if (ptr)
				*ptr = '\0';
			root = line;
			ptr = strchr(++ptr, ':');
			if (!ptr)
				continue;
			label = g_strdup_printf("linux-%s", ++ptr);
			ptr = strchr(label, ':');
			if (ptr)
				*ptr = '\0';
			fprintf(fp, "image=/boot/vmlinux\n"
					"\tlabel=%s\n"
					"\troot=%s\n"
					"\tread-only\n\n", label, root);
			FWUTIL_FREE(label);
		}
	}
	pclose(pp);
}

void fwyaboot_create_menu(FILE *fp, int flags)
{
	char *root = find_root();
	char *ptr = NULL;

	fprintf(fp, "#\n"
		"# /etc/yaboot.conf - configuration file for Yaboot\n"
		"# This file is generated automatically by yabootcfg\n"
		"#\n\n");
	ptr = gen_title();
	fprintf(fp, "init-message=\"%s\"\n", ptr);
	FWUTIL_FREE (ptr);
	fprintf(fp, "boot=%s\n", find_boot());
	ptr = of_path(root, 0);
	fprintf(fp, "device=%s\n", ptr);
	FWUTIL_FREE (ptr);
	ptr = of_path(root, 1);
	fprintf(fp, "partition=%s\n", ptr);
	FWUTIL_FREE (ptr);
	fprintf(fp, "delay=10\n"
		"timeout=40\n"
		"install=/usr/lib/yaboot/yaboot\n"
		"magicboot=/usr/lib/yaboot/ofboot\n\n");

	if (flags & FWYABOOT_CDBOOT)
		fprintf(fp, "enablecdboot\n");
	if (flags & FWYABOOT_OFBOOT)
		fprintf(fp, "enableofboot\n");
	if (flags & FWYABOOT_NETBOOT)
		fprintf(fp, "enablenetboot\n");
	fprintf(fp, "\n");

	fprintf(fp, "image=/boot/vmlinux\n"
		"\tlabel=linux\n"
		"\troot=%s\n"
		"\tread-only\n\n", root);
	os_prober(fp);
}

int fwyaboot_install()
{
	int ret;
	FILE *pp;

	pp = popen("/usr/sbin/mkofboot &> " FWYABOOT_LOGDEV, "w");
	fprintf(pp, "y\n");
	ret = pclose(pp);
	ret += system("/usr/sbin/ybin -v &> " FWYABOOT_LOGDEV);
	return ret;
}

#if 0
int main()
{
	fwyaboot_create_menu(stdout, FWYABOOT_CDBOOT | FWYABOOT_OFBOOT | FWYABOOT_NETBOOT);
	//printf("%d\n", fwyaboot_install());
	return 0;
}
#endif
