/*
 *  xwmconfig.c for frugalwareutils
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
#include <pacman.h>
#include <libfwdialog.h>
#include <libfwutil.h>
#include <libfwxwmconfig.h>
#include <setup.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>
#include <getopt.h>

int xwm_force=0;
int xwm_silent=0;

char* ask_wm(PM_DB *db)
{
	GList *list = fwxwm_list(db);
	char **dlist = fwdialog_glist(list);
	char *ptr;

	ptr = fwdialog_menu(_("Window managers"),
		_("Choose one from the window managers listed below. Window "
		"managers are responsible for the desktop layout."), 0, 0, 0, g_list_length(list)/2, dlist);
	free(dlist);
	fwxwm_release(db);
	return(ptr);
}

int run(int argc, char **argv)
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	char *ptr;
	unsigned int ret;
	struct stat buf;
	PM_DB *db;
	int opt;
	int option_index;
	struct option opts[] =
	{
		{"force",        no_argument,       0, 1000},
		{"help",         no_argument,       0, 1001},
		{"silent",       no_argument,       0, 1002},
		{"version",      no_argument,       0, 1003},
		{0, 0, 0, 0}
	};

	fwutil_i18ninit(__FILE__);

	while((opt = getopt_long(argc, argv, "", opts, &option_index)))
	{
		if(opt < 0)
			break;
		switch(opt)
		{
			case 1000: xwm_force = 1; break;
			case 1001: system("man xwmconfig"); return(0);
			case 1002: xwm_silent = 1; break;
			case 1003: printf("%s %s\n", argv[0], VERSION); return(0);
		}
	}

	init_dialog(input, dialog_state.output);
	fwdialog_backtitle(_("XDM configuration"));
	dialog_msgbox(_("Please wait"), _("Searching for desktop managers..."), 0, 0, 0);
	db = fwxwm_init();

	if(!xwm_force && (ret=fwxwm_checkdms(db)))
	{
		if(!xwm_silent)
		{
			if(ret & FWXWM_KDM)
				dialog_msgbox(_("KDM found"), _("KDM is installed, "
					"you can choose a window manager from the login menu."), 0, 0, 1);
			if(ret & FWXWM_GDM)
				dialog_msgbox(_("GDM found"), _("GDM is installed, "
					"you can choose a window manager from the login menu."), 0, 0, 1);
		}
		if(argv!=NULL)
			end_dialog();
		return(0);
	}

	if(stat("/etc/sysconfig/desktop", &buf))
	{
		dialog_msgbox(_("No X server found"), _("The X server is not "
			"installed, please issue a 'pacman -S x11' command as root."), 0, 0, 1);
		if(argv!=NULL)
			end_dialog();
		return(0);
	}

	ptr = ask_wm(db);
	fwxwm_set(ptr);
	free(ptr);

	if(argv!=NULL)
		end_dialog();
	return(0);
}

plugin_t plugin =
{
	"xwmconfig",
	"XDM configuration",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
