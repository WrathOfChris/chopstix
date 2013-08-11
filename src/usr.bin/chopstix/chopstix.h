/* $Gateweaver: chopstix.h,v 1.67 2007/09/24 14:31:27 cmaxwell Exp $ */
#ifndef CHOPSTIX_H
#define CHOPSTIX_H
#include <sys/queue.h>
#include <sys/param.h>
#include <stdarg.h>
#include <stdlib.h>
#include "chopstix_asn1.h"
#include "chopstix_api.h"
#include "str.h"

#define CONFFILE 	"/etc/chopstix.conf"
#define LICENCEFILE	"/etc/chopstix.licence"
#define REVOKEFILE	"/etc/chopstix.crl"
#define CUSTDBFILE	"/var/db/chopstix/chopstix.db"
#define MENUDBFILE	"/var/db/chopstix/chopstix.db"
#define ORDERDBFILE	"/var/db/chopstix/chopstix.db"
#define LOG_DEST	LOG_USER

#define PHONE_NPA_MAX	999
#define PHONE_NXX_MAX	999
#define PHONE_NUM_MAX	9999
#define PHONE_EXT_MAX	99999

#define KEY_BSPACE		0x08	/* CTRL-H, <backspace> */
#define KEY_TAB			0x09
#define KEY_NEWORDER	0x0E	/* CTRL-N */
#define KEY_PRINTORDER	0x10	/* CTRL-P */

#define CHOPSTIX_COLOR_NORMAL	COLOR_PAIR(1)
#define CHOPSTIX_COLOR_STATUS	COLOR_PAIR(2)
#define CHOPSTIX_COLOR_ERROR	COLOR_PAIR(3)
#define CHOPSTIX_COLOR_FORM		COLOR_PAIR(4)
#define CHOPSTIX_COLOR_ALERT	COLOR_PAIR(5)
#define CHOPSTIX_COLOR_TITLE	COLOR_PAIR(6)
#define CHOPSTIX_COLOR_HELP		COLOR_PAIR(7)
#define CHOPSTIX_COLOR_WINDOW	COLOR_PAIR(8)

#define MVADDSTR(type, y, x, s) do {	\
	attron(CHOPSTIX_COLOR_##type);		\
	mvaddstr((y), (x), (s));			\
	attroff(CHOPSTIX_COLOR_##type);		\
} while (0)

#define ADDSTR(type, s) do {			\
	attron(CHOPSTIX_COLOR_##type);		\
	addstr((s));						\
	attroff(CHOPSTIX_COLOR_##type);		\
} while (0)

#define ADDCH(type, c) do {				\
	attron(CHOPSTIX_COLOR_##type);		\
	addch((c));							\
	attroff(CHOPSTIX_COLOR_##type);		\
} while (0)

#define CHOPSTIX_CHAR_FORM		' '

#define CHOPSTIX_DATE_FORMAT	"%F %T"
#define CHOPSTIX_DATE_SIZE		sizeof("YYYY-MM-DD HH:MM:SS")
#define CHOPSTIX_PHONE_RAWSIZE	sizeof("NPANXXNMBRxEXTN5")
#define CHOPSTIX_PHONE_SIZE		sizeof("(NPA) NXX-NMBR xEXTN5")

/* default form label */
#define CHOPSTIX_FORM_LABEL		": "

/* maximum nesting depth for loop detection */
#define CHOPSTIX_FORM_NESTING	7

#define ALIGN_MONEY			7		/* $999.99 */
#define ALIGN_NUMBER		11		/* -2147483647 */
#define ALIGN_MONEYWIDTH	(ALIGN_NUMBER + 2)
/* size of number with label header and "$." */
#define ALIGN_MONEY_MAX		(strlen(CHOPSTIX_FORM_LABEL) + ALIGN_MONEYWIDTH)
#define ALIGN_SUBTOTAL_LABELSIZE	8
#define ALIGN_REASONWIDTH	40

#define LINE_TITLE			0
#define LINE_TITLE_SIZE		6
#define LINE_ORDER			(LINE_TITLE + LINE_TITLE_SIZE)
#define LINE_ORDER_MINSIZE	2
#define LINE_STATUS_SIZE	1
#define LINE_STATUS			(LINES - LINE_STATUS_SIZE)
#define LINE_TOTAL_SIZE		7
#define LINE_TOTAL			(LINES - LINE_TOTAL_SIZE - LINE_STATUS_SIZE)
#define LINE_SPECIAL		(LINE_TOTAL + 1)
#define LINE_SPECIAL_SIZE	3
#define LINE_PAYMENT_SIZE	1
#define LINE_PAYMENT		(LINE_STATUS - LINE_PAYMENT_SIZE)
#define LINE_ORDER_SIZE		(LINE_TOTAL - LINE_ORDER - 1)
#define LINE_REASON_SIZE	1
#define ORDER_FIELDS		5

#define PRINT_CMD			"/usr/bin/lpr"
#define PRINT_CMD_QUEUE		"-P"
#define PRINT_PAD			' '
#define PRINT_LINE			'-'
#define PRINT_NL			"\r\n"
#define PRINT_HALF_DEFAULT	38
#define PRINT_STARS			"***  "
#define PRINT_SHIFT			12
#define PRINT_DATE_FORMAT	"%F"
#define PRINT_DATE_SIZE		sizeof("YYYY-MM-DD")
#define PRINT_TIME_FORMAT	"%T"
#define PRINT_TIME_SIZE		sizeof("HH:MM:SS")
#define PRINT_FEED_LINES	8
#define PRINT_SIGNATURE		"Signature: _________________________"

/* Must be even, and twice the VL_HALF separater */
#define PRINT_VL			"        "
#define PRINT_VL_HALF		"    "

/* Licence allowable clock skew */
#define LICENCE_SKEW		(60 * 60 * 24)

typedef struct ChopstixStatus {
	char status[16384];
	int bad;
} ChopstixStatus;

typedef struct ChopstixConfig {
	char *tax1name;					/* GST */
	int tax1rate;
	char *tax1reg;
	char *tax2name;					/* PST */
	int tax2rate;
	char *tax2reg;
	int phoneprefix;				/* area code autoprefix */
	char module[MAXPATHLEN];		/* loadable module name */
	char rulemodule[MAXPATHLEN];	/* loadable module name */
	char licence[MAXPATHLEN];		/* path to license file */
	struct {
		char cmd[MAXPATHLEN];
		char queue[MAXPATHLEN];
		int columns;
		int feedlines;
		int title_bold;
		int order_bold;
		int show_city:1,
			show_province:1,
			show_country:1,
			cchide_customer:1,
			cchide_kitchen:1,
			ccsig_customer:1,
			ccsig_kitchen:1;
	} print;
	struct {
		char *custdb;
		char *menudb;
		char *orderdb;
		char *alldb;
	} database;
} ChopstixConfig;

typedef enum {
	FIELD_POS_NONE,
	FIELD_POS_RIGHT,
	FIELD_POS_LEFT,
	FIELD_POS_UP,
	FIELD_POS_DOWN,
	FIELD_POS_WLEFT,
	FIELD_POS_WRIGHT,
	FIELD_POS_HOME,
	FIELD_POS_END,
	/* FIELD CONTROL */
	FIELD_NEXT,						/* next field */
	FIELD_PREV,						/* prev field */
	FIELD_STORE,
	FIELD_BEGIN,					/* first displayable field on line */
	FIELD_HOTSTORE,
	/* FORMS */
	FORM_NONE,
	/* FORM CONTROL */
	FORM_ENTER_UP,
	FORM_ENTER_DOWN,
	FORM_ENTER_RIGHT,
	FORM_ENTER_LEFT,
	/* FORM ACTIONS */
	FORM_EXIT_UP,
	FORM_EXIT_DOWN,
	FORM_EXIT_RIGHT,
	FORM_EXIT_LEFT
} ChopstixFormAction;

typedef struct ChopstixFormField {
	TAILQ_ENTRY(ChopstixFormField) entry;
	int y, x;						/* (y,x) of the top-left corner */
	int w, l;						/* width, length */
	int iw;							/* input width */
	int cpos;						/* cursor position */
	int displayonly;				/* never allow cursor here */
	int rightalign;					/* align text to right of field */
	int hotfield;					/* field is 'hot' always store */
	int overwrite;					/* field just entered, overwrite on input */
	char *label;
	string input;
	int (*validate)(const char *, int *, void *);
	const char * (*getstr)(const char *, void *);
	int (*getcpos)(const char *, int, ChopstixFormAction, void *);
	int (*getrpos)(const char *, int, void *);
	ChopstixFormAction (*putdata)(const char *, void *);
	void *arg;
} ChopstixFormField;

typedef struct ChopstixForm {
	TAILQ_HEAD(ChopstixFormField_tq, ChopstixFormField) fields;
	int len;						/* length of the array */
	int cur;						/* current cursor position */
	int align;						/* align labels */
	char *labelsep;					/* label seperator, default ": " */
} ChopstixForm;

extern ChopstixConfig config;
extern ChopstixForm addrform;
extern ChopstixForm orderform;
extern ChopstixForm specialform;
extern ChopstixForm payform;
extern ChopstixHeader header;
extern ChopstixCustomer customer;	/* XXX for mockup only */
extern ChopstixOrder order;
extern ChopstixStatus status;
extern ChopstixForm discountform;	/* window */
extern ChopstixForm deliveryform;	/* window */
extern ChopstixForm chphoneform;	/* window */
extern ChopstixForm dailyinfoform;	/* window */
extern ChopstixForm cashinform;		/* window */
extern ChopstixForm creditform;		/* window */
extern int verbose;

/* chopstix.c */
void chopstix_exit(void);

/* display.c */
struct display {
	int has_colour;
	struct {
		int pos;		/* display starting at item # */
	} order;
};
void display_init(void);
void display_money(int, char *);
void display_pad(void);
void display_header(ChopstixHeader *);
void display_form(ChopstixForm *);
void display_window(ChopstixForm *);
void display_order(ChopstixOrder *);
void display_order_title(int);
void display_status(ChopstixStatus *);
void display_credits(ChopstixOrder *);
void display_refresh(void);
void display_exit(void);

/* input.c */
struct input {
	void (*keycb)(int);
};
void input_init(void);
void input_set_keycb(void (*)(int));
void input_handlekey(int, void *);
void input_exit(void);

/* licence.c */
int licence_init(void);
int licence_valid(void);

/* order.c */
void order_init(void);
void order_refresh(int);
void order_reinit(int, int *);
int order_new(void);
int order_getlines(ChopstixOrderItems *);
int order_getlast(const ChopstixPhone *, ChopstixOrder *);
ChopstixOrderItem * order_add_item(ChopstixOrder *);
const char * order_get_styletext(ChopstixItemStyles *, int);
void order_tally(ChopstixOrder *);
int order_post(void);
void order_edit_special(ChopstixFormField *, int);
void order_exit(void);
int order_getdaily_total(void);

/* menu.c */
void menu_init(void);
ChopstixMenuitem * menu_getitem(const ChopstixItemCode);
void menu_exit(void);

/* status.c */
void status_init(void);
void status_set(const char *, ...)
	__attribute__((__format__(printf, 1, 2)));
void status_err(const char *emsg, ...)
	__attribute__((__format__(printf, 1, 2)));
void status_warn(const char *emsg, ...)
	__attribute__((__format__(printf, 1, 2)));
void status_dberr(const char *, va_list);
void status_ruleerr(const char *, va_list);
void status_clear(void);
void status_exit(void);

/* customer.c */
void customer_init(void);
void customer_new(ChopstixCustomer *);
void customer_die(void);
int customer_getcredits(ChopstixCustomer *);
int customer_getcredit_total(void);
int customer_getcredit_remain(void);
void customer_exit(void);
int customer_load(struct ChopstixCustomer *);
int customer_post(struct ChopstixCustomer *);
int customer_update_phone(const char *);

/* parse.y */
int parse_config(const char *);

/* form.c */
#define form_getyx(a, y, x)	form_getyx_int((a), &(y), &(x))
void form_init(void);
void form_wipe_input(ChopstixForm *);
ChopstixFormField * form_field_new(const char *);
void form_field_add(ChopstixForm *, ChopstixFormField *);
void form_field_del(ChopstixForm *, ChopstixFormField *);
void form_field_insafter(ChopstixForm *, ChopstixFormField *,
		ChopstixFormField *);
void form_field_free(ChopstixFormField *);
void form_field_setyx(ChopstixFormField *, int, int);
void form_field_setwl(ChopstixFormField *, int, int);
void form_field_setiw(ChopstixFormField *, int);
void form_field_setarg(ChopstixFormField *, void *);
void form_field_setdisplayonly(ChopstixFormField *, int);
void form_field_setrightalign(ChopstixFormField *, int);
int form_field_llen(ChopstixFormField *);
int form_field_flen(ChopstixFormField *);
const char * form_field_getstr(ChopstixFormField *);
int form_field_getcpos(ChopstixFormField *);
int form_field_getrpos(ChopstixFormField *);
int form_field_putdata(ChopstixFormField *);
int form_getalign(ChopstixForm *);
int form_doalign(ChopstixForm *, ChopstixFormField *);
int form_field_active(ChopstixForm *, ChopstixFormField *);
ChopstixFormAction form_driver(ChopstixForm *, ChopstixFormAction);
ChopstixFormAction form_input(ChopstixForm *, int);
void form_getyx_int(ChopstixForm *, int *, int *);
void form_setfield_cur(ChopstixForm *, ChopstixFormField *);
void form_exit(void);
ChopstixFormField * getfield_cur(ChopstixForm *);

const char * form_field_getstr_default(const char *, void *);
const char * form_field_getstr_phone(const char *, void *);
const char * form_field_getstr_money(const char *, void *);
const char * form_field_getstr_discount(const char *, void *);
int form_field_validate_default(const char *, int *, void *);
int form_field_validate_number(const char *, int *, void *);
int form_field_validate_money(const char *, int *, void *);
int form_field_validate_phone(const char *, int *, void *);
int form_field_validate_discount(const char *, int *, void *);
int form_field_getcpos_default(const char *, int, ChopstixFormAction, void *);
int form_field_getcpos_phone(const char *, int, ChopstixFormAction, void *);
int form_field_getcpos_money(const char *, int, ChopstixFormAction, void *);
int form_field_getcpos_discount(const char *, int, ChopstixFormAction, void *);
int form_field_getrpos_default(const char *, int, void *);
int form_field_getrpos_phone(const char *, int, void *);

/* payment.c */
void payment_init(void);
void payment_reinit(void);
void payment_exit(void);

/* special.c */
void special_init(void);
void special_reinit(void);
void special_exit(void);

/* module.c */
void module_init(void);
void module_init_customer(struct custdb_functions *);
void module_init_menu(struct menudb_functions *);
void module_init_order(struct orderdb_functions *);
void module_exit(void);

/* print.c */
void print_init(void);
int print_order(ChopstixOrder *, int);
void print_exit(void);

/* window.c */
#define WINDOW_TEST(cf) \
	((cf) == &discountform || (cf) == &deliveryform || (cf) == &chphoneform \
	 || (cf) == &dailyinfoform || (cf) == &cashinform || (cf) == &creditform)
void window_init(void);
void window_reinit(void);
void window_exit(void);
void window_update_daily(ChopstixTotal *);
void window_update_cashin(ChopstixTotal *);
void window_update_credit(void);
void window_wipe(void);

/* rule.c */
void rule_init(void);
void rule_init_module(struct rule_functions *);
void rule_exit(void);
int rule_run(ChopstixOrder *, int);
int rule_getprice(char *);
ChopstixMenuitem * rule_getmenuitem(char *);

#endif
