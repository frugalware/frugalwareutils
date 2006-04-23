/*
 *  fwnetconfig.c for frugalwareutils
 *
 *  usefull functions for a network configurator
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
#include <fwdialog.h>
#include <fwutil.h>
#include <getopt.h>
#include <stdlib.h>
#include <glib.h>
#include <net/if.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

#include "fwnetconfig.h"

extern int f_util_dryrun;

int listprofiles(void)
{
	struct dirent *ent=NULL;
	DIR *dir;

	dir = opendir(NC_PATH);
	while((ent = readdir(dir)))
	{
		if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
			printf("%s\n", ent->d_name);
	}
	return(0);
}

/*
 * based on pacman's config parser, which is
 * Copyright (c) 2002-2006 by Judd Vinet <jvinet@zeroflux.org>
 */
profile_t *parseprofile(char *fn)
{
	FILE *fp;
	char line[PATH_MAX+1];
	int i, n=0;
	char *ptr = NULL;
	char *var = NULL;
	char interface[256] = "";
	profile_t *profile;
	interface_t *iface=NULL;

	profile = (profile_t*)malloc(sizeof(profile_t));
	if(profile==NULL)
		return(NULL);
	memset(profile, 0, sizeof(profile_t));

	ptr = g_strdup_printf(NC_PATH "/%s", fn);
	fp = fopen(ptr, "r");
	if(fp == NULL)
	{
		printf(_("%s: No such profile!\n"), fn);
		return(NULL);
	}
	FREE(ptr);

	while(fgets(line, PATH_MAX, fp))
	{
		n++;
		trim(line);
		if(strlen(line) == 0 || line[0] == '#')
			continue;
		if(line[0] == '[' && line[strlen(line)-1] == ']')
		{
			// new interface
			ptr = line;
			ptr++;
			strncpy(interface, ptr, min(255, strlen(ptr)-1));
			interface[min(255, strlen(ptr)-1)] = '\0';
			if(!strlen(interface))
			{
				fprintf(stderr, _("profile: line %d: bad interface name\n"), n);
				return(NULL);
			}
			if(strcmp(interface, "options"))
			{
				int found = 0;
				for (i=0; !found && i<g_list_length(profile->interfaces); i++)
				{
					iface = (interface_t*)g_list_nth_data(profile->interfaces, i);
					if(!strcmp(iface->name, interface))
						found=1;
				}
				if(!found)
				{
					// start a new interface record
					iface = (interface_t*)malloc(sizeof(interface_t));
					if(iface==NULL)
						return(NULL);
					memset(iface, 0, sizeof(interface_t));
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
				return(NULL);
			}
			trim(var);
			var = strtoupper(var);
			if(!strlen(interface))
			{
				fprintf(stderr, _("profile: line %d: all directives must belong to a section\n"), n);
				return(NULL);
			}
			if(ptr != NULL)
			{
				trim(ptr);
				if (!strcmp(var, "DNS"))
					profile->dnses = g_list_append(profile->dnses, strdup(ptr));
				if (!strcmp(var, "DOMAIN") && !strlen(profile->domain))
					strncpy(profile->domain, ptr, PATH_MAX);
				if (!strcmp(var, "DESC") && !strlen(profile->desc))
					strncpy(profile->desc, ptr, PATH_MAX);
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
					strncpy(iface->mac, ptr, MAC_MAX_SIZE);
				if(!strcmp(var, "DHCP_OPTS") && !strlen(iface->dhcp_opts))
					strncpy(iface->dhcp_opts, ptr, PATH_MAX);
				if(!strcmp(var, "ESSID") && !strlen(iface->essid))
					strncpy(iface->essid, ptr, ESSID_MAX_SIZE);
				if(!strcmp(var, "KEY") && !strlen(iface->key))
					strncpy(iface->key, ptr, ENCODING_TOKEN_MAX);
				if(!strcmp(var, "GATEWAY") && !strlen(iface->gateway))
					strncpy(iface->gateway, ptr, GW_MAX_SIZE);
			}
		}
		line[0] = '\0';
	}
	fclose(fp);
	return(profile);
}

int is_dhcp(interface_t *iface)
{
	int i, dhcp=0;
	for (i=0; i<g_list_length(iface->options); i++)
		if(!strcmp((char*)g_list_nth_data(iface->options, i), "dhcp"))
			dhcp=1;
	return(dhcp);
}

int ifdown(interface_t *iface)
{
	int dhcp, ret=0, i;
	char *ptr;
	FILE *fp;

	if(g_list_length(iface->pre_downs))
		for (i=0; i<g_list_length(iface->pre_downs); i++)
			nc_system((char*)g_list_nth_data(iface->pre_downs, i));

	dhcp = is_dhcp(iface);
	if(dhcp)
	{
		char line[7];
		ptr = g_strdup_printf("/etc/dhcpc/dhcpcd-%s.pid", iface->name);
		fp = fopen(ptr, "r");
		FREE(ptr);
		if(fp != NULL)
		{
			fgets(line, 6, fp);
			fclose(fp);
			i = atoi(line);
			if(i>0 && !f_util_dryrun)
				ret = kill(i, 15);
			else if (i>0)
				printf("kill(%d, 15);\n", i);
		}
	}
	else
	{
		if(g_list_length(iface->options)>1)
			for (i=0; i<g_list_length(iface->options); i++)
			{
				ptr = g_strdup_printf("ifconfig %s 0.0.0.0", iface->name);
				nc_system(ptr);
				FREE(ptr);
			}
		ptr = g_strdup_printf("ifconfig %s down", iface->name);
		nc_system(ptr);
		FREE(ptr);
	}

	if(g_list_length(iface->post_downs))
		for (i=0; i<g_list_length(iface->post_downs); i++)
			nc_system((char*)g_list_nth_data(iface->post_downs, i));

	return(ret);
}

int ifup(interface_t *iface)
{
	int dhcp, ret=0, i;
	char *ptr;

	if(g_list_length(iface->pre_ups))
		for (i=0; i<g_list_length(iface->pre_ups); i++)
			ret += nc_system((char*)g_list_nth_data(iface->pre_ups, i));

	dhcp = is_dhcp(iface);
	// initialize the device
	if(strlen(iface->mac))
	{
		ptr = g_strdup_printf("ifconfig %s hw ether %s", iface->name, iface->mac);
		ret += nc_system(ptr);
		FREE(ptr);
	}
	if(strlen(iface->essid))
	{
		ptr = g_strdup_printf("iwconfig %s essid %s", iface->name, iface->essid);
		ret += nc_system(ptr);
		FREE(ptr);
	}

	if(strlen(iface->key))
	{
		ptr = g_strdup_printf("iwconfig %s key %s", iface->name, iface->key);
		ret += nc_system(ptr);
		FREE(ptr);
	}

	// set up the interface
	if(dhcp)
	{
		if(strlen(iface->dhcp_opts))
			ptr = g_strdup_printf("dhcpcd %s %s", iface->dhcp_opts, iface->name);
		else
			ptr = g_strdup_printf("dhcpcd -t 10 %s", iface->name);
		ret += nc_system(ptr);
		FREE(ptr);
	}
	else if(g_list_length(iface->options)==1)
	{
		ptr = g_strdup_printf("ifconfig %s %s",
			iface->name, (char*)g_list_nth_data(iface->options, 0));
		ret += nc_system(ptr);
		FREE(ptr);
	}
	else
	{
		ptr = g_strdup_printf("ifconfig %s 0.0.0.0", iface->name);
		ret += nc_system(ptr);
		FREE(ptr);
		for (i=0; i<g_list_length(iface->options); i++)
		{
			ptr = g_strdup_printf("ifconfig %s:%d %s",
				iface->name, i+1, (char*)g_list_nth_data(iface->options, i));
			ret += nc_system(ptr);
			FREE(ptr);
		}
	}

	// setup the gateway
	if(!dhcp && strlen(iface->gateway))
	{
		ptr = g_strdup_printf("route add %s", iface->gateway);
		ret += nc_system(ptr);
		FREE(ptr);
	}
	if(g_list_length(iface->post_ups))
		for (i=0; i<g_list_length(iface->post_ups); i++)
			ret += nc_system((char*)g_list_nth_data(iface->post_ups, i));

	return(ret);
}

int setdns(profile_t* profile)
{
	int i;
	FILE *fp=NULL;

	if(g_list_length(profile->dnses) || strlen(profile->domain))
	{
		if(!f_util_dryrun)
			fp = fopen("/etc/resolv.conf", "w");
		if(f_util_dryrun || (fp != NULL))
		{
			if(strlen(profile->domain))
			{
				if(f_util_dryrun)
					printf("search %s\n", profile->domain);
				else
					fprintf(fp, "search %s\n", profile->domain);
			}
			if(g_list_length(profile->dnses))
				for (i=0; i<g_list_length(profile->dnses); i++)
					if(f_util_dryrun)
						printf("nameserver %s\n", (char*)g_list_nth_data(profile->dnses, i));
					else
						fprintf(fp, "nameserver %s\n", (char*)g_list_nth_data(profile->dnses, i));
			if(!f_util_dryrun)
				fclose(fp);
		}
		return(1);
	}
	return(0);
}

char *lastprofile(void)
{
	FILE *fp;
	char line[PATH_MAX+1];

	fp = fopen(NC_LOCK, "r");
	if(fp==NULL)
		return(NULL);
	fgets(line, PATH_MAX, fp);
	fclose(fp);
	trim(line);
	return(strdup(line));
}

int setlastprofile(char* str)
{
	FILE *fp;

	// sanility check
	if(!str)
		return(1);

	fp = fopen(NC_LOCK, "w");
	if(fp==NULL)
		return(1);
	fprintf(fp, str);
	fclose(fp);
	return(0);
}

int loup(void)
{
	int ret=0;

	ret += nc_system("ifconfig lo 127.0.0.1");
	ret += nc_system("route add -net 127.0.0.0 netmask 255.0.0.0 lo");
	return(ret);
}

int lodown(void)
{
	return(nc_system("ifconfig lo down"));
}

int is_wireless_device(char *dev)
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
	FREE(ptr);
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

char *hostname(char *ptr)
{
	char *str=ptr;

	while(*ptr++!='.');
	*--ptr='\0';
	return(str);
}

/* Copyright 1994 by David Niemi.  Written in about 30 minutes on 13 Aug.
 * The author places no restrictions on the use of this program, provided
 * that this copyright is preserved in any derived source code.
 */
char *netaddr(char *ip, char *nm)
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

int writeconfig(profile_t *profile, char *host, char *nettype)
{
	FILE *fp;
	interface_t* iface = (interface_t*)g_list_nth_data(profile->interfaces, 0);
	char *option = (char*)g_list_nth_data(iface->options, 0);
	char *dns = (char*)g_list_nth_data(profile->dnses, 0);
	char *network=NULL;
	char ipaddr[16], netmask[16];
	char *ptr;

	ptr = g_strdup_printf("%s/%s", NC_PATH, profile->name);
	fp = fopen(ptr, "w");
	FREE(ptr);
	if(fp==NULL)
		return(1);
	if(dns != NULL && strlen(dns))
	{
		fprintf(fp, "[options]\n");
		fprintf(fp, "dns = %s\n", dns);
	}
	if(strcmp(nettype, "lo"))
		fprintf(fp, "[%s]\n", iface->name);
	if(iface->essid != NULL && strlen(iface->essid))
		fprintf(fp, "essid = %s\n", iface->essid);
	if(iface->key != NULL && strlen(iface->key))
		fprintf(fp, "key = %s\n", iface->key);
	if(!strcmp(nettype, "dhcp"))
	{
		fprintf(fp, "options = dhcp\n");
		if(strlen(iface->dhcp_opts))
			fprintf(fp, "%s", iface->dhcp_opts);
	}
	else if (!strcmp(nettype, "static"))
	{
		if(option != NULL && strlen(option))
			fprintf(fp, "%s\n", option);
		if(strlen(iface->gateway))
			fprintf(fp, "gateway = default gw %s\n", iface->gateway);
	}
	fclose(fp);

	fp = fopen("/etc/HOSTNAME", "w");
	if(fp==NULL)
		return(1);
	fprintf(fp, "%s\n", host);
	fclose(fp);

	sscanf(option, "options = %s netmask %s", ipaddr, netmask);

	if(strcmp(nettype, "static"))
	{
		sprintf(ipaddr, "127.0.0.1");
		network = strdup("127.0.0.0");
	}
	else
		network=netaddr(ipaddr, netmask);

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
	FREE(network);
	return(0);
}
