/*
 *  xconfig.c for frugalwareutils
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
#include <libfwxconfig.h>
#include <setup.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libintl.h>

int run()
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	char *mdev, *res, *depth;
	struct stat buf;
	int needrelease, ret;

	i18ninit(__FILE__);
	init_dialog(input, dialog_state.output);
	dialog_backtitle(_("X configuration"));

	// sanility checks
	if(stat("/usr/bin/xinit", &buf))
	{
		dialog_msgbox(_("xinit missing"), _("Could not find xinit, please install "
			"it with pacman -S xinit."), 0, 0, 1);
		return(1);
	}

	if(stat("/usr/bin/xmessage", &buf))
	{
		dialog_msgbox(_("xmessage missing"), _("Could not find xmessage, please install "
			"it with pacman -S xmessage."), 0, 0, 1);
		return(1);
	}

	if(stat("/usr/bin/xsetroot", &buf))
	{
		dialog_msgbox(_("xsetroot missing"), _("Could not find xsetroot, please install "
			"it with pacman -S xsetroot."), 0, 0, 1);
		return(1);
	}

	dialog_msgbox(_("Configuring the X server"), _("Attemping to create "
		"an X config file..."), 0, 0, 0);
	needrelease = fwutil_init();
	mdev = fwx_get_mousedev();

	fwx_doprobe();
	while(1)
	{
		res = dialog_ask(_("Selecting resolution"),
			_("Please enter the screen resolution you want to use. "
			"You can use values such as 1024x768, 800x600 or 640x480. If unsure, just press ENTER."),
			"1024x768");
		depth = dialog_ask(_("Selecting color depth"),
			_("Please enter the color depth you want to use. If unsure, just press ENTER."),
			"24");
		fwx_doconfig(mdev, res, depth);
		end_dialog();
		ret = fwx_dotest();
		init_dialog(input, dialog_state.output);
		dialog_backtitle(_("X configuration"));
		if(!ret)
			break;
	}
	unlink("/root/xorg.conf.new");

	if(needrelease)
		fwutil_release();
	end_dialog();
	return(0);
}

plugin_t plugin =
{
	"xconfig",
	"X configuration",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
