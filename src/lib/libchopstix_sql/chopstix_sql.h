/* $Gateweaver: chopstix_sql.h,v 1.24 2007/10/15 16:34:59 cmaxwell Exp $ */
#ifndef CHOPSTIX_SQL_H
#define CHOPSTIX_SQL_H
#include "chopstix_api.h"
#include "chopstix_asn1.h"
#include "sqlite3.h"

/*
 * CUSTOMER DATABASE QUERIES
 */

#define SQL_CDB_GET_CUST \
	"SELECT ID, Name, Phone, PhoneExt, Address, Apt, Entry, Intersection, "\
		"Special, (SELECT count(ID) FROM orders WHERE CID = customers.ID)"\
	"FROM customers "\
	"WHERE Phone = ?"

#define SQL_CDB_PUT_CUST \
	"INSERT INTO customers (Phone, PhoneExt, Name, Address, Apt, Entry, Intersection, Special) "\
	"VALUES (?, ?, ?, ?, ?, ?, ?, ?)"

#define SQL_CDB_UPD_CUST \
	"UPDATE customers SET Name = ?, PhoneExt = ?, Address = ?, "\
	"Apt = ?, Entry = ?, Intersection = ?, Special = ? "\
	"WHERE Phone = ?"

#define SQL_CDB_UPD_CUSTPHONE \
	"UPDATE customers SET Phone = ? "\
	"WHERE Phone = ?"

#define SQL_CDB_GET_CRED \
	"SELECT Credit, Remain, Reason "\
	"FROM credits "\
	"WHERE CID = ? "\
	"ORDER BY ID"

#define SQL_CDB_GET_CRED_PHONE \
	"SELECT credits.ID, credits.Credit, credits.Remain, credits.Reason "\
	"FROM credits, customers "\
	"WHERE customers.Phone = ? AND credits.CID = customers.ID "\
	"ORDER BY credits.ID"

#define SQL_CDB_PUT_CRED \
	"INSERT INTO credits (CID, Credit, Remain, Reason) "\
	"SELECT ID, ?, ?, ? FROM customers "\
	"WHERE customers.Phone = ?"

#define SQL_CDB_UPD_CRED \
	"UPDATE credits SET Remain = ?, Reason = ? WHERE ID = ?"

/*
 * MENU DATABASE QUERIES
 */

#define SQL_MDB_GET_MENUITEM \
	"SELECT Code, Name, Price, Generation, Deleted "\
	"FROM menuitems "\
	"WHERE Code = ? "\
	"ORDER BY Generation DESC "\
	"LIMIT 1"

#define SQL_MDB_LOAD_MENU \
	"SELECT DISTINCT Code "\
	"FROM menuitems "\
	"ORDER BY Code"

#define SQL_MDB_GET_STYLES \
	"SELECT menustyles.Style, menustyles.ID "\
	"FROM menustyles, menuitems "\
	"WHERE menuitems.Code = ? AND menustyles.MID = menuitems.ID "\
		"AND menuitems.Generation = "\
			"(SELECT max(Generation) FROM menuitems WHERE Code = ?) "\
	"ORDER BY menustyles.Style"

#define SQL_MDB_GET_EXTRAS \
	"SELECT menuextras.Qty, menuextras.Code "\
	"FROM menuextras, menuitems "\
	"WHERE menuitems.Code = ? AND menuextras.MID = menuitems.ID "\
		"AND menuitems.Generation = "\
			"(SELECT max(Generation) FROM menuitems WHERE Code = ?) "\
	"ORDER BY menuextras.Code"

#define SQL_MDB_GET_SUBITEMS \
	"SELECT menusubitems.Qty, menusubitems.Code "\
	"FROM menusubitems, menuitems "\
	"WHERE menuitems.Code = ? AND menusubitems.MID = menuitems.ID "\
		"AND menuitems.Generation = "\
			"(SELECT max(Generation) FROM menuitems WHERE Code = ?) "\
	"ORDER BY menusubitems.Code"

#define SQL_MDB_PUT_MENUITEM \
	"INSERT INTO menuitems (Code, Generation, Name, Price) "\
	"VALUES (?, ?, ?, ?)"

#define SQL_MDB_PUT_MENUEXTRA \
	"INSERT INTO menuextras (MID, Qty, Code) "\
	"SELECT ID, ?, ? FROM menuitems "\
	"WHERE Code = ? and Generation = "\
		"(SELECT max(Generation) FROM menuitems WHERE Code = ?)"

#define SQL_MDB_PUT_MENUSTYLE \
	"INSERT INTO menustyles (MID, Style) "\
	"SELECT ID, ? FROM menuitems "\
	"WHERE Code = ? and Generation = "\
		"(SELECT max(Generation) FROM menuitems WHERE Code = ?)"

#define SQL_MDB_PUT_MENUSUBITEM \
	"INSERT INTO menusubitems (MID, Qty, Code) "\
	"SELECT ID, ?, ? FROM menuitems "\
	"WHERE Code = ? and Generation = "\
		"(SELECT max(Generation) FROM menuitems WHERE Code = ?)"

#define SQL_MDB_UPD_MENUITEM \
	"UPDATE menuitems SET Code = ?, Generation = ?, Name = ?, Price = ? "\
	"WHERE Code = ? AND Generation = ?"

#define SQL_MDB_UPD_MENUSTYLE \
	"UPDATE menustyles SET Style = ? "\
	"WHERE ID = ?"

#define SQL_MDB_UPD_MENUEXTRA \
	"UPDATE menuextras SET Qty = ?, Code = ? "\
	"WHERE ROWID = (SELECT ROWID FROM menuextras WHERE MID = "\
			"(SELECT ID FROM menuitems WHERE Code = ? AND Generation = "\
				"(SELECT max(Generation) FROM menuitems WHERE Code = ?)) "\
		"AND Code = ? AND Qty = ?)"

#define SQL_MDB_UPD_MENUSUBITEM \
	"UPDATE menusubitems SET Qty = ?, Code = ? "\
	"WHERE ROWID = (SELECT ROWID FROM menusubitems WHERE MID = "\
			"(SELECT ID FROM menuitems WHERE Code = ? AND Generation = "\
				"(SELECT max(Generation) FROM menuitems WHERE Code = ?)) "\
		"AND Code = ? AND Qty = ?)"

#define SQL_MDB_DEL_MENUITEM \
	"UPDATE menuitems SET Deleted = 1 "\
	"WHERE Code = ? AND Generation = "\
		"(SELECT max(Generation) FROM menuitems WHERE Code = ?)"

#define SQL_MDB_DEL_MENUSTYLE \
	"UPDATE menustyles SET MID = 0 "\
	"WHERE ID = ?"

#define SQL_MDB_DEL_MENUEXTRA \
	"UPDATE menuextras SET MID = 0 "\
	"WHERE ROWID = (SELECT ROWID FROM menuextras WHERE MID = "\
			"(SELECT ID FROM menuitems WHERE Code = ? AND Generation = "\
				"(SELECT max(Generation) FROM menuitems WHERE Code = ?)) "\
		"AND Code = ? AND Qty = ?)"

#define SQL_MDB_DEL_MENUSUBITEM \
	"UPDATE menusubitems SET MID = 0 "\
	"WHERE ROWID = (SELECT ROWID FROM menusubitems WHERE MID = "\
			"(SELECT ID FROM menuitems WHERE Code = ? AND Generation = "\
				"(SELECT max(Generation) FROM menuitems WHERE Code = ?)) "\
		"AND Code = ? AND Qty = ?)"

/*
 * ORDER DATABASE QUERIES
 */

#define SQL_ODB_GET_ORDER \
	"SELECT orders.Date, customers.Phone, orders.Type, orders.Special, "\
		"orders.Paytype, orders.Subtotal, orders.Discount, orders.Delivery, "\
		"orders.Credit, orders.Tax1, orders.Tax2, orders.Total "\
	"FROM orders, customers "\
	"WHERE orders.ID = ? AND orders.CID = customers.ID"

#define SQL_ODB_GET_LASTORDER \
	"SELECT orders.OID FROM orders, customers "\
	"WHERE orders.CID = customers.ID AND customers.Phone = ? "\
	"ORDER BY orders.Date DESC "\
	"LIMIT 1"

#define SQL_ODB_GET_DAILY_TOTAL \
	"SELECT Subtotal, Discount, Delivery, Credit, Tax1, Tax2, Total "\
	"FROM orders WHERE Date > ? AND Date < ?"

#define SQL_ODB_GET_DAILY_LIST \
	"SELECT ID FROM orders WHERE Date > ? AND Date < ?"

#define SQL_ODB_GET_TIME \
	"SELECT Date FROM orders WHERE ID = ?"

/*
 * LEFT OUTER JOIN inserts the menustyle ID when it matches the text style
 * stored with the orderitem.
 */
#define SQL_ODB_GET_ITEMS \
	"SELECT orderitems.Type, orderitems.Qty, menuitems.Code, "\
		"menustyles.ID, orderitems.Price, orderitems.Special "\
	"FROM orderitems, menuitems "\
		"LEFT OUTER JOIN menustyles "\
			"ON orderitems.MID = menustyles.MID "\
			"AND orderitems.Style = menustyles.Style "\
	"WHERE orderitems.OID = ? "\
		"AND orderitems.MID = menuitems.ID "\
	"ORDER BY orderitems.Line ASC, orderitems.Type"

#define SQL_ODB_PUT_ORDER \
	"INSERT INTO orders (CID, Type, Special, Paytype, Subtotal, Discount, "\
		"Delivery, Credit, Tax1, Tax2, Total) "\
	"SELECT ID, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "\
	"FROM customers "\
	"WHERE customers.Phone = ? "\
	"LIMIT 1"

/*
 * LEFT OUTER JOIN inserts the menustyles text for preservation.  The ITEM
 * query also inserts the current price.
 */
#define SQL_ODB_PUT_ITEM \
	"INSERT INTO orderitems (OID, Line, Type, MID, Qty, Style, Price, "\
		"Special) "\
	"SELECT ?, ?, ?, menuitems.ID, ?, menustyles.Style, menuitems.Price, ? "\
		"FROM menuitems "\
			"LEFT OUTER JOIN menustyles "\
				"ON menuitems.ID = menustyles.MID "\
				"AND menustyles.ID = ? "\
		"WHERE menuitems.code = ? AND menuitems.Generation = "\
			"(SELECT max(Generation) FROM menuitems WHERE Code = ?)"

#define SQL_ODB_PUT_SUBITEM \
	"INSERT INTO orderitems (OID, Line, Type, MID, Qty, Style, Price, "\
		"Special) "\
	"SELECT ?, ?, ?, menuitems.ID, ?, menustyles.Style, ?, ? "\
		"FROM menuitems "\
			"LEFT OUTER JOIN menustyles "\
				"ON menuitems.ID = menustyles.MID "\
				"AND menustyles.ID = ? "\
		"WHERE menuitems.code = ? AND menuitems.Generation = "\
			"(SELECT max(Generation) FROM menuitems WHERE Code = ?)"

#define SQL_ODB_PUT_PAYMENT \
	"INSERT INTO orderccpayments (OID, Number, Expiry) "\
	"VALUES (?, ?, ?)"

#define SQL_ODB_GET_PAYMENT \
	"SELECT Number, Expiry "\
	"FROM orderccpayments "\
	"WHERE OID = ?"

enum sql_type {
	SQL_CDB,
	SQL_MDB,
	SQL_ODB
};

struct sql_arg {
	sqlite3 *db;
	char *errmsg;
	void (*err)(const char *, va_list); 
	enum sql_type dbtype;
	union {
		struct {
			sqlite3_stmt *get_cust;
			sqlite3_stmt *put_cust;
			sqlite3_stmt *upd_cust;
			sqlite3_stmt *get_cred;
			sqlite3_stmt *get_cred_phone;
			sqlite3_stmt *upd_cred;
			sqlite3_stmt *put_cred;
		} cdb;
		struct {
			sqlite3_stmt *get_menuitem;
			sqlite3_stmt *get_menustyles;
			sqlite3_stmt *get_menuextras;
			sqlite3_stmt *get_menusubitems;
		} mdb;
		struct {
			sqlite3_stmt *get_order;
			sqlite3_stmt *get_lastorder;
			sqlite3_stmt *get_items;
			sqlite3_stmt *put_order;
			sqlite3_stmt *put_item;
			sqlite3_stmt *put_subitem;
			sqlite3_stmt *put_payment;
		} odb;
	} q;
};

struct sql_arg * sql_makearg(enum sql_type);
void sql_err(struct sql_arg *, const char *, ...)
	__attribute__((__format__(printf, 2, 3)));
void sql_warn(struct sql_arg *arg, const char *emsg, ...)
	__attribute__((__format__(printf, 2, 3)));
int sql_open(struct sql_arg *, const char *);
int sql_exec(struct sql_arg *, const char *, 
		int (*)(void *, int, char **, char **), void *);
int sql_printf(char **, const char *, ...);		/* cannot be printf format */
char * sql_strdup(const char *);
const char * sql_geterr(struct sql_arg *);
void sql_free(char *);
void sql_freearg(struct sql_arg *);
void sql_trace(void *, const char *);

/* typecast the void arg to our internal type */
#define SQLARG(a)	((struct sql_arg *)((a)->arg))

/* YYYY-MM-DD HH:MM:SS */
#define SQL_TIME_FMT	"%Y-%m-%d %T"

#define SQL_RETRY_MAX		9
#define SQL_RETRY_SCALE		100000			/* 9 retries == 4.5s */
#define SQL_RETRY_MAXWAIT	999999			/* usleep only sleeps <1sec */

/* helpful free/null macro */
#define SQLFREE(a)	do {	\
	sql_free(a);			\
	(a) = NULL;				\
} while (0)

/* Check that the internal arg is safe */
#define CHECKSQL(a) do {			\
	if (SQLARG(a)->db == NULL) {	\
		errno = EBADF;				\
		return -1;					\
	}								\
} while (0)

void str2phone(const char *, ChopstixPhone *);
void str2phoneext(const char *, ChopstixPhone *);
char * phone2str(const ChopstixPhone *);

/*
 * EXTERNAL MODULE HOOKS
 */
void odb_init(struct orderdb_functions *);
void cdb_init(struct custdb_functions *);
void mdb_init(struct menudb_functions *);

#endif
