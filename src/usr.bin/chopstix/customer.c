/* $Gateweaver: customer.c,v 1.32 2007/09/09 15:50:56 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: customer.c,v 1.32 2007/09/09 15:50:56 cmaxwell Exp $");

/* Compare two ChopstixPhone.  Evaluates !0 on failure */
#define PHONECMP(a, b)							\
	(	(a)->npa != (b)->npa					\
	 || (a)->nxx != (b)->nxx					\
	 || (a)->num != (b)->num					\
	 || ((a)->ext == NULL && (b)->ext != NULL) 	\
	 || ((a)->ext != NULL && (b)->ext == NULL) 	\
	 || ((a)->ext != NULL && (b)->ext != NULL && *(a)->ext != *(b)->ext))

/* database interface */
struct custdb_handle * custdb_open(const char *);
struct custdb_handle * custdb_create(const char *);
int custdb_read(struct custdb_handle *, ChopstixCustomer *);
int custdb_readkey(struct custdb_handle *, ChopstixPhone *);
int custdb_rewind(struct custdb_handle *);
int custdb_get(struct custdb_handle *, const ChopstixPhone *, ChopstixCustomer *);
int custdb_getfromoid(struct custdb_handle *, const ChopstixOrderKey,
		ChopstixCustomer *);
int custdb_put(struct custdb_handle *, const ChopstixCustomer *);
int custdb_update(struct custdb_handle *, const ChopstixCustomer *);
int custdb_update_phone(struct custdb_handle *, const ChopstixPhone *,
		const ChopstixPhone *);
int custdb_remove(struct custdb_handle *, const ChopstixPhone *);
int custdb_close(struct custdb_handle *);
const char *custdb_geterr(struct custdb_handle *);

static struct custdb_functions cdbf;
static struct custdb_handle *cdbh;

enum customer_state {
	STATE_NONE,
	STATE_PHONE,
	STATE_NAME,
	STATE_ADDRESS,
	STATE_APT,
	STATE_ENTRY,
	STATE_ISECT,
	STATE_DELIVER,
	STATE_REPS,
	STATE_MAX
};

/* internal functions */
static int customer_validate_deliver(const char *, int *, void *);
static ChopstixFormAction customer_putdata_phone(const char *, void *);
static ChopstixFormAction customer_putdata(const char *, void *);
static void initcust(ChopstixCustomer *);

/* globals */
ChopstixCustomer *ordercust;
ChopstixForm addrform;

void
customer_init(void)
{
	ChopstixFormField *ff = NULL;

	bzero(&cdbf, sizeof(cdbf));
	cdbf.err = &status_dberr;
	ordercust = NULL;

	/* use customerdb module */
	module_init_customer(&cdbf);

	if ((cdbh = custdb_open(config.database.custdb)) == NULL) {
		status_warn("cannot open customer database, see system log for details");
		chopstix_exit();
	}

	/*
	 * Address Form
	 */

	bzero(&addrform, sizeof(addrform));
	TAILQ_INIT(&addrform.fields);
	addrform.align = 1;
	if ((addrform.labelsep = strdup(": ")) == NULL)
		err(1, "allocating form");

	if ((ff = form_field_new("Phone Number")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 1, 0);
	form_field_setiw(ff, CHOPSTIX_PHONE_SIZE - 1);
	form_field_setarg(ff, (void *)STATE_PHONE);
	ff->getstr = &form_field_getstr_phone;
	ff->validate = &form_field_validate_phone;
	ff->getcpos = &form_field_getcpos_phone;
	ff->getrpos = &form_field_getrpos_phone;
	ff->putdata = &customer_putdata_phone;

	if ((ff = form_field_new("Name")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 1, 40);
	form_field_setarg(ff, (void *)STATE_NAME);
	ff->putdata = &customer_putdata;

	if ((ff = form_field_new("Address")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 2, 0);
	form_field_setarg(ff, (void *)STATE_ADDRESS);
	ff->putdata = &customer_putdata;

	if ((ff = form_field_new("Apt")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 3, 0);
	form_field_setiw(ff, 5);
	form_field_setarg(ff, (void *)STATE_APT);
	ff->putdata = &customer_putdata;

	if ((ff = form_field_new("Entry")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 3, 40);
	form_field_setiw(ff, 5);
	form_field_setarg(ff, (void *)STATE_ENTRY);
	ff->putdata = &customer_putdata;

	if ((ff = form_field_new("Main Intersection")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 4, 0);
	form_field_setarg(ff, (void *)STATE_ISECT);
	ff->putdata = &customer_putdata;

	if ((ff = form_field_new("Deliver To")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 5, 0);
	form_field_setiw(ff, strlen("DELIVERY"));
	form_field_setarg(ff, (void *)STATE_DELIVER);
	ff->putdata = &customer_putdata;
	ff->validate = &customer_validate_deliver;
	ff->hotfield = 1;

	if ((ff = form_field_new("Reps")) == NULL)
		err(1, "allocating fields");
	form_field_add(&addrform, ff);
	form_field_setyx(ff, LINE_TITLE + 5, 40);
	form_field_setiw(ff, ALIGN_NUMBER);
	form_field_setarg(ff, (void *)STATE_REPS);
	form_field_setdisplayonly(ff, 1);
}

void
customer_new(ChopstixCustomer *cust)
{
	ChopstixFormField *ff;
	char number[ALIGN_NUMBER + 1];
	int i;

	/* initialize the current customer */
	initcust(cust);
	ordercust = cust;

	if (config.phoneprefix) {
		snprintf(number, sizeof(number), "%03d", config.phoneprefix);
		TAILQ_FOREACH(ff, &addrform.fields, entry)
			if ((enum customer_state)ff->arg == STATE_PHONE) {
				if (!stracpy(&ff->input, number))
					status_warn("cannot set phone autoprefix");
				/* move over some characters */
				for (i = 0; i < strlen(number); i++)
					ff->cpos = ff->getcpos(ff->input.s, ff->cpos,
							FIELD_POS_RIGHT, ff->arg);
			}
	}
}

void
customer_die(void)
{
	ordercust = NULL;
}

/*
 * return the number of credits on file
 */
int
customer_getcredits(ChopstixCustomer *cust)
{
	if (cust->credit == NULL)
		return 0;
	return cust->credit->len;
}

int
customer_getcredit_total(void)
{
	size_t z;
	int total = 0;

	if (ordercust == NULL || ordercust->credit == NULL)
		return 0;
	for (z = 0; z < ordercust->credit->len; z++)
		total += ordercust->credit->val[z].credit;
	return total;
}

int
customer_getcredit_remain(void)
{
	size_t z;
	int total = 0;

	if (ordercust == NULL || ordercust->credit == NULL)
		return 0;
	for (z = 0; z < ordercust->credit->len; z++)
		total += ordercust->credit->val[z].remain;
	return total;
}

void
customer_exit(void)
{
	if (custdb_close(cdbh) == -1)
		status_err("cannot properly close customer database");
}

static void
parse_phonestr(const char *str, ChopstixPhone *phone)
{
	enum { PHONE_NPA, PHONE_NXX, PHONE_NUMB, PHONE_X, PHONE_EXT } state
		= PHONE_NPA;
	int bit = 2;

	free_ChopstixPhone(phone);
	bzero(phone, sizeof(*phone));

#define BITMULT(b) \
	((b) == 4 ? 10000 : (b) == 3 ? 1000 : (b) == 2 ? 100 : (b) == 1 ? 10 : 1)

	while (*str) {
		if (!(isdigit(*str) || *str == 'X' || *str == 'x'))
			return;

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

static const char *
customer_getraw_phone(ChopstixPhone *phone)
{
	static char str[CHOPSTIX_PHONE_RAWSIZE];
	int len;

	len = snprintf(str, sizeof(str), "%03d%03d%04d",
			phone->npa, phone->nxx, phone->num);
	if (phone->ext != NULL)
		snprintf(str + len, sizeof(str) - len, "x%-5d", *phone->ext);

	return str;
}

static int
customer_validate_deliver(const char *str, int *pos, void *arg)
{
	const char *s;

	if (strlen(str) > 1) {
		*pos = 1;
		return -1;
	}

	for (s = str; *s; *s++)
		if (tolower(*s) == 'p' || tolower(*s) == 'w' || tolower(*s) == 'v'
				|| tolower(*s) == 'd')
			break;
		else {
			*pos = s - str;
			return -1;
		}

	return 0;
}

/*
 * This is a bit different, when data is put to this field it manipulates the
 * raw input of all the other fields if a match is found.
 */
static ChopstixFormAction
customer_putdata_phone(const char *str, void *arg)
{
	ChopstixPhone phone;
	ChopstixFormField *ff;
	enum customer_state state;
	int errorcur = 0, new = 0, doextupdate = 0;
	char number[ALIGN_NUMBER + 1];
	ChopstixFormAction action = FIELD_NEXT;
	char *s;

	if (ordercust == NULL) {
		status_warn("order did not initialize customer");
		return FIELD_NEXT;
	}

	bzero(&phone, sizeof(phone));
	parse_phonestr(str, &phone);

	if (	(phone.ext && ordercust->phone.ext
				&& phone.ext != ordercust->phone.ext)
			|| (phone.ext != ordercust->phone.ext))
		doextupdate = 1;

	/* just free the old, and fill in the phone part */
	free_ChopstixCustomer(ordercust);

	/* try to pull from the database */
	if (custdb_get(cdbh, &phone, ordercust) == -1) {
		/* failed, so initialize the customer again */
		initcust(ordercust);

		/* copy the phone number in */
		if (copy_ChopstixPhone(&phone, &ordercust->phone)) {
			status_warn("cannot copy customer phone number");
			free_ChopstixPhone(&phone);
			return FIELD_NEXT;
		}

		/* and put a new record */
		if (custdb_put(cdbh, ordercust) == -1) {
			status_warn("cannot store customer in database: %s",
					custdb_geterr(cdbh));
			free_ChopstixPhone(&phone);
			return FIELD_NEXT;
		}

		new = 1;
	} else {
		/*
		 * PhoneExt handling.  This only runs when retrieving an existing
		 * customer, and only if the extensions do not match.
		 */
		if (doextupdate) {
			if (phone.ext == NULL) {
				free(ordercust->phone.ext);
				ordercust->phone.ext = NULL;
			} else if (ordercust->phone.ext == NULL) {
				ordercust->phone.ext = phone.ext;
				phone.ext = NULL;
			} else
				*ordercust->phone.ext = *phone.ext;

			if (custdb_update(cdbh, ordercust) == -1)
				status_warn("cannot update customer entry: %s",
						custdb_geterr(cdbh));
		}
	}

	free_ChopstixPhone(&phone);

	/* 
	 * (ordercust) is now completely valid
	 */

	TAILQ_FOREACH(ff, &addrform.fields, entry) {
		state = (enum customer_state)ff->arg;

		switch (state) {
			case STATE_PHONE:
				if (!stracpy(&ff->input,
							customer_getraw_phone(&ordercust->phone)))
					errorcur = addrform.cur;
				break;
			case STATE_NAME:
				if (ordercust->name)
					if (!stracpy(&ff->input, ordercust->name))
						errorcur = addrform.cur;
				break;
			case STATE_ADDRESS:
				if (ordercust->addr.addr)
					if (!stracpy(&ff->input, ordercust->addr.addr))
						errorcur = addrform.cur;
				break;
			case STATE_APT:
				if (ordercust->addr.apt)
					if (!stracpy(&ff->input, ordercust->addr.apt))
						errorcur = addrform.cur;
				break;
			case STATE_ENTRY:
				if (ordercust->addr.entry)
					if (!stracpy(&ff->input, ordercust->addr.entry))
						errorcur = addrform.cur;
				break;
			case STATE_ISECT:
				if (ordercust->isect.cross)
					if (!stracpy(&ff->input, ordercust->isect.cross))
						errorcur = addrform.cur;
				break;
			case STATE_DELIVER:
				s = NULL;
				switch (order.type) {
					case ORDER_NONE:
						/* EMPTY */
						break;
					case ORDER_WALKIN:
						s = "WALKIN";
						break;
					case ORDER_DELIVERY:
						s = "DELIVERY";
						break;
					case ORDER_PICKUP:
						s = "PICKUP";
						break;
					case ORDER_VOID:
						s = "VOID";
						break;
				}
				if (!stracpy(&ff->input, s))
					errorcur = addrform.cur;
				if (!new) {
					/* stop here after entering, and do not move */
					form_setfield_cur(&addrform, ff);
					action = FIELD_POS_NONE;
				}
				break;
			case STATE_REPS:
				snprintf(number, sizeof(number), "%d", ordercust->reps);
				if (!stracpy(&ff->input, number))
					errorcur = addrform.cur;
				break;
			case STATE_NONE:
			case STATE_MAX:
				break;
		}
	}

	/* load any special instructions */
	special_reinit();

	if (errorcur) {
		addrform.cur = errorcur;
		return FORM_NONE;
	}

	return action;
}

static ChopstixFormAction
customer_putdata(const char *str, void *arg)
{
	enum customer_state state = (enum customer_state)arg;
	char *s;
	ChopstixFormField *ff;
	ChopstixFormAction action = FIELD_NEXT;

	if (ordercust == NULL) {
		status_warn("order did not initialize customer");
		return FIELD_NEXT;
	}

	if ((s = strdup(str)) == NULL) {
		status_warn("no memory to store \"%s\"", str);
		return FIELD_NEXT;
	}

	switch (state) {
		case STATE_NONE:
			free(s);
			break;
		case STATE_PHONE:
			free(s);
			break;
		case STATE_NAME:
			if (ordercust->name)
				free(ordercust->name);
			ordercust->name = s;
			break;
		case STATE_ADDRESS:
			if (ordercust->addr.addr)
				free(ordercust->addr.addr);
			ordercust->addr.addr = s;
			break;
		case STATE_APT:
			if (ordercust->addr.apt)
				free(ordercust->addr.apt);
			ordercust->addr.apt = s;
			break;
		case STATE_ENTRY:
			if (ordercust->addr.entry)
				free(ordercust->addr.entry);
			ordercust->addr.entry = s;
			break;
		case STATE_ISECT:
			if (ordercust->isect.cross)
				free(ordercust->isect.cross);
			ordercust->isect.cross = s;
			break;
		case STATE_DELIVER:
			free(s);
			s = "";
			switch (tolower(*str)) {
				case 'w':
					order.type = ORDER_WALKIN;
					s = "WALKIN";
					break;
				case 'd':
					order.type = ORDER_DELIVERY;
					s = "DELIVERY";
					break;
				case 'p':
					order.type = ORDER_PICKUP;
					s = "PICKUP";
					break;
				case 'v':
					order.type = ORDER_VOID;
					s = "VOID";
					break;
				default:
					status_set("'P'ickup, 'D'elivery, 'W'alkin, 'V'oid");
					action = FORM_NONE;
					break;
			}
			TAILQ_FOREACH(ff, &addrform.fields, entry)
				if ((enum customer_state)ff->arg == STATE_DELIVER)
					stracpy(&ff->input, s);
			/* return before update */
			return action;
		case STATE_REPS:
			free(s);
			/* return before update */
			return action;
		case STATE_MAX:
			break;
	}

	if (custdb_update(cdbh, ordercust) == -1)
		status_warn("cannot update customer entry: %s", custdb_geterr(cdbh));

	return action;
}

/*
 * load customer data into form
 */
int
customer_load(struct ChopstixCustomer *cust)
{
	customer_putdata_phone(customer_getraw_phone(&cust->phone), NULL);

	return 0;
}

/*
 * post customer data to database
 */
int
customer_post(struct ChopstixCustomer *cust)
{
	if (custdb_update(cdbh, cust) == -1) {
		status_warn("cannot post customer data: %s", custdb_geterr(cdbh));
		return -1;
	}
	return 0;
}

int
customer_update_phone(const char *str)
{
	ChopstixPhone phone;
	bzero(&phone, sizeof(phone));

	parse_phonestr(str, &phone);

	if (custdb_update_phone(cdbh, &ordercust->phone, &phone) == -1) {
		/* error message is long enough */
		status_warn("%s", custdb_geterr(cdbh));
		return -1;
	}

	customer_putdata_phone(str, NULL);

	return 0;
}

/*
 * Initialize all required fields
 */
static void
initcust(ChopstixCustomer *cust)
{
	bzero(cust, sizeof(*cust));
	cust->name = strdup("");
	cust->addr.addr = strdup("");
	cust->addr.apt = strdup("");
	cust->addr.entry = strdup("");
	cust->isect.cross = strdup("");
}

/*
 * Customer Database Interface
 */

struct custdb_handle *
custdb_open(const char *dbfile)
{
	struct custdb_handle *ch;

	if (cdbf.open == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((ch = calloc(1, sizeof(struct custdb_handle))) == NULL)
		return NULL;

	if ((ch->dbfile = strdup(dbfile)) == NULL) {
		free(ch);
		return NULL;
	}

	if (cdbf.open(ch) == -1) {
		free(ch);
		return NULL;
	}

	return ch;
}

struct custdb_handle *
custdb_create(const char *dbfile)
{
	struct custdb_handle *ch;

	if (cdbf.create == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((ch = calloc(1, sizeof(struct custdb_handle))) == NULL)
		return NULL;

	if ((ch->dbfile = strdup(dbfile)) == NULL) {
		free(ch);
		return NULL;
	}

	if (cdbf.create(ch) == -1) {
		free(ch);
		return NULL;
	}

	return ch;
}

int
custdb_read(struct custdb_handle *ch, ChopstixCustomer *cust)
{
	if (ch && cdbf.read)
		return cdbf.read(ch, cust);

	errno = EINVAL;
	return -1;
}

int
custdb_readkey(struct custdb_handle *ch, ChopstixPhone *phone)
{
	if (ch && cdbf.readkey)
		return cdbf.readkey(ch, phone);

	errno = EINVAL;
	return -1;
}

int
custdb_rewind(struct custdb_handle *ch)
{
	if (ch && cdbf.rewind)
		return cdbf.rewind(ch);

	errno = EINVAL;
	return -1;
}

int
custdb_get(struct custdb_handle *ch, const ChopstixPhone *phone,
		ChopstixCustomer *cust)
{
	if (ch && cdbf.get)
		return cdbf.get(ch, phone, cust);

	errno = EINVAL;
	return -1;
}

int
custdb_getfromoid(struct custdb_handle *ch, const ChopstixOrderKey oid,
		ChopstixCustomer *cust)
{
	if (ch && cdbf.getfromoid)
		return cdbf.getfromoid(ch, oid, cust);

	errno = EINVAL;
	return -1;
}

int
custdb_put(struct custdb_handle *ch, const ChopstixCustomer *cust)
{
	if (ch && cdbf.put)
		return cdbf.put(ch, cust);

	errno = EINVAL;
	return -1;
}

int
custdb_update(struct custdb_handle *ch, const ChopstixCustomer *cust)
{
	if (ch && cdbf.update)
		return cdbf.update(ch, cust);

	errno = EINVAL;
	return -1;
}

int
custdb_update_phone(struct custdb_handle *ch, const ChopstixPhone *old,
		const ChopstixPhone *new)
{
	if (ch && cdbf.update_phone)
		return cdbf.update_phone(ch, old, new);

	errno = EINVAL;
	return -1;
}

int
custdb_remove(struct custdb_handle *ch, const ChopstixPhone *phone)
{
	if (ch && cdbf.remove)
		return cdbf.remove(ch, phone);

	errno = EINVAL;
	return -1;
}

int
custdb_close(struct custdb_handle *ch)
{
	if (ch && cdbf.close)
		return cdbf.close(ch);

	errno = EINVAL;
	return -1;
}

const char *
custdb_geterr(struct custdb_handle *ch)
{
	if (ch && cdbf.geterr)
		return cdbf.geterr(ch);
	
	return NULL;
}
