/*
 *  libfwxconfig.c for frugalwareutils
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <regex.h>
#include <sys/stat.h>

#include "xconfig-helper.h"

#define XORGCONFIG "/etc/X11/xorg.conf"
#define NEWCONFIG "/root/xorg.conf.new"
#define COREPOINTER "\"CorePointer\""
#define HORIZSYNC "31.5 - 64.3"
#define REFRESH "60-75"

/** @defgroup libfwxconfig Frugalware X Configuration library
 * @brief Functions to make X configuration easier
 * @{
 */

static void print_mouse_options(FILE *fp)
{
	fprintf(fp, "Option      \"ZAxisMapping\" \"4 5\"\n"
		"Option      \"Buttons\" \"3\"\n");
}

static void print_kbd_options(FILE *fp)
{
	char *ptr, *lang=NULL;
	
	ptr = getenv("LANG");
	if(ptr)
		lang = strdup(ptr);

	if(!lang || !strlen(lang) || !strncmp(lang, "en_", 3))
		fprintf(fp, "Option      \"XkbLayout\" \"us\"\n");
	else
	{
		ptr = strstr(lang, "_");
		*ptr = '\0';
		fprintf(fp, "Option      \"XkbLayout\" \"%s\"\n", lang);
	}
	if(lang)
		free(lang);
}

static void print_mouse_identifier(FILE *fp, int num, char *device, char *proto)
{
	if(!proto)
		proto = strdup("auto");
	fprintf(fp, "Identifier  \"Mouse%d\"\n", num);
	fprintf(fp, "Driver      \"mouse\"\n");
	print_mouse_options(fp);
	fprintf(fp, "Option      \"Protocol\" \"%s\"\n", proto);
	fprintf(fp, "Option      \"Device\" \"%s\"\n", device);
	fprintf(fp, "EndSection\n\n"
			"Section \"InputDevice\"\n");
}

/** Creates a config draft, which will be an input fro fwx_doconfig()
 * @return 0 on success, 1 on failure
 */
int fwx_doprobe()
{
	return(system("X -configure :1 2>/dev/null"));
}

static int reg_match(char *str, char *pattern)
{
	int result;
	regex_t reg;

	if(regcomp(&reg, pattern, REG_EXTENDED | REG_NOSUB | REG_ICASE) != 0)
		return(-1);

	result = regexec(&reg, str, 0, 0, 0);
	regfree(&reg);
	return(!(result));
}

/** Creates a configuration from the draft
 * @return 1 on success, 0 on failure
 */
int fwx_doconfig(char *mousedev, char *res, char *depth)
{
	char line[PATH_MAX+1];
	FILE *ofp, *nfp;
	struct stat buf;
	int start_looking=0;

	unlink(XORGCONFIG);

	ofp = fopen(NEWCONFIG, "r");
	if(!ofp)
		return(1);
	nfp = fopen(XORGCONFIG, "w");
	if(!nfp)
		return(1);
	while(fgets(line, PATH_MAX, ofp))
	{
		if(reg_match(line, "Protocol.*auto"))
		{
			print_mouse_options(nfp);
			fprintf(nfp, "Option      \"Protocol\" \"auto\"\n");
			line[0]='\0';
		}
		if(reg_match(line, "Identifier.*Mouse"))
		{
			print_mouse_identifier(nfp, 0, "/dev/psaux", "imps/2");
			print_mouse_identifier(nfp, 1, "/dev/tts/0", NULL);
			fprintf(nfp, "Identifier  \"Mouse3\"\n");
			line[0]='\0';
		}
		if(reg_match(line, "CorePointer"))
		{
			char cp[PATH_MAX+1] = "";
			if(!stat("/dev/psaux", &buf))
				sprintf(cp, "%s", COREPOINTER);
			fprintf(nfp, "InputDevice    \"Mouse0\" %s\n", cp);
			cp[0]='\0';
			if(!stat("/dev/tts/0", &buf))
				sprintf(cp, "%s", COREPOINTER);
			fprintf(nfp, "InputDevice    \"Mouse1\" %s\n", cp);
			line[0]='\0';
		}
		fprintf(nfp, "%s", line);
		if(reg_match(line, "usebios"))
		{
			// To disable blinking on some savage cards
			fprintf(nfp, "Option     \"UseBIOS\" \"No\"\n");
		}
		if(reg_match(line, "boardname"))
		{
			char *ptr = strstr(line, "\"");
			// S3 Virge/GX2, 2x AGP, TVOut - workaround for broken DDC
			if(!strcmp(ptr, "ViRGE/GX2"))
				fprintf(nfp, "Option          \"NoDDC\"\n");
		}
		if(reg_match(line, "Section.*Monitor"))
		{
			// X -configure leaves out the refresh frequency
			// We'll work around this.
			fprintf(nfp, "HorizSync    %s\n", HORIZSYNC);
			fprintf(nfp, "VertRefresh  %s\n", REFRESH);
			fprintf(nfp, "Option \"PreferredMode\" \"%s\"\n", res);
			fprintf(nfp, "Option       \"DPMS\"\n");
		}
		if(reg_match(line, "driver.*kbd"))
		{
			print_kbd_options(nfp);
		}
		if(reg_match(line, "Depth.*(16)|(24)"))
		{
			fprintf(nfp, "Modes \"%s\" \"800x600\" \"640x480\"\n", res);
		}
		if(reg_match(line, "Load.*type1"))
			// X -configure leaves out the freetype module.
			// We'll work around this.
			fprintf(nfp, "Load  \"freetype\"\n");
		if(reg_match(line, "Section.*Screen"))
			start_looking=1;
		if(start_looking)
			if(reg_match(line, "Monitor"))
			{
				fprintf(nfp, "DefaultDepth %s\n", depth);
				start_looking=0;
			}
	}
	fprintf(nfp, "Section \"DRI\"\n"
			"        Group        0\n"
			"        Mode         0666\n"
			"EndSection\n");
	fclose(ofp);
	fclose(nfp);
	return(0);

}

/** Performs a probe with the new config
 * @return 1 on success, 0 on failure
 */
int fwx_dotest()
{
	struct stat buf;

	system("XINITRC=/usr/libexec/xconfig-helper xinit -- :1");
	if(!stat(CLICKFILE, &buf))
	{
		unlink(CLICKFILE);
		return(0);
	}
	else
		return(1);
}

/** Get the name of the device used by gpm
 * @return the name of the device
 */
char *fwx_get_mousedev()
{
	FILE *pp;
	char line[256];

	pp = popen("source /etc/sysconfig/gpm; echo $dev", "r");
	fgets(line, 255, pp);
	pclose(pp);
	return(strdup(line));
}
/* @} */
