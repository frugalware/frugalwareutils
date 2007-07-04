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

int ask_mode()
{
	char *modes[] = 
	{
		_("MBR"), _("Install to Master Boot Record"),
		_("Floppy"), _("Install to a formatted floppy in /dev/fd0"),
		// see see http://lists.gnu.org/archive/html/bug-grub/2003-11/msg00085.html
		_("Root"), _("Install to superblock (do NOT use with XFS)"),
		_("Skip"), _("Skip the installation of GRUB.")
	};
	char *ptr;
	int ret=0;

	ptr = fwdialog_menu(_("Installing GRUB bootloader"),
		_("GRUB can be installed to a variety of places:\n\n"
		"\t1. The Master Boot Record of your first hard drive.\n"
		"\t2. A formatted floppy disk.\n"
		"\t3. The superblock of your root Linux partition.\n\n"
		"Option 3 requires setting the partition bootable with (c)fdisk\n"
		"Hint: Choose option 3 if you already have a boot manager installed.\n"
		"Which option would you like?"), 0, 0, 0, 4, modes);
	if(!strcmp(ptr, _("MBR")))
		ret=0;
	if(!strcmp(ptr, _("Floppy")))
		ret=1;
	if(!strcmp(ptr, _("Root")))
		ret=2;
	if(!strcmp(ptr, _("Skip")))
		ret=3;
	free(ptr);
	return(ret);
}

int run(int argc, char **argv)
{
	FILE *input = stdin, *fp;
	dialog_state.output = stderr;
	int mode, needrelease;
	struct stat buf;

	fwutil_i18ninit(__FILE__);
	init_dialog(input, dialog_state.output);
	fwdialog_backtitle(_("GRUB bootloader"));

	mode = ask_mode();
	if(mode!=3)
	{
		dialog_msgbox(_("Please wait"), _("Attempting to install the GRUB bootloader..."), 0, 0, 0);
		needrelease = fwutil_init();
		fwgrub_install(mode);
		// backup the old config if there is any
		if(!stat("/boot/grub/menu.lst", &buf))
			rename("/boot/grub/menu.lst", "/boot/grub/menu.lst.old");
		fp = fopen("/boot/grub/menu.lst", "w");
		if(fp)
		{
			fwgrub_create_menu(fp);
			fclose(fp);
		}
		if(needrelease)
			fwutil_release();
	}

	if(argv!=NULL)
		end_dialog();
	return(0);
}

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
