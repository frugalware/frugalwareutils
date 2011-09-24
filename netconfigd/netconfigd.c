/*
 *  netconfigd.c for frugalwareutils
 * 
 *  Copyright (c) 2011 by Miklos Vajna <vmiklos@frugalware.org>
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
#include <signal.h>
#include <stdlib.h>

static int term_interrupt = 0;

static void handle_sigterm(int signum)
{
	term_interrupt = 1;
}

static void handle_sighup(int signum)
{
	system("netconfig restart");
}

int main(int argc, char **argv)
{
	sigset_t mask, oldmask;

	signal(SIGTERM, handle_sigterm);
	signal(SIGHUP, handle_sighup);

	/* Set up the mask of signals to temporarily block. */
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);

	system("netconfig start");

	/* Wait for a signal to arrive. */
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	while (!term_interrupt)
		sigsuspend(&oldmask);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	system("netconfig stop");
	return 0;
}
