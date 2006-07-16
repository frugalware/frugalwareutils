/*
 *  libfwutil.h for frugalwareutils
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

#define VERSIONFILE "/etc/frugalware-release"

#ifdef _
#undef _
#endif
#ifndef FWGETTEXT_LIB
#define _(text) gettext(text)
#else
#define _(str) dgettext (FWGETTEXT_LIB, str)
#endif

#define FREE(p) do { if (p) { free(p); (p) = NULL; }} while(0)

#define min(p, q)  ((p) < (q) ? (p) : (q))

int fwutil_system(const char *cmd);
void fwutil_i18ninit(char *namespace);
char *fwutil_trim(char *str);
char *fwutil_strtoupper(char *str);
int fwutil_init();
void fwutil_release();
int fwutil_cp(char *src, char *dest);
#ifdef __G_LIB_H__
char *fwutil_glist_display(GList *list, char *sep);
#endif
