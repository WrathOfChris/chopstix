/* $Gateweaver: module.c,v 1.4 2005/12/05 22:19:18 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <dlfcn.h>
#include <err.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: module.c,v 1.4 2005/12/05 22:19:18 cmaxwell Exp $");

static void * module;
static void (*module_customer)(struct custdb_functions *);
static void (*module_menu)(struct menudb_functions *);
static void (*module_order)(struct orderdb_functions *);

void
module_init(void)
{
	const char *msg;

	module = NULL;
	module_customer = NULL;
	module_menu = NULL;
	module_order = NULL;

	if (strlen(config.module) == 0)
		return;

#ifdef PROFILE
	/* fake module loading */
	void cdb_init(struct custdb_functions *);
	void mdb_init(struct menudb_functions *);
	void odb_init(struct orderdb_functions *);
	module_customer = &cdb_init;
	module_menu = &mdb_init;
	module_order = &odb_init;
	msg = NULL;
#else
	/* try to load the module */
	if ((module = dlopen(config.module, RTLD_LAZY|RTLD_GLOBAL)) == NULL) {
		if ((msg = dlerror()) != NULL) {
			dlclose(module);
			err(1, "loading module \"%s\": %s", config.module, msg);
		}
		err(1, "cannot load module \"%s\"", config.module);
	}

	/* fetch the entry points */

	if ((module_customer = dlsym(module, "cdb_init")) == NULL) {
		if ((msg = dlerror()) != NULL) {
			dlclose(module);
			err(1, "loading customerdb from module \"%s\": %s",
					config.module, msg);
		}
		err(1, "cannot load customerdb from module \"%s\"", config.module);
	}

	if ((module_menu = dlsym(module, "mdb_init")) == NULL) {
		if ((msg = dlerror()) != NULL) {
			dlclose(module);
			err(1, "loading menudb from module \"%s\": %s",
					config.module, msg);
		}
		err(1, "cannot load menudb from module \"%s\"", config.module);
	}

	if ((module_order = dlsym(module, "odb_init")) == NULL) {
		if ((msg = dlerror()) != NULL) {
			dlclose(module);
			err(1, "loading orderdb from module \"%s\": %s",
					config.module, msg);
		}
		err(1, "cannot load orderdb from module \"%s\"", config.module);
	}
#endif
}

void
module_init_customer(struct custdb_functions *cdbf)
{
	if (module_customer != NULL)
		module_customer(cdbf);
}

void
module_init_menu(struct menudb_functions *mdbf)
{
	if (module_menu)
		module_menu(mdbf);
}

void
module_init_order(struct orderdb_functions *odbf)
{
	if (module_order)
		module_order(odbf);
}

void
module_exit(void)
{
}
