/* $Gateweaver: report.h,v 1.14 2007/10/15 17:26:38 cmaxwell Exp $ */
#ifndef FILEMAN_H
#define FILEMAN_H
#include <sys/param.h>
#include "cgi.h"
#include "chopstix_api.h"
#include "toolbox.h"

#define CONF_CONTTYPE	"Content-Type: text/html; charset=iso-8859-1"
#define CONF_CSSTYPE	"Content-Type: text/css; charset=iso-8859-1"
#define CONF_LOGOTYPE	"Content-Type: image/gif; charset=iso-8859-1"
#define CONF_FILE		"report.conf"
#define CONF_ADDR		"chopstix@manorsoft.ca"
#define CONF_TITLE		"Chopstix Reports"
#define CONF_DATABASE	"/var/db/chopstix/chopstix.db"
#define CONF_LOGO		"ChopstixLogo.gif"

#define PRINTF_MONEY(t) \
		printf("$%s%d.%02d", NEGSIGN((t)), DOLLARS((t)), CENTS((t)))

#define DATE_FORMAT		"%Y-%m-%d"

struct confinfo {
	char ct_html[MAXPATHLEN];
	char mailaddr[MAXPATHLEN];
	char htmldir[MAXPATHLEN];
	char baseurl[MAXPATHLEN];
	char title[MAXPATHLEN];
	char dbfile[MAXPATHLEN];
	char logofile[MAXPATHLEN];
	char ct_logo[MAXPATHLEN];
};

extern struct confinfo conf;
extern struct query *q;
//extern struct menudb_functions mdbf;
//extern struct menudb_handle mdbh;
extern struct orderdb_functions odbf;
extern struct orderdb_handle odbh;
extern struct custdb_functions cdbf;
extern struct custdb_handle cdbh;
extern ChopstixMenu menu;
extern time_t time_start;
extern time_t time_stop;
extern int rowselect;

ChopstixMenuitem * menu_getitem(const char *);

void cust_str2phone(const char *, ChopstixPhone *);

enum display_type { DISPLAY_SCREEN, DISPLAY_PRINT, DISPLAY_CSV };
extern enum display_type display_type;

void * item_load(void);
void item_free(void *);

enum time_type { TIME_START, TIME_END,
	TIME_TODAY, TIME_YESTERDAY, TIME_TOMORROW,
	TIME_THISWEEK, TIME_THISMONTH, TIME_THISYEAR,
	TIME_LASTWEEK, TIME_LASTMONTH, TIME_LASTYEAR };
time_t time_today(enum time_type);
time_t time_yesterday(enum time_type);
const char * time_print(time_t);
time_t time_parse(const char *);
time_t time_ext(enum time_type);

typedef	void (*render_cb)(const char *, void *);
void render_error(const char *fmt, ...);
int render_html(const char *, render_cb, void *);

void render_customer_credit(const char *, void *);
void render_customer_detail(const char *, void *);
void render_customer_info(const char *, void *);
void render_customer_list(const char *, void *);
void render_front(const char *, void *);
void render_items(const char *, void *);
void render_item_list(const char *, void *);
void render_order(const char *, void *);
void render_order_list(const char *, void *);
void render_order_totals(const char *, void *);
void render_order_item(const char *, void *);
void render_order_subitem(const char *, void *);
void render_payment(const char *, void *);

#endif
