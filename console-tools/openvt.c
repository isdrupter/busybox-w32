/* vi: set sw=4 ts=4: */
/*
 *  openvt.c - open a vt to run a command.
 *
 *  busyboxed by Quy Tonthat <quy@signal3.com>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* getopt not needed */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include "busybox.h"

#define VTNAME "/dev/tty%d"

int openvt_main(int argc, char **argv)
{
	int pid;
	int fd;
	int vtno;
	char vtname[sizeof VTNAME + 2];
	char * cmd = NULL;
	char * cmd_args = NULL;

	if (argc < 3)
        show_usage();

	if (!isdigit(argv[1][0]))
	        show_usage();

	vtno = (int) atol(argv[1]);

	/* if (vtno <= 0 || vtno > 63) */
	if (vtno <= 0 || vtno > 12)
		error_msg_and_die("Illegal vt number (%d)", vtno);	  

	sprintf(vtname, VTNAME, vtno);

	cmd = argv[2];
	cmd_args = xmalloc(80);
	cmd_args[0] = '\0';

	if((pid = fork()) == 0) {
		/* leave current vt */

#ifdef   ESIX_5_3_2_D
		if (setpgrp() < 0) {
#else
		if (setsid() < 0) {
#endif

			perror_msg_and_die("Unable to set new session");	  
		}
		close(0);			/* so that new vt becomes stdin */

		/* and grab new one */
		if ((fd = open(vtname, O_RDWR)) == -1)
			perror_msg_and_die("could not open %s", vtname);	  

		/* Reassign stdout and sterr */
		close(1);
		close(2);
		dup(fd);
		dup(fd);

		execvp(cmd, &argv[2]);
		/*execlp(cmd, cmd_args);*/
		_exit(1);
	}
	return EXIT_SUCCESS;
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
