/* $Gateweaver: customer.c,v 1.13 2007/09/29 03:24:08 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix_sql.h"

RCSID("$Gateweaver: customer.c,v 1.13 2007/09/29 03:24:08 cmaxwell Exp $");

static int cdb_open(struct custdb_handle *);
static int cdb_create(struct custdb_handle *);
#if 0
static int cdb_read(struct custdb_handle *, ChopstixCustomer *);
static int cdb_readkey(struct custdb_handle *, ChopstixPhone *);
static int cdb_rewind(struct custdb_handle *);
#endif
static int cdb_get(struct custdb_handle *, const ChopstixPhone *,
		ChopstixCustomer *);
static int cdb_put(struct custdb_handle *, const ChopstixCustomer *);
static int cdb_update(struct custdb_handle *, const ChopstixCustomer *);
static int cdb_update_phone(struct custdb_handle *, const ChopstixPhone *,
		const ChopstixPhone *);
#if 0
static int cdb_remove(struct custdb_handle *, const ChopstixPhone *);
#endif
static int cdb_close(struct custdb_handle *);
static const char * cdb_geterr(struct custdb_handle *);

char * phone2str(const ChopstixPhone *);
static char * phoneext2str(const ChopstixPhone *);

static void (*cdb_puterr)(const char *, va_list);

void
cdb_init(struct custdb_functions *cdbf)
{
	cdbf->open = &cdb_open;
	cdbf->create = &cdb_create;
#if 0
	cdbf->read = &cdb_read;
	cdbf->readkey = &cdb_readkey;
	cdbf->rewind = &cdb_rewind;
#endif
	cdbf->get = &cdb_get;
	cdbf->put = &cdb_put;
	cdbf->update = &cdb_update;
	cdbf->update_phone = &cdb_update_phone;
#if 0
	cdbf->remove = &cdb_remove;
#endif
	cdbf->close = &cdb_close;
	cdbf->geterr = &cdb_geterr;

	/* caller set error function */
	cdb_puterr = cdbf->err;
}

static int
cdb_open(struct custdb_handle *ch)
{
	if (SQLARG(ch) != NULL) {
		errno = EBADF;
		return -1;
	}

	if ((SQLARG(ch) = sql_makearg(SQL_CDB)) == NULL)
		return -1;
	SQLARG(ch)->err = cdb_puterr;

	if (sql_open(SQLARG(ch), ch->dbfile) == -1) {
		sql_freearg(SQLARG(ch));
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
cdb_create(struct custdb_handle *ch)
{
	errno = EACCES;
	return -1;
}

static int
cdb_get(struct custdb_handle *ch, const ChopstixPhone *phone,
		ChopstixCustomer *cust)
{
	char *phonestr;
	int64_t cust_ID = 0;
	sqlite3_stmt *q = NULL;
	ChopstixCredit *cc;
	int dupcount = 0;

	CHECKSQL(ch);

	/* get phonestr before zeroing, in case user sends same structure in */
	if ((phonestr = phone2str(phone)) == NULL)
		goto fail;

	bzero(cust, sizeof(*cust));

	/*
	 * CUSTOMER
	 */

	q = SQLARG(ch)->q.cdb.get_cust;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, phonestr, strlen(phonestr), SQLITE_STATIC)) {
		sql_err(SQLARG(ch), "cannot bind phone number to query");
		goto fail;
	}
	while (sqlite3_step(q) == SQLITE_ROW) {
		dupcount++;

		/* start by freeing, incase multiple customers match the same phone */
		free_ChopstixCustomer(cust);

		/* ID, Phone, PhoneExt, Address, Intersection, Special */
		cust_ID = sqlite3_column_int64(q, 0);
		cust->name = sql_strdup(sqlite3_column_text(q, 1));
		str2phone(sqlite3_column_text(q, 2), &cust->phone);
		str2phoneext(sqlite3_column_text(q, 3), &cust->phone);
		cust->addr.addr = sql_strdup(sqlite3_column_text(q, 4));
		cust->addr.apt = sql_strdup(sqlite3_column_text(q, 5));
		cust->addr.entry = sql_strdup(sqlite3_column_text(q, 6));
		cust->isect.cross = sql_strdup(sqlite3_column_text(q, 7));
		if ((cust->special = calloc(1, sizeof(cust->special))))
			*cust->special = sql_strdup(sqlite3_column_text(q, 8));
		cust->reps = sqlite3_column_int(q, 9);
	}
	sqlite3_reset(q);

	if (dupcount > 1)
		sql_warn(SQLARG(ch), "%d duplicate entries", dupcount);

	if (cust_ID == 0) {
		errno = ENOENT;
		return -1;
	}

	/*
	 * CREDITS
	 */
	q = SQLARG(ch)->q.cdb.get_cred;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, cust_ID)) {
		sql_err(SQLARG(ch), "cannot bind customer ID to query");
		goto fail;
	}
	if ((cust->credit = calloc(1, sizeof(*cust->credit))) == NULL)
		goto fail;
	cust->credit->len = 0;
	while (sqlite3_step(q) == SQLITE_ROW) {
		/* increase buffer */
		if ((cc = realloc(cust->credit->val, (cust->credit->len + 1) *
						sizeof(ChopstixCredit))) == NULL)
			goto fail;
		cust->credit->len++;
		cust->credit->val = cc;

		/* Credit, Remain, Reason */
		cust->credit->val[cust->credit->len - 1].credit
			= sqlite3_column_int(q, 0);
		cust->credit->val[cust->credit->len - 1].remain
			= sqlite3_column_int(q, 1);
		cust->credit->val[cust->credit->len - 1].reason
			= sql_strdup(sqlite3_column_text(q, 2));
	}
	sqlite3_reset(SQLARG(ch)->q.cdb.get_cred);

	return 0;

fail:
	free_ChopstixCustomer(cust);
	if (q)
		sqlite3_reset(q);
	return -1;
}

static int
cdb_put(struct custdb_handle *ch, const ChopstixCustomer *cust)
{
	char *phonestr, *phoneext, *special;
	sqlite3_stmt *q = NULL;

	CHECKSQL(ch);

	if ((phonestr = phone2str(&cust->phone)) == NULL)
		goto fail;
	if ((phoneext = phoneext2str(&cust->phone)) == NULL)
		goto fail;

	/*
	 * CUSTOMER
	 */

	q = SQLARG(ch)->q.cdb.put_cust;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, phonestr, strlen(phonestr), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 2, phoneext, strlen(phoneext), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, cust->name, strlen(cust->name),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 4, cust->addr.addr, strlen(cust->addr.addr),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 5, cust->addr.apt, strlen(cust->addr.apt),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 6, cust->addr.entry, strlen(cust->addr.entry),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 7, cust->isect.cross, strlen(cust->isect.cross),
				SQLITE_STATIC))
		goto fail;
	if (cust->special)
		special = *cust->special;
	else
		special = "";
	if (sqlite3_bind_text(q, 8, special, strlen(special), SQLITE_STATIC))
		goto fail;

	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(ch), "cannot add customer");
		goto fail;
	}
	sqlite3_reset(q);

	return 0;

fail:
	sql_err(SQLARG(ch), "cannot bind data to query");
	if (q)
		sqlite3_reset(q);
	return -1;
}

static int
cdb_update(struct custdb_handle *ch, const ChopstixCustomer *cust)
{
	char *phonestr, *phoneext, *special;
	sqlite3_stmt *q = NULL, *u = NULL;
	size_t cpos = 0;

	CHECKSQL(ch);

	if ((phonestr = phone2str(&cust->phone)) == NULL)
		goto fail;
	if ((phoneext = phoneext2str(&cust->phone)) == NULL)
		goto fail;

	/*
	 * CUSTOMER
	 */

	q = SQLARG(ch)->q.cdb.upd_cust;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, cust->name, strlen(cust->name),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 2, phoneext, strlen(phoneext), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, cust->addr.addr, strlen(cust->addr.addr),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 4, cust->addr.apt, strlen(cust->addr.apt),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 5, cust->addr.entry, strlen(cust->addr.entry),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 6, cust->isect.cross, strlen(cust->isect.cross),
				SQLITE_STATIC))
		goto fail;
	if (cust->special)
		special = *cust->special;
	else
		special = "";
	if (sqlite3_bind_text(q, 7, special, strlen(special), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 8, phonestr, strlen(phonestr), SQLITE_STATIC))
		goto fail;

	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(ch), "cannot update customer");
		goto fail;
	}
	sqlite3_reset(q);

	/* no need to do this if no credits are attached */
	if (cust->credit == NULL || cust->credit->len == 0)
		return 0;

	/*
	 * CREDITS
	 *
	 * Have to SELECT, compare against the passed array and then UPDATE any
	 * that have changed.
	 */

	q = SQLARG(ch)->q.cdb.get_cred_phone;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, phonestr, strlen(phonestr), SQLITE_STATIC)) {
		sql_err(SQLARG(ch), "cannot bind phone number to query");
		goto fail;
	}
	while (sqlite3_step(q) == SQLITE_ROW) {
		/* limit credit positional */
		if (cpos >= cust->credit->len)
			break;

		/* ensure we are still in order */
		if (cust->credit->val[cpos].reason == NULL ||
				sqlite3_column_int(q, 1) != cust->credit->val[cpos].credit) {
			sql_err(SQLARG(ch), "cannot update customer credit");
			break;
		}

		/* if nothing needs to change, continue */
		if (cust->credit->val[cpos].remain == sqlite3_column_int(q, 2) &&
				strcasecmp(cust->credit->val[cpos].reason,
					sqlite3_column_text(q, 3)) == 0) {
			cpos++;
			continue;
		}

		u = SQLARG(ch)->q.cdb.upd_cred;
		/* 1 = first parameter in prepared query */
		if (sqlite3_bind_int(u, 1, cust->credit->val[cpos].remain))
			goto fail;
		if (sqlite3_bind_text(u, 2, cust->credit->val[cpos].reason,
					strlen(cust->credit->val[cpos].reason), SQLITE_STATIC))
			goto fail;
		if (sqlite3_bind_int(u, 3, sqlite3_column_int(q, 0)))
			goto fail;

		if (sqlite3_step(u) != SQLITE_DONE) {
			sql_err(SQLARG(ch), "cannot update credit %zd", cpos + 1);
			goto fail;
		}
		sqlite3_reset(u);

		cpos++;
	}
	sqlite3_reset(q);

	for (; cpos < cust->credit->len; cpos++) {
		u = SQLARG(ch)->q.cdb.put_cred;
		/* 1 = first parameter in prepared query */
		if (sqlite3_bind_text(u, 4, phonestr, strlen(phonestr), SQLITE_STATIC))
			goto fail;
		if (sqlite3_bind_int(u, 1, cust->credit->val[cpos].credit))
			goto fail;
		if (sqlite3_bind_int(u, 2, cust->credit->val[cpos].remain))
			goto fail;
		if (sqlite3_bind_text(u, 3, cust->credit->val[cpos].reason,
					strlen(cust->credit->val[cpos].reason), SQLITE_STATIC))
			goto fail;

		if (sqlite3_step(u) != SQLITE_DONE) {
			sql_err(SQLARG(ch), "cannot insert credit %zd", cpos + 1);
			goto fail;
		}
		sqlite3_reset(u);
	}

	return 0;

fail:
	sql_err(SQLARG(ch), "cannot bind data to query");
	if (u)
		sqlite3_reset(u);
	if (q)
		sqlite3_reset(q);
	return -1;
}

static int
cdb_update_phone(struct custdb_handle *ch, const ChopstixPhone *old,
		const ChopstixPhone *new)
{
	char *phonestr_old, *phonestr_new;
	sqlite3_stmt *q = NULL;

	CHECKSQL(ch);

	if ((phonestr_old = strdup(phone2str(old))) == NULL)
		goto fail;
	if ((phonestr_new = phone2str(new)) == NULL)
		goto fail;

	if (sqlite3_prepare(SQLARG(ch)->db, SQL_CDB_UPD_CUSTPHONE,
				strlen(SQL_CDB_UPD_CUSTPHONE), &q, NULL)) {
		sql_err(SQLARG(ch), "CQ PREP UPD CUST PHONE");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, phonestr_new, strlen(phonestr_new),
				SQLITE_STATIC)
			|| sqlite3_bind_text(q, 2, phonestr_old, strlen(phonestr_old),
				SQLITE_STATIC)) {
		sql_err(SQLARG(ch), "cannot bind phone number to query");
		goto fail;
	}
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(ch), "cannot update customer phone number");
		goto fail;
	}
	sqlite3_finalize(q);
	free(phonestr_old);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	if (phonestr_old)
		free(phonestr_old);
	return -1;
}

static int
cdb_close(struct custdb_handle *ch)
{
	if (SQLARG(ch) == NULL) {
		errno = EINVAL;
		return -1;
	}
	sql_freearg(SQLARG(ch));

	return 0;
}

static const char *
cdb_geterr(struct custdb_handle *ch)
{
	if (SQLARG(ch))
		return sql_geterr(SQLARG(ch));
	else
		return "";
}

/*
 * Convert ChopstixPhone components to text digits
 */
char *
phone2str(const ChopstixPhone *phone)
{
	static char buf[sizeof("NPANXXNMBR")];
	int len;

	if (phone == NULL) {
		strlcpy(buf, "0000000000", sizeof(buf));
		return buf;
	}

	if ((len = snprintf(buf, sizeof(buf), "%03d%03d%04d",
				phone->npa, phone->nxx, phone->num)) == -1)
		return NULL;

	return buf;
}

static char *
phoneext2str(const ChopstixPhone *phone)
{
	static char buf[sizeof("EXTN5")];

	/* extension is optional */
	if (phone == NULL || phone->ext == NULL) {
		strlcpy(buf, "", sizeof(buf));
		return buf;
	}

	if (snprintf(buf, sizeof(buf), "%d", *phone->ext) == -1)
		return NULL;

	return buf;
}

/*
 * Stolen from the input routines in chopstix itself.
 *
 * mode 0 is phone
 * mode 1 is extension
 */
static void
str2phone_int(const char *str, ChopstixPhone *phone, int mode)
{
	enum { PHONE_NPA, PHONE_NXX, PHONE_NUMB, PHONE_X, PHONE_EXT } state
		= PHONE_NPA;
	int bit = 2;

	if (str == NULL || *str == '\0')
		return;

	/* do not wipe except on phone */
	if (mode == 1) {
		state = PHONE_EXT;
		if (phone->ext == NULL)
			if ((phone->ext = calloc(1, sizeof(*phone->ext))) == NULL)
				return;
	} else {
		free_ChopstixPhone(phone);
		bzero(phone, sizeof(*phone));
	}

#define BITMULT(b) \
	((b) == 4 ? 10000 : (b) == 3 ? 1000 : (b) == 2 ? 100 : (b) == 1 ? 10 : 1)

	while (*str) {
		if (!(isdigit(*str) || *str == 'X' || *str == 'x' || *str == ' '))
			return;
		
		/* database might be a bit corrupted */
		if (*str == ' ' || (tolower(*str) == 'x' && state != PHONE_X)) {
			*str++;
			continue;
		}

		switch (state) {
			case PHONE_NPA:
				phone->npa += (*str - '0') * BITMULT(bit);
				if (bit-- == 0) {
					state = PHONE_NXX;
					bit = 2;
				}
				break;

			case PHONE_NXX:
				phone->nxx += (*str - '0') * BITMULT(bit);
				if (bit-- == 0) {
					state = PHONE_NUMB;
					bit = 3;
				}
				break;

			case PHONE_NUMB:
				phone->num += (*str - '0') * BITMULT(bit);
				if (bit-- == 0) {
					state = PHONE_X;
					bit = 4;
				}
				break;

			case PHONE_X:
				/* it takes an 'X' to get to here */
				if (*str != 'x' && *str != 'X')
					break;
				if (phone->ext == NULL)
					if ((phone->ext = calloc(1, sizeof(*phone->ext))) == NULL)
						return;
				state = PHONE_EXT;
				break;

			case PHONE_EXT:
				*phone->ext *= 10;
				*phone->ext += (*str - '0');
				if (bit-- == 0)
					return;
				break;
		}
		*str++;
	}
}

void
str2phone(const char *str, ChopstixPhone *phone)
{
	if (str == NULL)
		return;
	str2phone_int(str, phone, 0);
}

void
str2phoneext(const char *str, ChopstixPhone *phone)
{
	if (str == NULL)
		return;
	str2phone_int(str, phone, 1);
}
