/* $Gateweaver: payment.c,v 1.3 2007/09/28 17:19:49 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "report.h"

static ChopstixTotal grand_total;
static int pay_totals[PAYMENT_DEBIT + 1];
static int pay_counts[PAYMENT_DEBIT + 1];
static int pay_allcount = 0;
static int total_ok = 0;

static CHOPSTIX_PAYMENTTYPE
payment_parse_type(const char *str)
{
	if (!strcmp(str, "CASH"))
		return PAYMENT_CASH;
	else if (!strcmp(str, "CREDIT"))
		return PAYMENT_CREDIT;
	else if (!strcmp(str, "VOID"))
		return PAYMENT_VOID;
	else if (!strcmp(str, "DEBIT"))
		return PAYMENT_DEBIT;

	return PAYMENT_NONE;
}

static int
payment_getrange_cb(const ChopstixOrder *order, void *arg)
{
	if (total_ok == 0) {
		if (order->payment.type <= PAYMENT_DEBIT) {
			pay_totals[order->payment.type] += order->total.total;
			pay_counts[order->payment.type]++;
		}
		pay_allcount++;

		grand_total.subtotal += order->total.subtotal;
		grand_total.discount += order->total.discount;
		grand_total.delivery += order->total.delivery;
		grand_total.credit += order->total.credit;
		grand_total.tax1 += order->total.tax1;
		grand_total.tax2 += order->total.tax2;
		grand_total.total += order->total.total;
	}

	return 0;
}

void
render_payment(const char *m, void *arg)
{
	CHOPSTIX_PAYMENTTYPE type;
	struct tm tm_start, tm_end;
	time_t start, end;

	bcopy(localtime(&time_start), &tm_start, sizeof(tm_start));
	start = mktime(&tm_start);

	bcopy(localtime(&time_stop), &tm_end, sizeof(tm_end));
	end = mktime(&tm_end);

	if (!strncmp(m, "PAY", 3) && !total_ok) {
		if (odbf.getrange(&odbh, start, end, payment_getrange_cb, NULL)
				== -1) {
			render_error("cannot load orders, see system log for details");
			return;
		}
		total_ok = 1;
	}

	if (!strcmp(m, "PAYCOUNT"))
		printf("%u", pay_allcount);
	else if (!strcmp(m, "PAYSUBTOTAL"))
		PRINTF_MONEY(grand_total.subtotal);
	else if (!strcmp(m, "PAYDISCOUNT"))
		PRINTF_MONEY(grand_total.discount);
	else if (!strcmp(m, "PAYDELIVERY"))
		PRINTF_MONEY(grand_total.delivery);
	else if (!strcmp(m, "PAYCREDIT"))
		PRINTF_MONEY(grand_total.credit);
	else if (!strcmp(m, "PAYTAX1"))
		PRINTF_MONEY(grand_total.tax1);
	else if (!strcmp(m, "PAYTAX2"))
		PRINTF_MONEY(grand_total.tax2);
	else if (!strcmp(m, "PAYTOTAL"))
		PRINTF_MONEY(grand_total.total);

	else if (!strncmp(m, "PAYTOTAL", 8) && m[8] != '\0') {
		if ((type = payment_parse_type(m+8)) <= PAYMENT_DEBIT)
			PRINTF_MONEY(pay_totals[type]);
		else
			PRINTF_MONEY(0);

	} else if (!strncmp(m, "PAYCOUNT", 8) && m[8] != '\0') {
		if ((type = payment_parse_type(m+8)) <= PAYMENT_DEBIT)
			printf("%d", pay_counts[type]);
		else
			printf("%d", 0);
	}
}
