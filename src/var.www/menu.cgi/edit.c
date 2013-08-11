/* $Gateweaver: edit.c,v 1.3 2007/09/27 01:46:04 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

static int edit_parse_price(const char *, const char **);

int
edit_menu(enum edit_type type, enum edit_cmd cmd)
{
	const char *s, *errstr;
	const char *menucodenew = NULL;
	ChopstixMenuitem item, *mi;
	ChopstixItemStyle style;
	ChopstixItemExtra extra, *e_old = NULL;
	ChopstixItemExtra subitem, *s_old = NULL;
	unsigned int u;

	bzero(&item, sizeof(item));
	bzero(&style, sizeof(style));
	bzero(&extra, sizeof(extra));

	if ((s = get_post_param(q, "menucode")) == NULL) {
		render_error("edit requires \"menucode\"");
		return -1;
	}
	if (menucode)
		free(menucode);
	menucode = strdup(s);
	(const char *)item.code = menucode;

	if ((s = get_post_param(q, "menugen")) == NULL) {
		render_error("edit requires \"menugen\"");
		return -1;
	}
	item.gen = strtonum(s, 0, INT_MAX, &errstr);
	if (errstr) {
		render_error("item generation number \"%s\" is %s", s, errstr);
		return -1;
	}

	switch (type) {
		case EDIT_NEW:
			if (menucode == NULL || *menucode == '\0') {
				render_error("item code missing");
				return -1;
			}
			item.name = "";
			if (mdbf.put_item(&mdbh, &item) == -1) {
				render_error("cannot create new item: %s", mdbf.geterr(&mdbh));
				return -1;
			}
			break;

		case EDIT_ITEM:
			if ((mi = menu_getitem(menucode)) == NULL) {
				render_error("item \"%s\" is not on the menu", menucode);
				return -1;
			}
			bcopy(mi, &item, sizeof(item));

			if ((s = get_post_param(q, "itemcodenew"))) {
				(const char *)item.code = s;
				menucodenew = item.code;
			}

			if ((s = get_post_param(q, "itempricenew"))) {
				item.price = edit_parse_price(s, &errstr);
				if (errstr) {
					render_error("item price \"%s\" is %s", s, errstr);
					return -1;
				}
			}

			if ((s = get_post_param(q, "itemnamenew")))
				(const char *)item.name = s;

			if (cmd == ECMD_CREATE) {
				item.gen++;
				item.flags.deleted = 0;
				if (mdbf.put_item(&mdbh, &item) == -1) {
					render_error("cannot create item \"%s\": %s",
							menucode, mdbf.geterr(&mdbh));
					return -1;
				}
				for (u = 0; u < item.styles.len; u++)
					if (mdbf.put_style(&mdbh, item.code, &item.styles.val[u])
							== -1) {
						render_error("cannot create item \"%s\" style \"%s\": "
								"%s", item.code, item.styles.val[u].name,
								mdbf.geterr(&mdbh));
						return -1;
					}
				for (u = 0; u < item.extras.len; u++)
					if (mdbf.put_extra(&mdbh, item.code, &item.extras.val[u])
							== -1) {
						render_error("cannot create item \"%s\" extra \"%s\": "
								"%s", item.code, item.extras.val[u].code,
								mdbf.geterr(&mdbh));
						return -1;
					}
				if (item.subitems)
					for (u = 0; u < item.subitems->len; u++)
						if (mdbf.put_subitem(&mdbh, item.code,
									&item.subitems->val[u]) == -1) {
							render_error("cannot create item \"%s\" subitem "
									"\"%s\": %s", item.code,
									item.subitems->val[u].code,
									mdbf.geterr(&mdbh));
							return -1;
						}

			} else if (cmd == ECMD_DELETE) {
				if (mdbf.remove_item(&mdbh, menucode) == -1) {
					render_error("cannot delete item \"%s\": %s",
							menucode, mdbf.geterr(&mdbh));
					return -1;
				}
			} else {
				if (mdbf.update_item(&mdbh, menucode, &item) == -1) {
					render_error("cannot change item \"%s\": %s",
							menucode, mdbf.geterr(&mdbh));
					return -1;
				}
			}

			break;

		case EDIT_STYLE:
			if ((mi = menu_getitem(menucode)) == NULL) {
				render_error("item \"%s\" is not on the menu", menucode);
				return -1;
			}

			if ((s = get_post_param(q, "stylenameold")) == NULL) {
				render_error("style name missing");
				return -1;
			}
			for (u = 0; u < mi->styles.len; u++)
				if (strcasecmp(s, mi->styles.val[u].name) == 0) {
					bcopy(&mi->styles.val[u], &style, sizeof(style));
					break;
				}
			if (cmd == ECMD_CHANGE && u == mi->styles.len) {
				render_error("item \"%s\" does not have style \"%s\"",
						menucode, s);
				return -1;
			}

			if ((s = get_post_param(q, "stylenamenew")))
				(const char *)style.name = s;

			if (cmd == ECMD_CREATE) {
				if (mdbf.put_style(&mdbh, menucode, &style) == -1) {
					render_error("cannot create style \"%s\": %s",
							style.name, mdbf.geterr(&mdbh));
					return -1;
				}
			} else if (cmd == ECMD_DELETE) {
				if (mdbf.remove_style(&mdbh, menucode, &style) == -1) {
					render_error("cannot delete style \"%s\": %s",
							style.name, mdbf.geterr(&mdbh));
					return -1;
				}
			} else {
				if (mdbf.update_style(&mdbh, menucode, &style) == -1) {
					render_error("cannot change style \"%s\": %s",
							style.name, mdbf.geterr(&mdbh));
					return -1;
				}
			}
			break;

		case EDIT_EXTRA:
			if ((mi = menu_getitem(menucode)) == NULL) {
				render_error("item \"%s\" is not on the menu", menucode);
				return -1;
			}

			if ((s = get_post_param(q, "extraqtyold")) && *s) {
				extra.qty = strtonum(s, INT_MIN, INT_MAX, &errstr);
				if (errstr) {
					render_error("item extra qty \"%s\" is %s", s, errstr);
					return -1;
				}
			}

			if ((s = get_post_param(q, "extracodeold")))
				(const char *)extra.code = s;

			for (u = 0; u < mi->extras.len; u++)
				if (mi->extras.val[u].qty == extra.qty &&
						strcasecmp(extra.code, mi->extras.val[u].code) == 0) {
					e_old = &mi->extras.val[u];
					bcopy(&mi->extras.val[u], &extra, sizeof(extra));
					break;
				}
			if (cmd == ECMD_CHANGE && (u == mi->extras.len || e_old == NULL)) {
				render_error("item \"%s\" does not have extra qty %d code "
						"\"%s\"", menucode, extra.qty, extra.code);
				return -1;
			}

			if ((s = get_post_param(q, "extraqtynew"))) {
				extra.qty = strtonum(s, INT_MIN, INT_MAX, &errstr);
				if (errstr) {
					render_error("item extra qty \"%s\" is %s", s, errstr);
					return -1;
				}
			}

			if ((s = get_post_param(q, "extracodenew")))
				(const char *)extra.code = s;

			if (cmd == ECMD_CREATE) {
				if (mdbf.put_extra(&mdbh, menucode, &extra) == -1) {
					render_error("cannot create extra \"%s\": %s",
							extra.code, mdbf.geterr(&mdbh));
					return -1;
				}
			} else if (cmd == ECMD_DELETE) {
				if (mdbf.remove_extra(&mdbh, menucode, &extra) == -1) {
					render_error("cannot delete extra \"%s\": %s",
							extra.code, mdbf.geterr(&mdbh));
					return -1;
				}
			} else {
				if (mdbf.update_extra(&mdbh, menucode, e_old, &extra) == -1) {
					render_error("cannot change extra \"%s\": %s",
							extra.code, mdbf.geterr(&mdbh));
					return -1;
				}
			}
			break;

		case EDIT_SUBITEM:
			if ((mi = menu_getitem(menucode)) == NULL) {
				render_error("item \"%s\" is not on the menu", menucode);
				return -1;
			}

			if ((s = get_post_param(q, "subitemqtyold")) && *s) {
				subitem.qty = strtonum(s, INT_MIN, INT_MAX, &errstr);
				if (errstr) {
					render_error("item subitem qty \"%s\" is %s", s, errstr);
					return -1;
				}
			}

			if ((s = get_post_param(q, "subitemcodeold")))
				(const char *)subitem.code = s;

			u = 0;
			if (mi->subitems)
				for (u = 0; u < mi->subitems->len; u++)
					if (mi->subitems->val[u].qty == subitem.qty &&
							strcasecmp(subitem.code, mi->subitems->val[u].code)
							== 0) {
						s_old = &mi->subitems->val[u];
						bcopy(&mi->subitems->val[u], &subitem, sizeof(subitem));
						break;
					}
			if (cmd == ECMD_CHANGE && (mi->subitems == NULL ||
						u == mi->subitems->len || s_old == NULL)) {
				render_error("item \"%s\" does not have subitem qty %d code "
						"\"%s\"", menucode, subitem.qty, subitem.code);
				return -1;
			}

			if ((s = get_post_param(q, "subitemqtynew"))) {
				subitem.qty = strtonum(s, INT_MIN, INT_MAX, &errstr);
				if (errstr) {
					render_error("item subitem qty \"%s\" is %s", s, errstr);
					return -1;
				}
			}

			if ((s = get_post_param(q, "subitemcodenew")))
				(const char *)subitem.code = s;

			if (cmd == ECMD_CREATE) {
				if (mdbf.put_subitem(&mdbh, menucode, &subitem) == -1) {
					render_error("cannot create subitem \"%s\": %s",
							subitem.code, mdbf.geterr(&mdbh));
					return -1;
				}
			} else if (cmd == ECMD_DELETE) {
				if (mdbf.remove_subitem(&mdbh, menucode, &subitem) == -1) {
					render_error("cannot delete subitem \"%s\": %s",
							subitem.code, mdbf.geterr(&mdbh));
					return -1;
				}
			} else {
				if (mdbf.update_subitem(&mdbh, menucode, s_old, &subitem)
						== -1) {
					render_error("cannot change subitem \"%s\": %s",
							subitem.code, mdbf.geterr(&mdbh));
					return -1;
				}
			}
			break;

		default:
			return -1;
	}

	/* code changed, switch to new code */
	if (menucodenew && strcmp(menucode, menucodenew) != 0) {
		if (menucode)
			free(menucode);
		menucode = strdup(menucodenew);
	}

	return 0;
}

#define INVALID 	1
#define TOOSMALL	2
#define TOOLARGE	3

static int
edit_parse_price(const char *str, const char **errstrp)
{
	const char *s;
	long long p = 0;
	int dot = -1, error = 0;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = errno;
	errno = 0;

	for (s = str; s && *s; s++)
		if (!isspace(*s) && *s != '$')
			break;
	for (; s && *s; s++)
		if (isdigit(*s)) {
			p *= 10;
			p += *s - '0';
			if (dot >= 0)
				++dot;
		} else if (*s == '.') {
			if (dot == -1)
				dot = 0;
		} else if (*s != ',' && *s != ' ') {
			error = INVALID;
			break;
		}

	if (dot == -1)
		dot = 0;
	while (++dot <= 2)
		p *= 10;
	while (--dot > 2)
		p /= 10;

	if (p > INT_MAX)
		error = TOOLARGE;
	else if (p < INT_MIN)
		error = TOOSMALL;
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		p = 0;

	return p;
}
