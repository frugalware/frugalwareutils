/*
 *  timeconfig.c for frugalwareutils
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
#include <glib.h>
#include <libfwdialog.h>
#include <libfwutil.h>
#include <libfwtimeconfig.h>
#include <setup.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

#define CLOCKFILE "/etc/hardwareclock"
#define ZONEDIR "/usr/share/zoneinfo"
#define ZONEFILE "/etc/localtime"

GList *zones=NULL;

char* ask_mode()
{
	char *modes[] = 
	{
		_("no"), _("Hardware clock is set to local time"),
		_("yes"), _("Hardware clock is set to UTC")
	};
	char *ptr;

	ptr = dialog_mymenu(_("Do you want to set hardware clock utc?"),
		_("Is the hardware clock set to Coordinated Universal Time "
		"(UTC/GMT)?  If it is, select 'yes' here. If the hardware "
		"clock is set to the current local time (this is how most PCs "
		"are set up), then say 'no' here.  If you are not sure what "
		"this is, you should answer 'no' here."), 0, 0, 0, 2, modes);
	if(!strcmp(ptr, "yes"))
	{
		free(ptr);
		ptr = strdup("UTC");
	}
	else
	{
		free(ptr);
		ptr = strdup("localtime");
	}
	return(ptr);
}

int sort_zones(gconstpointer a, gconstpointer b)
{
	return(strcmp(a, b));
}

GList *zone_scan(char *dir)
{
	GList *buf=NULL;
	int i;

	// search the zones
	fwtimeconfig_find(dir);

	// sort them and add a "   " item after each (required by libdialog)
	zones = g_list_sort(zones, sort_zones);
	for (i=0; i<g_list_length(zones); i++)
	{
		buf = g_list_append(buf, g_list_nth_data(zones, i));
		buf = g_list_append(buf, "   ");
	}
	g_list_free(zones);
	zones = buf;
	return(zones);
}

char *ask_zone(GList *zones)
{
	char **zonestrs = glist2dialog(zones);
	char *ptr, *ret;

	ret = dialog_mymenu(_("Timezone configuration"),
		_("Please select one of the following timezones for your "
		"machine:"), 0, 0, 0, g_list_length(zones)/2, zonestrs);
	free(zonestrs);
	ptr = g_strdup_printf("/usr/share/zoneinfo/%s", ret);
	free(ret);
	return(ptr);
}

int run()
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	char *ptr;
	GList *zones;

	i18ninit(__FILE__);
	if(argv!=NULL)
		init_dialog(input, dialog_state.output);
	dialog_backtitle(_("Time configuration"));

	ptr = ask_mode();
	fwtimeconfig_hwclockconf(CLOCKFILE, ptr);
	free(ptr);
	zones = zone_scan(ZONEDIR);
	ptr = ask_zone(zones);
	symlink(ptr, ZONEFILE);

	if(argv!=NULL)
		end_dialog();
	return(0);
}

plugin_t plugin =
{
	"timeconfig",
	"Time configuration",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
