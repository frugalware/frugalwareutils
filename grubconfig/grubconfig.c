/*
 *  grubconfig.c for frugalwareutils
 *
 *  Copyright (c) 2003-2006 by Miklos Vajna <vmiklos@frugalware.org>
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
#include <libfwgrubconfig.h>
#include <setup.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

static
enum fwgrub_install_mode ask_mode()
{
	char *modes[] =
	{
		_("MBR"), _("Install to Master Boot Record of root hard drive"),
		_("Skip"), _("Skip the installation of GRUB.")
	};
	char *ptr;
	enum fwgrub_install_mode ret=-1;

	ptr = fwdialog_menu(_("Installing GRUB bootloader"),
		_("GRUB can be installed to a variety of places:\n\n"
		"\t1. The Master Boot Record of your root hard drive.\n"
		"Which option would you like?"), 0, 0, 0, 2, modes);
	if(!strcmp(ptr, _("Root")))
		ret=FWGRUB_INSTALL_MBR;
	else if(!strcmp(ptr, _("Skip")))
		ret=-1;
	free(ptr);
	return(ret);
}

static
int run(int argc, char **argv)
{
	FILE *input = stdin, *fp;
	dialog_state.output = stderr;
	struct stat buf;
	enum fwgrub_install_mode mode;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "--help"))
		{
			system("man grubconfig");
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
	fwdialog_backtitle(_("GRUB bootloader"));

	mode = ask_mode();
	if(mode!=-1)
	{
		dialog_msgbox(_("Please wait"), _("Attempting to install the GRUB bootloader..."), 0, 0, 0);
		fwgrub_install("/",mode);
		fwgrub_make_config();
	}

	if(argv!=NULL)
		end_dialog();
	return(0);
}

static
plugin_t plugin =
{
	"grubconfig",
	"GRUB bootloader",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
