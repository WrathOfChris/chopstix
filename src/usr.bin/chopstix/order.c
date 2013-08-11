/* $Gateweaver: order.c,v 1.61 2007/09/27 20:03:02 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <curses.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: order.c,v 1.61 2007/09/27 20:03:02 cmaxwell Exp $");

/* database interface */
struct orderdb_handle * orderdb_open(const char *);
struct orderdb_handle * orderdb_create(const char *);
int orderdb_read(struct orderdb_handle *, ChopstixOrder *);
int orderdb_readkey(struct orderdb_handle *, ChopstixOrderKey);
int orderdb_rewind(struct orderdb_handle *);
int orderdb_get(struct orderdb_handle *, const ChopstixOrderKey,
		ChopstixOrder *);
int orderdb_getlast(struct orderdb_handle *, const ChopstixPhone *,
		ChopstixOrder *);
int orderdb_getdaily_total(struct orderdb_handle *, ChopstixTotal *);
int orderdb_put(struct orderdb_handle *, const ChopstixOrder *, int *,
		time_t *);
int orderdb_update(struct orderdb_handle *, const ChopstixOrder *);
int orderdb_remove(struct orderdb_handle *, const ChopstixOrderKey);
int orderdb_close(struct orderdb_handle *);
const char *orderdb_geterr(struct orderdb_handle *);

static struct orderdb_functions odbf;
static struct orderdb_handle *odbh;

static ChopstixSubItems * order_add_subitems(ChopstixOrderItem *);
static int order_validate_style(const char *, int *, void *);
static ChopstixFormAction order_putdata(const char *, void *);
const char * order_get_styletext(ChopstixItemStyles *, int);
static int order_match_style(const char *, ChopstixItemCode, char **);

static int did_codechange(const char *, const ChopstixItemCode);

/* globals */
ChopstixOrder order;
ChopstixForm orderform;
static int ordertop;

enum orderformtype {
	OFTYPE_NONE = 0,
	OFTYPE_EMPTY,
	OFTYPE_ITEM,
	OFTYPE_STYLE,
	OFTYPE_SPECIAL,
	OFTYPE_SUBITEM,
	OFTYPE_SUBSTYLE,
	OFTYPE_SUBSPECIAL
};

enum orderformstate {
	STATE_NUM = 0,
	STATE_QTY,
	STATE_CODE,
	STATE_TEXT,
	STATE_MONEY
};

#define STATE_QTY_NONE		INT_MIN
#define STATE_CODE_NONE		((char *)-1)
#define STATE_TEXT_NONE		((char *)-1)
#define STATE_MONEY_NONE	INT_MIN

/*
 * NOTE: There is no more room in this inn!  If any more bits need to be kept,
 *       I'm going to have to break down and mallocate a passable structure.
 *
 * bits	name			range	shift
 * ----	----			-----	-----
 * 3:	orderformtype	(0-7)	29
 * 3:	orderformstate	(0-7)	26
 * 8:	on-screen field	(0-255)	18
 * 12:	order index		(0-4095) 6
 * 6:	subitem index	(0-63)	 0
 */
#define SETOFA(o, t, s, f, i, si) do {	\
	(o) = ((t) & 0x07) << 29;			\
	(o) += ((s) & 0x07) << 26;			\
	(o) += ((f) & 0xFF) << 18;			\
	(o) += ((i) & 0x0FFF) << 6;			\
	(o) += ((si) & 0x3F);				\
} while (0)

#define GETOFA_TYPE(o)		(((o) >> 29) & 0x07)
#define GETOFA_STATE(o)		(((o) >> 26) & 0x07)
#define GETOFA_FIELD(o)		(((o) >> 18) & 0xFF)
#define GETOFA_INDEX(o)		(((o) >> 6) & 0x0FFF)
#define GETOFA_SUBINDEX(o)	((o) & 0x3F)
#define OFA_INDEX_MAX		0x0FFF
#define OFA_SUBINDEX_MAX	0x3F

static ChopstixMenuitem *
order_getmenuitem(char *code)
{
	return menu_getitem(code);
}

void
order_init(void)
{
	/* database init */
	bzero(&odbf, sizeof(odbf));
	odbf.err = &status_dberr;
	odbf.getmenuitem = &order_getmenuitem;

	/* use orderdb module */
	module_init_order(&odbf);

	if ((odbh = orderdb_open(config.database.orderdb)) == NULL) {
		status_warn("cannot open order database, see system log for details");
		chopstix_exit();
	}

	/* init the current order global */
	bzero(&order, sizeof(order));

	/* Order Form */
	bzero(&orderform, sizeof(orderform));
	TAILQ_INIT(&orderform.fields);
	if ((orderform.labelsep = strdup("")) == NULL)
		err(1, "allocating form");

	/* wipe and set up any required fields */
	order_new();

	/* init just a single row for now */
	order_reinit(LINE_ORDER_SIZE - 1, NULL);
	order_refresh(1);
}

void
order_reinit(int fnum, int *otop)
{
	int olines = 0;
	ChopstixFormField *ff;
	uint32_t ofa, oftype;

	if (fnum < 0)
		fnum = 0;

	TAILQ_FOREACH(ff, &orderform.fields, entry) {
		if (ff->x == 0)
			olines++;
		
		/* fixup the coords */
		switch (GETOFA_STATE((uint32_t)ff->arg)) {
			case STATE_NUM:
			case STATE_QTY:
			case STATE_CODE:
				break;
			case STATE_TEXT:
				form_field_setiw(ff, COLS - 18 - ALIGN_MONEY_MAX);
				break;
			case STATE_MONEY:
				ff->x = COLS - ALIGN_MONEY_MAX;
				break;
		}
	}

	/* Add lines */
	while (olines < fnum) {
		oftype = OFTYPE_NONE;

		if ((ff = form_field_new(NULL)) == NULL)
			err(1, "allocating fields");
		form_field_add(&orderform, ff);
		form_field_setyx(ff, LINE_ORDER + 2 + olines, 0);
		form_field_setiw(ff, 5);
		SETOFA(ofa, oftype, STATE_NUM, olines, OFA_INDEX_MAX, OFA_SUBINDEX_MAX);
		form_field_setarg(ff, (void *)ofa);
		form_field_setdisplayonly(ff, 1);
		ff->putdata = &order_putdata;

		if ((ff = form_field_new(NULL)) == NULL)
			err(1, "allocating fields");
		form_field_add(&orderform, ff);
		form_field_setyx(ff, LINE_ORDER + 2 + olines, 6);
		form_field_setiw(ff, 5);
		SETOFA(ofa, oftype, STATE_QTY, olines, OFA_INDEX_MAX, OFA_SUBINDEX_MAX);
		form_field_setarg(ff, (void *)ofa);
		ff->putdata = &order_putdata;
		ff->validate = &form_field_validate_number;

		if ((ff = form_field_new(NULL)) == NULL)
			err(1, "allocating fields");
		form_field_add(&orderform, ff);
		form_field_setyx(ff, LINE_ORDER + 2 + olines, 12);
		form_field_setiw(ff, 5);
		SETOFA(ofa, oftype, STATE_CODE, olines, OFA_INDEX_MAX, OFA_SUBINDEX_MAX);
		form_field_setarg(ff, (void *)ofa);
		ff->putdata = &order_putdata;

		if ((ff = form_field_new(NULL)) == NULL)
			err(1, "allocating fields");
		form_field_add(&orderform, ff);
		form_field_setyx(ff, LINE_ORDER + 2 + olines, 18);
		form_field_setiw(ff, COLS - 18 - ALIGN_MONEY_MAX);	/* XXX */
		SETOFA(ofa, oftype, STATE_TEXT, olines, OFA_INDEX_MAX, OFA_SUBINDEX_MAX);
		form_field_setarg(ff, (void *)ofa);
		form_field_setdisplayonly(ff, 1);
		ff->putdata = &order_putdata;

		if ((ff = form_field_new(NULL)) == NULL)
			err(1, "allocating fields");
		form_field_add(&orderform, ff);
		form_field_setyx(ff, LINE_ORDER + 2 + olines,
				COLS - ALIGN_MONEY_MAX);					/* XXX */
		form_field_setiw(ff, ALIGN_MONEY_MAX);
		SETOFA(ofa, oftype, STATE_MONEY, olines, OFA_INDEX_MAX, OFA_SUBINDEX_MAX);
		form_field_setarg(ff, (void *)ofa);
		form_field_setrightalign(ff, 1);
		ff->getstr = &form_field_getstr_money;
		ff->validate = &form_field_validate_money;
		ff->getcpos = &form_field_getcpos_money;
		ff->putdata = &order_putdata;

		/* increment lines counter */
		olines++;
	}

	/* Delete lines */
	while (olines > fnum) {
		if ((ff = getfield_cur(&orderform)) == NULL)
			break;

		/* shift ordertop */
		if (ff->y - LINE_ORDER - LINE_ORDER_MINSIZE >= fnum)
			ordertop++;

		/* delete the last line */
		while ((ff = TAILQ_LAST(&orderform.fields, ChopstixFormField_tq))) {
			--orderform.cur;
			form_field_del(&orderform, ff);
			if (ff->x == 0) {
				form_field_free(ff);
				break;
			}
			form_field_free(ff);
		}

		/* decrement lines counter */
		olines--;
	}

	/* reset ordertop */
	if (otop)
		*otop = ordertop;
}

static void
order_formset(int fnum, int index, int subindex, enum orderformtype ftype,
		int qty, const char *code, const char *text, int price)
{
	ChopstixFormField *ff;
	char number[ALIGN_NUMBER + 1];
	uint32_t ofa;

	TAILQ_FOREACH(ff, &orderform.fields, entry) {
		/*
		 * this is the group of fields we're looking for.  Lets game the input
		 * since that's what's expected of us
		 */
		if (GETOFA_FIELD((uint32_t)ff->arg) == fnum) {

			/* update the ROW type */
			SETOFA(ofa, ftype,
					GETOFA_STATE((uint32_t)ff->arg),
					GETOFA_FIELD((uint32_t)ff->arg),
					index,
					subindex);
			form_field_setarg(ff, (void *)ofa);

			/* this is the FIELD type */
			switch (GETOFA_STATE((uint32_t)ff->arg)) {

				case STATE_NUM:
					/* Item number is always displayonly */
					if ((ftype == OFTYPE_EMPTY
								&& GETOFA_INDEX((uint32_t)ff->arg)
								< OFA_INDEX_MAX)
							|| ftype == OFTYPE_ITEM) {
						snprintf(number, sizeof(number), "%d.",
								GETOFA_INDEX((uint32_t)ff->arg) + 1);
						if (!stracpy(&ff->input, number))
							status_warn("cannot set item number %d",
									GETOFA_INDEX((uint32_t)ff->arg) + 1);
					} else
						stracpy(&ff->input, "");
					break;

				case STATE_QTY:
					/* Only editable on EMPTY, ITEM, SUBITEM */
					if (ftype == OFTYPE_EMPTY
							|| ftype == OFTYPE_ITEM
							|| ftype == OFTYPE_SUBITEM)
						form_field_setdisplayonly(ff, 0);
					else
						form_field_setdisplayonly(ff, 1);

					if (qty == STATE_QTY_NONE)
						break;

					snprintf(number, sizeof(number), "%d", qty);
					if (qty != -1) {
						if (!stracpy(&ff->input, number))
							status_warn("cannot set item qty %d", qty);
					} else
						stracpy(&ff->input, "");
					break;

				case STATE_CODE:
					/* Only editable on EMPTY, ITEM, SUBITEM */
					if (ftype == OFTYPE_EMPTY
							|| ftype == OFTYPE_ITEM
							|| ftype == OFTYPE_SUBITEM)
						form_field_setdisplayonly(ff, 0);
					else
						form_field_setdisplayonly(ff, 1);

					if (code == STATE_CODE_NONE)
						break;

					if (code) {
						if (!stracpy(&ff->input, code))
							status_warn("cannot set item code %s", code);
					} else
						stracpy(&ff->input, "");
					break;

				case STATE_TEXT:
					/* Only editable on STYLE, SPECIAL, SUBSTYLE, SUBSPECIAL */
					if (ftype == OFTYPE_STYLE || ftype == OFTYPE_SPECIAL
							|| ftype == OFTYPE_SUBSTYLE
							|| ftype == OFTYPE_SUBSPECIAL)
						form_field_setdisplayonly(ff, 0);
					else
						form_field_setdisplayonly(ff, 1);

					if (ftype == OFTYPE_STYLE || ftype == OFTYPE_SUBSTYLE) {
						ff->hotfield = 1;
						ff->validate = &order_validate_style;
					} else {
						ff->hotfield = 0;
						ff->validate = &form_field_validate_default;
					}

					if (text == STATE_TEXT_NONE)
						break;

					if (text) {
						if (!stracpy(&ff->input, text))
							status_warn("cannot set item text \"%s\"", text);
					} else
						stracpy(&ff->input, "");
					break;

				case STATE_MONEY:
					/* Only editable on SUBITEM */
					if (ftype == OFTYPE_SUBITEM)
						form_field_setdisplayonly(ff, 0);
					else
						form_field_setdisplayonly(ff, 1);

					if (price == STATE_MONEY_NONE)
						break;

					snprintf(number, sizeof(number), "%d", price);
					if (!stracpy(&ff->input, number))
						status_warn("cannot set item price %d", price);
					break;
			}

			/* refresh the cpos based on gamed input */
			ff->cpos = ff->getcpos(ff->input.s, ff->cpos, FIELD_POS_NONE,
					ff->arg);
		}

		if (GETOFA_TYPE((uint32_t)ff->arg) == OFTYPE_NONE)
			form_field_setdisplayonly(ff, 1);
	}
	return;
}

static void
order_formrunoff(int fnum)
{
	ChopstixFormField *ff;
	uint32_t ofa;

	TAILQ_FOREACH(ff, &orderform.fields, entry) {
		if (GETOFA_FIELD((uint32_t)ff->arg) >= fnum) {
			SETOFA(ofa,
					OFTYPE_NONE,
					GETOFA_STATE((uint32_t)ff->arg),
					GETOFA_FIELD((uint32_t)ff->arg),
					OFA_INDEX_MAX,
					OFA_SUBINDEX_MAX);
			form_field_setarg(ff, (void *)ofa);
			form_field_setdisplayonly(ff, 1);
			stracpy(&ff->input, "");
		}
	}
}

#define PREVLINE(ff, form, idx, sidx)								\
do {																\
	if ((ff) == TAILQ_FIRST(&(form)->fields))						\
		break;														\
	else															\
		--((form)->cur);											\
} while (((ff) = getfield_cur((form)))								\
			&& GETOFA_INDEX((uint32_t)((ff)->arg)) == (idx)			\
			&& ((sidx) == -1										\
				|| GETOFA_SUBINDEX((uint32_t)((ff)->arg)) == (sidx)))

#define NEXTLINE(ff, form, idx, sidx)								\
do {																\
	if (TAILQ_NEXT((ff), entry) == TAILQ_END(&(form)->fields))		\
		break;														\
	else															\
		++((form)->cur);											\
} while (((ff) = getfield_cur((form)))								\
			&& GETOFA_INDEX((uint32_t)((ff)->arg)) == (idx)			\
			&& ((sidx) == -1										\
				|| GETOFA_SUBINDEX((uint32_t)((ff)->arg)) == (sidx)))

void
order_edit_special(ChopstixFormField *ff, int make)
{
	unsigned index, subindex;
	enum orderformtype type;

	if (ff == NULL)
		return;

	index = GETOFA_INDEX((uint32_t)ff->arg);
	subindex = GETOFA_SUBINDEX((uint32_t)ff->arg);
	type = GETOFA_TYPE((uint32_t)ff->arg);

	/* if the line is below the bottow, insert for the last line */
	if (index == OFA_INDEX_MAX && make == 1) {

		/* move to the last field of the previous item line */
		PREVLINE(ff, &orderform, index, -1);
		if (GETOFA_INDEX((uint32_t)ff->arg) == OFA_INDEX_MAX)
			return;

		index = GETOFA_INDEX((uint32_t)ff->arg);
		subindex = GETOFA_SUBINDEX((uint32_t)ff->arg);
		type = GETOFA_TYPE((uint32_t)ff->arg);
	}

	if (index != OFA_INDEX_MAX && index < order.items.len) {

		if (type == OFTYPE_ITEM || type == OFTYPE_STYLE
				|| type == OFTYPE_SPECIAL) {
			if (make == 0) {
				if (order.items.val[index].special) {
					free(order.items.val[index].special);
					order.items.val[index].special = NULL;
				} else {
					/* fake delete - offset the FIELD_NEXT coming up */
					form_driver(&orderform, FIELD_PREV);
				}
				return;
			} else {
				if (order.items.val[index].special == NULL) {
					if ((order.items.val[index].special = calloc(1,
									sizeof(*order.items.val[index].special)))
							== NULL)
						return;
					*order.items.val[index].special = strdup("");
				}
			}

			/* advance to first field of next line */
			NEXTLINE(ff, &orderform, index, subindex);
		}

		if (subindex != OFA_SUBINDEX_MAX
				&& order.items.val[index].subitems != NULL
				&& subindex < order.items.val[index].subitems->len) {
			if (type == OFTYPE_SUBITEM || type == OFTYPE_SUBSTYLE
					|| type == OFTYPE_SUBSPECIAL) {
				if (make == 0) {
					if (order.items.val[index].subitems->val[subindex].special) {
						free(order.items.val[index].subitems->val[subindex].special);
						order.items.val[index].subitems->val[subindex].special = NULL;
					} else {
						/* fake delete - offset the FIELD_NEXT coming up */
						form_driver(&orderform, FIELD_PREV);
					}
					return;
				} else {
					if (order.items.val[index].subitems->val[subindex].special
							== NULL) {
						if ((order.items.val[index].subitems->val[subindex].special
									= calloc(1,
										sizeof(*order.items.val[index].subitems->val[subindex].special)))
								== NULL)
							return;
						*order.items.val[index].subitems->val[subindex].special = strdup("");
					}
				}
			}

			/* advance to first field of next line */
			NEXTLINE(ff, &orderform, index, subindex);
		}
	}
}

/*
 * Return the number of form lines this item will occupy.
 *
 * XXX any code change here needs to be changed in otop2item()
 */
static int
item_getlines(ChopstixOrderItem *item)
{
	ChopstixMenuitem *mi;
	int lines = 1;
	unsigned u;

	/* lookup real menu item, without touching subitems */
	if ((mi = menu_getitem(item->code)) != NULL)
		if (mi->styles.len > 0)
			lines++;

	/* special info for this item */
	if (item->special)
		lines++;

	if (item->subitems)
		for (u = 0; u < item->subitems->len; u++) {

			/* get the subitem menu item */
			if ((mi = menu_getitem(item->subitems->val[u].code)) == NULL) {
				status_warn("Item \"%s\" contains subitem \"%s\" not on menu",
						item->code, item->subitems->val[u].code);
				continue;
			}

			/* its a valid menu item, add a line */
			lines++;

			/* subitem has a style */
			if (mi->styles.len > 0)
				lines++;

			/* 
			 * DO NOT TOUCH THE SUBITEMS HERE AGAIN
			 *
			 * All subitems are already part of the orderitem.
			 */

			if (item->subitems->val[u].special)
				lines++;
		}

	return lines;
}

int
order_getlines(ChopstixOrderItems *items)
{
	unsigned u;
	int lines = 0;

	for (u = 0; u < items->len; u++)
		lines += item_getlines(&items->val[u]);

	return lines;
}

int
order_getlast(const ChopstixPhone *phone, ChopstixOrder *order)
{
	ChopstixOrder lastorder;
	bzero(&lastorder, sizeof(lastorder));

	if (orderdb_getlast(odbh, phone, &lastorder) == -1)
		return -1;
	free_ChopstixOrderKey(&order->key);
	free_ChopstixPhone(&order->customer.phone);
	free_ChopstixTime(&order->date);
	free_CHOPSTIX_ORDERTYPE(&order->type);
	free_ChopstixPayment(&order->payment);
	free_ChopstixOrderItems(&order->items);
	free_ChopstixSpecial(&order->special);
	free_ChopstixOrderItems(&order->ruleitems);
	free_ChopstixTotal(&order->total);

	if (copy_ChopstixOrderKey(&lastorder.key, &order->key)) goto fail;
	if (copy_ChopstixPhone(&lastorder.customer.phone, &order->customer.phone)) goto fail;
	if (copy_ChopstixTime(&lastorder.date, &order->date)) goto fail;
	if (copy_CHOPSTIX_ORDERTYPE(&lastorder.type, &order->type)) goto fail;
	if (copy_ChopstixPayment(&lastorder.payment, &order->payment)) goto fail;
	if (copy_ChopstixOrderItems(&lastorder.items, &order->items)) goto fail;
	if (copy_ChopstixSpecial(&lastorder.special, &order->special)) goto fail;
	if (copy_ChopstixOrderItems(&lastorder.ruleitems, &order->ruleitems)) goto fail;
	if (copy_ChopstixTotal(&lastorder.total, &order->total)) goto fail;
	order->total.disctype = DISCOUNT_RULE;
	order->total.delitype = DISCOUNT_RULE;
	order_tally(order);

	free_ChopstixOrder(&lastorder);
	return 0;

fail:
	free_ChopstixOrder(&lastorder);
	return -1;
}

int
order_getdaily_total(void)
{
	ChopstixTotal total;

	if (orderdb_getdaily_total(odbh, &total) == -1)
		return -1;
	window_update_daily(&total);

	return 0;
}

/*
 * Convert (otop) to the specific part of the order that should be displayed
 * otop		order top display line
 * inum		item number
 * style	item style line
 * special	item special line
 * isub		item subitem
 *
 * isub returns as -1 when not used
 *
 * XXX any code change here needs to be changed in item_getlines()
 */
static void
otop2item(ChopstixOrderItems *items, int otop, int *inum, int *style,
		int *special, int *isub)
{
	ChopstixOrderItem *item;
	ChopstixMenuitem *mi;
	unsigned u;
	int oline = 0, ilines;

	*inum = items->len;
	*style = 0;
	*special = 0;
	*isub = -1;

	if (oline == otop)
		return;

	for (u = 0; u < items->len; u++) {
		item = &items->val[u];

		/* get the number of display lines for this item */
		ilines = item_getlines(item);

		/* jump ahead item by item if possible */
		if ((oline + ilines) < otop) {
			oline += ilines;
			continue;
		}

		/* this is our item */
		*inum = u;
		oline++;

		/* Item line */
		if (oline == otop)
			break;

		if ((mi = menu_getitem(item->code)) != NULL)
			if (mi->styles.len > 0)
				oline++;

		/* Style line */
		if (oline == otop) {
			*style = 1;
			break;
		}

		if (item->special)
			oline++;

		/* Special line */
		if (oline == otop) {
			*special = 1;
			break;
		}

		if (item->subitems) {
			for (u = 0; u < item->subitems->len; u++) {
				if ((mi = menu_getitem(item->subitems->val[u].code)) != NULL) {
					oline++;
					*isub = u;

					/* Subitem line */
					if (oline == otop)
						break;

					if (mi->styles.len > 0)
						oline++;

					/* Subitem Style */
					if (oline == otop) {
						*style = 1;
						break;
					}

					if (item->subitems->val[u].special)
						oline++;

					/* Subitem Special */
					if (oline == otop) {
						*special = 1;
						break;
					}
				}
			}

			/* bail the outer for loop as well */
			if (oline == otop)
				break;
		}
	}
}

/* 
 * Fixup number of displayable form lines.  This has to take a parameter from
 * the display driver to instruct how many form lines need to be filled in.
 *
 * Top is the LOGICAL top row to be displayed.
 */
void
order_refresh(int otop)
{
	ChopstixOrderItem *item;
	ChopstixMenuitem *mi;
	ChopstixSubItem *subitem;
	int o_inum, o_isub, o_style, o_special;
	int f = 0;
	int u_item, u_sub;
	char *iname;
	int64_t price;

	/* Save the top, in case data entry needs to refresh the form */
	ordertop = otop;

	otop2item(&order.items, otop, &o_inum, &o_style, &o_special, &o_isub);

	/* start wherever otop tells us to start */
	for (u_item = o_inum; u_item < order.items.len; u_item++) {
		item = &order.items.val[u_item];

		/* no need to go any further */
		if (f > LINE_ORDER_SIZE)
			return;

		if ((mi = menu_getitem(item->code)) != NULL) {
			iname = mi->name;
			price = mi->price;
		} else {
			iname = "";
			price = 0;
		}

		/* first pass through may be partial based on flags */
		if (u_item == o_inum && o_isub == -1) {
			if (o_style == 0 && o_special == 0) {
				order_formset(f, u_item, OFA_SUBINDEX_MAX, OFTYPE_ITEM,
						item->qty, item->code, iname,
						ROLLOVER(price * item->qty));
				f++;
			}
			if (o_style == 1 || (o_style == 0 && o_special == 0))
				if (mi != NULL && mi->styles.len > 0) {
					order_formset(f, u_item, OFA_SUBINDEX_MAX, OFTYPE_STYLE,
							-1, NULL,
							order_get_styletext(&mi->styles, item->style), 0);
					f++;
				}
			if (o_special == 1 || (o_style == 0 && o_special == 0))
				if (item->special) {
					order_formset(f, u_item, OFA_SUBINDEX_MAX, OFTYPE_SPECIAL,
							-1, NULL, *item->special, 0);
					f++;
				}
		} else if (u_item == o_inum && o_isub >= 0) {
			/* DO NOTHING */
		} else {
			/* normally, just pump the data in */
			order_formset(f, u_item, OFA_SUBINDEX_MAX, OFTYPE_ITEM,
					item->qty, item->code, iname,
					ROLLOVER(price * item->qty));
			f++;
			if (mi != NULL && mi->styles.len > 0) {
				order_formset(f, u_item, OFA_SUBINDEX_MAX, OFTYPE_STYLE,
						-1, NULL,
						order_get_styletext(&mi->styles, item->style), 0);
				f++;
			}
			if (item->special) {
				order_formset(f, u_item, OFA_SUBINDEX_MAX, OFTYPE_SPECIAL,
						-1, NULL, *item->special, 0);
				f++;
			}
		}

		/* again with the subitems, start wherever otop tells us to start */
		if (item->subitems) {

			/*
			 * Normal processing is triggered by o_isub being -1.  Just
			 * pretend its really zero, but set away the special flags since
			 * passing -1 to an unsigned is really bad.
			 */
			if (o_isub == -1) {
				o_isub = 0;
				o_style = 0;
				o_special = 0;
			}

			for (u_sub = o_isub; u_sub < item->subitems->len; u_sub++) {
				subitem = &item->subitems->val[u_sub];

				if ((mi = menu_getitem(subitem->code)) != NULL)
					iname = mi->name;
				else {
					iname = "";
					continue;
				}

				if (o_isub >= 0) {
					/*
					 * when o_isub is >= 0, this is a partial line based on
					 * flags
					 */
					if (o_style == 0 && o_special == 0) {
						order_formset(f, u_item, u_sub, OFTYPE_SUBITEM,
								subitem->qty, subitem->code, iname,
								subitem->pricedelta);
						f++;
					}
					if (o_style == 1 || (o_style == 0 && o_special == 0))
						if (mi != NULL && mi->styles.len > 0) {
							order_formset(f, u_item, u_sub, OFTYPE_SUBSTYLE,
									-1, NULL, order_get_styletext(&mi->styles,
										subitem->style), 0);
							f++;
						}
					if (o_special == 1 || (o_style == 0 && o_special == 0))
						if (subitem->special) {
							order_formset(f, u_item, u_sub, OFTYPE_SUBSPECIAL,
									-1, NULL, *subitem->special, 0);
							f++;
						}

					/* reset isub for next run */
					o_isub = 0;
					o_style = 0;
					o_special = 0;
				} else {
					/* normal processing */
					order_formset(f, u_item, u_sub, OFTYPE_SUBITEM,
							subitem->qty, subitem->code, iname,
							subitem->pricedelta);
					f++;
					if (mi != NULL && mi->styles.len > 0) {
						order_formset(f, u_item, u_sub, OFTYPE_SUBSTYLE, -1,
								NULL, order_get_styletext(&mi->styles,
									subitem->style), 0);
						f++;
					}
					if (subitem->special && *subitem->special) {
						order_formset(f, u_item, u_sub, OFTYPE_SUBSPECIAL,
								-1, NULL, *subitem->special, 0);
						f++;
					}
				}
			}
		}

		/* o_isub only applies to the FIRST item being loaded */
		o_isub = -1;
		o_style = 0;
		o_special = 0;
	}

	/* and 1 empty row */
	order_formset(f, OFA_INDEX_MAX, OFA_SUBINDEX_MAX,
			OFTYPE_EMPTY, -1, NULL, NULL, 0);
	f++;

	order_formrunoff(f);
}

int
order_new(void)
{
	/* clean up anything in the old order */
	free_ChopstixOrder(&order);
	bzero(&order, sizeof(order));
	order.date = 0;

	/* initialize the customer */
	customer_new(&order.customer);

	return 0;
}

ChopstixOrderItem *
order_add_item(ChopstixOrder *order)
{
	ChopstixOrderItem *oi;

	if ((oi = realloc(order->items.val,
					(order->items.len + 1) * sizeof(ChopstixOrderItem)))
			== NULL) {
		status_warn("cannot add any more items, out of memory");
		return NULL;
	}

	order->items.val = oi;
	order->items.len += 1;

	bzero(&order->items.val[order->items.len - 1],
			sizeof(order->items.val[order->items.len - 1]));
	if ((order->items.val[order->items.len - 1].code = strdup("")) == NULL) {
		/* failure, just decrement the counter, the next ask will downsize */
		order->items.len -= 1;
		return NULL;
	}

	return &order->items.val[order->items.len - 1];
}

/*
 * Load the menu item, and iteratively convert each subitem description to a
 * full order subitem.
 */
static ChopstixSubItems *
order_add_subitems(ChopstixOrderItem *item)
{
	ChopstixSubItems *silist = NULL;
	ChopstixMenuitem *mi;
	unsigned u;

	if ((mi = menu_getitem(item->code)) == NULL)
		return NULL;
	if (mi->subitems == NULL)
		return NULL;

	if ((silist = calloc(1, sizeof(ChopstixSubItems))) == NULL)
		goto fail;
	if ((silist->val = calloc(mi->subitems->len, sizeof(ChopstixSubItem)))
			== NULL)
		goto fail;
	silist->len = mi->subitems->len;

	for (u = 0; u < silist->len; u++) {
		silist->val[u].qty = mi->subitems->val[u].qty * item->qty;
		if (copy_ChopstixItemCode(&mi->subitems->val[u].code,
					&silist->val[u].code))
			goto fail;
	}

	return silist;

fail:
	free_ChopstixSubItems(silist);
	return NULL;
}

/*
 * Update a list of subitems to reflect quantity changes.  This resets any
 * code changes.
 */
static void
order_fixqty_subitems(ChopstixOrderItem *item, int qty)
{
	ChopstixMenuitem *mi;
	unsigned u;

	if (item->subitems == NULL)
		return;
	if ((mi = menu_getitem(item->code)) == NULL)
		return;
	if (mi->subitems == NULL || item->subitems->len != mi->subitems->len)
		return;

	for (u = 0; u < item->subitems->len; u++) {
		if (did_codechange(item->subitems->val[u].code,
					mi->subitems->val[u].code)) {
			free_ChopstixSubItem(&item->subitems->val[u]);
			bzero(&item->subitems->val[u], sizeof(item->subitems->val[u]));
			if (copy_ChopstixItemCode(&mi->subitems->val[u].code,
						&item->subitems->val[u].code))
				return;
		}
		item->subitems->val[u].qty = mi->subitems->val[u].qty * qty;
	}
}

void
order_tally(ChopstixOrder *order)
{
	unsigned u, v;
	ChopstixMenuitem *mi;
	int64_t subtotal, tax1, tax2;

	order->total.subtotal = 0;
	order->total.tax1 = 0;
	order->total.tax2 = 0;
	order->total.total = 0;

	for (u = 0; u < order->items.len; u++) {
		/* skip over zero-coded items */
		if (order->items.val[u].code == 0)
			continue;

		if ((mi = menu_getitem(order->items.val[u].code)) == NULL) {
			/* do not print error if the code field is empty */
			if (order->items.val[u].code && strlen(order->items.val[u].code))
				status_warn("Item %s is not on the menu!",
						order->items.val[u].code);
			continue;
		}
		order->total.subtotal
			+= order->items.val[u].qty * mi->price;

		/* subitems */
		if (order->items.val[u].subitems)
			for (v = 0; v < order->items.val[u].subitems->len; v++) {
				/* skip over zero-coded items */
				if (order->items.val[u].subitems->val[v].code == 0)
					continue;
				order->total.subtotal
					+= order->items.val[u].subitems->val[v].pricedelta;
			}
	}

	/* run the rules processor */
	rule_run(order, 0);

	subtotal = order->total.subtotal;
	switch (order->total.disctype) {
		case DISCOUNT_NONE:
			break;
		case DISCOUNT_PERCENT:
			/* use the taxcalc, since its just a percentage */
			subtotal -= TAXCALC(order->total.subtotal, order->total.discount);
			break;
		case DISCOUNT_DOLLAR:
		case DISCOUNT_RULE:
			subtotal -= order->total.discount;
			break;
	}
	switch (order->total.delitype) {
		case DISCOUNT_NONE:
			break;
		case DISCOUNT_PERCENT:
			/* use the taxcalc, since its just a percentage */
			subtotal += TAXCALC(order->total.subtotal, order->total.delivery);
			break;
		case DISCOUNT_DOLLAR:
		case DISCOUNT_RULE:
			subtotal += order->total.delivery;
			break;
	}
	order->total.credit = MIN(subtotal, customer_getcredit_remain());
	if (order->total.credit > 0)
		subtotal -= order->total.credit;

	tax1 = TAXCALC(subtotal, config.tax1rate);
	order->total.tax1 = ROLLOVER(tax1);
	
	tax2 = TAXCALC(subtotal, config.tax2rate);
	order->total.tax2 = ROLLOVER(tax2);

	order->total.total = ROLLOVER(subtotal + tax1 + tax2);
}

int
order_post(void)
{
	struct timeval tv_start, tv_now, tv_diff;
	int custpost = 0, credval, credtot;
	size_t z;

	if (order.type == 0) {
		status_warn("delivery type must be entered");
		return -1;
	}
	if (order.payment.type == 0) {
		status_warn("payment type must be entered");
		return -1;
	}
	if (order.payment.type == PAYMENT_CREDIT) {
		if (order.payment.ccinfo == NULL
				|| order.payment.ccinfo->number == NULL
				|| strlen(order.payment.ccinfo->number) == 0
				|| order.payment.ccinfo->expiry == NULL
				|| strlen(order.payment.ccinfo->expiry) == 0) {
			status_warn("credit card info must be entered");
			return -1;
		}
	}

	/* final order tally */
	order_tally(&order);

	/* run the rules processor */
	rule_run(&order, 1);

	gettimeofday(&tv_start, NULL);
	if (orderdb_put(odbh, &order, &order.key, &order.date) == -1) {
		status_warn("order could not be posted: %s",
				orderdb_geterr(odbh));
		return -1;
	}
	gettimeofday(&tv_now, NULL);
	timersub(&tv_now, &tv_start, &tv_diff);
	status_set("SUCCESS %lu.%06lus", tv_diff.tv_sec, tv_diff.tv_usec);

	if (order.total.credit > 0) {
		custpost = 1;
		
		if (order.customer.credit == NULL)
			status_warn("credit posted but no credits on record");
		else {
			/* drain credits */
			credtot = order.total.credit;
			for (z = 0; z < order.customer.credit->len && credtot > 0; z++)
				if (order.customer.credit->val[z].remain > 0) {
					credval = MIN(order.customer.credit->val[z].remain,
							credtot);
					order.customer.credit->val[z].remain -= credval;
					credtot -= credval;
				}
		}
	}

	if (order.special && strlen(order.special) > 0
			&& (order.customer.special == NULL
				|| strcmp(order.special, *order.customer.special) != 0)) {
		if (order.customer.special == NULL)
			if ((order.customer.special = calloc(1,
							sizeof(*order.customer.special))) == NULL) {
				status_warn("cannot allocate customer special info");
				return -1;
			}
		free(*order.customer.special);
		if ((*order.customer.special = strdup(order.special)) == NULL) {
			status_warn("cannot copy customer special info");
			return -1;
		}

		custpost = 1;
	}

	if (custpost) {
		/* customer post prints a more detailed error message */
		if (customer_post(&order.customer) == -1)
			return -1;
	}

	return 0;
}

void
order_exit(void)
{
	if (orderdb_close(odbh) == -1)
		status_err("cannot properly close order database");
}

/*
 * Order Database Interface
 */

struct orderdb_handle *
orderdb_open(const char *dbfile)
{
	struct orderdb_handle *oh;

	if (odbf.open == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((oh = calloc(1, sizeof(struct orderdb_handle))) == NULL)
		return NULL;

	if ((oh->dbfile = strdup(dbfile)) == NULL) {
		free(oh);
		return NULL;
	}

	if (odbf.open(oh) == -1) {
		free(oh);
		return NULL;
	}

	return oh;
}

struct orderdb_handle *
orderdb_create(const char *dbfile)
{
	struct orderdb_handle *oh;

	if (odbf.create == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((oh = calloc(1, sizeof(struct orderdb_handle))) == NULL)
		return NULL;

	if ((oh->dbfile = strdup(dbfile)) == NULL) {
		free(oh);
		return NULL;
	}

	if (odbf.create(oh) == -1) {
		free(oh);
		return NULL;
	}

	return oh;
}

int
orderdb_read(struct orderdb_handle *oh, ChopstixOrder *order)
{
	if (oh && odbf.read)
		return odbf.read(oh, order);

	errno = EINVAL;
	return -1;
}

int
orderdb_readkey(struct orderdb_handle *oh, ChopstixOrderKey key)
{
	if (oh && odbf.readkey)
		return odbf.readkey(oh, key);

	errno = EINVAL;
	return -1;
}

int
orderdb_rewind(struct orderdb_handle *oh)
{
	if (oh && odbf.rewind)
		return odbf.rewind(oh);

	errno = EINVAL;
	return -1;
}

int
orderdb_get(struct orderdb_handle *oh, const ChopstixOrderKey key,
		ChopstixOrder *order)
{
	if (oh && odbf.get)
		return odbf.get(oh, key, order);

	errno = EINVAL;
	return -1;
}

int
orderdb_getlast(struct orderdb_handle *oh, const ChopstixPhone *phone,
		ChopstixOrder *order)
{
	if (oh && odbf.getlast)
		return odbf.getlast(oh, phone, order);

	errno = EINVAL;
	return -1;
}

int
orderdb_getdaily_total(struct orderdb_handle *oh, ChopstixTotal *total)
{
	if (oh && odbf.getdaily_total)
		return odbf.getdaily_total(oh, total);

	errno = EINVAL;
	return -1;
}

int
orderdb_put(struct orderdb_handle *oh, const ChopstixOrder *order, int *oid,
		time_t *otime)
{
	if (oh && odbf.put)
		return odbf.put(oh, order, oid, otime);

	errno = EINVAL;
	return -1;
}

int
orderdb_update(struct orderdb_handle *oh, const ChopstixOrder *order)
{
	if (oh && odbf.update)
		return odbf.update(oh, order);

	errno = EINVAL;
	return -1;
}

int
orderdb_remove(struct orderdb_handle *oh, const ChopstixOrderKey key)
{
	if (oh && odbf.remove)
		return odbf.remove(oh, key);

	errno = EINVAL;
	return -1;
}

int
orderdb_close(struct orderdb_handle *oh)
{
	if (oh && odbf.close)
		return odbf.close(oh);

	errno = EINVAL;
	return -1;
}

const char *
orderdb_geterr(struct orderdb_handle *oh)
{
	if (oh && odbf.geterr)
		return odbf.geterr(oh);

	return NULL;
}

static void
parse_moneystr(const char *str, int *money)
{
	int64_t val = 0;
	int neg = 1;	/* set to -1 to negate the value */

	/* a negative sign is permitted anywhere, including inline */
	if (strchr(str, '-') != NULL)
		neg = -1;

	while (*str) {
		if (!(isdigit(*str) || *str == '$' || *str == '.' || *str == '-'))
			return;

		if (isdigit(*str)) {
			val *= 10;
			val += (*str - '0');
		}

		*str++;
	}

	*money = ROLLOVER(val * neg);
}

static int
order_validate_style(const char *str, int *pos, void *arg)
{
	int index = GETOFA_INDEX((uint32_t)arg);
	int subindex = GETOFA_SUBINDEX((uint32_t)arg);
	enum orderformtype type = GETOFA_TYPE((uint32_t)arg);
	ChopstixItemCode code = NULL;

	if (type == OFTYPE_STYLE && index < order.items.len)
		code = order.items.val[index].code;
	else if (type == OFTYPE_SUBSTYLE && index < order.items.len
			&& order.items.val[index].subitems != NULL
			&& subindex < order.items.val[index].subitems->len)
		code = order.items.val[index].subitems->val[subindex].code;

	if (order_match_style(str, code, NULL) == 0) {
		ChopstixMenuitem *mi;
		unsigned int u;
		char buffer[1024];

		if ((mi = menu_getitem(code))) {
			strlcpy(buffer, "Styles: ", sizeof(buffer));
			for (u = 0; u < mi->styles.len; u++) {
				strlcat(buffer, mi->styles.val[u].name, sizeof(buffer));
				if (u < (mi->styles.len - 1))
					strlcat(buffer, ", ", sizeof(buffer));
			}
			status_set(buffer);
		}

		*pos = 0;
		while (*str && *code && tolower(*str) == tolower(*code)) {
			*pos++;
			*str++;
			*code++;
		}
		return -1;
	}

	return 0;
}

static int
order_update_item(uint32_t arg, ChopstixOrderItem *item)
{
	ChopstixMenuitem *mi;
	int64_t price = 0;
	char *iname = "";
	int ret = -1;

	if ((mi = menu_getitem(item->code)) != NULL) {
		price = item->qty * mi->price;
		iname = mi->name;
		ret = 0;
		if (mi->flags.deleted) {
			status_warn("Item %s (%s) has been deleted from the menu",
					item->code, mi->name);
			return -1;
		}
	}

	order_formset(GETOFA_FIELD(arg),
			GETOFA_INDEX(arg),
			GETOFA_SUBINDEX(arg),
			GETOFA_TYPE(arg),
			item->qty,
			item->code,
			iname,
			ROLLOVER(price));

	return ret;
}

static int 
order_update_subitem(uint32_t arg, ChopstixSubItem *subitem)
{
	ChopstixMenuitem *mi;

	if ((mi = menu_getitem(subitem->code)) == NULL)
		return -1;

	if (mi->flags.deleted) {
		status_warn("Subitem %s (%s) has been deleted from the menu",
				subitem->code, mi->name);
		return -1;
	}

	order_formset(GETOFA_FIELD(arg),
			GETOFA_INDEX(arg),
			GETOFA_SUBINDEX(arg),
			GETOFA_TYPE(arg),
			subitem->qty,
			subitem->code,
			mi->name,
			STATE_MONEY_NONE);

	return 0;
}

static int
order_match_style(const char *str, ChopstixItemCode code, char **styletext)
{
	ChopstixMenuitem *mi;
	unsigned u, m, matches;
	int mchars;
	char *s1, *s2;

	if (styletext)
		*styletext = NULL;

	if ((mi = menu_getitem(code)) == NULL)
		return 0;

	mchars = -1;
	matches = 0;
	m = 0;
	for (u = 0; u < mi->styles.len; u++) {
		(const char *)s1 = str;
		s2 = mi->styles.val[u].name;
		while (*s1 && *s2 && tolower(*s1) == tolower(*s2)) {
			if ((s1 - str) > mchars) {
				mchars = (s1 - str);
				m = u;
				matches = 1;
			} else if ((s1 - str) == mchars)
				matches++;
			*s1++;
			*s2++;
		}
	}

	/* case insensitive shortest prefix matching failed, try the whole thing */
	if (matches == 0 || matches > 1)
		for (u = 0; u < mi->styles.len; u++)
			if (strcasecmp(str, mi->styles.val[u].name) == 0) {
				matches = 1;
				m = u;
				break;
			}

	if (matches == 1) {
		if (styletext)
			*styletext = mi->styles.val[m].name;
		return mi->styles.val[m].num;
	}
	
	/* return the first match, but do not return styletext */
	if (matches > 1)
		for (u = 0; u < mi->styles.len; u++)
			if (strncasecmp(str, mi->styles.val[u].name, strlen(str)) == 0)
				return mi->styles.val[u].num;

	return 0;
}

const char *
order_get_styletext(ChopstixItemStyles *styles, int style)
{
	unsigned u;
	for (u = 0; u < styles->len; u++)
		if (styles->val[u].num == style)
			return styles->val[u].name;

	if (styles->len > 0)
		return styles->val[0].name;
	return "[INVALID]";
}

static ChopstixFormField *
get_formfield(int fieldnum, enum orderformtype type, enum orderformstate state)
{
	ChopstixFormField *ff;

	TAILQ_FOREACH(ff, &orderform.fields, entry)
		if (GETOFA_TYPE((uint32_t)ff->arg) == type
				&& GETOFA_STATE((uint32_t)ff->arg) == state)
			return ff;

	return NULL;
}

static int
did_codechange(const char *c1, const ChopstixItemCode c2)
{
	return strcasecmp(c1, c2);
}

/*
 * Need to discern the order item that this field applies to.  The field number
 * is the 0-based on-screen display number.
 */
static ChopstixFormAction
order_putdata(const char *str, void *arg)
{
	enum orderformstate state = GETOFA_STATE((uint32_t)arg);
	enum orderformtype type = GETOFA_TYPE((uint32_t)arg);
	int field = GETOFA_FIELD((uint32_t)arg);
	int index = GETOFA_INDEX((uint32_t)arg);
	int subindex = GETOFA_SUBINDEX((uint32_t)arg);
	const char *errstr = NULL;
	char *s = NULL;
	int i = 0;
	ChopstixFormField *ff;
	ChopstixFormAction action = FIELD_NEXT;
	ChopstixOrderItem item = {0};
	ChopstixSubItem subitem = {0};

	if (index == OFA_INDEX_MAX) {
		/* empty on empty */
		if (strlen(str) == 0)
			return FIELD_NEXT;

		/* grow the order */
		if (order_add_item(&order) == NULL) {
			status_warn("Cannot add additional items");
			return FIELD_NEXT;
		}
		index = order.items.len - 1;
		type = OFTYPE_ITEM;

		/* update the raw argument, not just the helper counters */
		SETOFA((uint32_t)arg, type, state, field, index, subindex);

		/* cannot formset here, it will change the calling arg */
	}

	if (index >= order.items.len) {
		status_warn("Field index %d exceeds maximum %d",
				index, order.items.len);
		return FIELD_NEXT;
	}

	if (order.items.val[index].subitems != NULL
			&& subindex != OFA_SUBINDEX_MAX
			&& subindex >= order.items.val[index].subitems->len) {
		status_warn("Field subindex %d exceeds maximum %d",
				subindex, order.items.val[index].subitems->len);
		return FIELD_NEXT;
	}

	if (order.items.val[index].subitems == NULL
			&& (type == OFTYPE_SUBITEM || type == OFTYPE_SUBSTYLE
				|| type == OFTYPE_SUBSPECIAL)) {
		status_warn("Subitem access attempted with no subitems");
		return FIELD_NEXT;
	}

	/*
	 * Be lazy, and allow setting of any field at any time.  Hopefully the
	 * displayonly tag will prevent weird thing from happening.
	 */
	switch (state) {

		case STATE_NUM:
			/* ignore the on-screen number */
			break;

		case STATE_QTY:
			if (strlen(str) == 0)
				i = -1;
			else {
				i = strtonum(str, INT_MIN, INT_MAX, &errstr);
				if (errstr) {
					status_warn("Quantity \"%s\" is %s", str, errstr);
					break;
				}
			}
			if (type == OFTYPE_ITEM || type == OFTYPE_EMPTY) {
				/* when quantity changes, update all subitems */
				if (order.items.val[index].qty != i)
					order_fixqty_subitems(&order.items.val[index], i);
				order.items.val[index].qty = i;
				order_update_item((uint32_t)arg, &order.items.val[index]);
			} else if (type == OFTYPE_SUBITEM) {
				order.items.val[index].subitems->val[subindex].qty = i;
				order_update_subitem((uint32_t)arg,
						&order.items.val[index].subitems->val[subindex]);
			}
			break;

		case STATE_CODE:
			if (type == OFTYPE_ITEM || type == OFTYPE_EMPTY) {
				bcopy(&order.items.val[index], &item, sizeof(item));
				if (did_codechange(str, item.code)) {
					(const char *)item.code = str;
					item.subitems = NULL;
				}
				if (order_update_item((uint32_t)arg, &item) == -1 &&
						item.code && *item.code) {
					action = FIELD_POS_NONE;
					break;
				}

				if (item.code != order.items.val[index].code) {
					if ((s = strdup(item.code)) == NULL)
						break;
					free_ChopstixItemCode(&order.items.val[index].code);
					order.items.val[index].code = s;

					if (order.items.val[index].subitems) {
						free_ChopstixSubItems(order.items.val[index].subitems);
						free(order.items.val[index].subitems);
						order.items.val[index].subitems = NULL;
					}
				}
				if (order.items.val[index].subitems == NULL)
					order.items.val[index].subitems =
						order_add_subitems(&order.items.val[index]);

			} else if (type == OFTYPE_SUBITEM) {
				bcopy(&order.items.val[index].subitems->val[subindex],
						&subitem, sizeof(subitem));
				if (did_codechange(str, subitem.code))
					(const char *)subitem.code = str;
				if (order_update_subitem((uint32_t)arg, &subitem) == -1 &&
						subitem.code && *subitem.code) {
					action = FIELD_POS_NONE;
					break;
				}

				if (subitem.code !=
						order.items.val[index].subitems->val[subindex].code) {
					if ((s = strdup(subitem.code)) == NULL)
						break;
					free_ChopstixItemCode(&order.items.val[index].subitems->val[subindex].code);
					order.items.val[index].subitems->val[subindex].code = s;
				}
			}
			break;

		case STATE_TEXT:
			if (type == OFTYPE_STYLE) {
				order.items.val[index].style
					= order_match_style(str, order.items.val[index].code, &s);
				if (s != NULL && (ff = get_formfield(field, type, state))) {
					if (!stracpy(&ff->input, s))
						status_warn("cannot set style text");
				} else if (*str)
					action = FIELD_POS_RIGHT;
			} else if (type == OFTYPE_SUBSTYLE) {
				order.items.val[index].subitems->val[subindex].style
					= order_match_style(str,
							order.items.val[index].subitems->val[subindex].code,
							&s);
				if (s != NULL && (ff = get_formfield(field, type, state))) {
					if (!stracpy(&ff->input, s))
						status_warn("cannot set substyle text");
				} else if (*str)
					action = FIELD_POS_RIGHT;
			} else if (type == OFTYPE_SPECIAL) {
				if ((s = strdup(str)) == NULL)
					break;
				if (order.items.val[index].special == NULL)
					if ((order.items.val[index].special = calloc(1,
									sizeof(*order.items.val[index].special)))
							== NULL)
						break;
				free_ChopstixSpecial(order.items.val[index].special);
				*order.items.val[index].special = s;
			} else if (type == OFTYPE_SUBSPECIAL) {
				if ((s = strdup(str)) == NULL)
					break;
				if (order.items.val[index].subitems->val[subindex].special
						== NULL)
					if ((order.items.val[index].subitems->val[subindex].special
								= calloc(1, sizeof(*order.items.val[index].subitems->val[subindex].special)))
							== NULL)
						break;
				free_ChopstixSpecial(order.items.val[index].subitems->val[subindex].special);
				*order.items.val[index].subitems->val[subindex].special= s;
			}
			break;

		case STATE_MONEY:
			if (type == OFTYPE_SUBITEM)
				parse_moneystr(str,
						&order.items.val[index].subitems->val[subindex].pricedelta);
			break;
	}

	order_refresh(ordertop);

	return action;
}
