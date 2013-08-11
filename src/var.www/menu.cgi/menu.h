/* $Gateweaver: menu.h,v 1.6 2007/10/15 17:26:38 cmaxwell Exp $ */
#ifndef FILEMAN_H
#define FILEMAN_H
#include <sys/param.h>
#include "cgi.h"
#include "chopstix_api.h"
#include "toolbox.h"

#define CONF_CONTTYPE	"Content-Type: text/html; charset=iso-8859-1"
#define CONF_CSSTYPE	"Content-Type: text/css; charset=iso-8859-1"
#define CONF_LOGOTYPE	"Content-Type: image/gif; charset=iso-8859-1"
#define CONF_FILE		"menu.conf"
#define CONF_ADDR		"chopstix@manorsoft.ca"
#define CONF_TITLE		"Chopstix Menu"
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
extern struct menudb_functions mdbf;
extern struct menudb_handle mdbh;
extern ChopstixMenu menu;
extern char *menucode;
extern int rowselect;
extern const char *action;

ChopstixMenuitem * menu_getitem(const char *);

void cust_str2phone(const char *, ChopstixPhone *);

enum edit_type { EDIT_NONE, EDIT_NEW, EDIT_ITEM, EDIT_STYLE, EDIT_EXTRA,
	EDIT_SUBITEM };
enum edit_cmd { ECMD_NONE, ECMD_CREATE, ECMD_CHANGE, ECMD_DELETE };
int edit_menu(enum edit_type, enum edit_cmd);

enum display_type { DISPLAY_SCREEN, DISPLAY_PRINT, DISPLAY_CSV };
extern enum display_type display_type;

typedef	void (*render_cb)(const char *, void *);
void render_error(const char *fmt, ...);
int render_html(const char *, render_cb, void *);

void render_front(const char *, void *);
void render_list(const char *, void *);
void render_item(const char *, void *);
void render_style(const char *, void *);
void render_extra(const char *, void *);
void render_edit(const char *, void *);
void render_edit_style(const char *, void *);
void render_edit_extra(const char *, void *);

#endif
