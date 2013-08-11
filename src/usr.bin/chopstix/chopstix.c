/* $Gateweaver: chopstix.c,v 1.24 2007/08/20 17:05:40 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <sys/time.h>
#include <err.h>
#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: chopstix.c,v 1.24 2007/08/20 17:05:40 cmaxwell Exp $");

ChopstixConfig config;
ChopstixHeader header;
int exitcode = 0;
char *exitmsg = NULL;
int verbose = 0;

static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-v] [-f file]\n", __progname);
	exit(1);
}

/*
 * Trigger the exit of the program.  This will happen once control is returned
 * to the event handler.  Main program catches the exitcode and prints the last
 * status message (if not already set)
 */
void
chopstix_exit(void)
{
	exitcode = 1;

	/* this can fail in out-of-memory conditions, oh well */
	exitmsg = strdup(status.status);

	event_loopexit(NULL);
}

int
main(int argc, char **argv)
{
	int ch;
	const char *conffile;

	conffile = CONFFILE;

	while ((ch = getopt(argc, argv, "f:v")) != -1) {
		switch (ch) {
			case 'f':
				conffile = optarg;
				break;
			case 'v':
				verbose++;
				break;
			default:
				usage();
				/* NOTREACHED */
		}
	}

	bzero(&config, sizeof(config));
	config.print.feedlines = -1;
	config.print.cchide_customer = 1;
	config.print.ccsig_customer = 1;
	bzero(&header, sizeof(header));
	if (parse_config(conffile))
		exit(1);

	/* set database files */
	if (config.database.custdb == NULL)
		if ((config.database.custdb = strdup(config.database.alldb
				? config.database.alldb : CUSTDBFILE)) == NULL)
			err(1, "cannot set customer database file");
	if (config.database.menudb == NULL)
		if ((config.database.menudb = strdup(config.database.alldb
				? config.database.alldb : MENUDBFILE)) == NULL)
			err(1, "cannot set menu database file");
	if (config.database.orderdb == NULL)
		if ((config.database.orderdb = strdup(config.database.alldb
				? config.database.alldb : ORDERDBFILE)) == NULL)
			err(1, "cannot set order database file");
	if (config.print.feedlines < 0)
		config.print.feedlines = PRINT_FEED_LINES;

	if (licence_init() == -1)
		errx(1, "cannot proceed without licence");

	event_init();

	module_init();
	rule_init();

	input_init();
	print_init();
	form_init();
	display_init();
	status_init();
	menu_init();
	customer_init();
	special_init();
	payment_init();
	window_init();
	order_init();

	display_refresh();

	signal(SIGPIPE, SIG_IGN);
	if (event_dispatch() == -1)
		err(1, "terminated abnormally");

	if (exitcode != 0 && exitmsg == NULL)
		exitmsg = strdup(status.status);

	order_exit();
	window_exit();
	payment_exit();
	special_exit();
	customer_exit();
	menu_exit();
	status_exit();
	display_exit();
	form_exit();
	print_exit();
	input_exit();

	rule_exit();
	module_exit();

	if (exitcode && exitmsg != NULL)
		errx(exitcode, "%s", exitmsg);
	else
		fprintf(stdout, "exiting normally\n");

	exit(0);
}
