/*
 *  yabootcfg.c for frugalwareutils
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

#include <stdio.h>
#include <glib.h>
#include <libfwdialog.h>
#include <libfwutil.h>
#include <libfwyabootcfg.h>
#include <setup.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

int ask_flags()
{
	char *flags[] = 
	{
		"O", _("Include OpenFirmware option"), "On",
		"N", _("Include Network boot option"), "On",
		"C", _("Include CD Boot option"), "On"
	};
	int i, ret = 0;
	GList *items;

	items = fwdialog_checklist(_("Installing Yaboot bootloader"),
			_("You can now configure some options, regarding the installation of the Yaboot bootloader"),
			0, 0, 0, 3, flags, FLAG_CHECK);
	for (i = 0; i < g_list_length(items); i++) {
		char *item = g_list_nth_data(items, i);
		switch (item[0]) {
			case 'O':
				ret |= FWYABOOT_OFBOOT;
				break;
			case 'N':
				ret |= FWYABOOT_NETBOOT;
				break;
			case 'C':
				ret |= FWYABOOT_CDBOOT;
				break;
			default:
				return 0;
		}
	}
	return ret;
}

int run(int argc, char **argv)
{
	FILE *input = stdin, *fp;
	dialog_state.output = stderr;
	int flags, needrelease;
	struct stat buf;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "--help"))
		{
			system("man yabootcfg");
			return(0);
		}
		else if(!strcmp(argv[1], "--version"))
		{
			printf("%s %s\n", argv[0], VERSION);
			return(0);
		}
	}

	fwutil_i18ninit(__FILE__);
	init_dialog(input, dialog_state.output);
	fwdialog_backtitle(_("Yaboot bootloader"));

	flags = ask_flags();
	dialog_msgbox(_("Please wait"), _("Attempting to install the Yaboot bootloader..."), 0, 0, 0);
	needrelease = fwutil_init();
	// backup the old config if there is any
	if(!stat("/etc/yaboot.conf", &buf))
		rename("/etc/yaboot.conf", "/etc/yaboot.conf.old");
	fp = fopen("/etc/yaboot.conf", "w");
	if(fp)
	{
		fwyaboot_create_menu(fp, flags);
		fclose(fp);
	}
	fwyaboot_install();
	if(needrelease)
		fwutil_release();

	if(argv!=NULL)
		end_dialog();
	return(0);
}

plugin_t plugin =
{
	"yabootcfg",
	"Yaboot bootloader",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
