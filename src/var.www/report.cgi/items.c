/* $Gateweaver: items.c,v 1.3 2007/09/28 17:19:49 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "report.h"

struct item {
	LIST_ENTRY(item) entry;
	char *code;
	int cnt;
	int rulecnt;
	int subcnt;
	int subdelta;
};
LIST_HEAD(item_list, item);

static int
item_cmp(struct item *a, struct item *b)
{
	long long A, B;
	const char *errstr;

	/* compare as integers if possible for both */
	A = strtonum(a->code, 0, LLONG_MAX, &errstr);
	if (errstr == NULL) {
		B = strtonum(b->code, 0, LLONG_MAX, &errstr);
		if (errstr == NULL) {
			if (A != B)
				return A < B ? -1 : 1;
			return 0;
		}
	}

	return strcmp(a->code, b->code);
}

/*
 * This is a mergesort that uses the LIST_*() macros
 */
static void
item_sort(struct item_list *list)
{
	struct item *self, *tail, *e, *p, *r;
	int insize = 1;
	int nmerges, psize, rsize, i;

	self = LIST_FIRST(list);
	if (self == NULL)
		return;

	for (;;) {
		p = self;
		self = NULL;
		tail = NULL;
		nmerges = 0;

		while (p) {
			nmerges++;
			r = p;

			for (psize = 0, i = 0; i < insize && r; psize++, i++)
				r = LIST_NEXT(r, entry);

			rsize = insize;
			while (psize > 0 || (rsize > 0 && r)) {
				if (psize == 0) {
					e = r;
					r = LIST_NEXT(r, entry);
					rsize--;
				} else if (rsize == 0 || r == NULL) {
					e = p;
					p = LIST_NEXT(p, entry);
					psize--;
				} else if (item_cmp(p, r) <= 0) {
					e = p;
					p = LIST_NEXT(p, entry);
					psize--;
				} else {
					e = r;
					r = LIST_NEXT(r, entry);
					rsize--;
				}

				if (tail) {
					LIST_REMOVE(e, entry);
					LIST_INSERT_AFTER(tail, e, entry);
				} else
					self = e;

				tail = e;
			}
			p = r;
		}
		if (nmerges <= 1)
			return;
		insize *= 2;
	}
}

static struct item *
item_get(struct item_list *list, const char *code)
{
	struct item *item, fake;
	(const char *)fake.code = code;
	LIST_FOREACH(item, list, entry)
		if (item_cmp(item, &fake) == 0)
			return item;

	return NULL;
}

static void
item_add(struct item_list *list, const char *code, int count)
{
	struct item *item;
	if ((item = item_get(list, code)) == NULL) {
		if ((item = calloc(1, sizeof(*item))) == NULL)
			return;
		if ((item->code = strdup(code)) == NULL) {
			free(item);
			return;
		}
		LIST_INSERT_HEAD(list, item, entry);
	}

	item->cnt += count;
}

static void
item_add_sub(struct item_list *list, const char *code, int count, int delta)
{
	struct item *item;
	if ((item = item_get(list, code)) == NULL) {
		if ((item = calloc(1, sizeof(*item))) == NULL)
			return;
		if ((item->code = strdup(code)) == NULL) {
			free(item);
			return;
		}
		LIST_INSERT_HEAD(list, item, entry);
	}

	item->subcnt += count;
	item->subdelta += delta;
}

static void
item_add_rule(struct item_list *list, const char *code, int count)
{
	struct item *item;
	if ((item = item_get(list, code)) == NULL) {
		if ((item = calloc(1, sizeof(*item))) == NULL)
			return;
		if ((item->code = strdup(code)) == NULL) {
			free(item);
			return;
		}
		LIST_INSERT_HEAD(list, item, entry);
	}

	item->rulecnt += count;
}

static int
item_getrange_cb(const ChopstixOrder *order, void *arg)
{
	struct item_list *list = arg;
	unsigned int u, s;

	for (u = 0; u < order->items.len; u++) {
		item_add(list, order->items.val[u].code, order->items.val[u].qty);
		if (order->items.val[u].subitems)
			for (s = 0; s < order->items.val[u].subitems->len; s++)
				item_add_sub(list,
						order->items.val[u].subitems->val[s].code,
						order->items.val[u].subitems->val[s].qty,
						order->items.val[u].subitems->val[s].pricedelta);
	}
	for (u = 0; u < order->ruleitems.len; u++) {
		item_add_rule(list, order->ruleitems.val[u].code,
				order->ruleitems.val[u].qty);
		if (order->ruleitems.val[u].subitems)
			for (s = 0; s < order->ruleitems.val[u].subitems->len; s++)
				item_add_sub(list,
						order->ruleitems.val[u].subitems->val[s].code,
						order->ruleitems.val[u].subitems->val[s].qty,
						order->ruleitems.val[u].subitems->val[s].pricedelta);
	}
	return 0;
}

void *
item_load(void)
{
	struct item_list *list;
	struct tm tm_start, tm_end;
	time_t start, end;

	if ((list = calloc(1, sizeof(*list))) == NULL)
		return NULL;

	bcopy(localtime(&time_start), &tm_start, sizeof(tm_start));
	start = mktime(&tm_start);

	bcopy(localtime(&time_stop), &tm_end, sizeof(tm_end));
	end = mktime(&tm_end);

	if (odbf.getrange(&odbh, start, end, item_getrange_cb, list) == -1) {
		render_error("cannot load orders, see system log for details");
		item_free(list);
		return NULL;
	}
	item_sort(list);

	return list;
}

void
item_free(void *arg)
{
	struct item_list *list = arg;
	struct item *item;
	if (list == NULL)
		return;
	while ((item = LIST_FIRST(list))) {
		LIST_REMOVE(item, entry);
		free(item->code);
		free(item);
	}
	free(list);
}

void
render_items(const char *m, void *arg)
{
	char fn[1024];
	struct item_list *list = arg;
	struct item *item;
	if (list == NULL)
		return;

	if (!strcmp(m, "ITEMLIST")) {
		LIST_FOREACH(item, list, entry) {
			if (item->cnt == 0)
				continue;
			snprintf(fn, sizeof(fn), "%s/item_info.html", conf.htmldir);
			render_html(fn, render_item_list, item);
		}
	} else if (!strcmp(m, "ITEMRULELIST")) {
		LIST_FOREACH(item, list, entry) {
			if (item->rulecnt == 0)
				continue;
			snprintf(fn, sizeof(fn), "%s/item_inforule.html", conf.htmldir);
			render_html(fn, render_item_list, item);
		}
	} else if (!strcmp(m, "ITEMSUBLIST")) {
		LIST_FOREACH(item, list, entry) {
			if (item->subcnt == 0)
				continue;
			snprintf(fn, sizeof(fn), "%s/item_infosub.html", conf.htmldir);
			render_html(fn, render_item_list, item);
		}
	}
}

void
render_item_list(const char *m, void *arg)
{
	struct item *item = (struct item *)arg;
	ChopstixMenuitem *mi;

	mi = menu_getitem(item->code);

	if (!strcmp(m, "ITEMQTY"))
		printf("%d", item->cnt);
	else if (!strcmp(m, "ITEMSUBQTY"))
		printf("%d", item->subcnt);
	else if (!strcmp(m, "ITEMRULEQTY"))
		printf("%d", item->rulecnt);
	else if (!strcmp(m, "ITEMCODE"))
		printf("%s", item->code);
	else if (!strcmp(m, "ITEMDESC")) {
		printf("%s", mi ? mi->name : "UNKNOWN");
	} else if (!strcmp(m, "ITEMPRICE"))
		PRINTF_MONEY(mi ? mi->price : 0);
	else if (!strcmp(m, "ITEMSUBTOTAL"))
		PRINTF_MONEY(mi ? mi->price * item->cnt : 0);
	else if (!strcmp(m, "ITEMRULETOTAL"))
		PRINTF_MONEY(mi ? mi->price * item->rulecnt : 0);
	else if (!strcmp(m, "ITEMDELTA"))
		PRINTF_MONEY(item->subdelta);
	else if (!strcmp(m, "ITEMTOTAL"))
		PRINTF_MONEY(mi ? mi->price * item->cnt + item->subdelta : 0);
	else
		printf("render_order_item: unknown macro '%s'<br>\n", m);
}
