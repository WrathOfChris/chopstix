/* $Gateweaver: print.c,v 1.38 2007/09/25 18:28:38 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "chopstix.h"

RCSID("$Gateweaver: print.c,v 1.38 2007/09/25 18:28:38 cmaxwell Exp $");

#define PRINT_10CPI			"\022"		/* aka compressed off */
#define PRINT_12CPI			"\033:"
#define PRINT_17CPI			"\017"		/* aka compressed */
#define PRINT_1DOUBLEWIDE	"\016"
#define PRINT_NEWPAGE		"\014"
#define PRINT_PERFSKIP_ON	"\033N\001"
#define PRINT_PERFSKIP_OFF	"\033O"
#define PRINT_UNDERLINE_ON	"\033-\001"
#define PRINT_UNDERLINE_OFF	"\033-\001"
#define PRINT_ITALICS_ON	"\0334"
#define PRINT_ITALICS_OFF	"\0335"
#define PRINT_BOLD_ON		"\033E"
#define PRINT_BOLD_OFF		"\033F"
#define PRINT_2WIDE_ON		"\033W1"
#define PRINT_2WIDE_OFF		"\033W0"
#define PRINT_FONT_TITLE	"\033!\041"
#define PRINT_FONT_NORMAL	"\033!\001"
#define PRINT_FONT_TITLE_BOLD	"\033!\051"
#define PRINT_FONT_ORDER_BOLD	"\033!\011"

/* enable 'half' printing for doublewide font */
#define PCOLS(pl) ((pl)->half ? (PRINT_HALF / 2) : PRINT_HALF)

int PRINT_HALF = PRINT_HALF_DEFAULT;

typedef struct PLine {
	int same;
	int half;
	int code;
	string left;
	string right;
} PLine;

typedef struct PPage {
	unsigned int len;
	PLine *val;
} PPage;

static PLine * page_add_printf(PPage *pp, const char *fmt, ...)
	__attribute__((__format__(printf, 2, 3)));
static PLine * page_add_printf_wrap(PPage *pp, int shift, const char *fmt, ...)
	__attribute__((__format__(printf, 3, 4)));
static int line_append_printf_ralign(PPage *pp, PLine *pl, const char *fmt, ...)
	__attribute__((__format__(printf, 3, 4)));

void
print_init(void)
{
	if (strlen(config.print.cmd) == 0)
		if (strlcpy(config.print.cmd, PRINT_CMD, sizeof(config.print.cmd))
				>= sizeof(config.print.cmd))
			err(1, "cannot set print command");

	if (config.print.columns > 0)
		PRINT_HALF = (config.print.columns - strlen(PRINT_VL)) / 2;
}

/*
 * Add a line, setting the 'same' flag
 */
static PLine *
page_addline(PPage *pp, int same)
{
	PLine *pl;

	if ((pl = realloc(pp->val, (pp->len + 1) * sizeof(PLine))) == NULL)
		return NULL;
	pp->len++;
	pp->val = pl;

	pp->val[pp->len - 1].same = same;
	pp->val[pp->len - 1].half = 0;
	pp->val[pp->len - 1].code = 0;
	bzero(&pp->val[pp->len - 1].left, sizeof(pp->val[pp->len - 1].left));
	bzero(&pp->val[pp->len - 1].right, sizeof(pp->val[pp->len - 1].right));

	return &pp->val[pp->len - 1];
}

static void
line_free(PLine *pl)
{
	if (pl) {
		strafree(&pl->left);
		strafree(&pl->right);
	}
}

static const char *
str_filter(const char *str)
{
	static char buf[1024];
	char *s;

	if (str == NULL || *str == '\0')
		return str;

	strlcpy(buf, str, sizeof(buf));
	s = buf;

	do {
		*s = toupper(*s);
	} while (*s++);
	return buf;
}

static int
page_to_buffer(PPage *pp, char **buf, size_t *len)
{
	string print = {0};
	unsigned u;
	int i;

	if (!stracpy(&print, ""))
		goto fail;

	for (u = 0; u < pp->len; u++) {
		/* LEFT */
		if (pp->val[u].left.s)
			if (!stracats(&print, &pp->val[u].left))
				goto fail;
		/* CODE does not add newline */
		if (pp->val[u].code)
			continue;
		if (pp->val[u].same || pp->val[u].right.s) {
			/* PAD */
			for (i = pp->val[u].left.len; i < PCOLS(&pp->val[u]); i++)
				if (!strainsc(&print, PRINT_PAD, print.len))
					goto fail;
			/* VERTICAL SEPARATOR */
			if (!stracat(&print, pp->val[u].half ? PRINT_VL_HALF : PRINT_VL))
				goto fail;
		}
		if ((pp->val[u].same && pp->val[u].left.s) || pp->val[u].right.s) {
			/* RIGHT */
			if (pp->val[u].same) {
				if (pp->val[u].left.s)
					if (!stracats(&print, &pp->val[u].left))
						goto fail;
			} else {
				if (pp->val[u].right.s)
					if (!stracats(&print, &pp->val[u].right))
						goto fail;
			}
		}
		/* NEWLINE */
		if (!stracat(&print, PRINT_NL))
			goto fail;
	}

	*buf = print.s;
	*len = print.len;

	return 0;

fail:
	strafree(&print);
	return -1;
}

static PLine *
page_add_printf(PPage *pp, const char *fmt, ...)
{
	va_list ap;
	char buf[1024];
	PLine *pl;

	if ((pl = page_addline(pp, 1)) == NULL)
		return NULL;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (!stracpy(&pl->left, buf)) {
		line_free(pl);
		return NULL;
	}

	return pl;
}

static PLine *
page_add_printf_wrap(PPage *pp, int shift, const char *fmt, ...)
{
	va_list ap;
	char *str, *s, *sp;
	PLine *pl = NULL;
	int i;
	int wrapped = 0;

	va_start(ap, fmt);
	if (vasprintf(&str, fmt, ap) == -1)
		return NULL;
	va_end(ap);
	
	s = str;
	while (*s) {
		if ((pl = page_addline(pp, 1)) == NULL)
			goto fail;

		if (wrapped) {
			/* pad the left */
			for (i = 0; i < shift; i++)
				if (!strainsc(&pl->left, PRINT_PAD, 0))
					goto fail;
		}
		wrapped++;

		for (i = MIN(shift, pl->left.len); i < PRINT_HALF; i++) {
			if (*s == '\0')
				break;
			if (!isgraph(*s) && *s != ' ') {
				*s++;
				/* reloop on newline */
				if (*(s - 1) == '\n' || *(s - 1) == '\r')
					break;
				continue;
			}

			/* smartwrap */
			(const char *)sp = s;
			while (*sp != '\0' && *sp != ' ' && *sp != '-')
				sp++;
			if ((sp - s) > (PRINT_HALF - i) && (sp - s) < (PRINT_HALF - shift))
				break;

			/* no space on newline, but ignore leading spaces in 1st line */
			if (wrapped > 1 && i == shift && *s == ' ') {
				while (*s++ == ' ')
					;
				*s--;
				continue;
			}

			if (!strainsc(&pl->left, toupper(*s++), pl->left.len))
				goto fail;
		}
	}
	free(str);

	/* return the last line used */
	return pl;

fail:
	free(str);
	line_free(pl);
	return NULL;
}

static int
line_add_printf_r(PLine *pl, const char *fmt, ...)
{
	va_list ap;
	char buf[1024];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (!stracpy(&pl->right, buf))
		return -1;
	pl->same = 0;

	return 0;
}

static int
line_append_printf_ralign(PPage *pp, PLine *pl, const char *fmt, ...)
{
	va_list ap;
	char buf[1024];
	int pad = 0;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if ((pl->left.len + strlen(buf)) > PRINT_HALF)
		if ((pl = page_addline(pp, 1)) == NULL)
			return 0;

	pad = PRINT_HALF - strlen(buf) - pl->left.len;

	while (pad-- > 0)
		if (!strainsc(&pl->left, PRINT_PAD, pl->left.len))
			return -1;

	if (!stracat(&pl->left, buf))
		return -1;

	return 0;
}

static PLine *
page_addstr_lr(PPage *pp, const char *l, const char *r)
{
	PLine *pl = NULL;

	if ((pl = page_addline(pp, 0)) == NULL)
		goto fail;

	if (l)
		if (!stracpy(&pl->left, l))
			goto fail;

	if (r)
		if (!stracpy(&pl->right, r))
			goto fail;

	return pl;

fail:
	line_free(pl);
	return NULL;
}

static PLine *
page_addcode(PPage *pp, const char *code)
{
	PLine *pl = NULL;

	if ((pl = page_addline(pp, 1)) == NULL)
		goto fail;

	pl->code = 1;

	if (code)
		if (!stracpy(&pl->left, code))
			goto fail;

	return pl;

fail:
	line_free(pl);
	return NULL;
}

static int
page_add_hline(PPage *pp, int l, int r)
{
	PLine *pl = NULL;
	int i;

	if ((pl = page_addline(pp, 0)) == NULL)
		goto fail;

	if (l)
		for (i = 0; i < PRINT_HALF; i++)
			if (!strainsc(&pl->left, PRINT_LINE, pl->left.len))
				goto fail;

	if (r)
		for (i = 0; i < PRINT_HALF; i++)
			if (!strainsc(&pl->right, PRINT_LINE, pl->right.len))
				goto fail;

	return 0;

fail:
	line_free(pl);
	return -1;
}

#define GETCENTRE(str) \
	(strlen((str)) >= PRINT_HALF ? 0 : ((PRINT_HALF - strlen((str))) / 2));

static int
line_pad_centre(PLine *pl)
{
	int len;

	if (pl->left.s == NULL && pl->right.s == NULL)
		return -1;

	if (pl->left.s) {
		len = GETCENTRE(pl->left.s);
		while (len-- > 0)
			if (!strainsc(&pl->left, PRINT_PAD, 0))
				return -1;
	}

	if (pl->same)
		return 0;

	if (pl->right.s) {
		len = GETCENTRE(pl->right.s);
		while (len-- > 0)
			if (!strainsc(&pl->right, PRINT_PAD, 0))
				return -1;
	}

	return 0;
}

static int
print_title(PPage *pp, ChopstixOrder *order)
{
	PLine *pl;

	if ((pl = page_addstr_lr(pp, NULL, "CUSTOMER COPY")) == NULL) return -1;
	pl->half = 1;
	if (line_pad_centre(pl) == -1) return -1;

	/* company name / phone */
	if ((pl = page_addstr_lr(pp, NULL, NULL)) == NULL) return -1;
	pl->half = 1;
	if ((pl = page_add_printf(pp, "%s", header.company)) == NULL) return -1;
	pl->half = 1;
	if (line_pad_centre(pl) == -1) return -1;
	if ((pl = page_add_printf(pp, "(%03d) %03d-%04d", header.phone.npa,
					header.phone.nxx, header.phone.num)) == NULL) return -1;
	pl->half = 1;
	if (line_pad_centre(pl) == -1) return -1;

	return 0;
}

static int
print_header(PPage *pp, ChopstixOrder *order, int reprint)
{
	PLine *pl;
	char datestr[PRINT_DATE_SIZE];

	/* date, deliverto */
	strftime(datestr, sizeof(datestr), PRINT_DATE_FORMAT,
			localtime(&order->date));
	if ((pl = page_add_printf(pp, "Date: %s", datestr)) == NULL) return -1;
	if ((pl = page_add_printf(pp, "Deliver To: %s",
				order->type == ORDER_PICKUP ? "PICKUP" :
				order->type == ORDER_DELIVERY ? "DELIVERY" :
				order->type == ORDER_WALKIN ? "WALKIN" :
				order->type == ORDER_VOID ? "VOID" : "???")) == NULL) return -1;
	if (reprint) {
		if (line_append_printf_ralign(pp, pl, "Order: *****") == -1) return -1;
	}  else
		if (line_append_printf_ralign(pp, pl, "Order: %d", order->key) == -1)
			return -1;

	if (page_add_hline(pp, 1, 1) == -1) return -1;

	return 0;
}

static int
print_customer(PPage *pp, ChopstixOrder *order)
{
	PLine *pl;
	char timestr[PRINT_TIME_SIZE];

	if ((pl = page_add_printf(pp,
				"Phone # (%03d) %03d-%04d",
				order->customer.phone.npa, order->customer.phone.nxx,
				order->customer.phone.num)) == NULL) return -1;
	if (line_append_printf_ralign(pp, pl, "Rep: %d", order->customer.reps) == -1)
		return -1;

	if (page_add_printf(pp,
				"Name     %s", str_filter(order->customer.name)) == NULL)
		return -1;
	if (page_add_printf(pp,
				"%s", str_filter(order->customer.addr.addr)) == NULL)
		return -1;
	if (page_add_printf(pp,
				"Apt      %-8s Entry: %s",
				order->customer.addr.apt,
				str_filter(order->customer.addr.entry)) == NULL)
		return -1;

	if (page_add_printf(pp,
				"Isect    %s", str_filter(order->customer.isect.cross))
			== NULL)
		return -1;

	strftime(timestr, sizeof(timestr), PRINT_TIME_FORMAT,
			localtime(&order->date));
	if (page_add_printf(pp,
				"Ordered  %s", timestr) == NULL)
		return -1;
	if (page_add_hline(pp, 1, 1) == -1) return -1;

	return 0;
}

static int
print_add_extras(ChopstixItemExtras *extras, ChopstixMenuitem *mi, int qty)
{
	unsigned mu, u;
	ChopstixItemExtra *ie;

	for (mu = 0; mu < mi->extras.len; mu++) {
		/* try to add to an existing code */
		for (u = 0; u < extras->len; u++)
			if (strcasecmp(extras->val[u].code, mi->extras.val[mu].code) == 0)
				break;
		if (u < extras->len) {
			extras->val[u].qty += mi->extras.val[mu].qty * qty;
		} else {
			if ((ie = realloc(extras->val, (extras->len + 1) * sizeof(*ie)))
					== NULL)
				return -1;
			extras->len++;
			extras->val = ie;
			extras->val[extras->len - 1].qty = mi->extras.val[mu].qty * qty;
			extras->val[extras->len - 1].code = strdup(mi->extras.val[mu].code);
		}
	}
	return 0;
}

static int
print_special(PPage *pp, const char *special, int shift)
{
	PLine *pl = NULL;
	int i;
	const char *s = special;
	char *sp;

	while (*s) {
		if ((pl = page_addline(pp, 1)) == NULL)
			goto fail;

		/* pad the left */
		for (i = 0; i < (PRINT_SHIFT - strlen(PRINT_STARS)); i++)
			if (!strainsc(&pl->left, PRINT_PAD, 0))
				goto fail;
		/* print the stars */
		if (!stracat(&pl->left, PRINT_STARS))
			goto fail;
		for (i = 0; i < shift; i++)
			if (!strainsc(&pl->left, PRINT_PAD, pl->left.len))
				goto fail;

		for (i = 0; i < (PRINT_HALF - PRINT_SHIFT - shift); i++) {
			if (*s == '\0')
				break;
			if (!isgraph(*s) && *s != ' ') {
				*s++;
				/* reloop on newline */
				if (*(s - 1) == '\n' || *(s - 1) == '\r')
					break;
				continue;
			}

			/* smartwrap */
			(const char *)sp = s;
			while (*sp != '\0' && *sp != ' ' && *sp != '-')
				sp++;
			if ((sp - s) > ((PRINT_HALF - PRINT_SHIFT - shift) - i)
					&& (sp - s) < (PRINT_HALF - PRINT_SHIFT - shift))
				break;

			/* no space on newline */
			if (i == 0 && *s == ' ') {
				while (*s++ == ' ')
					;
				*s--;
				continue;
			}

			if (!strainsc(&pl->left, toupper(*s++), pl->left.len))
				goto fail;
		}
	}

	return 0;

fail:
	line_free(pl);
	return -1;
}

static int
print_orderitems(PPage *pp, ChopstixOrder *order)
{
	PLine *pl;
	unsigned u_item, u_sub;
	ChopstixOrderItem *item;
	ChopstixSubItem *subitem;
	ChopstixMenuitem *mi;
	ChopstixItemExtras extras;

#define ITEMFORMAT			"%3d %6s  %s"
#define MSGFORMAT			"%3s %6s  %s"
#define MONEYFORMAT			"$%s%d.%02d"
#define STYLEFORMAT 		"            %s"
#define SPECIALFORMAT		"       ***  %s"
#define SUBITEMFORMAT		"%3d %6s   %s"
#define SUBSTYLEFORMAT 		"             %s"
#define SUBSPECIALFORMAT	"       ***   %s"
#define TOTALFORMAT			"%*s%s"

	bzero(&extras, sizeof(extras));

	/* column header */
	if ((pl = page_add_printf(pp, MSGFORMAT, "Qty", "Code", "Description"))
			== NULL) goto fail;
	if (line_append_printf_ralign(pp, pl, "Price") == -1) goto fail;

	for (u_item = 0; u_item < order->items.len; u_item++) {
		item = &order->items.val[u_item];

		/* if the menu code is wrong, make sure someone knows about it */
		if ((mi = menu_getitem(item->code)) == NULL) {
			page_add_printf(pp, MSGFORMAT, "***", item->code, "***");
			continue;
		}

		/* ITEM */
		if ((pl = page_add_printf_wrap(pp, PRINT_SHIFT, ITEMFORMAT,
						item->qty, item->code, str_filter(mi->name))) == NULL)
			goto fail;
		if (mi->price != 0)
			if (line_append_printf_ralign(pp, pl, MONEYFORMAT,
						NEGSIGN(mi->price * item->qty),
						DOLLARS(mi->price * item->qty),
						CENTS(mi->price * item->qty)) == -1)
				goto fail;

		/* ITEM STYLE */
		if (mi->styles.len > 0) {
			if (page_add_printf(pp, STYLEFORMAT,
						str_filter(order_get_styletext(&mi->styles,
								item->style))) == NULL)
				goto fail;
		}

		/* ITEM SPECIAL */
		if (item->special && *item->special && strlen(*item->special)) {
			if (print_special(pp, *item->special, 0) == -1)
				goto fail;
		}

		/* ITEM EXTRAS */
		if (print_add_extras(&extras, mi, item->qty) == -1)
			goto fail;

		if (item->subitems) {
			for (u_sub = 0; u_sub < item->subitems->len; u_sub++) {
				subitem = &item->subitems->val[u_sub];

				/* if the menu code is wrong, make sure someone knows about it */
				if ((mi = menu_getitem(subitem->code)) == NULL) {
					page_add_printf(pp, MSGFORMAT, "***", subitem->code, "***");
					continue;
				}

				/* SUBITEM */
				if ((pl = page_add_printf_wrap(pp, PRINT_SHIFT + 1,
								SUBITEMFORMAT, subitem->qty, subitem->code,
								str_filter(mi->name))) == NULL)
					goto fail;
				if (subitem->pricedelta != 0)
					if (line_append_printf_ralign(pp, pl, MONEYFORMAT,
								NEGSIGN(subitem->pricedelta),
								DOLLARS(subitem->pricedelta),
								CENTS(subitem->pricedelta)) == -1)
						goto fail;

				/* SUBITEM STYLE */
				if (mi->styles.len > 0) {
					if (page_add_printf(pp, SUBSTYLEFORMAT,
								str_filter(order_get_styletext(&mi->styles,
									subitem->style))) == NULL)
						goto fail;
				}

				/* SUBITEM SPECIAL */
				if (subitem->special && *subitem->special &&
						strlen(*subitem->special)) {
					if (print_special(pp, *subitem->special, 1) == -1)
						goto fail;
				}

				/* SUBITEM EXTRAS */
				if (print_add_extras(&extras, mi, subitem->qty) == -1)
					goto fail;

			}
		}
	}

	/*
	 * RULEITEMS
	 */

	if (order->ruleitems.len > 0)
		if (page_addstr_lr(pp, NULL, NULL) == NULL) goto fail;

	for (u_item = 0; u_item < order->ruleitems.len; u_item++) {
		item = &order->ruleitems.val[u_item];

		/* if the menu code is wrong, make sure someone knows about it */
		if ((mi = menu_getitem(item->code)) == NULL) {
			page_add_printf(pp, MSGFORMAT, "***", item->code, "***");
			continue;
		}

		/* ITEM */
		if ((pl = page_add_printf(pp, ITEMFORMAT, item->qty, item->code,
						str_filter(mi->name))) == NULL) goto fail;
		/* no price on rule items */
	}

	/*
	 * EXTRAS
	 */

	if (extras.len > 0)
		if (page_add_hline(pp, 1, 1) == -1) goto fail;

	for (u_item = 0; u_item < extras.len; u_item++) {

		if ((mi = menu_getitem(extras.val[u_item].code)) == NULL) {
			/* if the menu code is wrong, make sure someone knows about it */
			if (page_add_printf(pp, ITEMFORMAT, extras.val[u_item].qty,
						extras.val[u_item].code, "*** BAD CODE ***") == NULL)
				goto fail;
			continue;
		}

		if ((pl = page_add_printf(pp, ITEMFORMAT, extras.val[u_item].qty,
						extras.val[u_item].code, str_filter(mi->name))) == NULL)
			goto fail;
	}

	if (page_add_hline(pp, 1, 1) == -1) goto fail;

	/*
	 * SPECIAL INSTRUCTIONS
	 */
	if (order->special && strlen(order->special)) {
		int i;
		const char *s = order->special;
		char *sp;

		if (page_add_printf(pp, "SPECIAL INSTRUCTIONS:") == NULL) goto fail;

		while (*s) {
			if ((pl = page_addline(pp, 1)) == NULL)
				goto fail;

			for (i = 0; i < PRINT_HALF; i++) {
				if (*s == '\0')
					break;

				if (!isgraph(*s) && *s != ' ') {
					*s++;
					/* reloop on newline */
					if (*(s - 1) == '\n' || *(s - 1) == '\r')
						break;
					continue;
				}

				/* smartwrap */
				(const char *)sp = s;
				while (*sp != '\0' && *sp != ' ' && *sp != '-')
					sp++;
				if ((sp - s) > (PRINT_HALF - i) && (sp - s) < PRINT_HALF)
					break;

				/* no space on newline */
				if (i == 0 && *s == ' ') {
					while (*s++ == ' ')
						;
					*s--;
					continue;
				}

				if (!strainsc(&pl->left, toupper(*s++), pl->left.len))
					goto fail;
			}
		}
	}

	free_ChopstixItemExtras(&extras);
	return 0;

fail:
	free_ChopstixItemExtras(&extras);
	return -1;
}

static int
print_totals(PPage *pp, ChopstixOrder *order, int reprint)
{
	PLine *pl;
	int remain;

	/*
	 * TOTALS
	 */
	if ((pl = page_add_printf(pp, "%s", "")) == NULL) return -1;
	if (line_append_printf_ralign(pp, pl, "--------") == -1) return -1;
#define PRINT_TOTAL(n, p) do {												\
	if ((pl = page_add_printf(pp, TOTALFORMAT,								\
					(int)((PRINT_HALF - strlen("SUB-TOTAL") - ALIGN_MONEYWIDTH - 1)),\
					"",	n)) == NULL)										\
		return -1;															\
	if (line_append_printf_ralign(pp, pl, MONEYFORMAT,						\
				NEGSIGN(p), DOLLARS(p), CENTS(p)) == -1)					\
		return -1;															\
} while (0)
	PRINT_TOTAL("SUB-TOTAL", order->total.subtotal);
	if (order->total.discount)
		PRINT_TOTAL("DISCOUNT", DISCOUNT(&order->total));
	if (order->total.credit > 0)
		PRINT_TOTAL("CREDIT", order->total.credit);
	if (order->total.delivery != 0)
		PRINT_TOTAL("DELIVERY", order->total.delivery);
	PRINT_TOTAL(config.tax1name, order->total.tax1);
	PRINT_TOTAL(config.tax2name, order->total.tax2);
	if ((pl = page_add_printf(pp, "%s", "")) == NULL) return -1;
	if (reprint) {
		if (line_append_printf_ralign(pp, pl, "------- REPRINT -------") == -1)
			return -1;
	} else {
		if (line_append_printf_ralign(pp, pl, "--------") == -1) return -1;
	}
	PRINT_TOTAL("TOTAL", order->total.total);

	if (order->total.credit) {
		if ((pl = page_add_printf(pp, "%s", "")) == NULL) return -1;
		if ((pl = page_add_printf(pp, TOTALFORMAT,
						(int)((PRINT_HALF - strlen("CREDIT REMAINING") -
								ALIGN_MONEYWIDTH - 1)), "",
						"CREDIT REMAINING")) == NULL)
			return -1;
		remain = customer_getcredit_remain();
		if (line_append_printf_ralign(pp, pl, MONEYFORMAT,
				NEGSIGN(remain), DOLLARS(remain), CENTS(remain)) == -1)
			return -1;
	}

	return 0;
}

static PLine *
print_creditinfo(PPage *pp, ChopstixOrder *order)
{
	PLine *pl;
	char *s, *ccnum, *ccexp;
	unsigned u;

	if (order->payment.ccinfo == NULL
			|| (ccnum = order->payment.ccinfo->number) == NULL)
		ccnum = "**** NOT GIVEN ****";
	if (order->payment.ccinfo == NULL
			|| (ccexp = order->payment.ccinfo->expiry) == NULL)
		ccexp = "**/**";

	if ((s = strdup(ccnum)) == NULL)
		return NULL;
	if (strlen(s) > 4)
		for (u = 4; u < (strlen(s) - 4); u++)
			if (isdigit(s[u]))
				s[u] = '*';

	/* LEFT */
	if ((pl = page_add_printf(pp, "CC# %s EXP %s",
					config.print.cchide_kitchen ? s : ccnum, ccexp)) == NULL) {
		if (s != ccnum)
			free(s);
		return NULL;
	}
	/* RIGHT */
	if (line_add_printf_r(pl, "CC# %s EXP %s",
				config.print.cchide_customer ? s : ccnum, ccexp) == -1) {
		if (s != ccnum)
			free(s);
		return NULL;
	}

	if (s != ccnum)
		free(s);

	return pl;
}

static int
print_payment(PPage *pp, ChopstixOrder *order)
{
	PLine *pl;

	if (page_addstr_lr(pp, NULL, NULL) == NULL) return -1;
	if (config.tax1reg) {
		if ((pl = page_add_printf(pp, "%s# %s", config.tax1name,
						config.tax1reg)) == NULL) return -1;
		if (line_pad_centre(pl) == -1) return -1;
	}
	if (config.tax2reg) {
		if ((pl = page_add_printf(pp, "%s# %s", config.tax2name,
						config.tax2reg)) == NULL) return -1;
		if (line_pad_centre(pl) == -1) return -1;
	}

	if (page_addstr_lr(pp, NULL, NULL) == NULL) return -1;
	
	pl = NULL;
	switch (order->payment.type) {
		case PAYMENT_NONE:
			pl = page_add_printf(pp, "NONE");
			break;
		case PAYMENT_CASH:
			pl = page_add_printf(pp, "CASH");
			break;
		case PAYMENT_CREDIT:
			pl = print_creditinfo(pp, order);
			break;
		case PAYMENT_VOID:
			pl = page_add_printf(pp, "VOID");
			break;
		case PAYMENT_DEBIT:
			pl = page_add_printf(pp, "DEBIT");
			break;
		case PAYMENT_CHEQUE:
			pl = page_add_printf(pp, "CHEQUE");
			break;
		case PAYMENT_OTHER:
			pl = page_add_printf(pp, "OTHER");
			break;
	}
	if (pl == NULL)
		return 0;
	if (line_pad_centre(pl) == -1)
		return -1;

	if (order->payment.type == PAYMENT_CREDIT) {
		if (page_addstr_lr(pp, NULL, NULL) == NULL) return -1;
		if (page_addstr_lr(pp, NULL, NULL) == NULL) return -1;
		if ((pl = page_addstr_lr(pp,
						config.print.ccsig_kitchen ? PRINT_SIGNATURE : NULL,
						config.print.ccsig_customer ? PRINT_SIGNATURE : NULL))
				== NULL)
			return -1;
		if (line_pad_centre(pl) == -1)
			return -1;
	}

	return 0;
}

/*
 * ensure all of data on socket comes through. f==read || f==vwrite
 */
#define vwrite (ssize_t (*)(int, void *, size_t))write
static size_t
atomicio(ssize_t (*f) (int, void *, size_t), int fd, void *_s, size_t n)
{
	char *s = _s;
	size_t pos = 0;
	ssize_t res;

	while (n > pos) {
		res = (f) (fd, s + pos, n - pos);
		switch (res) {
		case -1:
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return 0;
		case 0:
			errno = EPIPE;
			return pos;
		default:
			pos += (u_int)res;
		}
	}
	return (pos);
}

static int
print_buffer(char *buf, size_t buf_len)
{
	pid_t pid;
	char *(args[4]);
	int pipes[2];
	int pipe_err[2];
	int pipe_warn[2];
	int exitcode = 0;
	char errbuf[BUFSIZ];
	ssize_t len;

	args[0] = config.print.cmd;
	if (strlen(config.print.queue) == 0)
		args[1] = 0;
	else {
		args[1] = PRINT_CMD_QUEUE;
		args[2] = config.print.queue;
		args[3] = 0;
	}

	if (pipe(pipes) == -1) {
		status_err("cannot open pipe to printer");
		return -1;
	}
	if (pipe(pipe_warn) == -1) {
		status_err("cannot open pipe to printer status");
		close(pipes[0]);
		close(pipes[1]);
		return -1;
	}
	if (pipe(pipe_err) == -1) {
		status_err("cannot open pipe to printer status (error)");
		close(pipe_warn[0]);
		close(pipe_warn[1]);
		close(pipes[0]);
		close(pipes[1]);
		return -1;
	}

	/* use vfork to halt until child is execv(2)'d */
	switch ((pid = vfork())) {
		case 0:
			/* child */
			close(pipe_err[0]);
			close(STDERR_FILENO);
			if (fcntl(pipe_err[1], F_DUPFD, STDERR_FILENO) == -1) {
				fprintf(stdout, "cannot plumb printer error output");
				_exit(1);
			}
			close(pipe_warn[0]);
			close(STDOUT_FILENO);
			if (fcntl(pipe_warn[1], F_DUPFD, STDOUT_FILENO) == -1) {
				fprintf(stderr, "cannot plumb printer error output");
				_exit(1);
			}
			close(pipes[0]);
			close(STDIN_FILENO);
			if (fcntl(pipes[1], F_DUPFD, STDIN_FILENO) == -1) {
				fprintf(stderr, "cannot plumb printer input");
				_exit(1);
			}
			if (execv(*args, args) == -1)
				fprintf(stderr, "cannot exec %s: %s", args[0], strerror(errno));
			_exit(127);
			break;
		case -1:
			/* error */
			status_err("cannot print order");
			close(pipes[0]);
			close(pipes[1]);
			close(pipe_err[0]);
			close(pipe_err[1]);
			return -1;
		default:
			/* parent */
			close(pipes[1]);
			close(pipe_warn[1]);
			close(pipe_err[1]);
			if (atomicio(vwrite, pipes[0], buf, buf_len) != buf_len)
				status_err("cannot print order");
			close(pipes[0]);
			/* wait for lpr to finish */
			wait(&exitcode);
			if (exitcode != 0) {
				if ((len = read(pipe_err[0], errbuf, sizeof(errbuf))) > 0) {
					status_warn("printer error: %s", errbuf);
					exitcode = -2;
				}
			} else  {
				if ((len = read(pipe_warn[0], errbuf, sizeof(errbuf))) > 0) {
					status_warn("printer warning: %s", errbuf);
					exitcode = -3;
				}
			}
			close(pipe_warn[0]);
			close(pipe_err[0]);
			break;
	}

	return exitcode;
}

int
print_order(ChopstixOrder *order, int reprint)
{
	PPage pp;
	PLine *pl;
	char *buf = NULL;
	size_t len = 0;
	int ret, linehack, i;
	unsigned u;
	char *order_code, *title_code;

	/* Licence must be valid for printing */
	if (!licence_valid())
		return -1;

	bzero(&pp, sizeof(pp));

	if (config.print.title_bold)
		title_code = PRINT_FONT_TITLE_BOLD;
	else
		title_code = PRINT_FONT_TITLE;

	if (config.print.order_bold)
		order_code = PRINT_FONT_ORDER_BOLD;
	else
		order_code = PRINT_FONT_NORMAL;

	/* turn on 2wide printing */
	if (page_addcode(&pp, title_code) == NULL) goto fail;

	linehack = PRINT_HALF;
	PRINT_HALF = PRINT_HALF / 2;
	if (print_title(&pp, order) == -1)
		goto fail;
	PRINT_HALF = linehack;

	/* turn off 2wide printing */
	if (page_addcode(&pp, order_code) == NULL) goto fail;

	/* address is smaller, and not half */
	if ((pl = page_add_printf(&pp, "%s", header.addr.addr)) == NULL) goto fail;
	if (line_pad_centre(pl) == -1) goto fail;

	if (config.print.show_city || config.print.show_province ||
			config.print.show_country) {
		if ((pl = page_addline(&pp, 1)) == NULL) goto fail;

		if (config.print.show_city)
			if (!stracat(&pl->left, header.city)) goto fail;

		if (config.print.show_province) {
			if (config.print.show_city)
				if (!stracat(&pl->left, ", ")) goto fail;
			if (!stracat(&pl->left, header.province)) goto fail;
		}

		if (config.print.show_country) {
			if (config.print.show_city || config.print.show_province)
				if (!stracat(&pl->left, ", ")) goto fail;
			if (!stracat(&pl->left, header.country)) goto fail;
		}
	}
	if (line_pad_centre(pl) == -1) goto fail;

	if (page_addstr_lr(&pp, NULL, NULL) == NULL) goto fail;
	if (print_header(&pp, order, reprint) == -1)
		goto fail;
	if (print_customer(&pp, order) == -1)
		goto fail;
	if (print_orderitems(&pp, order) == -1)
		goto fail;
	if (print_totals(&pp, order, reprint) == -1)
		goto fail;
	if (print_payment(&pp, order) == -1)
		goto fail;

	/* line feed blank */
	for (i = 0; i < config.print.feedlines; i++)
		if (page_addstr_lr(&pp, NULL, NULL) == NULL) return -1;

	if (page_to_buffer(&pp, &buf, &len) == -1) {
		status_warn("error generating order for print");
		return -1;
	}

	if ((ret = print_buffer(buf, len)) != 0) {
		/*
		 * -2 is an error (with message)
		 * -3 is a warning (with message)
		 */
		if (ret == -1) {
			status_warn("order %d printing failed: code %d",
					order->key, ret);
			return -1;
		} else if (ret == -2)
			return -1;
		else if (ret == -3)
			return 0;
		return -1;
	}

	status_set("order %d printed", order->key);
	return 0;

fail:
	for (u = 0; u < pp.len; u++)
		line_free(&pp.val[u]);
	free(pp.val);
	return -1;
}

void
print_exit(void)
{
}
