/*
 *  setup.c for frugalwareutils
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
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>
#include <dlfcn.h>

#include "setup.h"

#define PLUGDIR "/lib/frugalware"

GList *plugin_list;

int add_plugin(char *filename)
{
	void *handle;
	void *(*infop) (void);

	if ((handle = dlopen(filename, RTLD_NOW)) == NULL)
	{
		fprintf(stderr, "%s\n", dlerror());
		return(1);
	}

	if ((infop = dlsym(handle, "info")) == NULL)
	{
		fprintf(stderr, "%s\n", dlerror());
		return(1);
	}
	plugin_t *plugin = infop();
	plugin->handle = handle;
	plugin_list = g_list_append(plugin_list, plugin);

	return(0);
}

int init_plugins(char *dirname)
{
	char *filename, *ext;
	DIR *dir;
	struct dirent *ent;
	struct stat statbuf;

	dir = opendir(dirname);
	if (!dir)
	{
		perror(dirname);
		return(1);
	}
	while ((ent = readdir(dir)) != NULL)
	{
		filename = g_strdup_printf("%s/%s", dirname, ent->d_name);
		if (!stat(filename, &statbuf) && S_ISREG(statbuf.st_mode) &&
				(ext = strrchr(ent->d_name, '.')) != NULL)
			if (!strcmp(ext, ".so"))
				add_plugin(filename);
	}
	closedir(dir);
	return(0);
}

int cleanup_plugins()
{
	int i;
	plugin_t *plugin;

	for (i=0; i<g_list_length(plugin_list); i++)
	{
		plugin = g_list_nth_data(plugin_list, i);
		dlclose(plugin->handle);
	}
	return(0);
}

char* ask_what()
{
	int i;
	plugin_t *plugin;
	GList *pluglist=NULL;
	char **plugstrs;

	for (i=0; i<g_list_length(plugin_list); i++)
	{
		plugin = g_list_nth_data(plugin_list, i);
		pluglist = g_list_append(pluglist, plugin->name);
		pluglist = g_list_append(pluglist, plugin->desc);
	}
	plugstrs = glist2dialog(pluglist);
	return(dialog_mymenu(_("Control center"),
		_("Please select one of the following configuration tools "
		"to start:"), 0, 0, 0, g_list_length(pluglist)/2, plugstrs));
}

int show_menu()
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	char *ptr;
	int i;
	plugin_t *plugin;

	i18ninit(__FILE__);
	init_dialog(input, dialog_state.output);
	dialog_backtitle(_("General configuration"));

	while((ptr = ask_what()))
	{
		for (i=0; i<g_list_length(plugin_list); i++)
		{
			plugin = g_list_nth_data(plugin_list, i);
			if(!strcmp(plugin->name, ptr))
				plugin->run(1, NULL);
		}
		free(ptr);
	}

	end_dialog();
	return(0);
}

int main(int argc, char **argv)
{
	int i, ret=0;
	plugin_t *plugin;
	char *myname, *ptr;

	// drop the dir prefix if necessary
	ptr = strdup(argv[0]);
	if(strchr(ptr, '/'))
		myname = strrchr(ptr, '/')+1;
	else
		myname = ptr;

	init_plugins(PLUGDIR);

	if(strcmp(myname, "setup"))
	{
		// just call the given plugin and exit
		for (i=0; i<g_list_length(plugin_list); i++)
		{
			plugin = g_list_nth_data(plugin_list, i);
			if(!strcmp(plugin->name, myname))
			{
				plugin->run(argc, argv);
				ret = 1;
				break;
			}
		}
		if(!ret)
			fprintf(stderr, "Can't find plugin named '%s'!\n", myname);
	}
	else
	{
		// show a menu of config tools
		show_menu();
	}
	free(ptr);
	cleanup_plugins();
	return(0);
}
