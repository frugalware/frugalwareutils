/*
 *  netconfig.c for frugalwareutils
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
#include <net/if.h>
#include <glib.h>
#include <libfwdialog.h>
#include <libfwutil.h>
#include <libfwnetconfig.h>
#include <setup.h>
#include <getopt.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

extern int fwutil_dryrun;

int nco_usage   = 0;
int nco_fast   = 0;

fwnet_profile_t *sigprof;

int usage(const char *myname)
{
	printf(_("usage: %s [options] start|stop|restart|status|list\n"), myname);
	printf(_("       %s [options] [profile]\n"), myname);
	printf(_("-h | --help              This help.\n"));
	printf(_("-f | --fast              Fast mode, used by the setup.\n"));
	printf(_("                         SIGHUP will unload and reload the current profile.\n"));
	printf(_("     --dry-run           Do not actually perform the operation.\n"));
	return(0);
}

char *selnettype()
{
	int typenum=4;
	char *types[] =
	{
		"dhcp", _("Use a DHCP server"),
		"static", _("Use a static IP address"),
		"dsl", _("DSL connection"),
		"lo", _("No network - loopback connection only")
	};
	return(fwdialog_menu(_("Select network connection type"),
		_("Now we need to know how your machine connects to the network.\n"
		"If you have an internal network card and an assigned IP address, gateway, and DNS, use 'static' "
		"to enter these values. You may also use this if you have a DSL connection.\n"
		"If your IP address is assigned by a DHCP server (commonly used by cable modem services), select 'dhcp'. \n"
		"If you just have a network card to connect directly to a DSL modem, then select 'dsl'. \n"
		"Finally, if you do not have a network card, select the 'lo' choice. \n"),
		0, 0, 0, typenum, types));
}

int dsl_hook(fwnet_profile_t *profile, int confirm)
{
	struct stat buf;
	char *iface, *uname, *pass1, *pass2;

	// do we have pppoe?
	if(stat("/usr/sbin/pppoe", &buf))
		return(0);
	if(confirm && !fwdialog_yesno(_("DSL configuration"), _("Do you want to configure a DSL connetion now?")))
		return(0);
	uname = fwdialog_ask(_("Enter user name"),
			_("Enter your PPPoE user name:"), NULL);
	snprintf(profile->adsl_username, PATH_MAX, uname);
	while(1)
	{
		pass1 = fwdialog_password(_("Password"),
				_("Please enter your PPPoE password"));
		pass2 = fwdialog_password(_("Password"),
				_("Please re-enter your PPPoE password"));
		if(!strcmp(pass1, pass2))
		{
			snprintf(profile->adsl_password, PATH_MAX, pass1);
			break;
		}
		if(!fwdialog_yesno(_("Passwords don't match"),
					_("Sorry, the passwords do not match. Try again?")))
			return(1);
	}
	iface = fwdialog_ask(_("Enter interface name"),
			_("Enter the Ethernet interface connected to the DSL modem. It will be ethn, "
				"where 'n' is a number.\n"
				"If unsure, just hit enter.\n"
				"Enter interface name:"), "eth0");
	snprintf(profile->adsl_interface, IF_NAMESIZE, iface);
	if(nco_fast)
	{
		fwutil_system("mkdir /var/run");
		fwutil_system("mount -t devpts none /dev/pts");
		fwutil_system("pppoe-connect >/dev/tty4 2>/dev/tty4 &");
		return(0);
	}
	return(0);
}

int dialog_config(int argc, char **argv)
{
	FILE *input = stdin;
	fwnet_profile_t *newprofile=NULL;
	fwnet_interface_t *newinterface = NULL;
	char option[50];
	char *ptr;
	char *host, *nettype;
	char *ipaddr=NULL, *netmask=NULL, *dns=NULL, *iface=NULL;

	dialog_state.output = stderr;
	if(argv!=NULL)
		init_dialog(input, dialog_state.output);
	fwdialog_backtitle(_("Network configuration"));

	if(!nco_fast)
		host = fwdialog_ask(_("Enter hostname"), _("We'll need the name you'd like to give your host.\n"
		"The full hostname is needed, such as:\n\n"
		"frugalware.example.net\n\n"
		"Enter full hostname:"), "frugalware.example.net");
	else
		host = strdup("frugalware.example.net");
	nettype = selnettype();

	if((newprofile = (fwnet_profile_t*)malloc(sizeof(fwnet_profile_t))) == NULL)
		return(1);
	memset(newprofile, 0, sizeof(fwnet_profile_t));
	// TODO: here the profile name ('default') is hardwired
	sprintf(newprofile->name, "default");
	if((newinterface = (fwnet_interface_t*)malloc(sizeof(fwnet_interface_t))) == NULL)
		return(1);
	memset(newinterface, 0, sizeof(fwnet_interface_t));
	if(strcmp(nettype, "lo"))
	{
		iface = fwdialog_ask(_("Enter interface name"),
		_("We'll need the name of the interface you'd like to use for your network connection.\n"
		"If unsure, just hit enter.\n"
		"Enter interface name:"), "eth0");
		snprintf(newinterface->name, IF_NAMESIZE, iface);
		newprofile->interfaces = g_list_append(newprofile->interfaces, newinterface);
	}

	if(strcmp(nettype, "lo") && fwnet_is_wireless_device(iface))
	{
		ptr = fwdialog_ask(_("Extended network name"), _("It seems that this network card has a wireless "
			"extension. In order to use it, you must set your extended netwok name (ESSID). Enter your ESSID:"),
			NULL);
		snprintf(newinterface->essid, FWNET_ESSID_MAX_SIZE, ptr);
		FWUTIL_FREE(ptr);
		ptr = fwdialog_ask(_("Encryption key"), _("If you have an encryption key, then please enter it below.\n"
			"Examples: 4567-89AB-CD or  s:password"), NULL);
		snprintf(newinterface->key, FWNET_ENCODING_TOKEN_MAX, ptr);
		FWUTIL_FREE(ptr);
	}
	if(!strcmp(nettype, "dhcp"))
	{
		ptr = fwdialog_ask(_("Set DHCP hostname"), _("Some network providers require that the DHCP hostname be"
			"set in order to connect. If so, they'll have assigned a hostname to your machine. If you were"
			"assigned a DHCP hostname, please enter it below. If you do not have a DHCP hostname, just"
			"hit enter."), NULL);
		if(strlen(ptr))
			snprintf(newinterface->dhcp_opts, PATH_MAX, "-t 10 -h %s\n", ptr);
		else
			newinterface->dhcp_opts[0]='\0';
		FWUTIL_FREE(ptr);
		sprintf(option, "dhcp");
		newinterface->options = g_list_append(newinterface->options, option);
	}
	else if(!strcmp(nettype, "static"))
	{
		// options = ip netmask netmask
		ipaddr = fwdialog_ask(_("Enter ip address"), _("Enter your IP address for the local machine."), NULL);
		netmask = fwdialog_ask(_("Enter netmask for local network"),
			_("Enter your netmask. This will generally look something like this: 255.255.255.0\n"
			"If unsure, just hit enter."), "255.255.255.0");
		if(strlen(ipaddr))
			snprintf(option, 49, "%s netmask %s", ipaddr, netmask);
		newinterface->options = g_list_append(newinterface->options, option);
		FWUTIL_FREE(ipaddr);
		FWUTIL_FREE(netmask);
		ptr = fwdialog_ask(_("Enter gateway address"), _("Enter the address for the gateway on your network. "
			"If you don't have a gateway on your network just hit enter, without entering any ip address."),
			NULL);
		if(strlen(ptr))
			snprintf(newinterface->gateway, FWNET_GW_MAX_SIZE, "default gw %s", ptr);
		FWUTIL_FREE(ptr);
		dns = fwdialog_ask(_("Select nameserver"), _("Please give the IP address of the name server to use. You can"
			"add more Domain Name Servers later by editing /etc/sysconfig/network/default.\n"
			"If you don't have a name server on your network just hit enter, without entering any ip address."),
			NULL);
		newprofile->dnses = g_list_append(newprofile->dnses, dns);
	}
	if(!strcmp(nettype, "static"))
		dsl_hook(newprofile, 1);
	if(!strcmp(nettype, "dsl"))
		dsl_hook(newprofile, 0);

	if(fwdialog_yesno(_("Adjust configuration files"), _("Accept these settings and adjust configuration files?"))
		&& !fwutil_dryrun)
		fwnet_writeconfig(newprofile, host);

	g_list_free(newinterface->options);
	FWUTIL_FREE(newinterface);
	g_list_free(newprofile->interfaces);
	FWUTIL_FREE(newprofile);
	FWUTIL_FREE(host);
	FWUTIL_FREE(nettype);
	FWUTIL_FREE(ipaddr);
	FWUTIL_FREE(netmask);
	FWUTIL_FREE(dns);
	if(argv!=NULL)
		end_dialog();
	return(0);
}

int run(int argc, char **argv)
{
	int opt;
	int option_index;
	static struct option opts[] =
	{
		{"help",           no_argument,       0, 'h'},
		{"fast",           no_argument,       0, 'f'},
		{"dry-run",        no_argument,       0, 1000},
		{0, 0, 0, 0}
	};
	char *fn=NULL;
	int ret=0, i;
	fwnet_profile_t *profile;

	while((opt = getopt_long(argc, argv, "hfl", opts, &option_index)))
	{
		if(opt < 0)
			break;
		switch(opt)
		{
			case 1000: fwutil_dryrun = 1; break;
			case 'h':  nco_usage  = 1; break;
			case 'f':  nco_fast   = 1; break;
		}
	}
	fwutil_i18ninit(__FILE__);
	if(nco_usage)
	{
		usage(argv[0]);
		return(0);
	}

	if(optind < argc)
	{
		if(!strcmp("list", argv[optind]))
		{
			fwnet_listprofiles();
			return(0);
		}
		if((fn=fwnet_lastprofile()) || !strcmp("stop", argv[optind]) || !strcmp("status", argv[optind]))
		{
			if((!strcmp("stop", argv[optind])) && !fn)
				return(127);
			if((!strcmp("status", argv[optind])) && !fn)
			{
				printf(_("No profile loaded.\n"));
				return(1);
			}
			else if ((!strcmp("status", argv[optind])) && fn)
			{
				printf(_("Current profile: %s\n"), fn);
				return(0);
			}
			profile = fwnet_parseprofile(fn);
			if(profile!=NULL)
				// unload the old profile
				for (i=0; i<g_list_length(profile->interfaces); i++)
					fwnet_ifdown((fwnet_interface_t*)g_list_nth_data(profile->interfaces, i), profile);
			if(!strcmp("stop", argv[optind]))
			{
				fwnet_lodown();
				if(!fwutil_dryrun)
					fwnet_setlastprofile(NULL);
				return(0);
			}
		}
		// load the default for 'start' and for 'restart' if not yet started
		if(!strcmp("start", argv[optind]) || (!strcmp("restart", argv[optind]) && !fn))
			fn = strdup("default");
		// load the target profile if != 'restart'
		else if (strcmp("restart", argv[optind]))
			fn = strdup(argv[optind]);
		// load the new profile
		profile = fwnet_parseprofile(fn);
		if(profile==NULL)
			return(1);
		if(!fwnet_lastprofile())
			fwnet_loup();
		for (i=0; i<g_list_length(profile->interfaces); i++)
			ret += fwnet_ifup((fwnet_interface_t*)g_list_nth_data(profile->interfaces, i), profile);
		fwnet_setdns(profile);
		if(!fwutil_dryrun)
			fwnet_setlastprofile(fn);
		FWUTIL_FREE(fn);
	}
	else
		dialog_config(argc, argv);
	return(ret);
}

plugin_t plugin =
{
	"netconfig",
	"Network configuration",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}
