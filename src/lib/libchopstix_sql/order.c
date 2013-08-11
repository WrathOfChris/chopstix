/* $Gateweaver: order.c,v 1.21 2007/09/28 17:14:14 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chopstix_sql.h"

RCSID("$Gateweaver: order.c,v 1.21 2007/09/28 17:14:14 cmaxwell Exp $");

static int odb_open(struct orderdb_handle *);
static int odb_create(struct orderdb_handle *);
#if 0
static int odb_read(struct orderdb_handle *, ChopstixOrder *);
static int odb_readkey(struct orderdb_handle *, ChopstixOrderKey);
static int odb_rewind(struct orderdb_handle *);
#endif
static int odb_get(struct orderdb_handle *, const ChopstixOrderKey,
		ChopstixOrder *);
static int odb_getlast(struct orderdb_handle *, const ChopstixPhone *,
		ChopstixOrder *);
static int odb_getdaily_total(struct orderdb_handle *, ChopstixTotal *);
static int odb_getrange(struct orderdb_handle *, time_t, time_t,
		int (*)(const ChopstixOrder *, void *), void *);
static int odb_put(struct orderdb_handle *, const ChopstixOrder *, int *,
		time_t *);
static int odb_update(struct orderdb_handle *, const ChopstixOrder *);
#if 0
static int odb_remove(struct custdb_handle *, const int64_t);
#endif
static int odb_close(struct orderdb_handle *);
static const char * odb_geterr(struct orderdb_handle *);
static ChopstixMenuitem * odb_getmenuitem(char *);

static void (*odb_puterr)(const char *, va_list);
static ChopstixMenuitem * (*odb_getmenuitem_int)(char *);

void
odb_init(struct orderdb_functions *odbf)
{
	odbf->open = &odb_open;
	odbf->create = &odb_create;
#if 0
	odbf->read = &odb_read;
	odbf->readkey = &odb_readkey;
	odbf->rewind = &odb_rewind;
#endif
	odbf->get = &odb_get;
	odbf->getlast = &odb_getlast;
	odbf->getdaily_total = &odb_getdaily_total;
	odbf->getrange = &odb_getrange;
	odbf->put = &odb_put;
	odbf->update = &odb_update;
#if 0
	odbf->remove = &odb_remove;
#endif
	odbf->close = &odb_close;
	odbf->geterr = &odb_geterr;

	/* caller set error function */
	odb_puterr = odbf->err;
	odb_getmenuitem_int = odbf->getmenuitem;
}

static int
odb_open(struct orderdb_handle *oh)
{
	if (SQLARG(oh) != NULL) {
		errno = EBADF;
		return -1;
	}

	if ((SQLARG(oh) = sql_makearg(SQL_ODB)) == NULL)
		return -1;
	SQLARG(oh)->err = odb_puterr;

	if (sql_open(SQLARG(oh), oh->dbfile) == -1) {
		sql_freearg(SQLARG(oh));
		return -1;
	}

	return 0;
}

/*
 * SQLite must be created manually for the moment.
 *
 * XXX embed the table create SQL commands into here?
 */
static int
odb_create(struct orderdb_handle *oh)
{
	errno = EACCES;
	return -1;
}

static int
odb_get(struct orderdb_handle *oh, const ChopstixOrderKey orderid,
		ChopstixOrder *order)
{
	sqlite3_stmt *q = NULL;
	ChopstixOrderItems *items;
	ChopstixOrderItem *oi;
	ChopstixSubItems *subitems;
	ChopstixSubItem *si;
	CHOPSTIX_ITEMTYPE itype;
	struct tm tm;
	const char *special;

	CHECKSQL(oh);
	bzero(order, sizeof(*order));
	bzero(&tm, sizeof(tm));

	/*
	 * ORDER
	 *
	 * Also gets the customer phone number, to be handled by cdb_get
	 */

	q = SQLARG(oh)->q.odb.get_order;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, orderid)) {
		sql_err(SQLARG(oh), "cannot bind order ID to query");
		goto fail;
	}
	if (sqlite3_step(q) == SQLITE_ROW) {
		/*
		 * Date, Phone, Type, Special, Paytype, Subtotal, Discount, Delivery,
		 * Tax1, Tax2, Total
		 */
		if (strptime(sqlite3_column_text(q, 0), SQL_TIME_FMT, &tm) == NULL) {
			sql_err(SQLARG(oh), "bad order time format");
			goto fail;
		}
		order->key = orderid;
		order->date = timegm(&tm);
		str2phone(sqlite3_column_text(q, 1), &order->customer.phone);
		order->type = sqlite3_column_int(q, 2);
		order->special = sql_strdup(sqlite3_column_text(q, 3));
		order->payment.type = sqlite3_column_int(q, 4);

		order->total.subtotal = sqlite3_column_int(q, 5);
		order->total.discount = sqlite3_column_int(q, 6);
		order->total.delivery = sqlite3_column_int(q, 7);
		order->total.credit = sqlite3_column_int(q, 8);
		order->total.tax1 = sqlite3_column_int(q, 9);
		order->total.tax2 = sqlite3_column_int(q, 10);
		order->total.total = sqlite3_column_int(q, 11);
	} else {
		sql_err(SQLARG(oh), "GET ORDER id=%d", orderid);
		errno = ENOENT;
		goto fail;
	}
	sqlite3_reset(q);

	/*
	 * ORDER ITEMS
	 */

	q = SQLARG(oh)->q.odb.get_items;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, orderid)) {
		sql_err(SQLARG(oh), "cannot bind order ID to query");
		goto fail;
	}
	while (sqlite3_step(q) == SQLITE_ROW) {
		itype = sqlite3_column_int(q, 0);

		/*
		 * Type, Qty, Code, Style, Price, Special
		 *
		 * This query relies on the ordering of Type as follows:
		 * 	NORMAL, SUBITEM
		 *
		 * Subitems are appended to the LAST normal item.
		 */
		switch (itype) {

			case ITEM_NORMAL:
				items = &order->items;
				/* increase items */
				if ((oi = realloc(items->val, (items->len + 1) *
								sizeof(ChopstixOrderItem))) == NULL)
					goto fail;
				items->len++;
				items->val = oi;
				bzero(&items->val[items->len - 1],
						sizeof(items->val[items->len - 1]));

				items->val[items->len - 1].type = itype;
				items->val[items->len - 1].qty = sqlite3_column_int(q, 1);
				items->val[items->len - 1].code
					= sql_strdup(sqlite3_column_text(q, 2));
				items->val[items->len - 1].style
					= sqlite3_column_int(q, 3);
				/* 4:Price unused */
				if ((special = sqlite3_column_text(q, 5)) != NULL) {
					if ((items->val[items->len - 1].special = calloc(1,
									sizeof(*items->val[items->len - 1].special)))
							== NULL)
						goto fail;
					*items->val[items->len - 1].special = sql_strdup(special);
				}
				break;

			case ITEM_SUBITEM:
				if (order->items.val[order->items.len - 1].subitems == NULL)
					if ((order->items.val[order->items.len - 1].subitems
								= calloc(1, sizeof(ChopstixSubItems))) == NULL)
						goto fail;
				subitems = order->items.val[order->items.len - 1].subitems;
				/* increase subitems */
				if ((si = realloc(subitems->val, (subitems->len + 1) *
								sizeof(ChopstixSubItem))) == NULL)
					goto fail;
				subitems->len++;
				subitems->val = si;
				bzero(&subitems->val[subitems->len - 1],
						sizeof(subitems->val[subitems->len - 1]));

				/* 0:Type unused */
				subitems->val[subitems->len - 1].qty = sqlite3_column_int(q, 1);
				subitems->val[subitems->len - 1].code
					= sql_strdup(sqlite3_column_text(q, 2));
				subitems->val[subitems->len - 1].style
					= sqlite3_column_int(q, 3);
				subitems->val[subitems->len - 1].pricedelta
					= sqlite3_column_int(q, 4);
				if ((special = sqlite3_column_text(q, 5)) != NULL) {
					if ((subitems->val[subitems->len - 1].special = calloc(1,
									sizeof(*subitems->val[subitems->len - 1].special)))
							== NULL)
						goto fail;
					*subitems->val[subitems->len - 1].special
						= sql_strdup(special);
				}
				break;

			case ITEM_EXTRA:
				break;

			case ITEM_RULE:
				items = &order->ruleitems;
				/* increase items */
				if ((oi = realloc(items->val, (items->len + 1) *
								sizeof(ChopstixOrderItem))) == NULL)
					goto fail;
				items->len++;
				items->val = oi;
				bzero(&items->val[items->len - 1],
						sizeof(items->val[items->len - 1]));

				items->val[items->len - 1].type = itype;
				items->val[items->len - 1].qty = sqlite3_column_int(q, 1);
				items->val[items->len - 1].code
					= sql_strdup(sqlite3_column_text(q, 2));
				items->val[items->len - 1].style
					= sqlite3_column_int(q, 3);
				/* 4:Price unused */
				if ((special = sqlite3_column_text(q, 5)) != NULL) {
					if ((items->val[items->len - 1].special = calloc(1,
									sizeof(*items->val[items->len - 1].special)))
							== NULL)
						goto fail;
					*items->val[items->len - 1].special = sql_strdup(special);
				}
				break;
		}
	}
	sqlite3_reset(q);

	/*
	 * PAYMENT (if credit)
	 */

	if (order->payment.type == PAYMENT_CREDIT) {
		if (sqlite3_prepare(SQLARG(oh)->db, SQL_ODB_GET_PAYMENT,
					strlen(SQL_ODB_GET_PAYMENT), &q, NULL)) {
			sql_err(SQLARG(oh), "OQ PREP GET PAYMENT");
			goto fail;
		}
		if (sqlite3_bind_int(q, 1, order->key)) {
			sql_err(SQLARG(oh), "cannot bind order ID to query");
			sqlite3_finalize(q);
			q = NULL;
			goto fail;
		}
		if (sqlite3_step(q) == SQLITE_ROW) {
			if (order->payment.ccinfo == NULL)
				if ((order->payment.ccinfo = calloc(1,
								sizeof(*order->payment.ccinfo))) == NULL) {
					sqlite3_finalize(q);
					q = NULL;
					goto fail;
				}
			order->payment.ccinfo->number = sql_strdup(
					sqlite3_column_text(q, 0));
			order->payment.ccinfo->expiry = sql_strdup(
					sqlite3_column_text(q, 1));
		}
		sqlite3_finalize(q);
		q = NULL;
	}

	return 0;

fail:
	if (q)
		sqlite3_reset(q);
	return -1;
}

static int
odb_getlast(struct orderdb_handle *oh, const ChopstixPhone *phone,
		ChopstixOrder *order)
{
	char *phonestr;
	sqlite3_stmt *q = NULL;
	int64_t order_ID = -1;

	CHECKSQL(oh);

	if (phone == NULL)
		phonestr = "%";
	else
		if ((phonestr = phone2str(phone)) == NULL)
			goto fail;

	q = SQLARG(oh)->q.odb.get_lastorder;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, phonestr, strlen(phonestr), SQLITE_STATIC)) {
		sql_err(SQLARG(oh), "cannot bind phone to query");
		goto fail;
	}
	if (sqlite3_step(q) == SQLITE_ROW)
		order_ID = sqlite3_column_int64(q, 0);
	else {
		errno = ENOENT;
		goto fail;
	}
	sqlite3_reset(q);

	return odb_get(oh, order_ID, order);

fail:
	if (q)
		sqlite3_reset(q);
	return -1;
}

static int
odb_getdaily_total(struct orderdb_handle *oh, ChopstixTotal *totals)
{
	sqlite3_stmt *q = NULL;
	int64_t subtotal = 0, discount = 0, delivery = 0, credit = 0, tax1 = 0,
			tax2 = 0, total = 0;
	char s_start[sizeof("YYYY-MM-DD HH:MM:SS")];
	char s_end[sizeof("YYYY-MM-DD HH:MM:SS")];
	struct tm tm_start, tm_end;
	time_t now, start, end;

	CHECKSQL(oh);

	/*
	 * Database is in UTC, yet 'daily' means the local day.
	 * Set localtime values from midnight-midnight.
	 * Convert back to GMT/UTC for string conversion
	 */
	now = time(NULL);
	bcopy(localtime(&now), &tm_start, sizeof(struct tm));
	tm_start.tm_min = 0;
	tm_start.tm_sec = 0;
	tm_start.tm_hour = 0;
	bcopy(localtime(&now), &tm_end, sizeof(struct tm));
	tm_end.tm_hour = 23;
	tm_end.tm_min = 59;
	tm_end.tm_sec = 60;

	start = mktime(&tm_start);
	end = mktime(&tm_end);

	strftime(s_start, sizeof(s_start), SQL_TIME_FMT, gmtime(&start));
	strftime(s_end, sizeof(s_end), SQL_TIME_FMT, gmtime(&end));

	if (sqlite3_prepare(SQLARG(oh)->db, SQL_ODB_GET_DAILY_TOTAL,
				strlen(SQL_ODB_GET_DAILY_TOTAL), &q, NULL)) {
		sql_err(SQLARG(oh), "OQ PREP GET DAILY_TOTAL");
		goto fail;
	 }
	if (sqlite3_bind_text(q, 1, s_start, strlen(s_start), SQLITE_STATIC)) {
		sql_err(SQLARG(oh), "cannot bind start date to query");
		goto fail;
	}
	if (sqlite3_bind_text(q, 2, s_end, strlen(s_end), SQLITE_STATIC)) {
		sql_err(SQLARG(oh), "cannot bind end date to query");
		goto fail;
	}
	while (sqlite3_step(q) == SQLITE_ROW) {
		subtotal += sqlite3_column_int(q, 0);
		discount += sqlite3_column_int(q, 1);
		delivery += sqlite3_column_int(q, 2);
		credit += sqlite3_column_int(q, 3);
		tax1 += sqlite3_column_int(q, 4);
		tax2 += sqlite3_column_int(q, 5);
		total += sqlite3_column_int(q, 6);
	}
	sqlite3_finalize(q);

	totals->subtotal = ROLLOVER(subtotal);
	totals->disctype = DISCOUNT_DOLLAR;
	totals->discount = ROLLOVER(discount);
	totals->delitype = DISCOUNT_DOLLAR;
	totals->delivery = ROLLOVER(delivery);
	totals->credit = ROLLOVER(credit);
	totals->tax1 = ROLLOVER(tax1);
	totals->tax2 = ROLLOVER(tax2);
	totals->total = ROLLOVER(total);

	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
odb_getrange(struct orderdb_handle *oh, time_t t_start, time_t t_end,
		int (*cbfunc)(const ChopstixOrder *, void *), void *cbarg)
{
	sqlite3_stmt *q = NULL;
	char s_start[sizeof("YYYY-MM-DD HH:MM:SS")];
	char s_end[sizeof("YYYY-MM-DD HH:MM:SS")];
	ChopstixOrder o = {0};

	CHECKSQL(oh);

	/*
	 * Database is in UTC, so use represent using gmtime.
	 */
	strftime(s_start, sizeof(s_start), SQL_TIME_FMT, gmtime(&t_start));
	strftime(s_end, sizeof(s_end), SQL_TIME_FMT, gmtime(&t_end));

	if (sqlite3_prepare(SQLARG(oh)->db, SQL_ODB_GET_DAILY_LIST,
				strlen(SQL_ODB_GET_DAILY_LIST), &q, NULL)) {
		sql_err(SQLARG(oh), "OQ PREP GET DAILY_LIST");
		goto fail;
	}
	if (sqlite3_bind_text(q, 1, s_start, strlen(s_start), SQLITE_STATIC)) {
		sql_err(SQLARG(oh), "cannot bind start date to query");
		goto fail;
	}
	if (sqlite3_bind_text(q, 2, s_end, strlen(s_end), SQLITE_STATIC)) {
		sql_err(SQLARG(oh), "cannot bind end date to query");
		goto fail;
	}

	while (sqlite3_step(q) == SQLITE_ROW) {
		free_ChopstixOrder(&o);
		o.key = sqlite3_column_int(q, 0);
		if (odb_get(oh, o.key, &o) == -1)
			goto fail;

		if (cbfunc)
			if (cbfunc(&o, cbarg) == -1)
				goto fail;
	}
	free_ChopstixOrder(&o);

	sqlite3_finalize(q);

	return 0;

fail:
	free_ChopstixOrder(&o);
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
add_extras(ChopstixItemExtras *extras, ChopstixMenuitem *mi, int qty)
{
#ifdef POST_EXTRAS
	unsigned mu, u;
	ChopstixItemExtra *ie;

	/* gracefully handle this case, nothing can be done about it now */
	if (mi == NULL)
		return 0;

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
#endif
	return 0;
}

static int
odb_put(struct orderdb_handle *oh, const ChopstixOrder *order, int *oid,
		time_t *otime)
{
	char *phonestr;
	sqlite3_stmt *q = NULL;
	ChopstixOrderItem *item;
	ChopstixSubItem *subitem;
	unsigned u_item, u_sub;
	int64_t order_ID;
	ChopstixItemExtras extras;
	struct tm tm;
	int retry = 0, ret;

	CHECKSQL(oh);
	bzero(&extras, sizeof(extras));

	if ((phonestr = phone2str(&order->customer.phone)) == NULL)
		goto fail;

	/*
	 * Wrap the entire order entry in a transaction
	 */
	if (sql_exec(SQLARG(oh), "BEGIN TRANSACTION;", NULL, NULL) == -1)
		goto fail;

	/*
	 * ORDER
	 */

	q = SQLARG(oh)->q.odb.put_order;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, order->type))
		goto fail;
	if (order->special) {
		if (sqlite3_bind_text(q, 2, order->special, strlen(order->special),
					SQLITE_STATIC))
			goto fail;
	} else
		if (sqlite3_bind_null(q, 2))
			goto fail;
	if (sqlite3_bind_int(q, 3, order->payment.type))
		goto fail;
	if (sqlite3_bind_int(q, 4, order->total.subtotal))
		goto fail;
	if (order->total.disctype == DISCOUNT_PERCENT) {
		if (sqlite3_bind_int(q, 5, TAXCALC(order->total.subtotal,
						order->total.discount)))
			goto fail;
	} else {
		if (sqlite3_bind_int(q, 5, order->total.discount))
			goto fail;
	}
	if (order->total.delitype == DISCOUNT_PERCENT) {
		if (sqlite3_bind_int(q, 6, TAXCALC(order->total.subtotal,
						order->total.delivery)))
			goto fail;
	} else {
		if (sqlite3_bind_int(q, 6, order->total.delivery))
			goto fail;
	}
	if (sqlite3_bind_int(q, 7, order->total.credit))
		goto fail;
	if (sqlite3_bind_int(q, 8, order->total.tax1))
		goto fail;
	if (sqlite3_bind_int(q, 9, order->total.tax2))
		goto fail;
	if (sqlite3_bind_int(q, 10, order->total.total))
		goto fail;
	if (sqlite3_bind_text(q, 11, phonestr, strlen(phonestr), SQLITE_STATIC))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(oh), "cannot post order");
		goto fail;
	}
	sqlite3_reset(q);

	/* grab the orderID for all the items and subitems */
	order_ID = sqlite3_last_insert_rowid(SQLARG(oh)->db);

	/*
	 * ITEMS
	 */

	/* 1 = first parameter in prepared query */
	for (u_item = 0; u_item < order->items.len; u_item++) {
		item = &order->items.val[u_item];

		q = SQLARG(oh)->q.odb.put_item;
		if (sqlite3_bind_int64(q, 1, order_ID))
			goto fail;
		if (sqlite3_bind_int(q, 2, (u_item + 1)))	/* Line */
			goto fail;
		if (sqlite3_bind_int(q, 3, ITEM_NORMAL))
			goto fail;
		if (sqlite3_bind_int(q, 4, item->qty))
			goto fail;
		if (item->special) {
			if (sqlite3_bind_text(q, 5, *item->special, strlen(*item->special),
						SQLITE_STATIC))
				goto fail;
		} else {
			if (sqlite3_bind_null(q, 5))
				goto fail;
		}
		if (sqlite3_bind_int(q, 6, item->style))
			goto fail;
		if (sqlite3_bind_text(q, 7, item->code, strlen(item->code),
					SQLITE_STATIC))
			goto fail;
		if (sqlite3_bind_text(q, 8, item->code, strlen(item->code),
					SQLITE_STATIC))
			goto fail;
		if (sqlite3_step(q) != SQLITE_DONE) {
			sql_err(SQLARG(oh), "cannot add orderitem");
			goto fail;
		}
		sqlite3_reset(q);

		if (add_extras(&extras, odb_getmenuitem(item->code), item->qty) == -1)
			goto fail;

		/*
		 * SUBITEMS
		 */
		if (item->subitems != NULL) {
			q = SQLARG(oh)->q.odb.put_subitem;
			for (u_sub = 0; u_sub < item->subitems->len; u_sub++) {
				subitem = &item->subitems->val[u_sub];

				if (sqlite3_bind_int64(q, 1, order_ID))
					goto fail;
				if (sqlite3_bind_int(q, 2, (u_item + 1)))	/* Line */
					goto fail;
				if (sqlite3_bind_int(q, 3, ITEM_SUBITEM))
					goto fail;
				if (sqlite3_bind_int(q, 4, subitem->qty))
					goto fail;
				if (sqlite3_bind_int(q, 5, subitem->pricedelta))
					goto fail;
				if (subitem->special) {
					if (sqlite3_bind_text(q, 6, *subitem->special,
								strlen(*subitem->special), SQLITE_STATIC))
						goto fail;
				} else {
					if (sqlite3_bind_null(q, 6))
						goto fail;
				}
				if (sqlite3_bind_int(q, 7, subitem->style))
					goto fail;
				if (sqlite3_bind_text(q, 8, subitem->code,
							strlen(subitem->code), SQLITE_STATIC))
					goto fail;
				if (sqlite3_bind_text(q, 9, subitem->code,
							strlen(subitem->code), SQLITE_STATIC))
					goto fail;
				if (sqlite3_step(q) != SQLITE_DONE) {
					sql_err(SQLARG(oh), "cannot add ordersubitem");
					goto fail;
				}
				sqlite3_reset(q);

				if (add_extras(&extras, odb_getmenuitem(subitem->code),
							subitem->qty) == -1)
					goto fail;
			}
		}
	}

	/*
	 * EXTRAS
	 */

#ifdef POST_EXTRAS
#error "POST_EXTRAS calculates, but does not POST yet"
#endif

	/*
	 * RULEITEMS
	 */

	q = SQLARG(oh)->q.odb.put_subitem;
	for (u_item = 0; u_item < order->ruleitems.len; u_item++) {
		item = &order->ruleitems.val[u_item];

		if (sqlite3_bind_int64(q, 1, order_ID))
			goto fail;
		if (sqlite3_bind_int(q, 2, 0))				/* Line */
			goto fail;
		if (sqlite3_bind_int(q, 3, ITEM_RULE))
			goto fail;
		if (sqlite3_bind_int(q, 4, item->qty))
			goto fail;
		if (sqlite3_bind_int(q, 5, 0))				/* ruleitems are 'free' */
			goto fail;
		if (sqlite3_bind_null(q, 6))				/* no special stored */
			goto fail;
		if (sqlite3_bind_null(q, 7))				/* no style stored */
			goto fail;
		if (sqlite3_bind_text(q, 8, item->code,
					strlen(item->code), SQLITE_STATIC))
			goto fail;
		if (sqlite3_step(q) != SQLITE_DONE) {
			sql_err(SQLARG(oh), "cannot add order ruleitem");
			goto fail;
		}
		sqlite3_reset(q);
	}

	/*
	 * PAYMENT (if CC)
	 */
	if (order->payment.type == PAYMENT_CREDIT) {
		q = SQLARG(oh)->q.odb.put_payment;

		if (sqlite3_bind_int64(q, 1, order_ID))
			goto fail;
		if (order->payment.ccinfo && order->payment.ccinfo->number) {
			if (sqlite3_bind_text(q, 2, order->payment.ccinfo->number,
						strlen(order->payment.ccinfo->number), SQLITE_STATIC))
				goto fail;
		} else
			if (sqlite3_bind_null(q, 2))
				goto fail;
		if (order->payment.ccinfo && order->payment.ccinfo->expiry) {
			if (sqlite3_bind_text(q, 3, order->payment.ccinfo->expiry,
						strlen(order->payment.ccinfo->expiry), SQLITE_STATIC))
				goto fail;
		} else
			if (sqlite3_bind_null(q, 3))
				goto fail;
		if (sqlite3_step(q) != SQLITE_DONE) {
			sql_err(SQLARG(oh), "cannot add order payment");
			goto fail;
		}
		sqlite3_reset(q);
	}

	/*
	 * OK, Post the entire ORDER + ITEMS + SUBITEMS + ... all in one hooah.
	 */
	do {
		if ((ret = sqlite3_exec(SQLARG(oh)->db, "COMMIT TRANSACTION;", NULL,
						NULL, NULL)) == SQLITE_BUSY) {
			sql_warn(SQLARG(oh), "SQL cannot commit order, retrying %d/%d",
					retry + 1, SQL_RETRY_MAX);
			usleep(MIN(retry * SQL_RETRY_SCALE, SQL_RETRY_MAXWAIT));
		}
	} while (ret == SQLITE_BUSY && ++retry < SQL_RETRY_MAX);
	if (ret) {
		sql_err(SQLARG(oh), "SQL cannot commit transaction");
		goto fail;
	}

	/*
	 * ORDER DATE
	 */
	if (sqlite3_prepare(SQLARG(oh)->db, SQL_ODB_GET_TIME,
				strlen(SQL_ODB_GET_TIME), &q, NULL)) {
		sql_err(SQLARG(oh), "OQ PREP GET TIME");
		goto fail;
	}
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, order_ID)) {
		sql_err(SQLARG(oh), "cannot bind order_ID to date query");
		sqlite3_finalize(q);
		q = NULL;
		goto fail;
	}
	if (sqlite3_step(q) == SQLITE_ROW) {
		if (strptime(sqlite3_column_text(q, 0), SQL_TIME_FMT, &tm) == NULL) {
			sql_err(SQLARG(oh), "bad order time format");
			sqlite3_finalize(q);
			q = NULL;
			goto fail;
		}
		*otime = timegm(&tm);
	} else
		sql_err(SQLARG(oh), "cannot get order date");
	sqlite3_finalize(q);
	q = NULL;

	*oid = order_ID;

	return 0;

fail:
	if (q)
		sqlite3_reset(q);
	/* Failing to unwind cannot be helped, something is seriously amiss */
	sql_exec(SQLARG(oh), "ROLLBACK TRANSACTION;", NULL, NULL);
	free_ChopstixItemExtras(&extras);
	return -1;
}

static int
odb_update(struct orderdb_handle *oh, const ChopstixOrder *order)
{
	return 0;
}

static int
odb_close(struct orderdb_handle *oh)
{
	if (SQLARG(oh) == NULL) {
		errno = EINVAL;
		return -1;
	}
	sql_freearg(SQLARG(oh));
	return 0;
}

static const char *
odb_geterr(struct orderdb_handle *oh)
{
	if (SQLARG(oh))
		return sql_geterr(SQLARG(oh));
	else
		return "";
}

static ChopstixMenuitem *
odb_getmenuitem(char *code)
{
	if (odb_getmenuitem_int)
		return odb_getmenuitem_int(code);

	return NULL;
}
