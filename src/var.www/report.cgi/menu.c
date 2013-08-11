/* $Gateweaver: menu.c,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <string.h>
#include "report.h"

ChopstixMenuitem *
menu_getitem(const char *code)
{
	unsigned int u;

	if (code == NULL || strlen(code) == 0)
		return NULL;

	for (u = 0; u < menu.len; u++)
		if (strcasecmp(code, menu.val[u].code) == 0)
			return &menu.val[u];

	return NULL;
}
