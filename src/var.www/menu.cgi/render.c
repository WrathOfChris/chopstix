/* $Gateweaver: render.c,v 1.4 2007/09/27 01:46:04 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

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
	tbstring sa = {0};

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
					if (action == NULL)
						action = "front";
					printf("%s", action);
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
				else if (!strcmp(a, "MENUCODE")) {
					printf("%s", html_esc(menucode, &sa, 0));
				} else if (!strcmp(a, "QUERY"))
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
	tbstrfree(&sa);
	return 0;
}

void
render_front(const char *m, void *arg)
{
	if (!strcmp(m, "%%NOTAREALTAG%%")) {
		;
	} else
		printf("render_front: unknown macro '%s'", m);
}

void
render_list(const char *m, void *arg)
{
	char fn[MAXPATHLEN];
	unsigned int u;

	if (!strcmp(m, "ITEMLIST")) {
		snprintf(fn, sizeof(fn), "%s/item.html", conf.htmldir);
		for (u = 0; u < menu.len; u++) {
			/* skip over deleted items */
			if (menu.val[u].flags.deleted)
				continue;
			render_html(fn, render_item, &menu.val[u]);
		}
	} else
		printf("render_list: unknown macro '%s'", m);
}

void
render_item(const char *m, void *arg)
{
	ChopstixMenuitem *mi = (ChopstixMenuitem *)arg;
	tbstring sa = {0};
	char fn[MAXPATHLEN];
	unsigned int u;

	if (mi == NULL)
		if ((mi = menu_getitem(menucode)) == NULL)
			return;

	if (!strcmp(m, "ITEMCODE"))
		printf("%s", html_esc(mi->code, &sa, 0));
	else if (!strcmp(m, "ITEMGEN"))
		printf("%d", mi->gen);
	else if (!strcmp(m, "ITEMFLAGS"))
		printf("%s", mi->flags.deleted ? "Deleted" : "Active");
	else if (!strcmp(m, "ITEMNAME"))
		printf("%s", html_esc(mi->name, &sa, 0));
	else if (!strcmp(m, "ITEMPRICE"))
		PRINTF_MONEY(mi->price);
	else if (!strcmp(m, "ITEMSTYLES")) {
		snprintf(fn, sizeof(fn), "%s/item_style.html", conf.htmldir);
		for (u = 0; u < mi->styles.len; u++)
			render_html(fn, render_style, &mi->styles.val[u]);
	} else if (!strcmp(m, "ITEMEXTRAS")) {
		snprintf(fn, sizeof(fn), "%s/item_extra.html", conf.htmldir);
		for (u = 0; u < mi->extras.len; u++)
			render_html(fn, render_extra, &mi->extras.val[u]);
	} else if (!strcmp(m, "ITEMSUBITEMS")) {
		snprintf(fn, sizeof(fn), "%s/item_subitem.html", conf.htmldir);
		if (mi->subitems)
			for (u = 0; u < mi->subitems->len; u++)
				render_html(fn, render_extra, &mi->subitems->val[u]);
	} else
		printf("render_item: unknown macro '%s'", m);

	tbstrfree(&sa);
}

void
render_style(const char *m, void *arg)
{
	ChopstixItemStyle *style = (ChopstixItemStyle *)arg;
	tbstring sa = {0};

	if (!strcmp(m, "ITEMSTYLENAME"))
		printf("%s", html_esc(style->name, &sa, 0));
	else
		printf("render_style: unknown macro '%s'", m);

	tbstrfree(&sa);
}

void
render_extra(const char *m, void *arg)
{
	ChopstixItemExtra *extra = (ChopstixItemExtra *)arg;
	ChopstixMenuitem *mi;
	tbstring sa = {0};

	if (!strcmp(m, "ITEMEXTRAQTY"))
		printf("%d", extra->qty);
	else if (!strcmp(m, "ITEMEXTRACODE"))
		printf("%s", html_esc(extra->code, &sa, 0));
	else if (!strcmp(m, "ITEMEXTRANAME")) {
		if ((mi = menu_getitem(extra->code)))
			printf("%s", html_esc(mi->name, &sa, 0));
	} else
		printf("render_extra: unknown macro '%s'", m);

	tbstrfree(&sa);
}

void
render_edit(const char *m, void *arg)
{
	ChopstixMenuitem *mi = (ChopstixMenuitem *)arg;
	tbstring sa = {0};
	char fn[MAXPATHLEN];
	unsigned int u;

	if (mi == NULL)
		if ((mi = menu_getitem(menucode)) == NULL)
			return;

	if (!strcmp(m, "ITEMCODE"))
		printf("%s", html_esc(mi->code, &sa, 0));
	else if (!strcmp(m, "ITEMGEN"))
		printf("%d", mi->gen);
	else if (!strcmp(m, "ITEMFLAGS"))
		printf("%s", mi->flags.deleted ? "Deleted" : "Active");
	else if (!strcmp(m, "ITEMNAME"))
		printf("%s", html_esc(mi->name, &sa, 0));
	else if (!strcmp(m, "ITEMPRICE"))
		PRINTF_MONEY(mi->price);
	else if (!strcmp(m, "EDITSTYLES")) {
		snprintf(fn, sizeof(fn), "%s/edit_style.html", conf.htmldir);
		for (u = 0; u < mi->styles.len; u++)
			render_html(fn, render_edit_style, &mi->styles.val[u]);
	} else if (!strcmp(m, "EDITEXTRAS")) {
		snprintf(fn, sizeof(fn), "%s/edit_extra.html", conf.htmldir);
		for (u = 0; u < mi->extras.len; u++)
			render_html(fn, render_edit_extra, &mi->extras.val[u]);
	} else if (!strcmp(m, "EDITSUBITEMS")) {
		snprintf(fn, sizeof(fn), "%s/edit_subitem.html", conf.htmldir);
		if (mi->subitems)
			for (u = 0; u < mi->subitems->len; u++)
				render_html(fn, render_edit_extra, &mi->subitems->val[u]);
	} else
		printf("render_list: unknown macro '%s'", m);

	tbstrfree(&sa);
}

void
render_edit_style(const char *m, void *arg)
{
	ChopstixItemStyle *style = (ChopstixItemStyle *)arg;
	ChopstixMenuitem *mi;
	tbstring sa = {0};

	if ((mi = menu_getitem(menucode)) == NULL)
		return;

	if (!strcmp(m, "ITEMCODE"))
		printf("%s", html_esc(mi->code, &sa, 0));
	else if (!strcmp(m, "ITEMGEN"))
		printf("%d", mi->gen);
	else if (!strcmp(m, "ITEMFLAGS"))
		printf("%s", mi->flags.deleted ? "Deleted" : "Active");
	else if (!strcmp(m, "ITEMSTYLENAME"))
		printf("%s", html_esc(style->name, &sa, 0));
	else
		printf("render_style: unknown macro '%s'", m);

	tbstrfree(&sa);
}

void
render_edit_extra(const char *m, void *arg)
{
	ChopstixItemExtra *extra = (ChopstixItemExtra *)arg;
	ChopstixMenuitem *mi;
	tbstring sa = {0};

	if ((mi = menu_getitem(menucode)) == NULL)
		return;

	if (!strcmp(m, "ITEMCODE"))
		printf("%s", html_esc(mi->code, &sa, 0));
	else if (!strcmp(m, "ITEMGEN"))
		printf("%d", mi->gen);
	else if (!strcmp(m, "ITEMFLAGS"))
		printf("%s", mi->flags.deleted ? "Deleted" : "Active");
	else if (!strcmp(m, "ITEMEXTRAQTY"))
		printf("%d", extra->qty);
	else if (!strcmp(m, "ITEMEXTRACODE"))
		printf("%s", html_esc(extra->code, &sa, 0));
	else if (!strcmp(m, "ITEMEXTRANAME")) {
		if ((mi = menu_getitem(extra->code)))
			printf("%s", html_esc(mi->name, &sa, 0));
	} else
		printf("render_extra: unknown macro '%s'", m);

	tbstrfree(&sa);
}
