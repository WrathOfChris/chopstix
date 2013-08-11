/* $Gateweaver: rules.c,v 1.2 2005/12/03 22:40:45 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <stdlib.h>
#include <string.h>
#include "chopstix_api.h"

void rm_init(struct rule_functions *);
static int rm_run(ChopstixOrder *, int);
static ChopstixOrderItem * rm_add_item(ChopstixOrder *, const char *);
static void rm_err(const char *, ...)
	__attribute__((__format__(printf, 1, 2)));
static int rm_getprice(char *);

static void (*rm_puterr)(const char *, va_list);
static int (*rm_getprice_int)(char *);

void
rm_init(struct rule_functions *rf)
{
	rf->run = &rm_run;
	
	rm_puterr = rf->err;
	rm_getprice_int = rf->getprice;
}

static int
rm_run(ChopstixOrder *order, int post)
{
	free_ChopstixOrderItems(&order->ruleitems);

	if (order->total.disctype == DISCOUNT_RULE) {
		order->total.disctype = DISCOUNT_NONE;
		order->total.discount = 0;
	}
	if (order->total.delitype == DISCOUNT_RULE) {
		order->total.delitype = DISCOUNT_NONE;
		order->total.delivery = 0;
	}
	
	/* Use these functions, if necessary */
	(void)rm_add_item(order, NULL);
	(void)rm_getprice(NULL);

	return 0;
}

static ChopstixOrderItem *
rm_add_item(ChopstixOrder *order, const char *code)
{
	ChopstixOrderItem *oi;
	unsigned int u;

	if (code == NULL)
		return NULL;

	for (u = 0; u < order->ruleitems.len; u++)
		if (strcasecmp(code, order->ruleitems.val[u].code) == 0)
			return &order->ruleitems.val[u];

	if ((oi = realloc(order->ruleitems.val,
					(order->ruleitems.len + 1) * sizeof(ChopstixOrderItem)))
			== NULL) {
		rm_err("cannot allocate memory for rule item");
		return NULL;
	}

	order->ruleitems.len++;
	order->ruleitems.val = oi;

	bzero(&order->ruleitems.val[order->ruleitems.len - 1],
			sizeof(ChopstixOrderItem));
	if (code == NULL || (order->ruleitems.val[order->ruleitems.len - 1].code
				= strdup(code)) == NULL) {
		order->ruleitems.len -= 1;
		rm_err("cannot copy item code \"%s\" into rule item", code);
		return NULL;
	}

	return &order->ruleitems.val[order->ruleitems.len - 1];
}

static void
rm_err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (rm_puterr)
		rm_puterr(fmt, ap);
	va_end(ap);
}

static int
rm_getprice(char *code)
{
	if (rm_getprice_int && code)
		return rm_getprice_int(code);

	rm_err("cannot lookup price for code \"%s\", check rule items on receipt",
			code);
	return 0;
}
