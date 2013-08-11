/* $Gateweaver: parse.c,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "toolbox.h"

#define CONF_MAXLINE	8192
#define CONF_MAXKEY		128

/*
 * Simple config parser.  Pass an array formed as follows:
 *
 * tbkeyval conflist[] = {
 * 	{ "key",	val,	sizeof(val) },
 * 	{ NULL,		NULL,	0 }
 * };
 *
 * Config file will look like this:
 *
 * # comment
 * key	value
 */

int
tbloadconf(const char *filename, tbkeyval *c)
{
	FILE *fp;
	char s[CONF_MAXLINE], k[CONF_MAXKEY];
	char *t, *v;
	size_t i, l;

	if ((fp = fopen(filename, "r")) == NULL)
		return errno;
	while ((t = fgets(s, sizeof(s), fp))) {
		while (*t == ' ' || *t == '\t')
			t++;
		for (i = 0; i < sizeof(k) - 1 && *t && *t != ' ' && *t != '\t'; ++i)
			k[i] = *t++;
		k[i] = 0;
		if (!k[0] || k[0] == '\n' || k[0] == '#')
			continue;
		for (i = 0; c[i].key != NULL && strcmp(c[i].key, k); ++i)
			;
		if (c[i].key == NULL)
			continue;
		v = c[i].val;
		l = c[i].len;
		while (*t == ' ' || *t == '\t')
			t++;
		for (i = 0; i < l - 1 && *t && *t != '\n'; ++i)
			v[i] = *t++;
	}
	fclose(fp);
	return TBOX_SUCCESS;
}
