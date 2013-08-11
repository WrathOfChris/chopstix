/* $Gateweaver: window.c,v 1.15 2007/09/22 15:00:03 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <curses.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: window.c,v 1.15 2007/09/22 15:00:03 cmaxwell Exp $");

ChopstixForm discountform;
ChopstixForm deliveryform;
ChopstixForm chphoneform;
ChopstixForm dailyinfoform;
ChopstixForm cashinform;
ChopstixForm creditform;

static ChopstixFormAction window_discount_putdata(const char *, void *);
static ChopstixFormAction window_delivery_putdata(const char *, void *);
static ChopstixFormAction window_chphone_putdata(const char *, void *);
static ChopstixFormAction window_dailyinfo_putdata(const char *, void *);
static ChopstixFormAction window_cashin_putdata(const char *, void *);
static ChopstixFormAction window_credit_putdata(const char *, void *);
static const char * money2str(int);

enum daily_type {
	DAILY_NONE,
	DAILY_ORDER,
	DAILY_TAX1,
	DAILY_TAX2,
	DAILY_TOTAL
};

enum cashin_type {
	CASHIN_NONE,
	CASHIN_TOTAL,
	CASHIN_CASH_IN,
	CASHIN_CASH_OUT
};

enum credit_type {
	CREDIT_TOTAL,
	CREDIT_REMAIN,
	CREDIT_THIS,
	CREDIT_REASON
};

#define OFFSET_DELIVERY	19		/* offset of 'Delivery' on order screen */

void
window_init(void)
{
	ChopstixFormField *ff = NULL;
	int y;

	/* 
	 * DISCOUNT
	 */

	bzero(&discountform, sizeof(discountform));
	TAILQ_INIT(&discountform.fields);
	if ((discountform.labelsep = strdup(": ")) == NULL)
		err(1, "allocating form");

	if ((ff = form_field_new("Discount")) == NULL)
		err(1, "allocating fields");
	form_field_add(&discountform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2,
			(COLS - ALIGN_MONEY - strlen("Discount")) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setrightalign(ff, 1);
	ff->putdata = &window_discount_putdata;
	ff->getstr = &form_field_getstr_discount;
	ff->validate = &form_field_validate_discount;
	ff->getcpos = &form_field_getcpos_discount;

	/* 
	 * DELIVERY
	 */

	bzero(&deliveryform, sizeof(deliveryform));
	TAILQ_INIT(&deliveryform.fields);
	if ((deliveryform.labelsep = strdup(": ")) == NULL)
		err(1, "allocating form");

	if ((ff = form_field_new("Delivery")) == NULL)
		err(1, "allocating fields");
	form_field_add(&deliveryform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2,
			(COLS - ALIGN_MONEY - strlen("Delivery")) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setrightalign(ff, 1);
	ff->putdata = &window_delivery_putdata;
	ff->getstr = &form_field_getstr_discount;
	ff->validate = &form_field_validate_discount;
	ff->getcpos = &form_field_getcpos_discount;

	bzero(&chphoneform, sizeof(chphoneform));
	TAILQ_INIT(&chphoneform.fields);
	if ((chphoneform.labelsep = strdup(": ")) == NULL)
		err(1, "allocating form");

	if ((ff = form_field_new("New Phone #")) == NULL)
		err(1, "allocating fields");
	form_field_add(&chphoneform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2,
			(COLS - CHOPSTIX_PHONE_SIZE - strlen("New Phone #")) / 2);
	form_field_setiw(ff, CHOPSTIX_PHONE_SIZE);
	ff->getstr = &form_field_getstr_phone;
	ff->validate = &form_field_validate_phone;
	ff->getcpos = &form_field_getcpos_phone;
	ff->getrpos = &form_field_getrpos_phone;
	ff->putdata = &window_chphone_putdata;

	/*
	 * DAILY INFO
	 */

	y = -1;
	bzero(&dailyinfoform, sizeof(dailyinfoform));
	TAILQ_INIT(&dailyinfoform.fields);
	if ((dailyinfoform.labelsep = strdup(": ")) == NULL)
		err(1, "allocating form");
	dailyinfoform.align = 1;

	if ((ff = form_field_new("Orders")) == NULL)
		err(1, "allocating fields");
	form_field_add(&dailyinfoform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 6) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setdisplayonly(ff, 1);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)DAILY_ORDER);

	if ((ff = form_field_new(config.tax1name ? config.tax1name : "Tax1"))
			== NULL)
		err(1, "allocating fields");
	form_field_add(&dailyinfoform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 6) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setdisplayonly(ff, 1);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)DAILY_TAX1);

	if ((ff = form_field_new(config.tax2name ? config.tax2name : "Tax2"))
			== NULL)
		err(1, "allocating fields");
	form_field_add(&dailyinfoform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 6) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setdisplayonly(ff, 1);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)DAILY_TAX2);

	if ((ff = form_field_new("Total")) == NULL)
		err(1, "allocating fields");
	form_field_add(&dailyinfoform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 6) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setdisplayonly(ff, 1);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)DAILY_TOTAL);

	if ((ff = form_field_new("PRESS")) == NULL)
		err(1, "allocating fields");
	form_field_add(&dailyinfoform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 6) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setarg(ff, (void *)DAILY_NONE);
	ff->putdata = &window_dailyinfo_putdata;

	/*
	 * CASH IN FORM
	 */
	y = -1;
	bzero(&cashinform, sizeof(cashinform));
	TAILQ_INIT(&cashinform.fields);
	if ((cashinform.labelsep = strdup(": ")) == NULL)
		err(1, "allocating form");
	cashinform.align = 1;

	if ((ff = form_field_new("Total")) == NULL)
		err(1, "allocating fields");
	form_field_add(&cashinform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 8) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setdisplayonly(ff, 1);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)CASHIN_TOTAL);
	ff->putdata = &window_cashin_putdata;
	ff->getstr = &form_field_getstr_discount;
	ff->validate = &form_field_validate_discount;
	ff->getcpos = &form_field_getcpos_discount;

	if ((ff = form_field_new("Cash In")) == NULL)
		err(1, "allocating fields");
	form_field_add(&cashinform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 8) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)CASHIN_CASH_IN);
	ff->putdata = &window_cashin_putdata;
	ff->getstr = &form_field_getstr_discount;
	ff->validate = &form_field_validate_discount;
	ff->getcpos = &form_field_getcpos_discount;

	if ((ff = form_field_new("Cash Out")) == NULL)
		err(1, "allocating fields");
	form_field_add(&cashinform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 8) / 2);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setrightalign(ff, 1);
	form_field_setdisplayonly(ff, 1);
	form_field_setarg(ff, (void *)CASHIN_CASH_OUT);
	ff->putdata = &window_cashin_putdata;
	ff->getstr = &form_field_getstr_discount;
	ff->validate = &form_field_validate_discount;
	ff->getcpos = &form_field_getcpos_discount;

	if ((ff = form_field_new("PRESS")) == NULL)
		err(1, "allocating fields");
	form_field_add(&cashinform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			(COLS - ALIGN_MONEYWIDTH - 8) / 2);
	form_field_setrightalign(ff, 1);
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setarg(ff, (void *)CASHIN_NONE);
	ff->putdata = &window_cashin_putdata;

	/* 
	 * CREDITS
	 */

	y = -1;
	bzero(&creditform, sizeof(creditform));
	TAILQ_INIT(&creditform.fields);
	if ((creditform.labelsep = strdup(": ")) == NULL)
		err(1, "allocating form");
	creditform.align = 1;

	if ((ff = form_field_new(" Total")) == NULL)
		err(1, "allocating fields");
	form_field_add(&creditform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			MAX(OFFSET_DELIVERY, (COLS - ALIGN_REASONWIDTH - 8) / 2));
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setdisplayonly(ff, 1);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)CREDIT_TOTAL);

	if ((ff = form_field_new("Remain")) == NULL)
		err(1, "allocating fields");
	form_field_add(&creditform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			MAX(OFFSET_DELIVERY, (COLS - ALIGN_REASONWIDTH - 8) / 2));
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setdisplayonly(ff, 1);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)CREDIT_REMAIN);

	if ((ff = form_field_new("Credit")) == NULL)
		err(1, "allocating fields");
	form_field_add(&creditform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			MAX(OFFSET_DELIVERY, (COLS - ALIGN_REASONWIDTH - 8) / 2));
	form_field_setiw(ff, ALIGN_MONEYWIDTH);
	form_field_setrightalign(ff, 1);
	form_field_setarg(ff, (void *)CREDIT_THIS);
	ff->putdata = &window_credit_putdata;
	ff->getstr = &form_field_getstr_discount;
	ff->validate = &form_field_validate_discount;
	ff->getcpos = &form_field_getcpos_discount;

	if ((ff = form_field_new("Reason")) == NULL)
		err(1, "allocating fields");
	form_field_add(&creditform, ff);
	form_field_setyx(ff, LINE_ORDER + LINE_ORDER_SIZE / 2 + y++,
			MAX(OFFSET_DELIVERY, (COLS - ALIGN_REASONWIDTH - 8) / 2));
	form_field_setiw(ff, ALIGN_REASONWIDTH);
	form_field_setarg(ff, (void *)CREDIT_REASON);
	ff->putdata = &window_credit_putdata;

	/*
	 * REINIT
	 */

	window_wipe();
}

void
window_reinit(void)
{
	ChopstixFormField *ff;
	int y;

	TAILQ_FOREACH(ff, &discountform.fields, entry) {
		ff->y = LINE_ORDER + LINE_ORDER_SIZE / 2;
		ff->x = (COLS - ALIGN_MONEY - strlen("Discount")) / 2;
	}

	TAILQ_FOREACH(ff, &deliveryform.fields, entry) {
		ff->y = LINE_ORDER + LINE_ORDER_SIZE / 2;
		ff->x = (COLS - ALIGN_MONEY - strlen("Delivery")) / 2;
	}

	TAILQ_FOREACH(ff, &deliveryform.fields, entry) {
		ff->y = LINE_ORDER + LINE_ORDER_SIZE / 2;
		ff->x = (COLS - CHOPSTIX_PHONE_SIZE - strlen("New Phone #")) / 2;
	}

	y = -1;
	TAILQ_FOREACH(ff, &cashinform.fields, entry) {
		ff->y = LINE_ORDER + LINE_ORDER_SIZE / 2 + y++;
		ff->x = (COLS - ALIGN_MONEYWIDTH - 8) / 2;
	}

	y = -1;
	TAILQ_FOREACH(ff, &creditform.fields, entry) {
		ff->y = LINE_ORDER + LINE_ORDER_SIZE / 2 + y++;
		ff->x = MAX(OFFSET_DELIVERY, (COLS - ALIGN_REASONWIDTH - 8) / 2);
	}
}

void
window_wipe(void)
{
	ChopstixFormField *ff;

	TAILQ_FOREACH(ff, &discountform.fields, entry) {
		stracpy(&ff->input, "");
	}
	TAILQ_FOREACH(ff, &deliveryform.fields, entry) {
		stracpy(&ff->input, "");
	}
	TAILQ_FOREACH(ff, &chphoneform.fields, entry) {
		char number[ALIGN_NUMBER];
		int i;
		if (config.phoneprefix) {
			snprintf(number, sizeof(number), "%03d", config.phoneprefix);
			if (!stracpy(&ff->input, number))
				status_warn("cannot set phone autoprefix");
			/* move over some characters */
			for (i = 0; i < strlen(number); i++)
				ff->cpos = ff->getcpos(ff->input.s, ff->cpos,
						FIELD_POS_RIGHT, ff->arg);
		}
	}
	TAILQ_FOREACH(ff, &creditform.fields, entry) {
		stracpy(&ff->input, "");
	}
#if 0
	TAILQ_FOREACH(ff, &dailyinfoform.fields, entry) {
	}
#endif
}

void
window_exit(void)
{
}

static void
parse_moneystr(const char *str, int *money, CHOPSTIX_DISCOUNTTYPE *type)
{
	int64_t val = 0;
	int neg = 1;	/* set to -1 to negate the value */

	/* a negative sign is permitted anywhere, including inline */
	if (strchr(str, '-') != NULL)
		neg = -1;

	if (type)
		*type = DISCOUNT_NONE;

	while (*str) {
		if (!(isdigit(*str) || *str == '$' || *str == '.' || *str == '-'
					|| *str == '%'))
			return;

		if (isdigit(*str)) {
			val *= 10;
			val += (*str - '0');
		} else if (*str == '%')
			if (type)
				*type = DISCOUNT_PERCENT;

		*str++;
	}

	if (type)
		if (*type == DISCOUNT_NONE)
			*type = DISCOUNT_DOLLAR;

	*money = ROLLOVER(val * neg);
}

/*
 * Unset discount with zero-string
 */
static ChopstixFormAction
window_discount_putdata(const char *str, void *arg)
{
	if (strlen(str) == 0) {
		order.total.disctype = DISCOUNT_NONE;
		order.total.discount = 0;
	} else
		parse_moneystr(str, &order.total.discount, &order.total.disctype);
	return FIELD_NEXT;
}

static ChopstixFormAction
window_delivery_putdata(const char *str, void *arg)
{
	if (strlen(str) == 0) {
		order.total.delitype = DISCOUNT_NONE;
		order.total.delivery = 0;
	} else
		parse_moneystr(str, &order.total.delivery, &order.total.delitype);
	return FIELD_NEXT;
}

static ChopstixFormAction
window_chphone_putdata(const char *str, void *arg)
{
	if (customer_update_phone(str) == -1)
		return FORM_NONE;

	return FIELD_NEXT;
}

static ChopstixFormAction
window_dailyinfo_putdata(const char *str, void *arg)
{
	return FIELD_NEXT;
}

static ChopstixFormAction
window_cashin_putdata(const char *str, void *arg)
{
	ChopstixFormField *ff;
	int cashin = 0, total = 0, cashout = 0;
	enum cashin_type type;

	TAILQ_FOREACH(ff, &cashinform.fields, entry) {
		type = (enum cashin_type)ff->arg;
		switch (type) {

			case CASHIN_TOTAL:
				parse_moneystr(ff->input.s, &total, NULL);
				break;

			case CASHIN_CASH_IN:
				parse_moneystr(str, &cashin, NULL);
				if (!stracpy(&ff->input, money2str(ROLLOVER(cashin))))
					status_warn("cash in is incorrect");
				break;

			case CASHIN_CASH_OUT:
				cashout = cashin - total;
				if (!stracpy(&ff->input, money2str(ROLLOVER(cashout))))
					status_warn("cash out is incorrect");
				break;

			case CASHIN_NONE:
				stracpy(&ff->input, "ENTER");
				break;
		}
	}

	return FIELD_NEXT;
}

static ChopstixFormAction
window_credit_putdata(const char *str, void *arg)
{
	ChopstixFormField *ff, *ff2;
	ChopstixCredit *cc;
	int creditval = 0;

	TAILQ_FOREACH(ff, &creditform.fields, entry)
		if ((enum credit_type)ff->arg == (enum credit_type)arg &&
				(enum credit_type)arg == CREDIT_REASON) {
			/* pull credit value from other form field */
			TAILQ_FOREACH(ff2, &creditform.fields, entry)
				if ((enum credit_type)ff2->arg == CREDIT_THIS)
					parse_moneystr(ff2->input.s, &creditval, NULL);

			/* no complaints without credit */
			if (creditval == 0)
				break;

			if (order.customer.credit == NULL)
				if ((order.customer.credit = calloc(1,
								sizeof(*order.customer.credit)))
						== NULL) {
					status_warn("cannot create credit");
					break;
				}
			if ((cc = realloc(order.customer.credit->val,
							(order.customer.credit->len + 1) *
							sizeof(ChopstixCredit))) == NULL) {
				status_warn("cannot enter credit");
				break;
			}
			order.customer.credit->len++;
			order.customer.credit->val = cc;

			cc[order.customer.credit->len - 1].credit =
				cc[order.customer.credit->len - 1].remain =
				creditval;
			cc[order.customer.credit->len - 1].reason =
				strdup(ff->input.s);

			customer_post(&order.customer);

			/* clear fields after post */
			TAILQ_FOREACH(ff2, &creditform.fields, entry) {
				switch ((enum credit_type)ff2->arg) {
					case CREDIT_THIS:
					case CREDIT_REASON:
						stracpy(&ff2->input, "");
						break;
					case CREDIT_TOTAL:
						if (!stracpy(&ff2->input, money2str(ROLLOVER(
											customer_getcredit_total()))))
							status_warn("credit total is incorrect");
						break;

					case CREDIT_REMAIN:
						if (!stracpy(&ff2->input, money2str(ROLLOVER(
											customer_getcredit_remain()))))
							status_warn("credit remain is incorrect");
						break;
				}
			}
		}

	return FIELD_NEXT;
}

static const char *
money2str(int money)
{
	static char str[ALIGN_MONEYWIDTH + 1];

	if (money == 0)
		str[0] = '\0';
	else if (money == INT_MAX || money == INT_MIN)
		snprintf(str, sizeof(str), "$%s########.##",
				money < 0 ? "-" : "");
	else
		snprintf(str, sizeof(str), "$%s%0d.%02d",
				NEGSIGN(money), DOLLARS(money), CENTS(money));

	return str;
}

void
window_update_daily(ChopstixTotal *total)
{
	ChopstixFormField *ff;
	enum daily_type type;
	int64_t sum;

	TAILQ_FOREACH(ff, &dailyinfoform.fields, entry) {
		type = (enum daily_type)ff->arg;
		switch (type) {
			case DAILY_ORDER:
				sum = total->subtotal - total->discount + total->delivery;
				if (!stracpy(&ff->input, money2str(ROLLOVER(sum))))
					status_warn("daily order subtotal incorrect");
				break;
			case DAILY_TAX1:
				if (!stracpy(&ff->input, money2str(total->tax1)))
					status_warn("daily tax1 incorrect");
				break;
			case DAILY_TAX2:
				if (!stracpy(&ff->input, money2str(total->tax2)))
					status_warn("daily tax2 incorrect");
				break;
			case DAILY_TOTAL:
				if (!stracpy(&ff->input, money2str(total->total)))
					status_warn("daily total incorrect");
				break;
			case DAILY_NONE:
				stracpy(&ff->input, "ENTER");
				break;
		}
	}
}

void
window_update_credit(void)
{
	ChopstixFormField *ff;
	enum credit_type type;

	TAILQ_FOREACH(ff, &creditform.fields, entry) {
		type = (enum credit_type)ff->arg;
		switch (type) {

			case CREDIT_TOTAL:
				if (!stracpy(&ff->input,
							money2str(ROLLOVER(customer_getcredit_total()))))
					status_warn("credit total is incorrect");
				break;

			case CREDIT_REMAIN:
				if (!stracpy(&ff->input,
							money2str(ROLLOVER(customer_getcredit_remain()))))
					status_warn("credit remain is incorrect");
				break;

			case CREDIT_THIS:
			case CREDIT_REASON:
				break;
		}
	}
}

/*
 * Calculate cash in/out
 * This relies or the ordering of fields to be exactly like the switch
 * statement!
 */
void
window_update_cashin(ChopstixTotal *total)
{
	ChopstixFormField *ff;
	enum cashin_type type;
	char number[ALIGN_NUMBER];

	TAILQ_FOREACH(ff, &cashinform.fields, entry) {
		type = (enum cashin_type)ff->arg;
		switch (type) {
			case CASHIN_TOTAL:
				snprintf(number, sizeof(number), "%d", total->total);
				if (!stracpy(&ff->input, number))
					status_warn("cash in/out total incorrect");
				break;

			case CASHIN_CASH_IN:
			case CASHIN_CASH_OUT:
				stracpy(&ff->input, "");
				break;

			case CASHIN_NONE:
				stracpy(&ff->input, "ENTER");
				break;
		}
	}
}
