/* $Gateweaver: units.c,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toolbox.h"
#include "toolbox_asn1.h"

const char *
tbox_units(int flags, const tbunits *units)
{
	static char *buf;	/* dynamic buffer */
	char *s;
	unsigned int u;

	if (units == NULL)
		return NULL;

	if (buf) {
		free(buf);
		buf = NULL;
	}

	for (u = 0; units[u].name; u++)
		if (flags & units[u].mult) {
			if (buf == NULL) {
				if (asprintf(&buf, "%s", units[u].name) == -1)
					goto fail;
			} else {
				s = buf;
				if (asprintf(&buf, "%s,%s", units[u].name, s) == -1) {
					buf = NULL;
					free(s);
					goto fail;
				}
				free(s);
			}
		}

	if (buf == NULL)
		return "";

	return buf;

fail:
	free(buf);
	buf = NULL;
	return NULL;
}

const char *
tbox_units_short(int flags, const tbunits *units)
{
	static char buf[32];
	unsigned int u;
	int v = sizeof(buf) - 2;

	if (units == NULL)
		return NULL;

	for (u = 0; units[u].name && v >= 0; u++)
		if (flags & units[u].mult)
			buf[v--] = *(units[u].name);
		else
			buf[v--] = '-';
	buf[sizeof(buf) - 1] = '\0';

	return (buf + v + 1);
}

int
tbox_units_parse(const char *str, const tbunits *units, int *val)
{
	const char *p;
	unsigned int u;
	int unknown = 0;

	if (str == NULL || units == NULL || val == NULL)
		return TBOX_API_INVALID;

	*val = 0;

	p = str;
	while (*p) {
		while (isspace(*p) || *p == ',')
			++p;
		for (u = 0; units[u].name; u++)
			if (strncasecmp(p, units[u].name, strlen(units[u].name)) == 0) {
				*val += units[u].mult;
				p += strlen(units[u].name);
				break;
			}
		if (units[u].name == NULL) {
			/* allow the use of 'none' but ignore it */
			if (strncasecmp(p, "none", 4) != 0)
				unknown++;
			while (!isspace(*p) && *p != ',' && *p != '\0')
				++p;
		}
	}

	if (unknown)
		return TBOX_NOTFOUND;

	return TBOX_SUCCESS;
}
