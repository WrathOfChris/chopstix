/* $Gateweaver: chopstix_api.h,v 1.17 2007/09/28 17:07:56 cmaxwell Exp $ */
#ifndef CHOPSTIX_API_H
#define CHOPSTIX_API_H
#include <sys/param.h>
#include <stdarg.h>
#include "chopstix_asn1.h"

#ifndef RCSID
#define RCSID(msg) \
static const char rcsid[] = { msg }
#endif

#define DOLLARS(a)	((a) / 100)
#define CENTS(a)	(abs((a) - (DOLLARS(a) * 100)))
/* Print '-' when (a) is less than a dollar negative */
#define NEGSIGN(a)	(((a) < 0 && (a) > -100) ? "-" : "")
/* handle overflows */
#define ROLLOVER(a) \
	(((a) < INT_MIN) ? INT_MIN : (((a) > INT_MAX) ? INT_MAX : (a)))

/*
 * This abuses the properties of an integer and order of operation to
 * correctly round using fixed-point math.
 *
 * Basically, it adds (subtracts) 50/100 before dividing, which 'rounds'
 * up beyond the next integer which is promptly truncated.
 */
#define TAXCALC(sub, rate) \
	(((sub) * (rate) + (((rate) > 0) ? (50) : (-50))) / 100)

#define DISCOUNT(totals) 									\
	(((totals)->disctype == DISCOUNT_PERCENT)				\
		? TAXCALC((totals)->subtotal, (totals)->discount)	\
		: (totals)->discount)

#define DELIVERY(totals)									\
	(((totals)->delitype == DISCOUNT_PERCENT)				\
		? TAXCALC((totals)->subtotal, (totals)->delivery)	\
	  	: (totals)->delivery)

struct orderdb_handle {
	void *arg;
	char *dbfile;
};
struct orderdb_functions {
	int (*open)(struct orderdb_handle *);
	int (*create)(struct orderdb_handle *);
	int (*read)(struct orderdb_handle *, ChopstixOrder *);
	int (*readkey)(struct orderdb_handle *, ChopstixOrderKey);
	int (*rewind)(struct orderdb_handle *);
	int (*get)(struct orderdb_handle *, const ChopstixOrderKey, ChopstixOrder *);
	int (*getlast)(struct orderdb_handle *, const ChopstixPhone *,
			ChopstixOrder *);
	int (*getdaily_total)(struct orderdb_handle *, ChopstixTotal *);
	int (*getrange)(struct orderdb_handle *, time_t, time_t,
			int (*)(const ChopstixOrder *, void *), void *);
	int (*put)(struct orderdb_handle *, const ChopstixOrder *, int *oid,
			time_t *);
	int (*update)(struct orderdb_handle *, const ChopstixOrder *);
	int (*remove)(struct orderdb_handle *, const ChopstixOrderKey);
	int (*close)(struct orderdb_handle *);
	const char * (*geterr)(struct orderdb_handle *);
	/* set by caller */
	void (*err)(const char *, va_list);
	ChopstixMenuitem * (*getmenuitem)(char *);
};

struct menudb_handle {
	void *arg;
	char *dbfile;
};
struct menudb_functions {
	int (*open)(struct menudb_handle *);
	int (*create)(struct menudb_handle *);
	int (*read)(struct menudb_handle *, ChopstixMenuitem *);
	int (*readkey)(struct menudb_handle *, ChopstixItemCode);
	int (*rewind)(struct menudb_handle *);
	int (*get)(struct menudb_handle *, const ChopstixItemCode,
			ChopstixMenuitem *);
	int (*load)(struct menudb_handle *, ChopstixMenu *);
	int (*put_item)(struct menudb_handle *, const ChopstixMenuitem *);
	int (*put_style)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemStyle *);
	int (*put_extra)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemExtra *);
	int (*put_subitem)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemExtra *);
	int (*update_item)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixMenuitem *);
	int (*update_style)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemStyle *);
	int (*update_extra)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemExtra *, const ChopstixItemExtra *);
	int (*update_subitem)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemExtra *, const ChopstixItemExtra *);
	int (*remove_item)(struct menudb_handle *, const ChopstixItemCode);
	int (*remove_style)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemStyle *);
	int (*remove_extra)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemExtra *);
	int (*remove_subitem)(struct menudb_handle *, const ChopstixItemCode,
			const ChopstixItemExtra *);
	int (*close)(struct menudb_handle *);
	const char * (*geterr)(struct menudb_handle *);
	/* set by caller */
	void (*err)(const char *, va_list); 
};

struct custdb_handle {
	void *arg;
	char *dbfile;
};
struct custdb_functions {
	int (*open)(struct custdb_handle *);
	int (*create)(struct custdb_handle *);
	int (*read)(struct custdb_handle *, ChopstixCustomer *);
	int (*readkey)(struct custdb_handle *, ChopstixPhone *);
	int (*rewind)(struct custdb_handle *);
	int (*get)(struct custdb_handle *, const ChopstixPhone *, ChopstixCustomer *);
	int (*getfromoid)(struct custdb_handle *, const ChopstixOrderKey, ChopstixCustomer *);
	int (*put)(struct custdb_handle *, const ChopstixCustomer *);
	int (*update)(struct custdb_handle *, const ChopstixCustomer *);
	int (*update_phone)(struct custdb_handle *, const ChopstixPhone *,
			const ChopstixPhone *);
	int (*remove)(struct custdb_handle *, const ChopstixPhone *);
	int (*close)(struct custdb_handle *);
	const char * (*geterr)(struct custdb_handle *);
	/* set by caller */
	void (*err)(const char *, va_list); 
};

struct rule_functions {
	int (*run)(ChopstixOrder *, int);
	/* set by caller */
	void (*err)(const char *, va_list);
	int (*getprice)(char *);
	ChopstixMenuitem * (*getmenuitem)(char *);
};

#endif
