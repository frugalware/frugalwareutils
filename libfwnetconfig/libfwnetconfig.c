/*
 *  libfwnetconfig.c for frugalwareutils
 *
 *  useful functions for network configuration
 * 
 *  Copyright (c) 2006, 2007, 2008, 2009, 2011 by Miklos Vajna <vmiklos@frugalware.org>
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
#define FWUTIL_GETTEXT "libfwnetconfig"
#include <libfwutil.h>
#include <getopt.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <libintl.h>
#include <string.h>
#include <ctype.h>
#include <libudev.h>

#include "libfwnetconfig.h"

extern int fwutil_dryrun;

/** @defgroup libfwnetconfig Frugalware Network Configuration library
 * @brief Functions to make network configuration easier
 * @{
 */

/** Prints a list of profiles available.
 * @return 1 on failure, 0 on success
 */
int fwnet_listprofiles(void)
{
	struct dirent *ent=NULL;
	DIR *dir;

	dir = opendir(FWNET_PATH);
	while((ent = readdir(dir)))
	{
		if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
			printf("%s\n", ent->d_name);
	}
	return(0);
}

/** Parses a profile. Based on pacman's config parser, which is
 * Copyright (c) 2002-2006 by Judd Vinet <jvinet@zeroflux.org>
 * @param fn pathname of the profile
 * @return the parsed profile
 */
fwnet_profile_t *fwnet_parseprofile(char *fn)
{
	FILE *fp;
	char line[PATH_MAX+1];
	int i, n=0;
	char *ptr = NULL;
	char *var = NULL;
	char interface[256] = "";
	fwnet_profile_t *profile;
	fwnet_interface_t *iface=NULL;

	profile = (fwnet_profile_t*)malloc(sizeof(fwnet_profile_t));
	if(profile==NULL)
		return(NULL);
	memset(profile, 0, sizeof(fwnet_profile_t));

	ptr = g_strdup_printf(FWNET_PATH "/%s", fn);
	fp = fopen(ptr, "r");
	if(fp == NULL)
	{
		printf(_("%s: No such profile!\n"), fn);
		FWUTIL_FREE (profile);
		return(NULL);
	}
	FWUTIL_FREE(ptr);
	sprintf (profile->name, "%s", fn);

	while(fgets(line, PATH_MAX, fp))
	{
		n++;
		fwutil_trim(line);
		if(strlen(line) == 0 || line[0] == '#')
			continue;
		if(line[0] == '[' && line[strlen(line)-1] == ']')
		{
			// new interface
			ptr = line;
			ptr++;
			strncpy(interface, ptr, fwutil_min(255, strlen(ptr)-1));
			interface[fwutil_min(255, strlen(ptr)-1)] = '\0';
			if(!strlen(interface))
			{
				fprintf(stderr, _("profile: line %d: bad interface name\n"), n);
				FWUTIL_FREE (profile);
				return(NULL);
			}
			if(strcmp(interface, "options"))
			{
				int found = 0;
				for (i=0; !found && i<g_list_length(profile->interfaces); i++)
				{
					iface = (fwnet_interface_t*)g_list_nth_data(profile->interfaces, i);
					if(!strcmp(iface->name, interface))
						found=1;
				}
				if(!found)
				{
					// start a new interface record
					iface = (fwnet_interface_t*)malloc(sizeof(fwnet_interface_t));
					if(iface==NULL)
					{
						FWUTIL_FREE (profile);
						return(NULL);
					}
					memset(iface, 0, sizeof(fwnet_interface_t));
					strncpy(iface->name, interface, IF_NAMESIZE);
					profile->interfaces = g_list_append(profile->interfaces, iface);
				}
			}
		}
		else
		{
			// directive
			ptr = line;
			var = strsep(&ptr, "=");
			if(var == NULL)
			{
				fprintf(stderr, _("profile: line %d: syntax error\n"), n);
				FWUTIL_FREE (profile);
				return(NULL);
			}
			fwutil_trim(var);
			var = fwutil_strtoupper(var);
			if(!strlen(interface))
			{
				fprintf(stderr, _("profile: line %d: all directives must belong to a section\n"), n);
				FWUTIL_FREE (profile);
				return(NULL);
			}
			if(ptr != NULL)
			{
				fwutil_trim(ptr);
				if (!strcmp(var, "DNS"))
					profile->dnses = g_list_append(profile->dnses, strdup(ptr));
				if (!strcmp(var, "DOMAIN") && !strlen(profile->domain))
					strncpy(profile->domain, ptr, PATH_MAX);
				if (!strcmp(var, "DESC") && !strlen(profile->desc))
					strncpy(profile->desc, ptr, PATH_MAX);
				if (!strcmp(var, "ADSL_USERNAME") && !strlen(profile->adsl_username))
					strncpy(profile->adsl_username, ptr, PATH_MAX);
				if (!strcmp(var, "ADSL_PASSWORD") && !strlen(profile->adsl_password))
					strncpy(profile->adsl_password, ptr, PATH_MAX);
				if (!strcmp(var, "ADSL_INTERFACE") && !strlen(profile->adsl_interface))
					strncpy(profile->adsl_interface, ptr, PATH_MAX);
				if (!strcmp(var, "OPTIONS"))
					iface->options = g_list_append(iface->options, strdup(ptr));
				if (!strcmp(var, "PRE_UP"))
					iface->pre_ups = g_list_append(iface->pre_ups, strdup(ptr));
				if (!strcmp(var, "POST_UP"))
					iface->post_ups = g_list_append(iface->post_ups, strdup(ptr));
				if (!strcmp(var, "PRE_DOWN"))
					iface->pre_downs = g_list_append(iface->pre_downs, strdup(ptr));
				if (!strcmp(var, "POST_DOWN"))
					iface->post_downs = g_list_append(iface->post_downs, strdup(ptr));
				if(!strcmp(var, "MAC") && !strlen(iface->mac))
					strncpy(iface->mac, ptr, FWNET_MAC_MAX_SIZE);
				if(!strcmp(var, "DHCP_OPTS") && !strlen(iface->dhcp_opts))
					strncpy(iface->dhcp_opts, ptr, PATH_MAX);
				if(!strcmp(var, "DHCPCLIENT") && !strlen(iface->dhcpclient))
					strncpy(iface->dhcpclient, ptr, PATH_MAX);
				if(!strcmp(var, "ESSID") && !strlen(iface->essid))
					strncpy(iface->essid, ptr, FWNET_ESSID_MAX_SIZE);
				if(!strcmp(var, "MODE") && !strlen(iface->mode))
					strncpy(iface->mode, ptr, FWNET_MODE_MAX_SIZE);
				if(!strcmp(var, "KEY") && !strlen(iface->key))
					strncpy(iface->key, ptr, FWNET_ENCODING_TOKEN_MAX);
				if(!strcmp(var, "SCAN_SSID") && !strlen(iface->key))
					iface->scan_ssid = (toupper(*ptr) == 'Y');
				if(!strcmp(var, "WPA_PSK") && !strlen(iface->wpa_psk))
					strncpy(iface->wpa_psk, ptr, PATH_MAX);
				if(!strcmp(var, "WPA_DRIVER") && !strlen(iface->wpa_driver))
					strncpy(iface->wpa_driver, ptr, PATH_MAX);
				if(!strcmp(var, "WPA_SUPPLICANT"))
					iface->wpa_supplicant = (toupper(*ptr) == 'Y');
				if(!strcmp(var, "GATEWAY") && !strlen(iface->gateway))
					strncpy(iface->gateway, ptr, FWNET_GW_MAX_SIZE);
			}
		}
		line[0] = '\0';
	}
	fclose(fp);
	return(profile);
}

/** Check if an interface contains "options = dhcp".
 * @param iface the interface struct pointer
 * @return 1 if true, 0 if false
 */
int fwnet_is_dhcp(fwnet_interface_t *iface)
{
	int i, dhcp=0;
	for (i=0; i<g_list_length(iface->options); i++)
		if(!strcmp((char*)g_list_nth_data(iface->options, i), "dhcp"))
			dhcp=1;
	return(dhcp);
}

/** Returns a list of interfaces available with descriptions.
 * @return NULL on failure, GList of fwnet_interface_desc_t on success
 */
GList *fwnet_iflist()
{
	GList *ret = NULL, *iflist = NULL;
	char desc[128];
	DIR *dir;
	struct dirent *ent;
	int i;

	// /sys/class/net/
	dir = opendir(FWNET_IFPATH);
	if(!dir)
		return NULL;
	while((ent = readdir(dir)))
	{
		if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "lo"))
			continue;
		iflist = g_list_append(iflist, g_strdup(ent->d_name));
	}
	closedir(dir);
	iflist = g_list_sort(iflist, (GCompareFunc)strcmp);
	for(i=0; i<g_list_length(iflist); i++)
	{
		if (fwnet_ifdesc(g_list_nth_data(iflist, i), desc, sizeof(desc)))
			snprintf (desc, sizeof(desc), _("Unknown device"));
		ret = g_list_append(ret, g_strdup(g_list_nth_data(iflist, i)));
		ret = g_list_append(ret, g_strdup(desc));
	}
	g_list_free(iflist);
	return ret;
}

/** Shut down an interface
 * @param iface the interface struct pointer
 * @return 1 on failure, 0 on success
 */
int fwnet_ifdown(fwnet_interface_t *iface, fwnet_profile_t *profile)
{
	int dhcp, ret=0, i;
	char *ptr;
	FILE *fp;

	if(g_list_length(iface->pre_downs))
		for (i=0; i<g_list_length(iface->pre_downs); i++)
			fwutil_system((char*)g_list_nth_data(iface->pre_downs, i));

	if(strlen(profile->adsl_interface) && !strcmp(profile->adsl_interface, iface->name) &&
		strlen(profile->adsl_username) && strlen(profile->adsl_password))
	{
		ret += fwutil_system("pppoe-stop");
	}
	dhcp = fwnet_is_dhcp(iface);
	if(dhcp)
	{
		char line[7];
		if (!strcmp("dhclient", iface->dhcpclient)) {
			ptr = g_strdup_printf("/var/run/dhclient-%s.pid", iface->name);
		} else
			ptr = g_strdup_printf("/var/run/dhcpcd-%s.pid", iface->name);
		fp = fopen(ptr, "r");
		FWUTIL_FREE(ptr);
		if(fp != NULL)
		{
			fgets(line, 6, fp);
			fclose(fp);
			i = atoi(line);
			if(i>0 && !fwutil_dryrun)
				ret = kill(i, 15);
			else if (i>0)
				printf("kill(%d, 15);\n", i);
			
			// dhclient and the latest dhcpcd do not bring the interface down fully
			ptr = g_strdup_printf("ifconfig %s down", iface->name);
			fwutil_system(ptr);
			FWUTIL_FREE(ptr);
		}
	}
	else
	{
		if(g_list_length(iface->options)>1)
			for (i=0; i<g_list_length(iface->options); i++)
			{
				ptr = g_strdup_printf("ifconfig %s 0.0.0.0", iface->name);
				fwutil_system(ptr);
				FWUTIL_FREE(ptr);
			}
		ptr = g_strdup_printf("ifconfig %s down", iface->name);
		fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}
	if(strlen(iface->wpa_psk) || iface->wpa_supplicant)
	{
		ptr = g_strdup("killall wpa_supplicant");
		fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}

	if(g_list_length(iface->post_downs))
		for (i=0; i<g_list_length(iface->post_downs); i++)
			fwutil_system((char*)g_list_nth_data(iface->post_downs, i));

	return(ret);
}

static int copyfile(char *from, char *to)
{
	int oldmask;
	FILE *ip, *op;
	size_t len;
	char buf[4097];

	oldmask = umask(0077);
	ip = fopen(from, "r");
	if(!ip)
		return(1);
	op = fopen(to, "w");
	if(!op)
		return(1);
	while((len = fread(buf, 1, 4096, ip)))
		fwrite(buf, 1, len, op);
	fclose(ip);
	fclose(op);
	umask(oldmask);
	return(0);
}

static int update_adsl_conf(char *iface, char *user)
{
	FILE *ip, *op;
	char line[256];

	copyfile("/etc/ppp/pppoe.conf", "/etc/ppp/pppoe.conf-bak");
	ip = fopen("/etc/ppp/pppoe.conf-bak", "r");
	if(!ip)
		return(1);
	op = fopen("/etc/ppp/pppoe.conf", "w");
	if(!op)
		return(1);
	while(fgets(line, 255, ip))
	{
		if(!strncmp(line, "ETH=", 4))
			fprintf(op, "ETH='%s'\n", iface);
		else if(!strncmp(line, "USER=", 5))
			fprintf(op, "USER='%s'\n", user);
		else
			fprintf(op, "%s", line);
	}
	fclose(ip);
	fclose(op);
	return(0);
}

static int update_secrets(char *path, char *user, char *pass)
{
	FILE *fp;
	int oldmask;

	oldmask = umask(0077);
	unlink(path);
	fp = fopen(path, "w");
	if(!fp)
		return(1);
	fprintf(fp, "\"%s\"\t*\t\"%s\"\n", user, pass);
	fclose(fp);
	umask(oldmask);
	return(0);
}

static int update_wpa_conf(char *ssid, int scan_ssid, char *psk)
{
	FILE *fp;

	fp = fopen("/etc/wpa_supplicant.conf", "w");
	if(!fp)
		return(1);
	fprintf(fp,
	        "# WARNING! Machine generated ephemeral file, do not edit!\n"
	        "# For a simple setup, edit instead\n"
	        "#   /etc/sysconfig/network/<your profile>.\n"
	        "# If you insist on managing this file by yourself, see\n"
	        "# netconfig(1) / EXAMPLES / Using wpa_supplicant.\n"
	        "\n"
	        "ctrl_interface=/var/run/wpa_supplicant\n"
	        "\n"
	        "network={\n\tssid=\"%s\"\n", ssid);
	if (scan_ssid)
		fprintf(fp, "\tscan_ssid=1\n");
	if (strlen(psk) == 64)
		// that's probably a psk hash, not a real key
		fprintf(fp, "\tpsk=%s\n", psk);
	else
		fprintf(fp, "\tpsk=\"%s\"\n", psk);
	fprintf(fp, "}\n");
	fclose(fp);
	return(0);
}

static int check_devpath(const char *path, char *iface)
{
	int ret = 0;
	char *devpath = strdup(path);
	char *ptr = strrchr(devpath, '/');

	if (ptr && strcmp(iface, ++ptr) == 0)
		ret = 1;
	free(devpath);
	return ret;
}

static int wait_interface(char *iface)
{
	struct udev *udev = NULL;
	struct udev_monitor *udev_monitor = NULL;
	struct udev_enumerate *udev_enumerate = NULL;
	struct udev_list_entry *item = NULL, *first = NULL;
	int rc = -1;

	udev = udev_new();
	if (!udev) {
		fprintf(stderr, "unable to create udev context");
		goto finish;
	}

	/* subscribe to udev events */
	udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
	if (!udev_monitor) {
		fprintf(stderr, "unable to create netlink socket\n");
		goto finish;
	}
	udev_monitor_set_receive_buffer_size(udev_monitor, 128*1024*1024);
	if (udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "net", NULL) < 0) {
		fprintf(stderr, "unable to add matching subsystem to monitor\n");
		goto finish;
	}
	if (udev_monitor_enable_receiving(udev_monitor) < 0) {
		fprintf(stderr, "unable to subscribe to udev events\n");
		goto finish;
	}

	/* then enumerate over existing ones */
	udev_enumerate = udev_enumerate_new(udev);
	if (!udev_enumerate) {
		fprintf(stderr, "unable to create an an enumeration context\n");
		goto finish;
	}
	if (udev_enumerate_add_match_subsystem(udev_enumerate, "net") < 0) {
		fprintf(stderr, "unable to add mathing subsystem to enumerate\n");
		goto finish;
	}
	if (udev_enumerate_scan_devices(udev_enumerate) < 0) {
		fprintf(stderr, "unable to scan devices\n");
		goto finish;
	}
	first = udev_enumerate_get_list_entry(udev_enumerate);
	udev_list_entry_foreach(item, first) {
		if (check_devpath(udev_list_entry_get_name(item), iface)) {
			/* the interface is already up */
			rc = 0;
			goto finish;
		}
	}

	while (1) {
		struct udev_device *device;

		device = udev_monitor_receive_device(udev_monitor);
		if (device == NULL || strcmp("add", udev_device_get_action(device)) != 0)
			continue;
		int found = 0;
		if (check_devpath(udev_device_get_devpath(device), iface))
			/* the interface is just added */
			found = 1;
		udev_device_unref(device);
		if (found)
			break;
	}

	rc = 0;
finish:
	if (udev_enumerate)
		udev_enumerate_unref(udev_enumerate);
	if (udev_monitor)
		udev_monitor_unref(udev_monitor);
	if (udev)
		udev_unref(udev);
	return rc;
}

/** Bring up an interface
 * @param iface the interface struct pointer
 * @return 1 on failure, 0 on success
 */
int fwnet_ifup(fwnet_interface_t *iface, fwnet_profile_t *profile)
{
	int dhcp, ret=0, i;
	char *ptr;

	if(g_list_length(iface->pre_ups))
		for (i=0; i<g_list_length(iface->pre_ups); i++)
			ret += fwutil_system((char*)g_list_nth_data(iface->pre_ups, i));

	dhcp = fwnet_is_dhcp(iface);

	// wait for the interface
	wait_interface(iface->name);

	// initialize the device
	if(strlen(iface->wpa_psk) || iface->wpa_supplicant)
	{
		if(strlen(iface->wpa_psk))
			update_wpa_conf(iface->essid, iface->scan_ssid, iface->wpa_psk);
		if(strlen(iface->wpa_driver))
			ptr = g_strdup_printf("wpa_supplicant -i%s -D%s -c /etc/wpa_supplicant.conf -B", iface->name, iface->wpa_driver);
		else
			ptr = g_strdup_printf("wpa_supplicant -i%s -Dwext -c /etc/wpa_supplicant.conf -B", iface->name);
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}
	if(strlen(iface->mac))
	{
		ptr = g_strdup_printf("ifconfig %s hw ether %s", iface->name, iface->mac);
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}
	if(strlen(iface->mode))
	{
		ptr = g_strdup_printf("iwconfig %s mode %s", iface->name, iface->mode);
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}
	if(strlen(iface->essid))
	{
		ptr = g_strdup_printf("iwconfig %s essid \"%s\"", iface->name, iface->essid);
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}

	if(strlen(iface->key))
	{
		ptr = g_strdup_printf("iwconfig %s key %s", iface->name, iface->key);
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}

	// set up the interface
	if(dhcp)
	{
		if (!strcmp(iface->dhcpclient, "dhclient"))
			ptr = g_strdup_printf("dhclient -pf /var/run/dhclient-%s.pid -lf /var/lib/dhclient/dhclient-%s.leases -q %s", iface->name, iface->name, iface->name);
		else {
			if(strlen(iface->dhcp_opts))
				ptr = g_strdup_printf("dhcpcd %s %s", iface->dhcp_opts, iface->name);
			else
				ptr = g_strdup_printf("dhcpcd -t 10 %s", iface->name);
		}
		
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}
	else if(g_list_length(iface->options)==1)
	{
		ptr = g_strdup_printf("ifconfig %s %s",
			iface->name, (char*)g_list_nth_data(iface->options, 0));
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}
	else if(g_list_length(iface->options)>1)
	{
		ptr = g_strdup_printf("ifconfig %s 0.0.0.0", iface->name);
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
		for (i=0; i<g_list_length(iface->options); i++)
		{
			ptr = g_strdup_printf("ifconfig %s:%d %s",
				iface->name, i+1, (char*)g_list_nth_data(iface->options, i));
			ret += fwutil_system(ptr);
			FWUTIL_FREE(ptr);
		}
	}

	// setup the gateway
	if(!dhcp && strlen(iface->gateway))
	{
		ptr = g_strdup_printf("route add %s", iface->gateway);
		ret += fwutil_system(ptr);
		FWUTIL_FREE(ptr);
	}
	if(strlen(profile->adsl_interface) && !strcmp(profile->adsl_interface, iface->name) &&
		strlen(profile->adsl_username) && strlen(profile->adsl_password))
	{
		update_adsl_conf(iface->name, profile->adsl_username);
		update_secrets("/etc/ppp/pap-secrets", profile->adsl_username, profile->adsl_password);
		update_secrets("/etc/ppp/chap-secrets", profile->adsl_username, profile->adsl_password);
		ret += fwutil_system("pppoe-start");
	}
	if(g_list_length(iface->post_ups))
		for (i=0; i<g_list_length(iface->post_ups); i++)
			ret += fwutil_system((char*)g_list_nth_data(iface->post_ups, i));

	return(ret);
}

/** Update the dns settings based on a profile
 * @param profile the profile struct pointer
 * @return 1 on failure, 0 on success
 */
int fwnet_setdns(fwnet_profile_t* profile)
{
	int i;
	FILE *fp=NULL;

	if(g_list_length(profile->dnses) || strlen(profile->domain))
	{
		if(!fwutil_dryrun)
			fp = fopen("/etc/resolv.conf", "w");
		if(fwutil_dryrun || (fp != NULL))
		{
			if(strlen(profile->domain))
			{
				if(fwutil_dryrun)
					printf("search %s\n", profile->domain);
				else
					fprintf(fp, "search %s\n", profile->domain);
			}
			if(g_list_length(profile->dnses))
				for (i=0; i<g_list_length(profile->dnses); i++)
					if(fwutil_dryrun)
						printf("nameserver %s\n", (char*)g_list_nth_data(profile->dnses, i));
					else
						fprintf(fp, "nameserver %s\n", (char*)g_list_nth_data(profile->dnses, i));
			if(!fwutil_dryrun)
				fclose(fp);
		}
		return(1);
	}
	return(0);
}

/** Get the name of the last used profile.
 * @return the name on success, NULL on failure
 */
char *fwnet_lastprofile(void)
{
	FILE *fp;
	char line[PATH_MAX+1];

	fp = fopen(FWNET_LOCK, "r");
	if(fp==NULL)
		return(NULL);
	fgets(line, PATH_MAX, fp);
	fclose(fp);
	fwutil_trim(line);
	return(strdup(line));
}

/** Set the name of the last used profile.
 * @param str the name of the profile
 * @return 1 on failure, 0 on success
 */
int fwnet_setlastprofile(char* str)
{
	int fd;
	FILE *fp;

	if(!str)
	{
		if(unlink(FWNET_LOCK)==-1)
			return(1);
		else
			return(0);
	}

	fd = creat(FWNET_LOCK, 0644);
	fp = fdopen(fd, "w");
	if(fp==NULL)
		return(1);
	fprintf(fp, str);
	fclose(fp);
	return(0);
}

/** Brings up the 'lo' interface.
 * @return 1 on failure, 0 on success
 */
int fwnet_loup(void)
{
	int ret=0;

	ret += fwutil_system("ifconfig lo 127.0.0.1");
	ret += fwutil_system("route add -net 127.0.0.0 netmask 255.0.0.0 lo");
	return(ret);
}

/** Shut down the 'lo' interface.
 * @return 1 on failure, 0 on success
 */
int fwnet_lodown(void)
{
	return(fwutil_system("ifconfig lo down"));
}

/** Check if an interface is a wireless device.
 * @param dev the name of the interface
 * @return 1 if yes, 0 if not
 */
int fwnet_is_wireless_device(char *dev)
{
	FILE *pp;
	char *ptr;
	char line[256];
	struct stat buf;

	if(stat("/usr/sbin/iwconfig", &buf))
	{
		/* no iwconfig found */
		return(0);
	}

	ptr = g_strdup_printf("iwconfig %s 2>&1", dev);
	pp = popen(ptr, "r");
	FWUTIL_FREE(ptr);
	if(pp==NULL)
		return(0);
	while(fgets(line, 255, pp))
		if(strstr(line, "no wireless extensions"))
		{
			pclose(pp);
			return(0);
		}
	pclose(pp);
	return(1);
}

/** Drop the domain suffix from a hostname+domainname.
 * @param ptr the full hostname+domainname
 * @return the hostname
 */
static char *hostname(char *ptr)
{
	char *str=ptr;

	while(*ptr++!='.');
	*--ptr='\0';
	return(str);
}

/** Get a network address based on an ip address and a netmask.
 * Copyright 1994 by David Niemi.  Written in about 30 minutes on 13 Aug.
 * The author places no restrictions on the use of this program, provided
 * that this copyright is preserved in any derived source code.
 * @param ip the ip address
 * @param nm the netmask address
 * @return the network address
 */
static char *netaddr(char *ip, char *nm)
{
	unsigned long netmask, ipaddr, netaddr;
	int in[4], i;
	unsigned char na[4];

	// sanility checks for netmask
	if (4 != sscanf(ip,"%d.%d.%d.%d", &in[0],&in[1],&in[2],&in[3]))
		// invalid netmask
		return(NULL);
	for (i=0; i<4; ++i)
		if (in[i]<0 || in[i]>255)
			// invalid octet in netmask
			return(NULL);
	netmask = in[3] + 256 * (in[2] + 256 * (in[1] + 256 * in[0]));

	// sanility check for ip
	if (4 != sscanf(nm,"%d.%d.%d.%d", &in[0],&in[1],&in[2],&in[3]))
		// invalid ip
		return(NULL);
	for (i=0; i<4; ++i)
		if (in[i]<0 || in[i]>255)
			// invalied octet in ip
			return(NULL);

	ipaddr = in[3] + 256 * (in[2] + 256 * (in[1] + 256 * in[0]));

	netaddr = ipaddr & netmask;
	na[0] = netaddr / 256 / 256 / 256;
	na[1] = (netaddr / 256 / 256) % 256;
	na[2] = (netaddr / 256) % 256;
	na[3] = netaddr % 256;

	return(g_strdup_printf("%d.%d.%d.%d", na[0], na[1], na[2], na[3]));
}

/** Dumps a profile to a text file.
 * @param profile the profile to dump
 * @param host the hostname (optional)
 * @return 1 on failure, 0 on success
 */
int fwnet_writeconfig(fwnet_profile_t *profile, char *host)
{
	FILE *fp;
	char *network=NULL;
	char ipaddr[16] = "", netmask[16] = "";
	char *ptr;
	int oldmask, i, j, staticip = 0;

	oldmask = umask(0077);
	ptr = g_strdup_printf("%s/%s", FWNET_PATH, profile->name);
	unlink(ptr);
	fp = fopen(ptr, "w");
	FWUTIL_FREE(ptr);
	if(fp==NULL)
		return(1);
	if(((char*)g_list_nth_data(profile->dnses, 0) && strlen((char*)g_list_nth_data(profile->dnses, 0))) ||
			strlen(profile->adsl_username) || strlen(profile->adsl_password) ||
			strlen(profile->adsl_interface) || strlen(profile->desc))
		fprintf(fp, "[options]\n");
	if (strlen(profile->desc))
		fprintf(fp, "desc = %s\n", profile->desc);
	for(i=0;i<g_list_length(profile->dnses);i++)
	{
		char *dns = (char*)g_list_nth_data(profile->dnses, i);
		if(dns && strlen(dns))
			fprintf(fp, "dns = %s\n", dns);
	}
	if(strlen(profile->adsl_username))
		fprintf(fp, "adsl_username = %s\n", profile->adsl_username);
	if(strlen(profile->adsl_password))
		fprintf(fp, "adsl_password = %s\n", profile->adsl_password);
	if(strlen(profile->adsl_interface))
		fprintf(fp, "adsl_interface = %s\n", profile->adsl_interface);
	for(i=0;i<g_list_length(profile->interfaces);i++)
	{
		fwnet_interface_t* iface = (fwnet_interface_t*)g_list_nth_data(profile->interfaces, i);
		fprintf(fp, "[%s]\n", iface->name);
		if(iface->essid != NULL && strlen(iface->essid))
			fprintf(fp, "essid = %s\n", iface->essid);
		if(iface->mode != NULL && strlen(iface->mode))
			fprintf(fp, "mode = %s\n", iface->mode);
		if(iface->key != NULL && strlen(iface->key))
			fprintf(fp, "key = %s\n", iface->key);
		if(iface->wpa_psk != NULL && strlen(iface->wpa_psk))
			fprintf(fp, "wpa_psk = %s\n", iface->wpa_psk);
		if(iface->wpa_driver != NULL && strlen(iface->wpa_driver))
			fprintf(fp, "wpa_driver = %s\n", iface->wpa_driver);
		if(iface->wpa_supplicant)
			fprintf(fp, "wpa_supplicant = yes\n");
		if(fwnet_is_dhcp(iface))
		{
			fprintf(fp, "options = dhcp\n");
			if(strlen(iface->dhcp_opts))
				fprintf(fp, "dhcp_opts = %s\n", iface->dhcp_opts);
			if(strlen(iface->dhcpclient))
				fprintf(fp, "dhcpclient = %s\n", iface->dhcpclient);
		}
		else
		{
			for(j=0;j<g_list_length(iface->options);j++)
			{
				char *option = (char*)g_list_nth_data(iface->options, j);
				if(option != NULL && strlen(option))
				{
					if(!strlen(ipaddr))
						sscanf(option, "%s netmask %s", ipaddr, netmask);
					fprintf(fp, "options = %s\n", option);
					staticip = 1;
				}
			}
			if(strlen(iface->gateway))
				fprintf(fp, "gateway = %s\n", iface->gateway);
		}
		for(j=0;j<g_list_length(iface->pre_ups);j++)
		{
			char *command = (char*)g_list_nth_data(iface->pre_ups, j);
			if(command && strlen(command))
				fprintf(fp, "pre_up = %s\n", command);
		}
		for(j=0;j<g_list_length(iface->pre_downs);j++)
		{
			char *command = (char*)g_list_nth_data(iface->pre_downs, j);
			if(command && strlen(command))
				fprintf(fp, "pre_down = %s\n", command);
		}
		for(j=0;j<g_list_length(iface->post_ups);j++)
		{
			char *command = (char*)g_list_nth_data(iface->post_ups, j);
			if(command && strlen(command))
				fprintf(fp, "post_up = %s\n", command);
		}
		for(j=0;j<g_list_length(iface->post_downs);j++)
		{
			char *command = (char*)g_list_nth_data(iface->post_downs, j);
			if(command && strlen(command))
				fprintf(fp, "post_down = %s\n", command);
		}
	}
	fclose(fp);

	if(host)
	{
		fp = fopen("/etc/HOSTNAME", "w");
		if(fp==NULL)
			return(1);
		fprintf(fp, "%s\n", host);
		fclose(fp);
		chmod("/etc/HOSTNAME", 0644);

		// for systemd
		fp = fopen("/etc/hostname", "w");
		if(fp==NULL)
			return(1);
		char *buf = strdup(host);
		char *ptr = strchr(buf, '.');
		if (ptr)
			*ptr = '\0';
		fprintf(fp, "%s\n", buf);
		free(buf);
		fclose(fp);
		chmod("/etc/hostname", 0644);

		if(!staticip)
		{
			sprintf(ipaddr, "127.0.0.1");
			network = strdup("127.0.0.0");
		}
		else
		{
			network=netaddr(ipaddr, netmask);
			if (!network)
				// probably invalid ip or netmask
				return(1);
		}

		rename("/etc/hosts", "/etc/hosts.old");
		fp = fopen("/etc/hosts", "w");
		if(fp==NULL)
			return(1);
		fprintf(fp, "#\n"
				"# hosts         This file describes a number of hostname-to-address\n"
				"#               mappings for the TCP/IP subsystem.  It is mostly\n"
				"#               used at boot time, when no name servers are running.\n"
				"#               On small systems, this file can be used instead of a\n"
				"#               'named' name server.  Just add the names, addresses\n"
				"#               and any aliases to this file...\n"
				"#\n\n"
				"# For loopbacking.\n"
				"127.0.0.1               localhost\n");
		fprintf(fp, "%s\t%s %s\n", ipaddr, host, hostname(host));
		fprintf(fp, "\n# End of hosts.\n");
		fclose(fp);
		chmod("/etc/hosts", 0644);

		fp = fopen("/etc/networks", "w");
		if(fp==NULL)
			return(1);
		fprintf(fp, "#\n"
				"# networks      This file describes a number of netname-to-address\n"
				"#               mappings for the TCP/IP subsystem.  It is mostly\n"
				"#               used at boot time, when no name servers are running.\n"
				"#\n\n"
				"loopback        127.0.0.0\n");
		fprintf(fp, "localnet        %s\n", network);
		fprintf(fp, "\n# End of networks.\n");
		fclose(fp);
		chmod("/etc/networks", 0644);
	}
	FWUTIL_FREE(network);
	umask(oldmask);
	return(0);
}
/* @} */
