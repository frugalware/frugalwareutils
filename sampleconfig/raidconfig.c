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

char* ask_mode()
{
	char *modes[] = 
	{
		_("no"), _("no, i don't want to do so"),
		_("yes"), _("yup! :)")
	};
	char *ptr;

	ptr = dialog_mymenu(_("question?"),
		_("loooooooong description"), 0, 0, 0, 2, modes);
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

int run(int argc, char **argv)
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	char *ptr;

	i18ninit(__FILE__);
	init_dialog(input, dialog_state.output);
	dialog_backtitle(_("RAID configuration"));

	ptr = ask_mode();
	free(ptr);

	if(argv!=NULL)
		end_dialog();
	return(0);
}

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
