/* $Gateweaver: payment.c,v 1.12 2007/09/22 15:00:03 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <curses.h>
#include <err.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: payment.c,v 1.12 2007/09/22 15:00:03 cmaxwell Exp $");

ChopstixForm payform;

static int payment_validate_type(const char *, int *, void *);
static int payment_validate_ccnum(const char *, int *, void *);
static int payment_validate_ccexp(const char *, int *, void *);
static ChopstixFormAction payment_putdata(const char *, void *);

enum payment_state {
	STATE_NONE,
	STATE_TYPE,
	STATE_CCNUM,
	STATE_CCEXP
};

void
payment_init(void)
{
	ChopstixFormField *ff;

	/*
	 * Payment form
	 */

	bzero(&payform, sizeof(payform));
	TAILQ_INIT(&payform.fields);
	if ((payform.labelsep = strdup(" ")) == NULL)
		err(1, "allocating form");

	if ((ff = form_field_new("Payment")) == NULL)
		err(1, "allocating fields");
	form_field_add(&payform, ff);
	form_field_setyx(ff, LINE_PAYMENT, 0);
	form_field_setiw(ff, 2);	/* "C" / "CC" / "V" */
	form_field_setarg(ff, (void *)STATE_TYPE);
	ff->putdata = &payment_putdata;
	ff->validate = &payment_validate_type;
	ff->hotfield = 1;

	if ((ff = form_field_new("CC#")) == NULL)
		err(1, "allocating fields");
	form_field_add(&payform, ff);
	form_field_setyx(ff, LINE_PAYMENT, 11);
	form_field_setiw(ff, 19);	/* 0000 0000 0000 0000 */
	form_field_setarg(ff, (void *)STATE_CCNUM);
	ff->putdata = &payment_putdata;
	ff->validate = &payment_validate_ccnum;

	if ((ff = form_field_new("EXP")) == NULL)
		err(1, "allocating fields");
	form_field_add(&payform, ff);
	form_field_setyx(ff, LINE_PAYMENT, 35);
	form_field_setiw(ff, 5);
	form_field_setarg(ff, (void *)STATE_CCEXP);
	ff->putdata = &payment_putdata;
	ff->validate = &payment_validate_ccexp;
}

/*
 * Fixup the yline of the form, since movement is based on LINES
 */
void
payment_reinit(void)
{
	ChopstixFormField *ff;

	TAILQ_FOREACH(ff, &payform.fields, entry)
		ff->y = LINE_PAYMENT;
}

void
payment_exit(void)
{
}

static int
payment_validate_type(const char *str, int *pos, void *arg)
{
	const char *s;
	int seenc = 0, seenv = 0, seend = 0;

	for (s = str; *s; *s++) {
		if (*s == 'c' || *s == 'C')
			seenc++;
		else if (*s == 'v' || *s == 'V')
			seenv++;
		else if (*s == 'd' || *s == 'D')
			seend++;
		else {
			*pos = s - str;
			return -1;
		}

		if ((seenc && seenv) || (seenc && seend)) {
			*pos = s - str;
			return -1;
		}
	}
	return 0;
}

static int
payment_validate_ccnum(const char *str, int *pos, void *arg)
{
	const char *s;
	for (s = str; *s; *s++)
		if (!(isdigit(*s) || *s == ' ')) {
			*pos = s - str;
			return -1;
		}
	return 0;
}

static int
payment_validate_ccexp(const char *str, int *pos, void *arg)
{
	const char *s;
	int seenslash = 0;

	for (s = str; *s; *s++) {
		if (!(isdigit(*s) || *s == '/')
				|| (*s == '/' && seenslash++ > 0)) {
			*pos = s - str;
			return -1;
		}
	}
	return 0;
}

static ChopstixFormAction
payment_putdata(const char *str, void *arg)
{
	enum payment_state state = (enum payment_state)arg;
	char *s;
	ChopstixFormAction action = FIELD_NEXT;
	ChopstixFormField *ff;

	if (state == STATE_TYPE) {
		if (strcasecmp(str, "cc") == 0) {
			s = "CC";
			order.payment.type = PAYMENT_CREDIT;
		} else if (tolower(*str) == 'v') {
			s = "V";
			order.payment.type = PAYMENT_VOID;
		} else if (strcasecmp(str, "c") == 0) {
			s = "C";
			order.payment.type = PAYMENT_CASH;
			action = FIELD_HOTSTORE;
		} else if (strcasecmp(str, "d") == 0) {
			s = "D";
			order.payment.type = PAYMENT_DEBIT;
		} else {
			(const char *)s = "";
			status_set("'C'ash, 'CC'redit card, 'V'oid, 'D'ebit");
			action = FORM_NONE;
		}
		TAILQ_FOREACH(ff, &payform.fields, entry)
			if ((enum payment_state)ff->arg == STATE_TYPE)
				stracpy(&ff->input, s);
	} else if (state == STATE_CCNUM || state == STATE_CCEXP) {
		if ((s = strdup(str)) == NULL) {
			status_warn("cannot store \"%s\"", str);
			return FIELD_NEXT;
		}
		if (order.payment.ccinfo == NULL)
			if ((order.payment.ccinfo = calloc(1,
							sizeof(*order.payment.ccinfo))) == NULL) {
				status_warn("cannot allocate CC info");
				return FIELD_NEXT;
			}

		if (state == STATE_CCNUM) {
			free(order.payment.ccinfo->number);
			order.payment.ccinfo->number = s;
		} else if (state == STATE_CCEXP) {
			free(order.payment.ccinfo->expiry);
			order.payment.ccinfo->expiry = s;
		}
	}

	return action;
}
