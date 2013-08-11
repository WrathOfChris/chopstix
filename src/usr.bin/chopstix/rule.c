/* $Gateweaver: rule.c,v 1.3 2005/12/05 22:19:18 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: rule.c,v 1.3 2005/12/05 22:19:18 cmaxwell Exp $");

static void * rulemod;
static void (*rulemod_init)(struct rule_functions *);
static struct rule_functions rulefunc;

void
rule_init(void)
{
	const char *msg;

	rulemod = NULL;
	rulemod_init = NULL;

	bzero(&rulefunc, sizeof(rulefunc));
	rulefunc.err = &status_ruleerr;
	rulefunc.getprice = &rule_getprice;
	rulefunc.getmenuitem = &rule_getmenuitem;

	if (strlen(config.rulemodule) == 0)
		return;

#ifdef PROFILE
	/* fake module loading */
	void rm_init(void);
	rulemod_init = &rm_init;
	msg = NULL;
#else
	if ((rulemod = dlopen(config.rulemodule, RTLD_LAZY|RTLD_GLOBAL)) == NULL) {
		if ((msg = dlerror()) != NULL) {
			dlclose(rulemod);
			err(1, "loading module \"%s\": %s", config.rulemodule, msg);
		}
		err(1, "cannot load module \"%s\"", config.rulemodule);
	}

	if ((rulemod_init = dlsym(rulemod, "rm_init")) == NULL) {
		if ((msg = dlerror()) != NULL) {
			dlclose(rulemod);
			err(1, "loading rules from module \"%s\": %s",
					config.rulemodule, msg);
		}
		err(1, "cannot load rules from module \"%s\"", config.rulemodule);
	}
#endif

	rule_init_module(&rulefunc);
}

void
rule_init_module(struct rule_functions *rf)
{
	if (rulemod_init)
		rulemod_init(rf);
}

void
rule_exit(void)
{
}

int
rule_run(ChopstixOrder *order, int post)
{
	if (rulefunc.run)
		return rulefunc.run(order, post);

	errno = EINVAL;
	return -1;
}

int
rule_getprice(char *code)
{
	ChopstixMenuitem *mi;

	if ((mi = menu_getitem(code)) == NULL)
		return 0;

	return mi->price;
}

ChopstixMenuitem *
rule_getmenuitem(char *code)
{
	return menu_getitem(code);
}
