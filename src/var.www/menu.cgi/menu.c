/* $Gateweaver: menu.c,v 1.5 2007/09/27 01:46:04 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "menu.h"

struct confinfo conf;
struct query *q = NULL;
extern const char *__progname;

tbkeyval conf_parse[] = {
	{ "ct_html",	conf.ct_html,	sizeof(conf.ct_html) },
	{ "mailaddr",	conf.mailaddr,	sizeof(conf.mailaddr) },
	{ "baseurl",	conf.baseurl,	sizeof(conf.baseurl) },
	{ "title",		conf.title,		sizeof(conf.title) },
	{ "database",	conf.dbfile,	sizeof(conf.dbfile) },
	{ "logo",		conf.logofile,	sizeof(conf.logofile) },
	{ "ct_logo",	conf.ct_logo,	sizeof(conf.ct_logo) },
	{ NULL,			NULL,			0 }
};

struct menudb_functions mdbf = {0};
struct menudb_handle mdbh = {0};

ChopstixMenu menu;
enum display_type display_type = DISPLAY_SCREEN;
int rowselect = 0;
char *menucode = NULL;
const char *action = NULL;

static void dberr_syslog(const char *, va_list);

/* module init functions */
void mdb_init(struct menudb_functions *);

int
main(int argc, char **argv)
{
	char fn[MAXPATHLEN];
	const char *s;
	int ret;

	tbox_init();

	bzero(&conf, sizeof(conf));
	strlcpy(conf.ct_html, CONF_CONTTYPE, sizeof(conf.ct_html));
	strlcpy(conf.mailaddr, CONF_ADDR, sizeof(conf.mailaddr));
	strlcpy(conf.htmldir, ".", sizeof(conf.htmldir));
	strlcpy(conf.baseurl, __progname, sizeof(conf.baseurl));
	strlcpy(conf.title, CONF_TITLE, sizeof(conf.title));
	strlcpy(conf.dbfile, CONF_DATABASE, sizeof(conf.dbfile));
	strlcpy(conf.logofile, CONF_LOGO, sizeof(conf.logofile));
	strlcpy(conf.ct_logo, CONF_LOGOTYPE, sizeof(conf.ct_logo));

	/* extra careful with cgi */
	umask(007);

#ifdef DEBUG_GDB
	fprintf(stderr, "GDBDEBUG: pid %u\n", getpid());
	sleep(10);
#endif

	if ((ret = tbloadconf(CONF_FILE, conf_parse))) {
		warnx("cannot load config file \"%s\": %s",
				CONF_FILE, tb_error(ret));
		/* FALLTHROUGH */
	}

	if ((q = get_query()) == NULL) {
		render_error("get_query");
		goto done;
	}

	if ((action = get_query_param(q, "action")) == NULL)
		action = "front";

	/* libchopstix_sql needs logging enabled */
	log_init(0);

	/* menu db handler */
	mdbh.dbfile = conf.dbfile;
	bzero(&menu, sizeof(menu));
	mdbf.err = &dberr_syslog;
	mdb_init(&mdbf);
	if (mdbf.open(&mdbh) == -1) {
		render_error("cannot open menu database");
		goto done;
	}
	if (mdbf.load(&mdbh, &menu) == -1) {
		render_error("cannot load menu, see system log for details");
		goto done;
	}

	if ((s = get_query_param(q, "menucode")))
		menucode = strdup(s);
	else
		menucode = strdup("");

	if ((s = get_query_param(q, "display"))) {
		if (!strcasecmp(s, "print"))
			display_type = DISPLAY_PRINT;
		else if (!strcasecmp(s, "csv"))
			display_type = DISPLAY_CSV;
	}

	if (strcmp(action, "front") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/front.html", conf.htmldir);
		render_html(fn, &render_front, NULL);

	} else if (strcmp(action, "css") == 0) {
		printf("%s\n\n", CONF_CSSTYPE);
		if (display_type == DISPLAY_PRINT)
			snprintf(fn, sizeof(fn), "%s/menu_print.css", conf.htmldir);
		else if (display_type == DISPLAY_CSV)
			snprintf(fn, sizeof(fn), "%s/menu_csv.css", conf.htmldir);
		else
			snprintf(fn, sizeof(fn), "%s/menu.css", conf.htmldir);
		render_html(fn, NULL, NULL);

	} else if (strcmp(action, "logo") == 0) {
		char buf[1024];
		struct stat st;
		ssize_t n;
		int fd;
		snprintf(fn, sizeof(fn), "%s/%s", conf.htmldir, conf.logofile);
		if ((fd = open(fn, O_RDONLY)) == -1) {
			render_error("cannot open logo");
			goto done;
		}
		fstat(fd, &st);
		printf( "%s\n"
				"Content-Length: %lld\n"
				"Content-Disposition: inline; filename=\"%s\"\n\n",
				CONF_LOGOTYPE, st.st_size, basename(conf.logofile));
		fflush(stdout);
		while ((n = read(fd, buf, sizeof(buf))) > 0)
			write(STDOUT_FILENO, buf, n);
		close(fd);

	} else if (strcmp(action, "edit") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/edit.html", conf.htmldir);
		render_html(fn, &render_edit, NULL);

	} else if (strncmp(action, "edit", 4) == 0) {
		enum edit_type etype = EDIT_NONE;
		enum edit_cmd ecmd = ECMD_NONE;

		if (strcmp(action, "editnew") == 0)
			etype = EDIT_NEW;
		else if (strcmp(action, "edititem") == 0)
			etype = EDIT_ITEM;
		else if (strcmp(action, "editstyle") == 0)
			etype = EDIT_STYLE;
		else if (strcmp(action, "editextra") == 0)
			etype = EDIT_EXTRA;
		else if (strcmp(action, "editsubitem") == 0)
			etype = EDIT_SUBITEM;
		else {
			render_error("invalid edit mode \"%s\"", action);
			goto done;
		}

		/* reset action to 'edit' for rendering */
		action = "edit";

		if (read_post(q, 0, tokenize_cb, &q->post_params) == -1) {
			render_error("read_post");
			goto done;
		}

		if ((s = get_post_param(q, "cmd")) == NULL) {
			render_error("edit requires \"cmd\"");
			goto done;
		}
		if (strcasecmp(s, "create") == 0)
			ecmd = ECMD_CREATE;
		else if (strcasecmp(s, "change") == 0)
			ecmd = ECMD_CHANGE;
		else if (strcasecmp(s, "delete") == 0)
			ecmd = ECMD_DELETE;
		else {
			render_error("invalid edit cmd mode \"%s\"", s);
			goto done;
		}
		
		if (edit_menu(etype, ecmd) == -1)
			goto done;

		/* reload menu, it changed */
		free_ChopstixMenu(&menu);
		bzero(&menu, sizeof(menu));
		if (mdbf.load(&mdbh, &menu) == -1) {
			render_error("cannot reload menu, see system log for details");
			goto done;
		}

		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/edit.html", conf.htmldir);
		render_html(fn, &render_edit, NULL);

	} else if (strcmp(action, "info") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/info.html", conf.htmldir);
		render_html(fn, &render_item, NULL);

	} else if (strcmp(action, "list") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/list.html", conf.htmldir);
		render_html(fn, &render_list, NULL);

	} else {
		render_error("unknown command");
	}

done:
	fflush(stdout);
	if (q)
		free_query(q);
	mdbf.close(&mdbh);
	return 0;
}

/*
 * Passthrough to syslog SQL errors from the database module
 */
static void
dberr_syslog(const char *fmt, va_list ap)
{
	vlog(LOG_ERR, fmt, ap);
}

ChopstixMenuitem *
menu_getitem(const char *code)
{
	unsigned int u;

	if (code == NULL || strlen(code) == 0)
		return NULL;

	for (u = 0; u < menu.len; u++)
		if (strcasecmp(code, menu.val[u].code) == 0)
			return &menu.val[u];

	return NULL;
}
