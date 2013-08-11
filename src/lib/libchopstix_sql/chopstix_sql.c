/* $Gateweaver: chopstix_sql.c,v 1.11 2007/09/25 19:35:28 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix_sql.h"

RCSID("$Gateweaver: chopstix_sql.c,v 1.11 2007/09/25 19:35:28 cmaxwell Exp $");

struct sql_arg *
sql_makearg(enum sql_type type)
{
	struct sql_arg *arg;

	if ((arg = calloc(1, sizeof(struct sql_arg))) == NULL)
		return NULL;

	arg->dbtype = type;

	return arg;
}

/*
 * Log an internal error, including the library error
 */
void
sql_err(struct sql_arg *arg, const char *emsg, ...)
{
	char *nfmt;
	va_list ap;

	if (arg->errmsg) {
		free(arg->errmsg);
		arg->errmsg = NULL;
	}

	if (emsg == NULL)
		asprintf(&arg->errmsg, "%s", strerror(errno));
	else {
		if (asprintf(&nfmt, "%s: %s", emsg, sqlite3_errmsg(arg->db)) == -1) {
			/* oh well, we tried and something borked */
			(const char *)nfmt = emsg;
		}
		va_start(ap, emsg);
		vasprintf(&arg->errmsg, nfmt, ap);
		/* and to the error handler, if available */
		if (arg->err)
			arg->err(nfmt, ap);
		va_end(ap);
		if (nfmt != emsg)
			free(nfmt);
	}
}

/*
 * Log an internal warning, but not the library error
 */
void
sql_warn(struct sql_arg *arg, const char *emsg, ...)
{
	va_list ap;

	if (arg->errmsg) {
		free(arg->errmsg);
		arg->errmsg = NULL;
	}

	if (emsg == NULL)
		asprintf(&arg->errmsg, "%s", strerror(errno));
	else {
		va_start(ap, emsg);
		vasprintf(&arg->errmsg, emsg, ap);
		va_end(ap);
	}
}

int
sql_open(struct sql_arg *arg, const char *database)
{
	if (sqlite3_open(database, &arg->db)) {
		sql_err(arg, "Cannot open database \"%s\"", database);
		sqlite3_close(arg->db);
		return -1;
	}

#ifdef SQL_TRACE
	sqlite3_trace(arg->db, &sql_trace, arg);
#endif

	/*
	 * Prepare all SQL queries ahead of time
	 */
	switch (arg->dbtype) {

		case SQL_CDB:
			if (sqlite3_prepare(arg->db, SQL_CDB_GET_CUST,
						strlen(SQL_CDB_GET_CUST), &arg->q.cdb.get_cust, NULL)) {
				sql_err(arg, "CQ PREP GET CUST");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_CDB_PUT_CUST,
						strlen(SQL_CDB_PUT_CUST), &arg->q.cdb.put_cust, NULL)) {
				sql_err(arg, "CQ PREP PUT CUST");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_CDB_UPD_CUST,
						strlen(SQL_CDB_UPD_CUST), &arg->q.cdb.upd_cust, NULL)) {
				sql_err(arg, "CQ PREP UPD CUST");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_CDB_GET_CRED,
						strlen(SQL_CDB_GET_CRED), &arg->q.cdb.get_cred, NULL)) {
				sql_err(arg, "CQ PREP GET CRED");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_CDB_GET_CRED_PHONE,
						strlen(SQL_CDB_GET_CRED_PHONE), &arg->q.cdb.get_cred_phone, NULL)) {
				sql_err(arg, "CQ PREP GET CRED PHONE");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_CDB_UPD_CRED,
						strlen(SQL_CDB_UPD_CRED), &arg->q.cdb.upd_cred, NULL)) {
				sql_err(arg, "CQ PREP UPD CRED");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_CDB_PUT_CRED,
						strlen(SQL_CDB_PUT_CRED), &arg->q.cdb.put_cred, NULL)) {
				sql_err(arg, "CQ PREP PUT CRED");
				goto fail;
			}
			break;

		case SQL_MDB:
			if (sqlite3_prepare(arg->db, SQL_MDB_GET_MENUITEM,
						strlen(SQL_MDB_GET_MENUITEM), &arg->q.mdb.get_menuitem,
						NULL)) {
				sql_err(arg, "MQ PREP GET MENUITEM");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_MDB_GET_STYLES,
						strlen(SQL_MDB_GET_STYLES), &arg->q.mdb.get_menustyles,
						NULL)) {
				sql_err(arg, "MQ PREP GET MENUSTYLE");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_MDB_GET_EXTRAS,
						strlen(SQL_MDB_GET_EXTRAS), &arg->q.mdb.get_menuextras,
						NULL)) {
				sql_err(arg, "MQ PREP GET MENUEXTRA");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_MDB_GET_SUBITEMS,
						strlen(SQL_MDB_GET_SUBITEMS),
						&arg->q.mdb.get_menusubitems, NULL)) {
				sql_err(arg, "MQ PREP GET MENUSUBITEMS");
				goto fail;
			}
			break;

		case SQL_ODB:
			if (sqlite3_prepare(arg->db, SQL_ODB_GET_ORDER,
						strlen(SQL_ODB_GET_ORDER), &arg->q.odb.get_order,
						NULL)) {
				sql_err(arg, "OQ PREP GET ORDER");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_ODB_GET_LASTORDER,
						strlen(SQL_ODB_GET_LASTORDER),
						&arg->q.odb.get_lastorder, NULL)) {
				sql_err(arg, "OQ PREP GET LASTORDER");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_ODB_GET_ITEMS,
						strlen(SQL_ODB_GET_ITEMS), &arg->q.odb.get_items,
						NULL)) {
				sql_err(arg, "OQ PREP GET ORDER");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_ODB_PUT_ORDER,
						strlen(SQL_ODB_PUT_ORDER), &arg->q.odb.put_order,
						NULL)) {
				sql_err(arg, "OQ PREP PUT ORDER");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_ODB_PUT_ITEM,
						strlen(SQL_ODB_PUT_ITEM), &arg->q.odb.put_item,
						NULL)) {
				sql_err(arg, "OQ PREP PUT ORDER");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_ODB_PUT_SUBITEM,
						strlen(SQL_ODB_PUT_SUBITEM), &arg->q.odb.put_subitem,
						NULL)) {
				sql_err(arg, "OQ PREP PUT ORDER");
				goto fail;
			}
			if (sqlite3_prepare(arg->db, SQL_ODB_PUT_PAYMENT,
						strlen(SQL_ODB_PUT_PAYMENT), &arg->q.odb.put_payment,
						NULL)) {
				sql_err(arg, "OQ PREP PUT PAYMENT");
				goto fail;
			}
			break;
	}

	return 0;

fail:
	switch (arg->dbtype) {

		case SQL_CDB:
			if (arg->q.cdb.get_cust)
				sqlite3_finalize(arg->q.cdb.get_cust);
			if (arg->q.cdb.put_cust)
				sqlite3_finalize(arg->q.cdb.put_cust);
			if (arg->q.cdb.upd_cust)
				sqlite3_finalize(arg->q.cdb.upd_cust);
			if (arg->q.cdb.get_cred)
				sqlite3_finalize(arg->q.cdb.get_cred);
			if (arg->q.cdb.get_cred_phone)
				sqlite3_finalize(arg->q.cdb.get_cred_phone);
			if (arg->q.cdb.upd_cred)
				sqlite3_finalize(arg->q.cdb.upd_cred);
			if (arg->q.cdb.put_cred)
				sqlite3_finalize(arg->q.cdb.put_cred);
			break;

		case SQL_MDB:
			if (arg->q.mdb.get_menuitem)
				sqlite3_finalize(arg->q.mdb.get_menuitem);
			if (arg->q.mdb.get_menustyles)
				sqlite3_finalize(arg->q.mdb.get_menustyles);
			if (arg->q.mdb.get_menuextras)
				sqlite3_finalize(arg->q.mdb.get_menuextras);
			if (arg->q.mdb.get_menusubitems)
				sqlite3_finalize(arg->q.mdb.get_menusubitems);
			break;

		case SQL_ODB:
			if (arg->q.odb.get_order)
				sqlite3_finalize(arg->q.odb.get_order);
			if (arg->q.odb.get_lastorder)
				sqlite3_finalize(arg->q.odb.get_lastorder);
			if (arg->q.odb.get_items)
				sqlite3_finalize(arg->q.odb.get_items);
			if (arg->q.odb.put_order)
				sqlite3_finalize(arg->q.odb.put_order);
			if (arg->q.odb.put_item)
				sqlite3_finalize(arg->q.odb.put_item);
			if (arg->q.odb.put_subitem)
				sqlite3_finalize(arg->q.odb.put_subitem);
			if (arg->q.odb.put_payment)
				sqlite3_finalize(arg->q.odb.put_payment);
			break;
	}

	return -1;
}

/*
 * Hide some of the details from the individual database routine.  May also
 * allow easier porting to another SQL DB later.
 */
int
sql_exec(struct sql_arg *arg, const char *query, 
		int (*cb)(void *, int, char **, char **), void *cbarg)
{
	if (sqlite3_exec(arg->db, query, cb, cbarg, NULL)) {
		sql_err(arg, "SQL exec \"%s\"", query);
		return -1;
	}

	return 0;
}

/*
 * Map to malloc'ing printf that supports '%q' to autoescape SQL statements.
 * Unfortunately, the '%q' addition precludes the use of compiler arg checks.
 */
int
sql_printf(char **q, const char *query, ...)
{
	va_list ap;

	va_start(ap, query);
	*q = sqlite3_vmprintf(query, ap);
	va_end(ap);

	if (*q == NULL)
		return -1;

	return 0;
}

char *
sql_strdup(const char *str)
{
	if (str == NULL)
		return strdup("");
	else
		return strdup(str);
}

const char *
sql_geterr(struct sql_arg *arg)
{
	if (arg->errmsg)
		return arg->errmsg;
	else
		return "";
}

void
sql_free(char *q)
{
	if (q)
		sqlite3_free(q);
}

void
sql_freearg(struct sql_arg *arg)
{
	if (arg) {
		switch (arg->dbtype) {
			case SQL_CDB:
				if (arg->q.cdb.get_cust)
					sqlite3_finalize(arg->q.cdb.get_cust);
				if (arg->q.cdb.put_cust)
					sqlite3_finalize(arg->q.cdb.put_cust);
				if (arg->q.cdb.upd_cust)
					sqlite3_finalize(arg->q.cdb.upd_cust);
				if (arg->q.cdb.get_cred)
					sqlite3_finalize(arg->q.cdb.get_cred);
				if (arg->q.cdb.get_cred_phone)
					sqlite3_finalize(arg->q.cdb.get_cred_phone);
				if (arg->q.cdb.upd_cred)
					sqlite3_finalize(arg->q.cdb.upd_cred);
				if (arg->q.cdb.put_cred)
					sqlite3_finalize(arg->q.cdb.put_cred);
				break;

			case SQL_MDB:
				if (arg->q.mdb.get_menuitem)
					sqlite3_finalize(arg->q.mdb.get_menuitem);
				if (arg->q.mdb.get_menustyles)
					sqlite3_finalize(arg->q.mdb.get_menustyles);
				if (arg->q.mdb.get_menuextras)
					sqlite3_finalize(arg->q.mdb.get_menuextras);
				if (arg->q.mdb.get_menusubitems)
					sqlite3_finalize(arg->q.mdb.get_menusubitems);
				break;

			case SQL_ODB:
				if (arg->q.odb.get_order)
					sqlite3_finalize(arg->q.odb.get_order);
				if (arg->q.odb.get_lastorder)
					sqlite3_finalize(arg->q.odb.get_lastorder);
				if (arg->q.odb.get_items)
					sqlite3_finalize(arg->q.odb.get_items);
				if (arg->q.odb.put_order)
					sqlite3_finalize(arg->q.odb.put_order);
				if (arg->q.odb.put_item)
					sqlite3_finalize(arg->q.odb.put_item);
				if (arg->q.odb.put_subitem)
					sqlite3_finalize(arg->q.odb.put_subitem);
				if (arg->q.odb.put_payment)
					sqlite3_finalize(arg->q.odb.put_payment);
				break;
		}
		sqlite3_close(arg->db);
		free(arg);
	}
}

void
sql_trace(void *arg, const char *query)
{
	struct sql_arg *sqlarg = arg;
	sql_err(sqlarg, "SQL: \"%s\"", query);
}
