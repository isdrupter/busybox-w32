/* vi: set sw=4 ts=4 ai: */
/*
 * Mini ipcalc implementation for busybox
 *
 * By Jordan Crouse <jordan@cosmicpenguin.net>
 *    Stephan Linz  <linz@li-pro.net>
 *
 * This is a complete reimplentation of the ipcalc program
 * from Redhat.  I didn't look at their source code, but there
 * is no denying that this is a loving reimplementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "busybox.h"

#define IPCALC_MSG(CMD,ALTCMD) if (mode & SILENT) {ALTCMD;} else {CMD;}

static unsigned long get_netmask(unsigned long ipaddr)
{
	if (ipaddr & 0xC0) {
		return 0x00FFFFFF;	/* Class C */
	}
	if (ipaddr & 0x10) {
		return 0x0000FFFF;	/* Class B */
	}
	return 0x000000FF;	/* Class A */
}

#define NETMASK   0x01
#define BROADCAST 0x02
#define NETWORK   0x04
#define HOSTNAME  0x08
#define SILENT    0x80

int ipcalc_main(int argc, char **argv)
{
	unsigned char mode = 0;

	unsigned long netmask = 0;
	unsigned long broadcast = 0;
	unsigned long network = 0;
	unsigned long ipaddr = 0;

	int opt = 0;

	struct option long_options[] = {
		{"netmask", no_argument, NULL, 'n'},
		{"broadcast", no_argument, NULL, 'b'},
		{"network", no_argument, NULL, 'w'},
#ifdef CONFIG_FEATURE_IPCALC_FANCY
		{"hostname", no_argument, NULL, 'h'},
		{"silent", no_argument, NULL, 's'},
#endif
		{NULL, 0, NULL, 0}
	};


	while ((opt = getopt_long(argc, argv,
#ifdef CONFIG_FEATURE_IPCALC_FANCY
							  "nbwhs",
#else
							  "nbw",
#endif
							  long_options, NULL)) != EOF) {
		if (opt == 'n')
			mode |= NETMASK;
		else if (opt == 'b')
			mode |= BROADCAST;
		else if (opt == 'w')
			mode |= NETWORK;
#ifdef CONFIG_FEATURE_IPCALC_FANCY
		else if (opt == 'h')
			mode |= HOSTNAME;
		else if (opt == 's')
			mode |= SILENT;
#endif
		else {
			show_usage();
		}
	}

	if (mode & (BROADCAST | NETWORK)) {
		if (argc - optind > 2) {
			show_usage();
		}
	} else {
		if (argc - optind != 1) {
			show_usage();
		}
	}

	ipaddr = inet_addr(argv[optind]);

	if (ipaddr == INADDR_NONE) {
		IPCALC_MSG(error_msg_and_die("bad IP address: %s\n", argv[optind]),
				   exit(EXIT_FAILURE));
	}


	if (argc - optind == 2) {
		netmask = inet_addr(argv[optind + 1]);
	}

	if (ipaddr == INADDR_NONE) {
		IPCALC_MSG(error_msg_and_die("bad netmask: %s\n", argv[optind + 1]),
				   exit(EXIT_FAILURE));
	}

	/* JHC - If the netmask wasn't provided then calculate it */
	if (!netmask) {
		netmask = get_netmask(ipaddr);
	}

	if (mode & NETMASK) {
		printf("NETMASK=%s\n", inet_ntoa((*(struct in_addr *) &netmask)));
	}

	if (mode & BROADCAST) {
		broadcast = (ipaddr & netmask) | ~netmask;
		printf("BROADCAST=%s\n", inet_ntoa((*(struct in_addr *) &broadcast)));
	}

	if (mode & NETWORK) {
		network = ipaddr & netmask;
		printf("NETWORK=%s\n", inet_ntoa((*(struct in_addr *) &network)));
	}
#ifdef CONFIG_FEATURE_IPCALC_FANCY
	if (mode & HOSTNAME) {
		struct hostent *hostinfo;
		int x;

		hostinfo = gethostbyaddr((char *) &ipaddr, sizeof(ipaddr), AF_INET);
		if (!hostinfo) {
			IPCALC_MSG(error_msg("cannot find hostname for %s", argv[optind]);
					   herror(NULL);
					   putc('\n', stderr);,);
			exit(EXIT_FAILURE);
		}
		for (x = 0; hostinfo->h_name[x]; x++) {
			hostinfo->h_name[x] = tolower(hostinfo->h_name[x]);
		}

		printf("HOSTNAME=%s\n", hostinfo->h_name);
	}
#endif

	return EXIT_SUCCESS;
}

/* END CODE */
/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
