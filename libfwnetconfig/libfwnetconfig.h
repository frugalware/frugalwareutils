/*
 *  libfwnetconfig.h for frugalwareutils
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

#ifndef _LIBFWNETCONFIG_H
#define _LIBFWNETCONFIG_H

#include <net/if.h>
#include <glib.h>

#define FWNET_MAC_MAX_SIZE 17
#define FWNET_ESSID_MAX_SIZE 32
#define FWNET_MODE_MAX_SIZE 32
#define FWNET_ENCODING_TOKEN_MAX   32
#define FWNET_GW_MAX_SIZE 27

#define FWNET_VERSIONFILE "/etc/frugalware-release"
#define FWNET_PATH "/etc/sysconfig/network"
#define FWNET_LOCK "/var/run/netconfig"
#define FWNET_IFPATH "/sys/class/net"

typedef struct __fwnet_interface_t {
	char name[IF_NAMESIZE+1];
	GList *options;
	GList *pre_ups;
	GList *post_ups;
	GList *pre_downs;
	GList *post_downs;
	char mac[FWNET_MAC_MAX_SIZE+1];
	char dhcp_opts[PATH_MAX+1];
	char dhcpclient[PATH_MAX+1];
	char essid[FWNET_ESSID_MAX_SIZE+1];
	char mode[FWNET_MODE_MAX_SIZE+1];
	char key[FWNET_ENCODING_TOKEN_MAX+1];
	char wpa_psk[PATH_MAX+1];
	char wpa_driver[PATH_MAX+1];
	char gateway[FWNET_GW_MAX_SIZE+1];
} fwnet_interface_t;

typedef struct __fwnet_profile_t {
	char name[256];
	GList *dnses;
	char desc[PATH_MAX+1];
	char domain[PATH_MAX+1];
	char adsl_username[PATH_MAX+1];
	char adsl_password[PATH_MAX+1];
	char adsl_interface[PATH_MAX+1];
	GList *interfaces; // GList of interface_t*
} fwnet_profile_t;

int fwnet_listprofiles(void);
fwnet_profile_t *fwnet_parseprofile(char *fn);
int fwnet_is_dhcp(fwnet_interface_t *iface);
GList *fwnet_iflist();
int fwnet_ifdown(fwnet_interface_t *iface, fwnet_profile_t *profile);
int fwnet_ifup(fwnet_interface_t *iface, fwnet_profile_t *profile);
int fwnet_setdns(fwnet_profile_t* profile);
char *fwnet_lastprofile(void);
int fwnet_setlastprofile(char* str);
int fwnet_loup(void);
int fwnet_lodown(void);
int fwnet_is_wireless_device(char *dev);
int fwnet_writeconfig(fwnet_profile_t *profile, char *host);
int fwnet_ifdesc(const char *iface, char *desc, int size);

#endif
