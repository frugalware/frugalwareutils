/*
 *  libfwdialog.h for frugalwareutils
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

#include <dialog.h>

void fwdialog_backtitle(char *title);
int fwdialog_confirm(void);
void fwdialog_exit(void);
char *fwdialog_ask(char *title, char *desc, char *init);
char *fwdialog_password(char *title, char *desc);
char *fwdialog_menu(const char *title, const char *cprompt, int height, int width,
	int menu_height, int item_no, char **items);
int fwdialog_yesno(char *title, char *desc);
char **fwdialog_glist(GList *list);
