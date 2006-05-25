/*
 *  libfwxconfig.h for frugalwareutils
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

int fwx_init();
void fwx_release();
void fwx_print_mouse_options(FILE *fp);
void fwx_print_kbd_options(FILE *fp);
void fwx_print_mouse_identifier(FILE *fp, int num, char *device, char *proto);
int fwx_doprobe();
int fwx_doconfig(char *mousedev, char *res, char *depth);
int fwx_dotest();
char *fwx_get_mousedev();
