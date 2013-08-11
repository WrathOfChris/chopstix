/* $Gateweaver: menu.c,v 1.13 2007/09/27 01:43:32 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix_sql.h"

RCSID("$Gateweaver: menu.c,v 1.13 2007/09/27 01:43:32 cmaxwell Exp $");

static int mdb_open(struct menudb_handle *);
static int mdb_create(struct menudb_handle *);
#if 0
static int mdb_read(struct menudb_handle *, ChopstixMenuitem *);
static int mdb_readkey(struct menudb_handle *, ChopstixItemCode);
static int mdb_rewind(struct menudb_handle *);
#endif
static int mdb_get(struct menudb_handle *, const ChopstixItemCode,
		ChopstixMenuitem *);
static int mdb_load(struct menudb_handle *, ChopstixMenu *);

static int mdb_put_item(struct menudb_handle *, const ChopstixMenuitem *);
static int mdb_put_style(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemStyle *);
static int mdb_put_extra(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemExtra *);
static int mdb_put_subitem(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemExtra *);

static int mdb_update_item(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixMenuitem *);
static int mdb_update_style(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemStyle *);
static int mdb_update_extra(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemExtra *, const ChopstixItemExtra *);
static int mdb_update_subitem(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemExtra *, const ChopstixItemExtra *);
static int mdb_remove_item(struct menudb_handle *, const ChopstixItemCode);
static int mdb_remove_style(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemStyle *);
static int mdb_remove_extra(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemExtra *);
static int mdb_remove_subitem(struct menudb_handle *, const ChopstixItemCode,
		const ChopstixItemExtra *);
static int mdb_close(struct menudb_handle *);
static const char * mdb_geterr(struct menudb_handle *);
static void (*mdb_puterr)(const char *, va_list);

void
mdb_init(struct menudb_functions *mdbf)
{
	mdbf->open = &mdb_open;
	mdbf->create = &mdb_create;
#if 0
	mdbf->read = &mdb_read;
	mdbf->readkey = &mdb_readkey;
	mdbf->rewind = &mdb_rewind;
#endif
	mdbf->get = &mdb_get;
	mdbf->load = &mdb_load;

	mdbf->put_item = &mdb_put_item;
	mdbf->put_style = &mdb_put_style;
	mdbf->put_extra = &mdb_put_extra;
	mdbf->put_subitem = &mdb_put_subitem;

	mdbf->update_item = &mdb_update_item;
	mdbf->update_style = &mdb_update_style;
	mdbf->update_extra = &mdb_update_extra;
	mdbf->update_subitem = &mdb_update_subitem;

	mdbf->remove_item = &mdb_remove_item;
	mdbf->remove_style = &mdb_remove_style;
	mdbf->remove_extra = &mdb_remove_extra;
	mdbf->remove_subitem = &mdb_remove_subitem;

	mdbf->close = &mdb_close;
	mdbf->geterr = &mdb_geterr;

	/* caller set error function */
	mdb_puterr = mdbf->err;
}

static int
mdb_open(struct menudb_handle *mh)
{
	if (SQLARG(mh) != NULL) {
		errno = EBADF;
		return -1;
	}

	if ((SQLARG(mh) = sql_makearg(SQL_MDB)) == NULL)
		return -1;
	SQLARG(mh)->err = mdb_puterr;

	if (sql_open(SQLARG(mh), mh->dbfile) == -1) {
		sql_freearg(SQLARG(mh));
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
mdb_create(struct menudb_handle *mh)
{
	errno = EACCES;
	return -1;
}

static int
mdb_get(struct menudb_handle *mh, const ChopstixItemCode code,
		ChopstixMenuitem *item)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	bzero(item, sizeof(*item));

	/*
	 * PRIMARY MENUITEM
	 */

	q = SQLARG(mh)->q.mdb.get_menuitem;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, code, strlen(code), SQLITE_STATIC)) {
		sql_err(SQLARG(mh), "cannot bind code number to query");
		goto fail;
	}
	if (sqlite3_step(q) == SQLITE_ROW) {
		item->code = sql_strdup(sqlite3_column_text(q, 0));
		item->name = sql_strdup(sqlite3_column_text(q, 1));
		item->price = sqlite3_column_int(q, 2);
		item->gen = sqlite3_column_int(q, 3);
		item->flags.deleted = sqlite3_column_int(q, 4);
	} else {
		sql_err(SQLARG(mh), "GET MENUITEM code=%s", code);
		errno = ENOENT;
		goto fail;
	}
	sqlite3_reset(q);

	/*
	 * STYLES
	 */

	q = SQLARG(mh)->q.mdb.get_menustyles;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, code, strlen(code), SQLITE_STATIC)) {
		sql_err(SQLARG(mh), "cannot bind code number to query");
		goto fail;
	}
	if (sqlite3_bind_text(q, 2, code, strlen(code), SQLITE_STATIC)) {
		sql_err(SQLARG(mh), "cannot bind code number to query");
		goto fail;
	}
	while (sqlite3_step(q) == SQLITE_ROW) {
		ChopstixItemStyle *is;

		if ((is = realloc(item->styles.val, (item->styles.len + 1) *
						sizeof(ChopstixItemStyle))) == NULL)
			goto fail;
		item->styles.len++;
		item->styles.val = is;

		item->styles.val[item->styles.len - 1].name
			= sql_strdup(sqlite3_column_text(q, 0));
		item->styles.val[item->styles.len - 1].num
			= sqlite3_column_int(q, 1);
	}
	sqlite3_reset(SQLARG(mh)->q.mdb.get_menustyles);

	/*
	 * EXTRAS
	 */

	q = SQLARG(mh)->q.mdb.get_menuextras;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, code, strlen(code), SQLITE_STATIC)) {
		sql_err(SQLARG(mh), "cannot bind code number to query");
		goto fail;
	}
	if (sqlite3_bind_text(q, 2, code, strlen(code), SQLITE_STATIC)) {
		sql_err(SQLARG(mh), "cannot bind code number to query");
		goto fail;
	}
	while (sqlite3_step(q) == SQLITE_ROW) {
		ChopstixItemExtra *ie;

		if ((ie = realloc(item->extras.val, (item->extras.len + 1) *
						sizeof(ChopstixItemExtra))) == NULL)
			goto fail;
		item->extras.len++;
		item->extras.val = ie;

		item->extras.val[item->extras.len - 1].qty
			= sqlite3_column_int(q, 0);
		item->extras.val[item->extras.len - 1].code
			= sql_strdup(sqlite3_column_text(q, 1));
	}
	sqlite3_reset(SQLARG(mh)->q.mdb.get_menuextras);

	/*
	 * SUBITEMS (Same as extras)
	 */

	q = SQLARG(mh)->q.mdb.get_menusubitems;
	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, code, strlen(code), SQLITE_STATIC)) {
		sql_err(SQLARG(mh), "cannot bind code number to query");
		goto fail;
	}
	if (sqlite3_bind_text(q, 2, code, strlen(code), SQLITE_STATIC)) {
		sql_err(SQLARG(mh), "cannot bind code number to query");
		goto fail;
	}
	while (sqlite3_step(q) == SQLITE_ROW) {
		ChopstixItemExtra *ie;

		if (item->subitems == NULL)
			if ((item->subitems = calloc(1, sizeof(*item->subitems))) == NULL)
				goto fail;

		if ((ie = realloc(item->subitems->val, (item->subitems->len + 1)
						* sizeof(ChopstixItemExtra)))
				== NULL)
			goto fail;
		item->subitems->len++;
		item->subitems->val = ie;

		item->subitems->val[item->subitems->len - 1].qty
			= sqlite3_column_int(q, 0);
		item->subitems->val[item->subitems->len - 1].code
			= sql_strdup(sqlite3_column_text(q, 1));
	}
	sqlite3_reset(SQLARG(mh)->q.mdb.get_menusubitems);

	return 0;

fail:
	free_ChopstixMenuitem(item);
	if (q)
		sqlite3_reset(q);
	return -1;
}

static int
mdb_load(struct menudb_handle *mh, ChopstixMenu *menu)
{
	sqlite3_stmt *q = NULL;
	ChopstixMenuitem *mi;
	ChopstixItemCode code;

	CHECKSQL(mh);

	bzero(menu, sizeof(*menu));

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_LOAD_MENU,
				strlen(SQL_MDB_LOAD_MENU), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP LOAD MENU");
		goto fail;
	}

	while (sqlite3_step(q) == SQLITE_ROW) {
		if ((mi = realloc(menu->val, (menu->len + 1) *
						sizeof(ChopstixMenuitem))) == NULL)
			goto fail;
		menu->len++;
		menu->val = mi;

		(const char *)code = sqlite3_column_text(q, 0);
		if (mdb_get(mh, code, &menu->val[menu->len - 1]) == -1)
			goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	free_ChopstixMenu(menu);
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_put_item(struct menudb_handle *mh, const ChopstixMenuitem *mi)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_PUT_MENUITEM,
				strlen(SQL_MDB_PUT_MENUITEM), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP PUT MENUITEM");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, mi->code, strlen(mi->code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 2, mi->gen))
		goto fail;
	if (sqlite3_bind_text(q, 3, mi->name, strlen(mi->name), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 4, mi->price))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot put menu item");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_put_style(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemStyle *style)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_PUT_MENUSTYLE,
				strlen(SQL_MDB_PUT_MENUSTYLE), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP PUT MENUSTYLE");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, style->name, strlen(style->name),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 2, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot put menu style");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_put_extra(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemExtra *extra)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_PUT_MENUEXTRA,
				strlen(SQL_MDB_PUT_MENUEXTRA), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP PUT MENUEXTRA");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, extra->qty))
		goto fail;
	if (sqlite3_bind_text(q, 2, extra->code, strlen(extra->code),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 4, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot put menu extra");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_put_subitem(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemExtra *subitem)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_PUT_MENUSUBITEM,
				strlen(SQL_MDB_PUT_MENUSUBITEM), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP PUT MENUSUBITEM");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, subitem->qty))
		goto fail;
	if (sqlite3_bind_text(q, 2, subitem->code, strlen(subitem->code),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 4, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot put menu subitem");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_update_item(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixMenuitem *mi)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_UPD_MENUITEM,
				strlen(SQL_MDB_UPD_MENUITEM), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP UPD MENUITEM");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, mi->code, strlen(mi->code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 2, mi->gen))
		goto fail;
	if (sqlite3_bind_text(q, 3, mi->name, strlen(mi->name), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 4, mi->price))
		goto fail;
	if (sqlite3_bind_text(q, 5, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 6, mi->gen))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot update menu item");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_update_style(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemStyle *style)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_UPD_MENUSTYLE,
				strlen(SQL_MDB_UPD_MENUSTYLE), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP UPD MENUSTYLE");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, style->name, strlen(style->name),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 2, style->num))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot update menu item");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_update_extra(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemExtra *old, const ChopstixItemExtra *new)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_UPD_MENUEXTRA,
				strlen(SQL_MDB_UPD_MENUEXTRA), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP UPD MENUEXTRA");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, new->qty))
		goto fail;
	if (sqlite3_bind_text(q, 2, new->code, strlen(new->code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 4, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 5, old->code, strlen(old->code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 6, old->qty))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot update menu extra");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_update_subitem(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemExtra *old, const ChopstixItemExtra *new)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_UPD_MENUSUBITEM,
				strlen(SQL_MDB_UPD_MENUSUBITEM), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP UPD MENUSUBITEM");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, new->qty))
		goto fail;
	if (sqlite3_bind_text(q, 2, new->code, strlen(new->code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 4, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 5, old->code, strlen(old->code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 6, old->qty))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot update menu subitem");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_remove_item(struct menudb_handle *mh, const ChopstixItemCode code)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_DEL_MENUITEM,
				strlen(SQL_MDB_DEL_MENUITEM), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP DEL MENUITEM");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 2, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot delete menu item");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_remove_style(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemStyle *style)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_DEL_MENUSTYLE,
				strlen(SQL_MDB_DEL_MENUSTYLE), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP DEL MENUSTYLE");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_int(q, 1, style->num))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot delete menu style");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_remove_extra(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemExtra *extra)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_DEL_MENUEXTRA,
				strlen(SQL_MDB_DEL_MENUEXTRA), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP DEL MENUEXTRA");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 2, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, extra->code, strlen(extra->code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 4, extra->qty))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot delete menu extra");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_remove_subitem(struct menudb_handle *mh, const ChopstixItemCode code,
		const ChopstixItemExtra *subitem)
{
	sqlite3_stmt *q = NULL;

	CHECKSQL(mh);

	if (sqlite3_prepare(SQLARG(mh)->db, SQL_MDB_DEL_MENUSUBITEM,
				strlen(SQL_MDB_DEL_MENUSUBITEM), &q, NULL)) {
		sql_err(SQLARG(mh), "MQ PREP DEL MENUSUBITEM");
		goto fail;
	}

	/* 1 = first parameter in prepared query */
	if (sqlite3_bind_text(q, 1, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 2, code, strlen(code), SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_text(q, 3, subitem->code, strlen(subitem->code),
				SQLITE_STATIC))
		goto fail;
	if (sqlite3_bind_int(q, 4, subitem->qty))
		goto fail;
	if (sqlite3_step(q) != SQLITE_DONE) {
		sql_err(SQLARG(mh), "cannot delete menu subitem");
		goto fail;
	}
	sqlite3_finalize(q);
	return 0;

fail:
	if (q)
		sqlite3_finalize(q);
	return -1;
}

static int
mdb_close(struct menudb_handle *mh)
{
	if (SQLARG(mh) == NULL) {
		errno = EINVAL;
		return -1;
	}
	sql_freearg(SQLARG(mh));

	return 0;
}

static const char *
mdb_geterr(struct menudb_handle *mh)
{
	if (SQLARG(mh))
		return sql_geterr(SQLARG(mh));
	else
		return "";
}
