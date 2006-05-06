/*
 *  libfwraidconfig.c for frugalwareutils
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
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <libintl.h>
#include <glib.h>
#include <libfwutil.h>
#include <parted/parted.h>
#include <sys/stat.h>

#include "libfwraidconfig.h"

GList *fwraid_parts=NULL;

/** @defgroup libfwraidconfig Frugalware RAID Configuration library
 * @brief Functions to make RAID configuration easier
 * @{
 */

/** Suggest the next unused /dev/mdX name
 * @return the new name
 */
char* fwraid_suggest_devname()
{
	FILE *fp;
	char line[PATH_MAX+1];
	char last='0' - 1;

	if((fp=fopen("/proc/mdstat", "r")) == NULL)
		return(NULL);
	while(fgets(line, PATH_MAX, fp))
		if(!strncmp(line, "md", 2) && (strlen(line)>2) && (line[2] > last))
			last = line[2];
	fclose(fp);
	return(g_strdup_printf("/dev/md%c", ++last));
	
}

/** Creates a RAID device node manually.
 * @param devname the name of the device
 * @return 1 on failure, 0 on success
 */
int fwraid_mknod_md(char *devname)
{
	struct stat buf;
	int ret;
	char *ptr;

	if(!stat(devname, &buf))
		// device already created
		return(0);
	ptr = g_strdup_printf("mknod %s b 9 %s", devname, devname + strlen("/dev/md"));
	
	ret = system(ptr);
	free(ptr);
	return(ret);
}

int _fwraid_partdetails(PedPartition *part)
{
	char *pname, *ptr;

	pname = ped_partition_get_path(part);

	// remove the unnecessary "p-1" suffix from sw raid device names
	if((ptr = strstr(pname, "p-1"))!=NULL)
		*ptr='\0';

	fwraid_parts = g_list_append(fwraid_parts, pname);
	fwraid_parts = g_list_append(fwraid_parts, g_strdup_printf("%dGB", (int)part->geom.length/1953125));

	return(0);
}

int _fwraid_listparts(PedDisk *disk)
{
	PedPartition *part = NULL;
	PedPartition *extpart = NULL;

	if(ped_disk_next_partition(disk, NULL)==NULL)
		// no partition detected
		return(1);
	for(part=ped_disk_next_partition(disk, NULL);
		part!=NULL;part=part->next)
	{
		if((part->num>0) && (part->type != PED_PARTITION_EXTENDED) && ped_partition_get_flag(part, PED_PARTITION_RAID))
			_fwraid_partdetails(part);
		if(part->type == PED_PARTITION_EXTENDED)
			for(extpart=part->part_list;
				extpart!=NULL;extpart=extpart->next)
				if(extpart->num>0 && ped_partition_get_flag(extpart, PED_PARTITION_RAID))
					_fwraid_partdetails(extpart);
	}
	return(0);
}

/** List raid-able partitions.
 * @return the list of partitions
 */
GList *fwraid_lst_parts()
{
	PedDevice *dev = NULL;
	PedDisk *disk = NULL;

	ped_device_probe_all();
	for(dev=ped_device_get_next(NULL);dev!=NULL;dev=dev->next)
	{
		if(dev->read_only)
			// no raid cd, thanks
			continue;
		disk = ped_disk_new(dev);
		_fwraid_listparts(disk);
	}
	return(fwraid_parts);
}

/** An mdadm wrapper
 * @param devname device name of the raid
 * @param level the raid level
 * @param devices list of the real device names
 * @return 1 on failure, 0 on success
 */
int fwraid_create_md(char *devname, int level, GList *devices)
{
	char *ptr = g_list_display(devices, " ");
	// TODO: this "yes" is an ugly hack to pass various warnings
	char *cmd = g_strdup_printf("yes |mdadm --create --verbose %s "
		"--level=%d --raid-devices=%d %s >%s 2>%s",
		devname, level, g_list_length(devices), ptr, FWRAID_LOGDEV, FWRAID_LOGDEV);
	int ret;
	
	ret = system(cmd);
	free(ptr);
	free(cmd);
	return(ret);
}
