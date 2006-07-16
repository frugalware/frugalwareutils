/*
 *  mouseconfig.c for frugalwareutils
 * 
 *  Copyright (c) 2006 by Miklos Vajna <vmiklos@frugalware.org>
 *
 *  Several ideas from Patrick J. Volkerding's mouseconfig script
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
#include <libfwmouseconfig.h>
#include <setup.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

char* ask_mouse()
{
	char *modes[] = 
	{
		"ps2", _("PS/2 port mouse (most desktops and laptops)"),
		"imps2", _("Microsoft PS/2 Intellimouse"),
		"bare", _("2 button Microsoft compatible serial mouse"),
		"ms", _("3 button Microsoft compatible serial mouse"),
		"mman", _("Logitech serial MouseMan and similar devices"),
		"msc", _("MouseSystems serial (most 3 button serial mice)"),
		"pnp", _("Plug and Play (serial mice that do not work with ms)"),
		"usb", _("USB connected mouse"),
		"ms3", _("Microsoft serial Intellimouse"),
		"netmouse", _("Genius Netmouse on PS/2 port"),
		"logi", _("Some serial Logitech devices"),
		"logim", _("Make serial Logitech behave like msc"),
		"atibm", _("ATI XL busmouse (mouse card)"),
		"inportbm", _("Microsoft busmouse (mouse card)"),
		"logibm", _("Logitech busmouse (mouse card)"),
		"ncr", _("A pointing pen (NCR3125) on some laptops"),
		"twid", _("Twiddler keyboard, by HandyKey Corp"),
		"genitizer", _("Genitizer tablet (relative mode)"),
		"js", _("Use a joystick as a mouse"),
		"wacom", _("Wacom serial graphics tablet")
	};
	char *ptr;

	ptr = dialog_mymenu(_("Configure the console mouse support (GPM)"),
		_("This part of the configuration process will create a /dev/mouse link pointing to your default mouse "
		"device.  You can change the /dev/mouse link later in the /etc/sysconfig/gpm configuration file if the "
		"mouse doesn't work, or if you switch to a different type of pointing device. We will also use the "
		"information about the mouse to set the correct protocol for gpm, the Linux mouse server. Please select "
		"a mouse type from the list below:"), 0, 0, 0, 20, modes);
	return(ptr);
}

char* ask_port()
{
	char *ports[] = 
	{
		"/dev/ttyS0", _("(COM1: under DOS)"),
		"/dev/ttyS1", _("(COM2: under DOS)"),
		"/dev/ttyS2", _("(COM3: under DOS)"),
		"/dev/ttyS3", _("(COM4: under DOS)")
	};
	char *ptr;

	ptr = dialog_mymenu(_("Select Serial Port"),
		_("Your mouse requires a serial port. Which one would you like to use?"),
		0, 0, 0, 4, ports);
	return(ptr);
}

int run(int argc, char **argv)
{
	FILE *input = stdin;
	dialog_state.output = stderr;

	char *mouse_type=NULL, *mtype=NULL, *link=NULL;

	fwutil_i18ninit(__FILE__);
	if(argv!=NULL)
		init_dialog(input, dialog_state.output);
	dialog_backtitle(_("Mouse configuration"));

	if(fwmouse_detect_usb())
	{
		mtype=strdup("imps2");
		link=strdup("input/mice");
	}
	else
	{
		mouse_type = ask_mouse();
		if(!strcmp(mouse_type, "bar") || !strcmp(mouse_type, "ms") || !strcmp(mouse_type, "mman") ||
			!strcmp(mouse_type, "msc") || !strcmp(mouse_type, "genitizer") || !strcmp(mouse_type, "pnp") ||
			!strcmp(mouse_type, "ms3") || !strcmp(mouse_type, "logi") || !strcmp(mouse_type, "logim") ||
			!strcmp(mouse_type, "wacom") || !strcmp(mouse_type, "twid"))
		{
			link=ask_port();
			mtype=mouse_type;
			mouse_type=NULL;
		}
		else if(!strcmp(mouse_type, "ps2"))
		{
			link = strdup("psaux");
			mtype = strdup("ps2");
		}
		else if(!strcmp(mouse_type, "ncr"))
		{
			link = strdup("psaux");
			mtype = strdup("ncr");
		}
		else if(!strcmp(mouse_type, "imps2"))
		{
			link = strdup("psaux");
			mtype = strdup("imps2");
		}
		else if(!strcmp(mouse_type, "logibm"))
		{
			link = strdup("logibm");
			mtype = strdup("ps2");
		}
		else if(!strcmp(mouse_type, "atibm"))
		{
			link = strdup("atibm");
			mtype = strdup("ps2");
		}
		else if(!strcmp(mouse_type, "inportbm"))
		{
			link = strdup("inportbm");
			mtype = strdup("bm");
		}
		else if(!strcmp(mouse_type, "js"))
		{
			link = strdup("js0");
			mtype = strdup("js");
		}
		else
		{
			// usb
			link = strdup("input/mice");
			mtype = strdup("imps2");
		}
	}
	fwmouse_writeconfig(link, mtype);

	FREE(mouse_type);
	FREE(mtype);
	FREE(link);

	// deinit dialog if we're called directly, not via setup
	if(argv!=NULL)
		end_dialog();
	return(0);
}

plugin_t plugin =
{
	"mouseconfig",
	"Mouse configuration",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
