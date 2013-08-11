/* $Gateweaver: report.c,v 1.13 2007/09/28 17:19:49 cmaxwell Exp $ */
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
#include "report.h"

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
struct orderdb_functions odbf = {0};
struct orderdb_handle odbh = {0};
struct custdb_functions cdbf = {0};
struct custdb_handle cdbh = {0};

ChopstixMenu menu;
time_t time_start;
time_t time_stop;
enum display_type display_type = DISPLAY_SCREEN;
int rowselect = 0;

static void dberr_syslog(const char *, va_list);

/* module init functions */
void mdb_init(struct menudb_functions *);
void odb_init(struct orderdb_functions *);
void cdb_init(struct custdb_functions *);

int
main(int argc, char **argv)
{
	char fn[MAXPATHLEN];
	const char *action, *s;
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

	/* order db handler */
	odbh.dbfile = conf.dbfile;
	odbf.err = &dberr_syslog;
	odb_init(&odbf);
	if (odbf.open(&odbh) == -1) {
		render_error("cannot open order database");
		goto done;
	}

	/* customer db handler */
	cdbh.dbfile = conf.dbfile;
	cdbf.err = &dberr_syslog;
	cdb_init(&cdbf);
	if (cdbf.open(&cdbh) == -1) {
		render_error("cannot open customer database");
		goto done;
	}

	if ((s = get_query_param(q, "start")))
		time_start = time_parse(s);
	else
		time_start = time_today(TIME_START);

	if ((s = get_query_param(q, "stop")))
		time_stop = time_parse(s);
	else
		time_stop = time_today(TIME_END);

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
			snprintf(fn, sizeof(fn), "%s/report_print.css", conf.htmldir);
		else if (display_type == DISPLAY_CSV)
			snprintf(fn, sizeof(fn), "%s/report_csv.css", conf.htmldir);
		else
			snprintf(fn, sizeof(fn), "%s/report.css", conf.htmldir);
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

	} else if (strcmp(action, "orders") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/order.html", conf.htmldir);
		render_html(fn, &render_order, NULL);

	} else if (strcmp(action, "orderdetail") == 0) {
		int64_t order_ID;
		const char *code, *errstr;
		ChopstixOrder order;
		/*
		 * Single order detail
		 */
		if ((code = get_query_param(q, "code")) == NULL) {
			render_error("need code for order detail");
			goto done;
		}
		order_ID = strtonum(code, 0, LLONG_MAX, &errstr);
		if (errstr) {
			render_error("'%s' is %s", code, errstr);
			goto done;
		}

		if (odbf.get(&odbh, order_ID, &order) == -1) {
			render_error("cannot load order %lld, see system log for details",
					order_ID);
			goto done;
		}
		if (cdbf.get(&cdbh, &order.customer.phone, &order.customer)
				== -1) {
			render_error("cannot load customer, see system log for details");
			goto done;
		}

		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/order_detail.html", conf.htmldir);
		render_html(fn, &render_order_list, &order);

	} else if (strcmp(action, "customers") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/customer.html", conf.htmldir);
		render_html(fn, &render_customer_list, NULL);

	} else if (strcmp(action, "custdetail") == 0) {
		ChopstixCustomer cust;
		ChopstixPhone phone;
		const char *code;

		bzero(&cust, sizeof(cust));
		snprintf(fn, sizeof(fn), "%s/cust_detail.html", conf.htmldir);

		if ((code = get_query_param(q, "cust")) == NULL) {
			render_error("need phone for customer detail");
			goto done;
		}
		cust_str2phone(code, &phone);
		if (cdbf.get(&cdbh, &phone, &cust) == -1) {
			render_error("cannot find customer");
			goto done;
		}

		printf("%s\n\n", conf.ct_html);
		render_html(fn, &render_customer_info, &cust);
		free_ChopstixCustomer(&cust);

	} else if (strcmp(action, "items") == 0) {
		void *list;
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/item.html", conf.htmldir);
		list = item_load();
		render_html(fn, &render_items, list);
		item_free(list);

	} else if (strcmp(action, "payments") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/payment.html", conf.htmldir);
		render_html(fn, &render_payment, NULL);

	} else if (strcmp(action, "time") == 0) {
		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/time.html", conf.htmldir);
		render_html(fn, NULL, NULL);

	} else {
		render_error("unknown command");
	}

done:
	fflush(stdout);
	if (q)
		free_query(q);
	cdbf.close(&cdbh);
	odbf.close(&odbh);
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
