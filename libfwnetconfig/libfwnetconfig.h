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

#define MAC_MAX_SIZE 17
#define ESSID_MAX_SIZE 32
#define ENCODING_TOKEN_MAX   32
#define GW_MAX_SIZE 26

#define VERSIONFILE "/etc/frugalware-release"
#define NC_PATH "/etc/sysconfig/network"
#define NC_LOCK "/var/run/netconfig"

typedef struct __interface_t {
	char name[IF_NAMESIZE+1];
	GList *options;
	GList *pre_ups;
	GList *post_ups;
	GList *pre_downs;
	GList *post_downs;
	char mac[MAC_MAX_SIZE+1];
	char dhcp_opts[PATH_MAX+1];
	char essid[ESSID_MAX_SIZE+1];
	char key[ENCODING_TOKEN_MAX+1];
	char gateway[GW_MAX_SIZE+1];
} interface_t;

typedef struct __profile_t {
	char name[256];
	GList *dnses;
	char desc[PATH_MAX+1];
	char domain[PATH_MAX+1];
	char adsl_username[PATH_MAX+1];
	char adsl_password[PATH_MAX+1];
	char adsl_interface[PATH_MAX+1];
	GList *interfaces; // GList of interface_t*
} profile_t;

int fwnet_listprofiles(void);
profile_t *fwnet_parseprofile(char *fn);
int fwnet_ifdown(interface_t *iface, profile_t *profile);
int fwnet_ifup(interface_t *iface, profile_t *profile);
int fwnet_setdns(profile_t* profile);
char *fwnet_lastprofile(void);
int fwnet_setlastprofile(char* str);
int fwnet_loup(void);
int fwnet_lodown(void);
int fwnet_is_wireless_device(char *dev);
int fwnet_writeconfig(profile_t *profile, char *host, char *nettype);
