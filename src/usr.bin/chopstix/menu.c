/* $Gateweaver: menu.c,v 1.11 2007/09/27 15:30:44 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: menu.c,v 1.11 2007/09/27 15:30:44 cmaxwell Exp $");

static struct menudb_functions mdbf;
static struct menudb_handle *mdbh;
static ChopstixMenu menu;

struct menudb_handle * menudb_open(const char *);
int menudb_get(struct menudb_handle *, const ChopstixItemCode,
		ChopstixMenuitem *);
int menudb_load(struct menudb_handle *, ChopstixMenu *);
int menudb_close(struct menudb_handle *);
const char * menudb_geterr(struct menudb_handle *);

static int
menu_sort_cmp(const void *a, const void *b)
{
	const ChopstixMenuitem *A = a, *B = b;
	int ret;

	if ((ret = strcmp(A->code, B->code)) == 0)
		if ((ret = strlen(A->code) - strlen(B->code)) != 0)
			return ret < 0 ? -1 : 1;

	return ret;
}

void
menu_init(void)
{
	bzero(&mdbf, sizeof(mdbf));
	mdbf.err = &status_dberr;

	bzero(&menu, sizeof(menu));

	/* use menudb module */
	module_init_menu(&mdbf);

	if ((mdbh = menudb_open(config.database.menudb)) == NULL)
		chopstix_exit();

	if (menudb_load(mdbh, &menu) == -1) {
		status_warn("cannot load menu module, see system log for details");
		chopstix_exit();
	}

	if (menu.len > 0)
		qsort(menu.val, menu.len, sizeof(ChopstixMenuitem), menu_sort_cmp);
}

ChopstixMenuitem *
menu_getitem(const ChopstixItemCode code)
{
	ChopstixMenuitem item = {0};

	if (code == NULL || strlen(code) == 0 || menu.len == 0)
		return NULL;

	item.code = code;
	return bsearch(&item, menu.val, menu.len, sizeof(ChopstixMenuitem),
			menu_sort_cmp);
}

void
menu_exit(void)
{
}

struct menudb_handle *
menudb_open(const char *dbfile)
{
	struct menudb_handle *mh;

	if (mdbf.open == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((mh = calloc(1, sizeof(struct menudb_handle))) == NULL)
		return NULL;

	if ((mh->dbfile = strdup(dbfile)) == NULL) {
		free(mh);
		return NULL;
	}

	if (mdbf.open(mh) == -1) {
		free(mh);
		return NULL;
	}

	return mh;
}

int
menudb_get(struct menudb_handle *mh, const ChopstixItemCode code,
		ChopstixMenuitem *item)
{
	if (mh && mdbf.get)
		return mdbf.get(mh, code, item);

	errno = EINVAL;
	return -1;
}

int
menudb_load(struct menudb_handle *mh, ChopstixMenu *menu)
{
	if (mh && mdbf.load)
		return mdbf.load(mh, menu);

	errno = EINVAL;
	return -1;
}

int
menudb_close(struct menudb_handle *mh)
{
	if (mh && mdbf.close)
		return mdbf.close(mh);

	errno = EINVAL;
	return -1;
}

const char *
menudb_geterr(struct menudb_handle *mh)
{
	if (mh && mdbf.geterr)
		return mdbf.geterr(mh);

	return NULL;
}
