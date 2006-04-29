/*
 *  libfwdialog.c for frugalwareutils
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
#include <dialog.h>
#define FWGETTEXT_LIB "libfwdialog"
#include <libfwutil.h>
#include <getopt.h>
#include <stdlib.h>
#include <glib.h>
#include <net/if.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

void dialog_backtitle(char *title)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(VERSIONFILE, "r")) == NULL)
		return;
	fgets(line, 127, fp);
	line[strlen(line)-1]='\0';
	fclose(fp);
	if(dialog_vars.backtitle)
		FREE(dialog_vars.backtitle);
	dialog_vars.backtitle=g_strdup_printf("%s - %s %s", title, line, _("Setup"));
	dlg_put_backtitle();
	dlg_clear();
}

int dialog_confirm(void)
{
	int ret;
	dialog_vars.defaultno=1;
	ret = dialog_yesno(_("Exit"), _("Are you sure you want to exit?"), 0, 0);
	dialog_vars.defaultno=0;
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

void dialog_exit(void)
{
	end_dialog();
	exit(0);
}

char *dialog_ask(char *title, char *desc, char *init)
{
	char my_buffer[MAX_LEN + 1] = "";
	int ret;

	dlg_put_backtitle();
	dlg_clear();
	while(1)
	{
		dialog_vars.input_result = my_buffer;
		ret = dialog_inputbox(title, desc, 0, 0, init, 0);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(dialog_confirm())
			dialog_exit();
	}
	return(strdup(my_buffer));
}

char *dialog_mymenu(const char *title, const char *cprompt, int height, int width,
	int menu_height, int item_no, char **items)
{
	int ret;
	char my_buffer[MAX_LEN + 1] = "";

	while(1)
	{
		dialog_vars.input_result = my_buffer;
		dlg_put_backtitle();
		dlg_clear();
		ret = dialog_menu(title, cprompt, height, width, menu_height,
			item_no, items);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(dialog_confirm())
			dialog_exit();
	}
	return(strdup(dialog_vars.input_result));
}

int dialog_myyesno(char *title, char *desc)
{
	int ret;

	dlg_put_backtitle();
	dlg_clear();
	ret = dialog_yesno(title, desc, 0, 0);
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

char **glist2dialog(GList *list)
{
	int i;
	char **array;

	if((array = malloc(g_list_length(list)*sizeof(char*))) == NULL)
		return(NULL);
	for (i=0; i<g_list_length(list); i++)
	{
		array[i] = (char*)g_list_nth_data(list, i);
	}
	return(array);
}
