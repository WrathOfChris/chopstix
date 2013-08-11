/* $Gateweaver: orders.c,v 1.8 2007/09/28 18:02:06 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "report.h"

static ChopstixTotal grand_total;
static int total_ok = 0;

static int
order_getrange_cb(const ChopstixOrder *order, void *arg)
{
	const char *fn = arg;
	ChopstixOrder *v_order;

	if (fn) {
		(const ChopstixOrder *)v_order = order;
		render_html(fn, render_order_list, v_order);
	}

	if (total_ok == 0) {
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
render_order(const char *m, void *arg)
{
	char fn[1024];
	struct tm tm_start, tm_end;
	time_t start, end;

	bcopy(localtime(&time_start), &tm_start, sizeof(tm_start));
	start = mktime(&tm_start);

	bcopy(localtime(&time_stop), &tm_end, sizeof(tm_end));
	end = mktime(&tm_end);

	if (!strcmp(m, "ORDERLIST")) {
		snprintf(fn, sizeof(fn), "%s/order_list.html", conf.htmldir);
		if (odbf.getrange(&odbh, start, end, order_getrange_cb, fn) == -1) {
			render_error("cannot load orders, see system log for details");
			return;
		}
		total_ok = 1;

	} else if (!strcmp(m, "ORDERTOTALS")) {
		if (!total_ok) {
			if (odbf.getrange(&odbh, start, end, order_getrange_cb, NULL)
					== -1) {
				render_error("cannot load orders, see system log for details");
				return;
			}
			total_ok = 1;
		}
		snprintf(fn, sizeof(fn), "%s/order_total.html", conf.htmldir);
		render_html(fn, render_order_totals, &grand_total);

	} else
		printf("render_order: unknown macro '%s'<br>\n", m);
}

void
render_order_list(const char *m, void *arg)
{
	ChopstixOrder *order = (ChopstixOrder *)arg;
	char fn[1024];
	unsigned int u, s;
	tbstring sa = {0};
	tbstring sa2 = {0};

	if (!strcmp(m, "ORDERCODE"))
		printf("%d", order->key);

	else if (!strcmp(m, "ORDERCODEEDIT"))
		printf("<a href=\"%s?action=orderdetail&amp;code=%d\">%d</a>",
				conf.baseurl, order->key, order->key);

	else if (!strcmp(m, "ORDERDATETIME")) {
		char datetime[sizeof("YYYY-MM-DD HH:MM:SS")];
		strftime(datetime, sizeof(datetime), "%F %T",
				localtime(&order->date));
		printf("%s", datetime);

	} else if (!strcmp(m, "ORDERTYPE")) {
		switch (order->type) {
			case ORDER_NONE:
				printf("NONE");
				break;
			case ORDER_PICKUP:
				printf("PICKUP");
				break;
			case ORDER_DELIVERY:
				printf("DELIVERY");
				break;
			case ORDER_WALKIN:
				printf("WALKIN");
				break;
			case ORDER_VOID:
				printf("VOID");
				break;
		}

	} else if (!strcmp(m, "ORDERPAYTYPE")) {
		switch (order->payment.type) {
			case PAYMENT_NONE:
				printf("NONE");
				break;
			case PAYMENT_CASH:
				printf("CASH");
				break;
			case PAYMENT_CREDIT:
				printf("CREDIT");
				break;
			case PAYMENT_VOID:
				printf("VOID");
				break;
			case PAYMENT_DEBIT:
				printf("DEBIT");
				break;
			case PAYMENT_CHEQUE:
				printf("CHEQUE");
				break;
			case PAYMENT_OTHER:
				printf("OTHER");
				break;
		}

	} else if (!strcmp(m, "ORDERPAYDETAIL")) {
		switch (order->payment.type) {
			case PAYMENT_NONE:
				printf("NONE");
				break;
			case PAYMENT_CASH:
				printf("CASH");
				break;
			case PAYMENT_CREDIT:
				if (order->payment.ccinfo)
					printf("CREDIT - CC# %s EXP %s",
							html_esc(order->payment.ccinfo->number, &sa, 0),
							html_esc(order->payment.ccinfo->expiry, &sa2, 0));
				else
					printf("CREDIT - (INFO PURGED)");
				break;
			case PAYMENT_VOID:
				printf("VOID");
				break;
			case PAYMENT_DEBIT:
				printf("DEBIT");
				break;
			case PAYMENT_CHEQUE:
				printf("CHEQUE");
				break;
			case PAYMENT_OTHER:
				printf("OTHER");
				break;
		}

	} else if (!strcmp(m, "ORDERSPECIAL"))
		printf("%s", strlen(order->special)
				? html_esc(order->special, &sa, 0) : "&nbsp;");
	else if (!strcmp(m, "ORDERSUBTOTAL"))
		PRINTF_MONEY(order->total.subtotal);
	else if (!strcmp(m, "ORDERDISCOUNT"))
		PRINTF_MONEY(order->total.discount);
	else if (!strcmp(m, "ORDERDELIVERY"))
		PRINTF_MONEY(order->total.delivery);
	else if (!strcmp(m, "ORDERCREDIT"))
		PRINTF_MONEY(order->total.credit);
	else if (!strcmp(m, "ORDERTAX1"))
		PRINTF_MONEY(order->total.tax1);
	else if (!strcmp(m, "ORDERTAX2"))
		PRINTF_MONEY(order->total.tax2);
	else if (!strcmp(m, "ORDERTOTAL"))
		PRINTF_MONEY(order->total.total);
	else if (!strncmp(m, "CUST", 4))
		render_customer_info(m, &order->customer);
	else if (!strcmp(m, "ORDERITEMS")) {
		snprintf(fn, sizeof(fn), "%s/order_item.html", conf.htmldir);
		for (u = 0; u < order->items.len; u++) {
			render_html(fn, render_order_item, &order->items.val[u]);
			if (order->items.val[u].subitems)
				for (s = 0; s < order->items.val[u].subitems->len; s++)
					render_html(fn, render_order_subitem,
							&order->items.val[u].subitems->val[s]);
		}
	} else if (!strcmp(m, "ORDERRULEITEMS")) {
		snprintf(fn, sizeof(fn), "%s/order_item.html", conf.htmldir);
		for (u = 0; u < order->ruleitems.len; u++)
			render_html(fn, render_order_item, &order->ruleitems.val[u]);
	} else
		printf("render_order_list: unknown macro '%s'<br>\n", m);

	tbstrfree(&sa);
	tbstrfree(&sa2);
}

void
render_order_totals(const char *m, void *arg)
{
	ChopstixTotal *total= (ChopstixTotal *)arg;

	if (!strcmp(m, "ORDERSUBTOTAL"))
		PRINTF_MONEY(total->subtotal);
	else if (!strcmp(m, "ORDERDISCOUNT"))
		PRINTF_MONEY(total->discount);
	else if (!strcmp(m, "ORDERDELIVERY"))
		PRINTF_MONEY(total->delivery);
	else if (!strcmp(m, "ORDERCREDIT"))
		PRINTF_MONEY(total->credit);
	else if (!strcmp(m, "ORDERTAX1"))
		PRINTF_MONEY(total->tax1);
	else if (!strcmp(m, "ORDERTAX2"))
		PRINTF_MONEY(total->tax2);
	else if (!strcmp(m, "ORDERTOTAL"))
		PRINTF_MONEY(total->total);
	else
		printf("render_order_totals: unknown macro '%s'<br>\n", m);
}

void
render_order_item(const char *m, void *arg)
{
	ChopstixOrderItem *item = (ChopstixOrderItem *)arg;
	ChopstixMenuitem *mi;
	unsigned int u;

	mi = menu_getitem(item->code);

	if (!strcmp(m, "ITEMQTY"))
		printf("%d", item->qty);
	else if (!strcmp(m, "ITEMCODE"))
		printf("%s", item->code);
	else if (!strcmp(m, "ITEMDESC"))
		printf("%s", mi ? mi->name : "UNKNOWN");
	else if (!strcmp(m, "ITEMDETAIL")) {
		printf("%s", mi ? mi->name : "UNKNOWN");
		if (mi && item->style > 0)
			for (u = 0; u < mi->styles.len; u++)
				if (mi->styles.val[u].num == item->style)
					printf(" (%s)", mi->styles.val[u].name);
		if (item->special && *item->special && **item->special)
			printf(" \"%s\"", *item->special);
	} else if (!strcmp(m, "ITEMPRICE"))
		PRINTF_MONEY(mi ? mi->price : 0);
	else if (!strcmp(m, "ITEMSUBTOTAL"))
		PRINTF_MONEY(mi ? mi->price * item->qty : 0);
	else
		printf("render_order_item: unknown macro '%s'<br>\n", m);
}

void
render_order_subitem(const char *m, void *arg)
{
	ChopstixSubItem *item = (ChopstixSubItem *)arg;
	ChopstixMenuitem *mi;
	unsigned int u;

	mi = menu_getitem(item->code);

	if (!strcmp(m, "ITEMQTY"))
		printf("%d", item->qty);
	else if (!strcmp(m, "ITEMCODE"))
		printf("%s", item->code);
	else if (!strcmp(m, "ITEMDESC"))
		printf("%s", mi ? mi->name : "UNKNOWN");
	else if (!strcmp(m, "ITEMDETAIL")) {
		printf("%s", mi ? mi->name : "UNKNOWN");
		if (mi && item->style > 0)
			for (u = 0; u < mi->styles.len; u++)
				if (mi->styles.val[u].num == item->style)
					printf(" (%s)", mi->styles.val[u].name);
		if (item->special && *item->special && **item->special)
			printf(" \"%s\"", *item->special);
	} else if (!strcmp(m, "ITEMPRICE"))
		;	/* DO NOTHING */
	else if (!strcmp(m, "ITEMSUBTOTAL")) {
		if (item->pricedelta != 0)
			PRINTF_MONEY(item->pricedelta);
	} else
		printf("render_order_subitem: unknown macro '%s'<br>\n", m);
}
