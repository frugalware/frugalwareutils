/*
 *  libfwgrubconfig.c for frugalwareutils
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
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mntent.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <stdlib.h>
#include <libfwutil.h>
#include <glib.h>

typedef struct mdu_version_s {
	int major;
	int minor;
	int patchlevel;
} mdu_version_t;
#define MD_MAJOR 9
#define RAID_VERSION            _IOR (MD_MAJOR, 0x10, mdu_version_t)
#define FWGRUB_LOGDEV "/dev/tty4"


/** @defgroup libfwgrubconfig Frugalware GRUB Configuration library
 * @brief Functions to make GRUB configuration easier
 * @{
 */

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

static char *find_rootdev()
{
	struct stat buf;

	if(!stat("/",&buf))
		return(find_dev(buf.st_dev));
	return(NULL);
}

/* Copyright (C) 2001-2006 Neil Brown <neilb@suse.de> */
static int md_get_version(int fd)
{
	struct stat stb;
	mdu_version_t vers;

	if (fstat(fd, &stb)<0)
		return(1);
	if ((S_IFMT&stb.st_mode) != S_IFBLK)
		return(1);
	if (ioctl(fd, RAID_VERSION, &vers) == 0)
		return(0);
	return(1);
}

static int is_raid1_device(char *dev)
{
	int fd;

	fd = open(dev, O_RDWR, 0);
	if (md_get_version(fd))
		return(0);
	else
		return(1);
}

static GList *find_real_devs(char *dev)
{
	GList *ret=NULL;
	FILE *fp;
	char line[PATH_MAX];
	char *str, *ptr, *rdev;
	
	// drop "/dev/"
	dev += 5;

	fp = fopen("/proc/mdstat", "r");
	if(!fp)
		return(NULL);

	while(fgets(line, PATH_MAX, fp))
		if(!strncmp(line, dev, strlen(dev)))
			break;

	str = line;
	while((str = strstr(str, "[")))
	{
		rdev = str;
		while(*rdev!=' ')
			rdev--;
		rdev=strdup(++rdev);
		if((ptr = strstr(rdev, "[")))
			*ptr='\0';
		ret = g_list_append(ret, rdev);
		str++;
	}
	fclose(fp);
	return(ret);
}

// mode - 0: partition, 1: disk
static char *grub_convert(char *dev, int mode)
{
	char *disk, *ptr, *grubdisk;
	char line[PATH_MAX];
	int partnum=0;
	FILE *fp;

	// disk
	disk = strdup(dev);
	ptr = disk;
	while(*++ptr)
		if(isdigit(*ptr))
		{
			*ptr='\0';
			break;
		}

	// partnum
	if(!mode)
	{
		ptr = dev;
		while(*++ptr)
			if(isdigit(*ptr))
				break;
		partnum = atoi(ptr);
	}

	// grubdisk
	fp = fopen("/boot/grub/device.map", "r");
	if(!fp)
		return(NULL);

	while(fgets(line, PATH_MAX, fp))
		if(strstr(line, disk))
			break;
	fclose(fp);
	grubdisk = strdup(line+1);
	ptr = grubdisk;
	while(*++ptr)
		if(*ptr==')')
			break;
	*ptr='\0';

	if(partnum)
		ptr = g_strdup_printf("(%s,%d)", grubdisk, --partnum);
	else
		ptr = g_strdup_printf("(%s)", grubdisk);
	free(disk);
	free(grubdisk);
	return(ptr);
}

static char *get_mbr_dev()
{
	FILE *fp;
	char line[PATH_MAX], *str=NULL, *ptr;
	int i=0;

	fp = fopen("/proc/partitions", "r");
	if(!fp)
		return(NULL);
	while(fgets(line, PATH_MAX, fp))
		if(++i>2)
			break;
	fclose(fp);
	ptr=line;
	while(*++ptr)
		if(*ptr==' ')
			str = ptr+1;
	// drop the \n from the end
	if(!str)
		return(NULL);
	*(str+strlen(str)-1)='\0';
	return(g_strdup_printf("/dev/%s", str));
}

/** Installs grub to a given target
 * @param mode 0: mbr, 1: floppy, 2: root
 * @return 0 on succcess, 1 on error
 */
int fwgrub_install(int mode)
{
	char *rootdev=find_rootdev();
	FILE *pp;
	int i, ret=0;

	if(is_raid1_device(rootdev))
	{
		// manual installation
		DIR *dir;
		struct dirent *ent;
		char src[PATH_MAX], dest[PATH_MAX];

		dir = opendir("/usr/lib/grub/i386-pc");
		if (!dir)
			return(1);
		while ((ent = readdir(dir)) != NULL)
		{
			if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
				continue;
			snprintf(src, PATH_MAX, "/usr/lib/grub/i386-pc/%s", ent->d_name);
			snprintf(dest, PATH_MAX, "/boot/grub/%s", ent->d_name);
			fwutil_cp(src, dest);
		}
		pp = popen("grub --batch --device-map=/boot/grub/device.map %s >"
			FWGRUB_LOGDEV " 2>&1", "w");
		if(!pp)
			return(1);
		fprintf(pp, "quit\n");
		fclose(pp);
		if(mode==0)
		{
			// mbr
			GList *devs = find_real_devs(rootdev);
			for(i=0;i<g_list_length(devs);i++)
			{
				char *dev = g_list_nth_data(devs, i);
				pp = popen("grub --batch %s >" FWGRUB_LOGDEV
					" 2>&1", "w");
				if(!pp)
					return(1);
				fprintf(pp, "root %s\n", grub_convert(dev, 0));
				fprintf(pp, "setup %s\n", grub_convert(dev, 1));
			}
		}
		/* else if(mode==1)
		 * TODO: floppy */
		else if(mode==2)
		{
			// root
			GList *devs = find_real_devs(rootdev);
			for(i=0;i<g_list_length(devs);i++)
			{
				char *dev = g_list_nth_data(devs, i);
				pp = popen("grub --batch %s >" FWGRUB_LOGDEV
					" 2>&1", "w");
				if(!pp)
					return(1);
				fprintf(pp, "root %s\n", grub_convert(dev, 0));
				fprintf(pp, "setup %s\n", grub_convert(dev, 0));
			}
		}
	}
	else
	{
		char *dev=NULL, *ptr;
		if(mode==0)
			dev=get_mbr_dev();
		if(mode==1)
			dev=strdup("/dev/fd0");
		if(mode==2)
			dev=rootdev;
		if(mode!=1)
			ptr = g_strdup_printf("grub-install --no-floppy %s >"
				FWGRUB_LOGDEV " 2>&1", dev);
		else
			ptr = g_strdup_printf("grub-install %s >"
				FWGRUB_LOGDEV " 2>&1", dev);
		if(dev!=rootdev)
			free(dev);
		if((ret=system(ptr)))
			// this is needed for reiser, somehow the install is ok for the second time
			ret = system(ptr);
		free(ptr);
	}
	free(rootdev);
	return(ret);
}

/* based on coreutils/df.c, Copyright (C) 91, 1995-2004 Free Software Foundation, Inc. */
static char *find_mount_point(char *dir)
{
	char cwd[PATH_MAX] = "";
	char mp[PATH_MAX] = "";
	struct stat buf, last;

	getcwd(cwd, PATH_MAX);
	stat(dir, &buf);
	last=buf;
	chdir(dir);

	/* Now walk up DIR's parents until we find another filesystem or /,
	 * chdiring as we go. LAST holds stat information for the last place
	 * we visited.  */
	while(1)
	{
		struct stat st;
		stat("..", &st);
		if(st.st_dev != last.st_dev || st.st_ino == last.st_ino)
			/* cwd is the mount point.  */
			break;
		chdir("..");
		last=st;
	}
	getcwd(mp, PATH_MAX);
	chdir(cwd);
	return(strdup(mp));
}

static char *mount_dev(char *path)
{
	struct mntent *mnt;
	char *table = MOUNTED;
	FILE *fp;

	fp = setmntent (table, "r");
	if(!fp)
		return(NULL);
	while ((mnt = getmntent (fp)))
		if(!strcmp(mnt->mnt_dir, path))
			break;
	return(strdup(mnt->mnt_fsname));
}

static int write_entry(FILE *fp, char *title, char *grubbootdev, char *bootstr,
	char *kernel, char *rootdev, char *opts)
{
	if(!fp)
		return(1);
	fprintf(fp, "title %s\n", title);
	fprintf(fp, "\tkernel %s%s%s root=%s %s\n",
		grubbootdev, bootstr, kernel, rootdev, opts);
	return(0);
}

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

/** Crates a menu.lst
 * @param fp file pointer to write the menu.lst to
 */
void fwgrub_create_menu(FILE *fp)
{
	char *ptr, *bootstr, *title=gen_title();
	char *bootdev, *grubbootdev, *rootdev, *grubrootdev;
	GList *list;

	ptr = find_mount_point("/boot");
	bootdev = mount_dev(ptr);
	free(ptr);
	if(is_raid1_device(bootdev))
	{
		list = find_real_devs(bootdev);
		free(bootdev);
		bootdev = g_list_nth_data(list, 0);
		g_list_free(list);
	}
	grubbootdev=grub_convert(bootdev, 0);

	ptr = find_mount_point("/");
	rootdev = mount_dev(ptr);
	free(ptr);
	grubrootdev=grub_convert(rootdev, 0);
	if(!strcmp(rootdev, bootdev))
		bootstr = strdup("/boot");
	else
		bootstr = strdup("");

	fprintf(fp, "#\n# %s/grub/menu.lst - configuration file for GRUB\n", bootstr);
	fprintf(fp, "# This file is generated automatically by grubconfig\n#\n\n");
	fprintf(fp, "default=0\ntimeout=5\n");
	fprintf(fp, "gfxmenu %s%s/grub/message\n\n", grubbootdev, bootstr);
	write_entry(fp, title, grubbootdev, bootstr, "/vmlinuz", rootdev, "ro quiet vga=791");
	// TODO: other partitions

	free(bootstr);
	free(title);
	free(bootdev);
	free(grubbootdev);
	free(rootdev);
	free(grubrootdev);
}
/* @} */
