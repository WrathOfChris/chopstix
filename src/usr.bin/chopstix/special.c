/* $Gateweaver: special.c,v 1.8 2007/09/22 15:00:03 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <err.h>
#include <curses.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: special.c,v 1.8 2007/09/22 15:00:03 cmaxwell Exp $");

ChopstixForm specialform;

static ChopstixFormAction special_putdata(const char *, void *);

void
special_init(void)
{
	ChopstixFormField *ff = NULL;

	/* Special Form */
	bzero(&specialform, sizeof(specialform));
	TAILQ_INIT(&specialform.fields);
	if ((specialform.labelsep = strdup(CHOPSTIX_FORM_LABEL)) == NULL)
		err(1, "allocating form");

	if ((ff = form_field_new("Special Instructions")) == NULL)
		err(1, "allocating fields");
	form_field_add(&specialform, ff);
	form_field_setyx(ff, LINE_SPECIAL, 0);
	form_field_setwl(ff, COLS - ALIGN_MONEY_MAX - ALIGN_SUBTOTAL_LABELSIZE,
			LINE_SPECIAL_SIZE);
	ff->putdata = &special_putdata;
}

void
special_reinit(void)
{
	ChopstixFormField *ff;

	TAILQ_FOREACH(ff, &specialform.fields, entry) {
		ff->y = LINE_SPECIAL;

		if (order.customer.special != NULL
				&& (order.special == NULL || strlen(order.special) == 0))
			if (!stracpy(&ff->input, *order.customer.special))
				status_warn("cannot load customer special info");
	}
}

void
special_exit(void)
{
}

static ChopstixFormAction
special_putdata(const char *str, void *arg)
{
	char *s;
	int post = 0;

	if (order.customer.special == NULL
			|| strcmp(str, *order.customer.special) != 0)
		post = 1;

	if ((s = strdup(str)) == NULL) {
		status_warn("cannot store \"%s\"", str);
		return FIELD_NEXT;
	}

	free(order.special);
	order.special = s;

	if (order.customer.special == NULL)
		if ((order.customer.special = calloc(1,
						sizeof(*order.customer.special))) == NULL) {
			status_warn("cannot allocate customer special info");
			return FIELD_NEXT;
		}
	if ((s = strdup(order.special)) == NULL) {
		status_warn("cannot copy customer special info");
		return FIELD_NEXT;
	}
	free(*order.customer.special);
	*order.customer.special = s;

	/* it will give a better error message than we ever could */
	if (post)
		customer_post(&order.customer);

	return FIELD_NEXT;
}
