/*
 *  raidconfig.c for frugalwareutils
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
#include <libfwraidconfig.h>
#include <setup.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

static
char *ask_devname()
{
	char *ptr, *basename = fwraid_suggest_devname();

	ptr = fwdialog_ask(_("Select RAID device"),
		_("Please specify the raid device you want to create:"),
		basename);
	free(basename);
	return(ptr);
}

static
int ask_level()
{
	int ret;
	char *fwraid_levels[] =
	{
		"0", _("raid0 (stripe)"),
		"1", _("raid1 (mirror)"),
		"4", "raid4",
		"5", "raid5",
		"6", "raid6",
		"10", "raid10"
	};

	char *ptr = fwdialog_menu(_("Choose a RAID level"),
		_("Please choose what level of raid array you want to build"),
		0, 0, 0, 6, fwraid_levels);
	ret = atoi(ptr);
	free(ptr);
	return(ret);
}

static
GList *add_devices()
{
	GList *partlist = fwraid_lst_parts();
	char **partarray = fwdialog_glist(partlist);
	GList *devlist=NULL;
	char *ptr, *sptr, *dptr;
	int ret;
	char buf[MAX_LEN + 1] = "";

	dialog_vars.ok_label = strdup(_("Add"));
	dialog_vars.cancel_label = strdup(_("Finish"));

	while(1)
	{
		if(!g_list_length(devlist))
			sptr = strdup(_("The list is currently empty."));
		else
		{
			ptr = fwutil_glist_display(devlist, " ");
			sptr = g_strdup_printf(_("The current list contains: %s."), ptr);
			free(ptr);
		}

		dptr = g_strdup_printf(_("Please select the devices you want to build "
			"the array from. When you are ready, press 'Finish'.\n%s"),
			sptr);
		free(sptr);

		dialog_vars.input_result = buf;
		dlg_put_backtitle();
		dlg_clear();
		ret = dialog_menu(_("Selecting devices"), dptr, 0, 0, 0,
			g_list_length(partlist)/2, partarray);
		free(dptr);
		if (ret == DLG_EXIT_CANCEL)
			break;
		devlist = g_list_append(devlist, strdup(buf));
		buf[0]='\0';
	}
	FWUTIL_FREE(dialog_vars.ok_label);
	FWUTIL_FREE(dialog_vars.cancel_label);
	return(devlist);
}

static
int run(int argc, char **argv)
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	char *devname;
	int level;
	GList *devlist;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "--help"))
		{
			system("man raidconfig");
			return(0);
		}
		else if(!strcmp(argv[1], "--version"))
		{
			printf("%s %s\n", argv[0], VERSION);
			return(0);
		}
	}

	fwutil_i18ninit(__FILE__);
	if(argv!=NULL)
		init_dialog(input, dialog_state.output);
	fwdialog_backtitle(_("RAID configuration"));

	while(1)
	{
		devname = ask_devname();
		if(devname && !strncmp(devname,"/dev/",5))
			break;
	}
	level = ask_level();
	fwraid_mknod_md(devname);
	devlist = add_devices();
	fwraid_create_md(devname, level, devlist);

	free(devname);
	g_list_free(devlist);
	if(argv!=NULL)
		end_dialog();
	return(0);
}

static
plugin_t plugin =
{
	"raidconfig",
	"RAID configuration",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
