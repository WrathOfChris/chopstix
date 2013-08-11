/* $Gateweaver: render.c,v 1.7 2007/09/05 16:00:03 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "report.h"

void
render_error(const char *fmt, ...)
{
	va_list ap;
	char s[8192];
	tbstring sa = {0};

	va_start(ap, fmt);
	vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);
	printf("%s\n\n", conf.ct_html);
	printf("<html><head><title>Error</title></head><body>\n");
	printf("<h2>Error</h2><p><b>%s</b><p>\n", s);
	if (q != NULL) {
		printf("Request: <b>%s</b><br>\n",
		    html_esc(q->query_string, &sa, 0));
		printf("Address: <b>%s</b><br>\n",
		    html_esc(q->remote_addr, &sa, 0));
		if (q->user_agent != NULL)
			printf("User agent: <b>%s</b><br>\n",
			    html_esc(q->user_agent, &sa, 0));
	}
	printf("<p>Please send reports with instructions about how to "
	    "reproduce to <a href=\"mailto:%s\"><b>%s</b></a><p>\n",
	    conf.mailaddr, conf.mailaddr);
	printf("</body></html>\n");

	tbstrfree(&sa);
}

int
render_html(const char *html_fn, render_cb r, void *arg)
{
	FILE *f;
	char s[8192];
	char fn[MAXPATHLEN];
	const char *s2;

	if ((f = fopen(html_fn, "r")) == NULL) {
		printf("ERROR: fopen: %s: %s<br>\n", html_fn, strerror(errno));
		return (1);
	}
	while (fgets(s, sizeof(s), f)) {
		char *a, *b;

		for (a = s; (b = strstr(a, "%%")) != NULL;) {
			*b = 0;
			printf("%s", a);
			a = b + 2;
			if ((b = strstr(a, "%%")) != NULL) {
				*b = 0;
				if (!strcmp(a, "ACTION")) {
					if ((s2 = get_query_param(q, "action")) == NULL)
						s2 = "front";
					printf("%s", s2);
				} else if (!strcmp(a, "BASEURL"))
					printf("%s", conf.baseurl);
				else if (!strcmp(a, "BASEDIR"))
					printf("%s", conf.htmldir);
				else if (!strcmp(a, "BASECSS")) {
					printf("%s?action=css%s", conf.baseurl,
							display_type == DISPLAY_PRINT ?
							"&amp;display=print" :
							display_type == DISPLAY_CSV ?
							"&amp;display=csv" : "");
				} else if (!strcmp(a, "TITLE"))
					printf("%s", conf.title);
				else if (!strcmp(a, "HEADER")) {
					snprintf(fn, sizeof(fn), "%s/header.html", conf.htmldir);
					render_html(fn, NULL, NULL);
				} else if (!strcmp(a, "FOOTER")) {
					snprintf(fn, sizeof(fn), "%s/footer.html", conf.htmldir);
					render_html(fn, NULL, NULL);
				} else if (!strcmp(a, "ROW")) {
					printf("row%d", rowselect);
					rowselect = !rowselect;
				} else if (!strcmp(a, "ROWRESET"))
					rowselect = 0;
				else if (!strcmp(a, "TIMESTART"))
					printf("%s", time_print(time_start));
				else if (!strcmp(a, "TIMESTOP"))
					printf("%s", time_print(time_stop));
				else if (!strcmp(a, "TIMENOW")) {
					char datetime[sizeof("9999-12-31T23:59:60+0000")];
					time_t now = time(NULL);
					strftime(datetime, sizeof(datetime), "%FT%T%z",
							localtime(&now));
					printf("%s", datetime);
				} else if (!strcmp(a, "TIMETODAY"))
					printf("%s", time_print(time_ext(TIME_TODAY)));
				else if (!strcmp(a, "TIMEYESTERDAY"))
					printf("%s", time_print(time_ext(TIME_YESTERDAY)));
				else if (!strcmp(a, "TIMETOMORROW"))
					printf("%s", time_print(time_ext(TIME_TOMORROW)));
				else if (!strcmp(a, "TIMETHISWEEK"))
					printf("%s", time_print(time_ext(TIME_THISWEEK)));
				else if (!strcmp(a, "TIMETHISMONTH"))
					printf("%s", time_print(time_ext(TIME_THISMONTH)));
				else if (!strcmp(a, "TIMETHISYEAR"))
					printf("%s", time_print(time_ext(TIME_THISYEAR)));
				else if (!strcmp(a, "TIMELASTWEEK"))
					printf("%s", time_print(time_ext(TIME_LASTWEEK)));
				else if (!strcmp(a, "TIMELASTMONTH"))
					printf("%s", time_print(time_ext(TIME_LASTMONTH)));
				else if (!strcmp(a, "TIMELASTYEAR"))
					printf("%s", time_print(time_ext(TIME_LASTYEAR)));
				else if (!strcmp(a, "QUERY"))
					printf("%s", q->query_string);
#if 0
				else if (!strcmp(a, "RCSID"))
					printf("%s", rcsid);
#endif
				else if (r != NULL)
					(*r)(a, arg);
				a = b + 2;
			}
		}
		printf("%s", a);
	}
	fclose(f);
	return 0;
}

void
render_front(const char *m, void *arg)
{
	if (!strcmp(m, "%%NOTAREALTAG%%")) {
		;
	} else
		printf("render_front: unknown macro '%s'<br>\n", m);
}
