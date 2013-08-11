/* $Gateweaver: status.c,v 1.10 2005/12/05 22:19:18 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "chopstix.h"

RCSID("$Gateweaver: status.c,v 1.10 2005/12/05 22:19:18 cmaxwell Exp $");

ChopstixStatus status;
extern char *__progname;

void
status_init(void)
{
	bzero(&status, sizeof(status));
	status_set("OK");
	openlog(__progname, LOG_NDELAY, LOG_DEST);
}

void
status_set(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(status.status, sizeof(status.status), fmt, ap);
	va_end(ap);

	status.bad = 0;
}

void
status_clear(void)
{
	strlcpy(status.status, "", sizeof(status.status));
	status.bad = 0;
}

void
status_err(const char *emsg, ...)
{
	va_list ap;

	if (emsg == NULL)
		snprintf(status.status, sizeof(status.status), "%s", strerror(errno));
	else {
		va_start(ap, emsg);
		vsnprintf(status.status, sizeof(status.status), emsg, ap);
		va_end(ap);
		if (errno != 0) {
			strlcat(status.status, ": ", sizeof(status.status));
			strlcat(status.status, strerror(errno), sizeof(status.status));
		}
	}
	syslog(LOG_ERR, "%s", status.status);
	status.bad = 1;
}

void
status_dberr(const char *fmt, va_list ap)
{
	vsyslog(LOG_ERR, fmt, ap);
	status.bad = 1;
}

void
status_ruleerr(const char *fmt, va_list ap)
{
	vsyslog(LOG_ERR, fmt, ap);
	status.bad = 1;
}

void
status_warn(const char *emsg, ...)
{
	va_list ap;

	va_start(ap, emsg);
	vsnprintf(status.status, sizeof(status.status), emsg, ap);
	va_end(ap);
	syslog(LOG_WARNING, "%s", status.status);
	status.bad = 1;
}

void
status_exit(void)
{
	closelog();
}
