/* $Gateweaver: customers.c,v 1.3 2007/09/28 17:19:49 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "report.h"

struct cust {
	LIST_ENTRY(cust) entry;
	ChopstixCustomer info;
	int orders;
	int total;
};
LIST_HEAD(cust_list, cust);

static int
cust_cmp(struct cust *a, struct cust *b)
{
	if (a->info.phone.npa != b->info.phone.npa)
		return a->info.phone.npa < b->info.phone.npa ? -1 : 1;
	if (a->info.phone.nxx != b->info.phone.nxx)
		return a->info.phone.nxx < b->info.phone.nxx ? -1 : 1;
	if (a->info.phone.num != b->info.phone.num)
		return a->info.phone.num < b->info.phone.num ? -1 : 1;
	if (a->info.phone.ext == NULL && b->info.phone.ext)
		return -1;
	if (a->info.phone.ext && b->info.phone.ext == NULL)
		return 1;
	if (a->info.phone.ext && b->info.phone.ext &&
			*a->info.phone.ext != *b->info.phone.ext)
		return *a->info.phone.ext < *b->info.phone.ext ? -1 : 1;
	return 0;
}

/*
 * This is a mergesort that uses the LIST_*() macros
 */
static void
cust_sort(struct cust_list *list)
{
	struct cust *self, *tail, *e, *p, *r;
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
				} else if (cust_cmp(p, r) <= 0) {
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

static struct cust *
cust_get(struct cust_list *list, ChopstixCustomer *info)
{
	struct cust *cust, fake;
	fake.info = *info;
	LIST_FOREACH(cust, list, entry)
		if (cust_cmp(cust, &fake) == 0)
			return cust;

	return NULL;
}

static void
cust_add(struct cust_list *list, ChopstixCustomer *info, int total)
{
	struct cust *cust;
	if ((cust = cust_get(list, info)) == NULL) {
		if ((cust = calloc(1, sizeof(*cust))) == NULL)
			return;
		if (copy_ChopstixCustomer(info, &cust->info)) {
			free(cust);
			return;
		}
		LIST_INSERT_HEAD(list, cust, entry);
	}
	cust->orders++;
	cust->total += total;
}

void
cust_str2phone(const char *str, ChopstixPhone *phone)
{
	long long val;
	val = strtonum(str, 0, 19999999999LL, NULL);
	/* strip leading one */
	if (val >= 10000000000LL)
		val -= 10000000000LL;
	phone->num = val % 10000;
	val -= phone->num;
	phone->nxx = (val / 10000) % 1000;
	val -= phone->nxx * 10000;
	phone->npa = (val / 10000000);
}

static int
cust_getrange_cb(const ChopstixOrder *order, void *arg)
{
	struct cust_list *list = arg;
	ChopstixCustomer info;
	bzero(&info, sizeof(info));
	if (cdbf.get(&cdbh, &order->customer.phone, &info) == -1)
		return -1;
	cust_add(list, &info, order->total.total);
	free_ChopstixCustomer(&info);
	return 0;
}

void
render_customer_list(const char *m, void *arg)
{
	char fn[1024];
	struct cust_list list;
	struct cust *cust;
	struct tm tm_start, tm_end;
	time_t start, end;

	LIST_INIT(&list);

	bcopy(localtime(&time_start), &tm_start, sizeof(tm_start));
	start = mktime(&tm_start);

	bcopy(localtime(&time_stop), &tm_end, sizeof(tm_end));
	end = mktime(&tm_end);

	if (odbf.getrange(&odbh, start, end, cust_getrange_cb, &list) == -1) {
		render_error("cannot load orders, see system log for details");
		return;
	}
	cust_sort(&list);

	if (!strcmp(m, "CUSTLIST")) {
		LIST_FOREACH(cust, &list, entry) {
			if (cust->orders == 0)
				continue;
			snprintf(fn, sizeof(fn), "%s/cust_info.html", conf.htmldir);
			render_html(fn, render_customer_detail, cust);
		}
	}

	while ((cust = LIST_FIRST(&list))) {
		LIST_REMOVE(cust, entry);
		free_ChopstixCustomer(&cust->info);
		free(cust);
	}
}

void
render_customer_info(const char *m, void *arg)
{
	ChopstixCustomer *cust = (ChopstixCustomer *)arg;
	char fn[MAXPATHLEN];
	tbstring sa = {0};
	unsigned int u;
	int credit = 0;

	if (!strcmp(m, "CUSTID")) {
		printf("%03d%03d%04d", cust->phone.npa, cust->phone.nxx,
				cust->phone.num);
	} else if (!strcmp(m, "CUSTPHONE")) {
		printf("(%03d) %03d-%04d", cust->phone.npa, cust->phone.nxx,
				cust->phone.num);
		if (cust->phone.ext)
			printf(" x%d", *cust->phone.ext);
	} else if (!strcmp(m, "CUSTNAME")) {
		if (cust->name)
			printf("%s", html_esc(cust->name, &sa, 0));
	} else if (!strcmp(m, "CUSTADDR")) {
		if (cust->addr.addr)
			printf("%s", html_esc(cust->addr.addr, &sa, 0));
		if (cust->addr.apt && *cust->addr.apt) {
			tbstrfree(&sa);
			printf(" APT %s", html_esc(cust->addr.apt, &sa, 0));
		}
		if (cust->addr.entry && *cust->addr.entry) {
			tbstrfree(&sa);
			printf(" ENTRY %s", html_esc(cust->addr.entry, &sa, 0));
		}
	} else if (!strcmp(m, "CUSTISECT")) {
		if (cust->isect.cross)
			printf("%s", html_esc(cust->isect.cross, &sa, 0));
	} else if (!strcmp(m, "CUSTREPS"))
		printf("%d", cust->reps);
	else if (!strcmp(m, "CUSTSPECIAL")) {
		if (cust->special)
			printf("%s", html_esc(*cust->special, &sa, 0));
	} else if (!strcmp(m, "CUSTCREDITS"))
		printf("%d", cust->credit ? cust->credit->len : 0);
	else if (!strcmp(m, "CUSTCREDTOTAL")) {
		if (cust->credit)
			for (u = 0; u < cust->credit->len; u++)
				credit += cust->credit->val[u].credit;
		PRINTF_MONEY(credit);
	} else if (!strcmp(m, "CUSTCREDREMAIN")) {
		if (cust->credit)
			for (u = 0; u < cust->credit->len; u++)
				credit += cust->credit->val[u].remain;
		PRINTF_MONEY(credit);
	} else if (!strcmp(m, "CUSTCREDITINFO")) {
		if (cust->credit) {
			snprintf(fn, sizeof(fn), "%s/cust_credit.html", conf.htmldir);
			for (u = 0; u < cust->credit->len; u++)
				render_html(fn, render_customer_credit, &cust->credit->val[u]);
		}
	}

	tbstrfree(&sa);
}

void
render_customer_credit(const char *m, void *arg)
{
	ChopstixCredit *cred = (ChopstixCredit *)arg;
	tbstring sa = {0};

	if (!strcmp(m, "CUSTCREDIT"))
		PRINTF_MONEY(cred->credit);
	else if (!strcmp(m, "CUSTCREDITREMAIN"))
		PRINTF_MONEY(cred->remain);
	else if (!strcmp(m, "CUSTCREDITREASON"))
		printf("%s", html_esc(cred->reason, &sa, 0));

	tbstrfree(&sa);
}

void
render_customer_detail(const char *m, void *arg)
{
	struct cust *cust = (struct cust *)arg;

	if (!strcmp(m, "CUSTORDERS"))
		printf("%d", cust->orders);
	else if (!strcmp(m, "CUSTTOTAL"))
		PRINTF_MONEY(cust->total);
	else {
		/* let the info be rendered itself */
		render_customer_info(m, &cust->info);
	}
}
