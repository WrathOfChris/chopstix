/* $Gateweaver: display.c,v 1.59 2007/09/24 14:31:27 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/param.h>
#include <ctype.h>
#include <curses.h>
#include <err.h>
#include <event.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "chopstix.h"

RCSID("$Gateweaver: display.c,v 1.59 2007/09/24 14:31:27 cmaxwell Exp $");

static struct display display;
static struct event evsig_winch, evsig_int;
static ChopstixForm *curform;
static ChopstixForm *curform_butwindow;

enum {
	STATE_QTY,
	STATE_CODE
} state;
int ordertop;
int debugscreen = 0;
int printfailed = 0;

static void display_sighdlr(int, short, void *);
static void display_keycb(int);
#define GETCENTRE(str) \
	(strlen((str)) >= COLS ? 0 : ((COLS - strlen((str))) / 2));

void
display_init(void)
{
	bzero(&display, sizeof(display));

	if (initscr() == NULL)
		err(1, "cannot initialize display");

	if (has_colors() != FALSE)
		display.has_colour = 1;
	if (display.has_colour) {
		start_color();
#if 1
#define CHOPSTIX_BACKGROUND		COLOR_BLACK
		init_pair(1, COLOR_WHITE, CHOPSTIX_BACKGROUND);	/* NORMAL */
		bkgdset(CHOPSTIX_COLOR_NORMAL);
		clear();
		init_pair(2, COLOR_WHITE, COLOR_BLUE);			/* STATUS */
		init_pair(3, COLOR_YELLOW, COLOR_RED);			/* ERROR */
		init_pair(4, CHOPSTIX_BACKGROUND, COLOR_WHITE);	/* FORM */
		init_pair(5, COLOR_WHITE, COLOR_RED);			/* ALERT */
		init_pair(6, COLOR_CYAN, CHOPSTIX_BACKGROUND);	/* TITLE */
		init_pair(7, COLOR_YELLOW, COLOR_BLUE);			/* HELP */
		init_pair(8, COLOR_WHITE, COLOR_BLUE);			/* WINDOW */
#else
#define CHOPSTIX_BACKGROUND		COLOR_WHITE
		init_pair(1, COLOR_BLACK, CHOPSTIX_BACKGROUND);	/* NORMAL */
		bkgdset(CHOPSTIX_COLOR_NORMAL);
		clear();
		init_pair(2, COLOR_WHITE, COLOR_BLUE);			/* STATUS */
		init_pair(3, COLOR_YELLOW, COLOR_RED);			/* ERROR */
		init_pair(4, COLOR_WHITE, COLOR_RED);			/* FORM */
		init_pair(5, COLOR_WHITE, COLOR_RED);			/* ALERT */
		init_pair(6, COLOR_BLUE, CHOPSTIX_BACKGROUND);	/* TITLE */
		init_pair(7, COLOR_YELLOW, COLOR_BLUE);			/* HELP */
		init_pair(8, COLOR_WHITE, COLOR_BLUE);			/* WINDOW */
#endif
	}

	/* enable arrows and function keys */
	keypad(stdscr, TRUE);

	/* disable line buffering, but keep interrupts */
	cbreak();

	/* echoing is handled by us */
	noecho();

	/* force getch() to be non-blocking for use with libevent */
	nodelay(stdscr, TRUE);

	/* hook SIGWINCH to handle terminal resizing */
	signal_set(&evsig_winch, SIGWINCH, display_sighdlr, NULL);
	signal_add(&evsig_winch, NULL);

	/* do not allow SIGINT to terminate */
	signal_set(&evsig_int, SIGINT, display_sighdlr, NULL);
	signal_add(&evsig_int, NULL);

	state = STATE_QTY;
	ordertop = 1;

	input_set_keycb(&display_keycb);
	curform = &addrform;
}

/*
 * str is an optional label (ie: 'PST')
 */
void
display_money(int money, char *str)
{
	char number[ALIGN_MONEYWIDTH + 1];
	int y, x;
	int tx, cx;

	/* steal the line pointer */
	getyx(stdscr, y, x);

	snprintf(number, sizeof(number), "$%s%d.%02d",
				NEGSIGN(money), DOLLARS(money), CENTS(money));

	if (str) {
		/* 7 allows 999.99 orders without misaligning labels */
		cx = COLS - MAX(strlen(number),
				ALIGN_MONEY) - strlen(str) - strlen(CHOPSTIX_FORM_LABEL);
		for (tx = x; tx < cx; tx++)
			ADDCH(NORMAL, ' ');
		ADDSTR(NORMAL, str);
		cx = COLS - MAX(strlen(number), ALIGN_MONEY) -
			strlen(CHOPSTIX_FORM_LABEL);
		for (tx = x; tx < cx; tx++)
			ADDCH(NORMAL, ' ');
		ADDSTR(NORMAL, CHOPSTIX_FORM_LABEL);
	} else {
		cx = COLS - MAX(strlen(number), ALIGN_MONEY);
		for (tx = x; tx < cx; tx++)
			ADDCH(NORMAL, ' ');
	}

	x = COLS - strlen(number);
	MVADDSTR(NORMAL, y, x, number);
}

void
display_pad(void)
{
	int y, x;

	getyx(stdscr, y, x);
	while (x++ < COLS)
		addch(' ');
}

/*
 * Redisplay the top header
 */
void
display_header(ChopstixHeader *hdr)
{
	int centre;

	/* horizontal line */
	attron(CHOPSTIX_COLOR_HELP);
	mvhline(LINE_TITLE, 0, ACS_HLINE, COLS);
	attroff(CHOPSTIX_COLOR_HELP);
	move(LINE_TITLE, 0);
	ADDSTR(HELP | A_BOLD, "F1");
	ADDSTR(HELP, ":Help ");
	ADDSTR(HELP | A_BOLD, "F2");
	ADDSTR(HELP, ":Recall ");
	ADDSTR(HELP | A_BOLD, "F4");
	ADDSTR(HELP, ":ChPhone ");
	ADDSTR(HELP | A_BOLD, "F7");
	ADDSTR(HELP, ":New");

	move(LINE_TITLE, COLS - strlen("F8:ReRrnt F10:Cred F11:Disc F12:Dlvry"));
	ADDSTR(HELP | A_BOLD, "F8");
	ADDSTR(HELP, ":RePrnt ");
	ADDSTR(HELP | A_BOLD, "F10");
	ADDSTR(HELP, ":Cred ");
	ADDSTR(HELP | A_BOLD, "F11");
	ADDSTR(HELP, ":Disc ");
	ADDSTR(HELP | A_BOLD, "F12");
	ADDSTR(HELP, ":Dlvry");

	/* Company Name */
	centre = GETCENTRE(hdr->company);
	MVADDSTR(TITLE, LINE_TITLE, centre, hdr->company);
}

void
display_form(ChopstixForm *form)
{
	ChopstixFormField *ff;
	int lwidth = 0, len = 0, fwidth, this = 0, i, thislwidth;
	const char *str;

	/* align the input columns */
	lwidth = form_getalign(form);

	TAILQ_FOREACH(ff, &form->fields, entry) {
		this++;
		
		/* start HERE */
		move(ff->y, ff->x);

		/* if no width set, use to end of screen */
		fwidth = ff->w ? (ff->w - 1) : (COLS - ff->x - 1);

		/* label */
		if (ff->label) {
			ADDSTR(NORMAL, ff->label);
			ADDSTR(NORMAL, form->labelsep);
		}
		len = ff->label ? (strlen(ff->label) + strlen(form->labelsep)) : 0;

		if (form_doalign(form, ff))
			thislwidth = lwidth;
		else
			thislwidth = len;

		/* pad the label */
		if (thislwidth)
			while (len++ < thislwidth)
				ADDCH(NORMAL, ' ');

		/* set maximum input width */
		if (ff->iw)
			fwidth = MIN((len + ff->iw - 1), fwidth);

		/* copy the field string.  note: len == thislwidth */
		i = 0;

		/* XXX roving field header */
		if (curform == form && form_field_active(form, ff))
			attron(CHOPSTIX_COLOR_FORM);

		if ((str = form_field_getstr(ff))) {

			if (ff->rightalign)
				while (len <= fwidth - strlen(str)) {
					addch(CHOPSTIX_CHAR_FORM);
					len++;
				}

			while (*str) {
				if (len <= fwidth) {
					addch(*str++);
				} else {
					i++;
					len = thislwidth;
					move(ff->y + i, ff->x + len);
					if (i >= ff->l)
						break;
					addch(*str++);
				} 
				len++;
			}
		}

		/* pad the field */
		while (i < ff->l) {
			while (len <= fwidth) {
				addch(CHOPSTIX_CHAR_FORM);
				len++;
			}
			len = thislwidth;
			i++;
			move(ff->y + i, len);
		}
		attroff(CHOPSTIX_COLOR_FORM);
	}
}

void
display_window(ChopstixForm *form)
{
	int y = LINES, x = COLS, wy = 0, wx = 0, wiw = 0;
	int i, j;
	ChopstixFormField *ff;

	TAILQ_FOREACH(ff, &form->fields, entry) {
		if (ff->y < y)
			y = ff->y;
		if (ff->x < x)
			x = ff->x;
		if ((ff->y + ff->l) > wy)
			wy = ff->y + ff->l;
		if ((ff->x + ff->w) > wx)
			wx = ff->x + ff->w;
		if (ff->label) {
			if (ff->iw + strlen(ff->label) > wiw)
				wiw = ff->iw + strlen(ff->label);
		} else {
			if (ff->iw > wiw)
				wiw = ff->iw;
		}
	}
	if (wx - x > wiw)
		wiw = wx - x;
	if (form->labelsep)
		wiw += strlen(form->labelsep);

	attron(CHOPSTIX_COLOR_WINDOW);
	for (j = y - 1; j < wy + 1; j++) {
		move(j, x - 1);
		for (i = x - 1; i < x + wiw + 1; i++)
			addch(' ');
	}
	display_form(form);
	attroff(CHOPSTIX_COLOR_WINDOW);
}

/*
 * Deal with order display.  This is the only part allowed to move the cursor
 * willy-nilly
 */
void
display_order(ChopstixOrder *order)
{
	/* print the address block */
	display_form(&addrform);

	/* get ready for the order screen */
	mvhline(LINE_ORDER, 0, ACS_HLINE, COLS);

	if (order->date > 0) {
		char s[ALIGN_NUMBER + CHOPSTIX_DATE_SIZE + sizeof("[/]")];
		int x, len;
		len = snprintf(s, sizeof(s), "[%d/", order->key);
		strftime(s + len, sizeof(s) - len, CHOPSTIX_DATE_FORMAT "]",
				localtime(&order->date));
		x = COLS - strlen(s);
		MVADDSTR(ALERT, LINE_ORDER, x, s);
	}

	/* display the order fields */
	display_order_title(LINE_ORDER + 1);
	
	display_form(&orderform);
	if (WINDOW_TEST(curform))
		display_window(curform);

	/* Help text */
	move(LINE_TOTAL, 0);
	ADDSTR(HELP | A_BOLD, "Tab/Enter");
	ADDSTR(HELP, ":Store ");
	ADDSTR(HELP | A_BOLD, "Ins/Del");
	ADDSTR(HELP, ":Special ");
	ADDSTR(HELP | A_BOLD, "^P");
	ADDSTR(HELP, ":Post ");
	ADDSTR(HELP | A_BOLD, "^L");
	ADDSTR(HELP, ":Refresh ");
	ADDSTR(HELP | A_BOLD, "F5");
	ADDSTR(HELP, ":DailyTotal ");
	ADDSTR(HELP | A_BOLD, "F3");
	ADDSTR(HELP, ":LastPhone ");
	ADDSTR(HELP | A_BOLD, "^X");
	ADDSTR(HELP, ":Exit");
	attron(CHOPSTIX_COLOR_HELP);
	display_pad();
	attroff(CHOPSTIX_COLOR_HELP);

	/* display the subtotal */
	move(LINE_TOTAL + 1, 0);
	display_money(order->total.subtotal, "Subtotal");
	move(LINE_TOTAL + 2, 0);
	if (order->total.discount && order->total.credit > 0)
		display_money(DISCOUNT(&order->total) + order->total.credit, "Disc+Crd");
	else if (order->total.credit > 0)
		display_money(DISCOUNT(&order->total) + order->total.credit, "Credit");
	else
		display_money(DISCOUNT(&order->total), "Discount");
	move(LINE_TOTAL + 3, 0);
	display_money(DELIVERY(&order->total), "Delivery");
	move(LINE_TOTAL + 4, 0);
	display_money(order->total.tax1, config.tax1name);
	move(LINE_TOTAL + 5, 0);
	display_money(order->total.tax2, config.tax2name);
	move(LINE_TOTAL + 6, 0);
	display_money(order->total.total, "Total");

	/* special instructions, printed to left of totals */
	display_form(&specialform);

	/* payment info */
	display_form(&payform);

	if (LINES < LINE_TITLE_SIZE + 1 + LINE_ORDER_MINSIZE + LINE_TOTAL_SIZE
			+ LINE_STATUS_SIZE )
		status_set("SCREEN TOO SMALL!");
}

void
display_order_title(int line)
{
	move(line, 0);
	ADDSTR(TITLE, "Item");

	move(line, 6);
	ADDSTR(TITLE, "Qty.");

	move(line, 12);
	ADDSTR(TITLE, "Code");

	move(line, 18);
	ADDSTR(TITLE, "Description");

	move(line, COLS - 5);
	ADDSTR(TITLE, "Price");
}

void
display_status(ChopstixStatus *status)
{
	move(LINE_STATUS, 0);
	clrtoeol();

	if (status->bad)
		attron(CHOPSTIX_COLOR_ERROR);
	else
		attron(CHOPSTIX_COLOR_STATUS);

	addstr(status->status);
	display_pad();

	if (status->bad)
		attroff(CHOPSTIX_COLOR_ERROR);
	else
		attroff(CHOPSTIX_COLOR_STATUS);
}

/*
 * Display any credits/complaints the customer has on file in the database.
 * Complaints are never removed, though credits are applied down to zero.
 */
void
display_credits(ChopstixOrder *order)
{
	int cnum;
	char str[sizeof("[COMPLAINTS: ]") + ALIGN_NUMBER];

	if ((cnum = customer_getcredits(&order->customer)) > 0) {
		snprintf(str, sizeof(str), "[COMPLAINTS: %d]", cnum);
		attron(CHOPSTIX_COLOR_ALERT | A_BOLD);
		mvaddstr(LINE_STATUS, COLS - strlen(str), str);
		attroff(CHOPSTIX_COLOR_ALERT | A_BOLD);
	}
}

void
display_refresh(void)
{
	int y, x;

	if (debugscreen)
		for (y = 0; y < LINES; y++) {
			move(y, 0);
			for (x = 0; x < COLS; x++)
				ADDCH(NORMAL, 'X');
		}

	display_header(&header);
	display_order(&order);
	display_status(&status);
	display_credits(&order);

	form_getyx(curform, y, x);

	if (debugscreen)
		mvprintw(0,0,"cy:%d cx:%d ot:%d", y, x, ordertop);

	/* set the cursor */
	move(MIN(y, LINES), MIN(x, COLS - 1));

	refresh();
}

void
display_exit(void)
{
	if (display.has_colour) {
		attroff(CHOPSTIX_COLOR_STATUS);
		attroff(CHOPSTIX_COLOR_ERROR);
		attroff(CHOPSTIX_COLOR_FORM);
		attroff(CHOPSTIX_COLOR_ALERT);
		attroff(CHOPSTIX_COLOR_TITLE);
	}

	endwin();
}

static void
display_sighdlr(int sig, short which, void *arg)
{
	switch (sig) {
		case SIGINT:
			status_set("Press Ctrl-X to exit");
			display_refresh();
			break;
		case SIGWINCH:
		{
			struct winsize ws;

			if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
				resizeterm(ws.ws_row, ws.ws_col);

			/* clear before refresh */
			clear();

			/* reinit order form */
			order_reinit(LINE_ORDER_SIZE - 1, &ordertop);
			order_refresh(ordertop);

			/* reinit lower level forms */
			special_reinit();
			payment_reinit();
			window_reinit();

			display_refresh();
		}
		break;
	}
}

static void
display_keycb(int ch)
{
	ChopstixFormAction a = FIELD_POS_NONE;
	ChopstixFormField *ff;
	int lines;

reprocess:
	switch (ch) {
		case KEY_DOWN:
			a = form_driver(curform, FIELD_POS_DOWN);
			break;
		case KEY_UP:
			a = form_driver(curform, FIELD_POS_UP);
			break;
		case KEY_LEFT:
			a = form_driver(curform, FIELD_POS_LEFT);
			break;
		case KEY_RIGHT:
			a = form_driver(curform, FIELD_POS_RIGHT);
			break;
		case KEY_HOME:
			a = form_driver(curform, FIELD_POS_HOME);
			break;
		case KEY_END:
			a = form_driver(curform, FIELD_POS_END);
			break;

		case KEY_PREVIOUS:
			a = form_driver(curform, FIELD_PREV);
			break;

		case KEY_TAB:
		case KEY_NEXT:
		case '\n':
		case KEY_ENTER:
			/* MUST CHANGE POSTING keys in form selector below if changed */
			status_clear();
			a = form_driver(curform, FIELD_STORE);
			order_tally(&order);
			break;

		case KEY_DC:
			/* delete special text on orderform */
			if (curform != &orderform) {
				a = FORM_NONE;
				break;
			}
			order_edit_special(getfield_cur(&orderform), 0);
			order_refresh(ordertop);
			a = form_driver(curform, FIELD_NEXT);
			break;

		case KEY_IC:
			/* insert special text on orderform */
			if (curform != &orderform) {
				a = FORM_NONE;
				break;
			}
			order_edit_special(getfield_cur(&orderform), 1);
			order_refresh(ordertop);
			a = form_driver(curform, FIELD_NEXT);
			break;

		case KEY_NPAGE:
			/* Page Down order form (if possible) */
			lines = order_getlines(&order.items);
			if (ordertop < lines) {
				ordertop += MIN((LINE_ORDER_SIZE - 2), (lines - ordertop));
				order_refresh(ordertop);
				a = form_driver(&orderform, FORM_ENTER_UP);
			}
			break;

		case KEY_PPAGE:
			/* Page Up order form (if possible) */
			if (ordertop > 1) {
				ordertop -= MIN((LINE_ORDER_SIZE - 2), ordertop);
				if (ordertop == 0)
					ordertop = 1;
				order_refresh(ordertop);
				a = form_driver(&orderform, FORM_ENTER_DOWN);
			}
			break;

		case KEY_F(1):					/* help */
			status_set("Arrow keys, Tab/Enter, PgUp, PgDn");
			break;

		case KEY_F(2):					/* recall */
			if (order_getlast(&order.customer.phone, &order) == -1) {
				status_warn("no last order available");
				break;
			}
			if (customer_load(&order.customer) == -1) {
				status_warn("order references invalid customer");
				break;
			}
			ordertop = 1;
			order_refresh(ordertop);
			curform = &orderform;
			display_refresh();
			a = FORM_NONE;
			break;

		case KEY_F(3):					/* damnit I lost the phone number and
										   need to see the last order */
			if (order_getlast(NULL, &order) == -1) {
				status_warn("no last phone available");
				break;
			}
			if (customer_load(&order.customer) == -1) {
				status_warn("order references invalid customer");
				break;
			}
			ordertop = 1;
			order_refresh(ordertop);
			curform = &orderform;
			display_refresh();
			a = FORM_NONE;
			break;

		case KEY_F(4):					/* chphone */
			if (!WINDOW_TEST(curform))
				curform_butwindow = curform;
			curform = &chphoneform;
			display_refresh();
			a = FORM_ENTER_UP;
			break;

		case KEY_F(5):					/* daily totals */
			if (!WINDOW_TEST(curform))
				curform_butwindow = curform;
			curform = &dailyinfoform;
			order_getdaily_total();
			display_refresh();
			a = FORM_ENTER_UP;
			break;

		case KEY_F(8):					/* reprint */
			/*
			 * run the rules processor, recalling and reprinting did not print
			 * the rules properly
			 */
			rule_run(&order, 0);

			/*
			 * If there was a print failure (daemon missing, etc) allow the
			 * order to be reprinted without the reprint information.  But only
			 * once.
			 */
			if (printfailed) {
				if (print_order(&order, 0) == 0)
					printfailed = 0;
			} else
				print_order(&order, 1);
			a = FORM_NONE;
			break;

		case KEY_F(10):					/* credit */
			if (!WINDOW_TEST(curform))
				curform_butwindow = curform;
			curform = &creditform;
			window_update_credit();
			display_refresh();
			a = FORM_ENTER_UP;
			break;

		case KEY_F(11):					/* discount */
			if (!WINDOW_TEST(curform))
				curform_butwindow = curform;
			curform = &discountform;
			display_refresh();
			a = FORM_ENTER_UP;
			break;

		case KEY_F(12):					/* delivery */
			if (!WINDOW_TEST(curform))
				curform_butwindow = curform;
			curform = &deliveryform;
			display_refresh();
			a = FORM_ENTER_UP;
			break;

		case KEY_PRINTORDER:
			if (order_post() == -1) {
				/* failed, just abort.  can be wiped by NEWORDER */
				a = FORM_NONE;
				break;
			}
			if (print_order(&order, 0) == -1) {
				/* print failed, don't clear the order */
				printfailed = 1;
				a = FORM_NONE;
				break;
			}

			window_update_cashin(&order.total);
			if (!WINDOW_TEST(curform))
				curform_butwindow = curform;
			curform = &cashinform;
			display_refresh();
			a = FORM_ENTER_UP;
			break;

		case KEY_F(7):
		case KEY_NEWORDER:
			form_wipe_input(&addrform);
			form_wipe_input(&orderform);
			form_wipe_input(&specialform);
			form_wipe_input(&payform);
			window_wipe();
			order_new();
			ordertop = 1;
			printfailed = 0;
			order_refresh(ordertop);
			curform = &addrform;
			display_refresh();
			a = FORM_NONE;
			break;

		case 0x16:	/* ^V^V */
			if ((ff = getfield_cur(curform)))
				status_set("RAW INPUT: \"%s\"", ff->input.s);
			a = FORM_NONE;
			break;

		case 0x13:	/* ^V^S */
			debugscreen = !debugscreen;
			break;

		case KEY_BACKSPACE:
		case KEY_BSPACE:
			a = form_input(curform, KEY_BSPACE);
			break;

		default:
			/* input */
			if (debugscreen && ch > KEY_CODE_YES) {
				status_set("EXT KEY (oct): %o", ch);
				break;
			}
			if (isgraph(ch) || isspace(ch))
				a = form_input(curform, ch);
			break;
	}

	/*
	 * FORM SELECTOR
	 */

	do {
		if (a == FORM_EXIT_UP) {
			if (curform == &orderform) {
				/* orderform is special */
				if (ordertop > 1) {
					--ordertop;
					order_refresh(ordertop);
					a = form_driver(&orderform, FORM_ENTER_UP);
					break;
				} else
					curform = &addrform;
			} else if (curform == &specialform)
				curform = &orderform;
			else if (curform == &payform)
				curform = &specialform;
			else if (curform == &cashinform) {
				/* not allowed to exit UP from this form */
				a = form_driver(curform, FORM_ENTER_UP);
				break;
			} else if (WINDOW_TEST(curform))
				curform = curform_butwindow;
			else
				break;
			a = form_driver(curform, FORM_ENTER_DOWN);
		} else if (a == FORM_EXIT_DOWN) {
			if (curform == &addrform)
				curform = &orderform;
			else if (curform == &orderform) {
				/* orderform is special */
				if ((ordertop + LINE_ORDER_SIZE - 2)
						<= order_getlines(&order.items)) {
					++ordertop;
					order_refresh(ordertop);
					a = form_driver(&orderform, FIELD_BEGIN);
					break;
				} else
					curform = &specialform;
			} else if (curform == &specialform)
				curform = &payform;
			else if (curform == &payform) {
				/*
				 * enta-enta-enta-enta
				 * User is ready to post the form, so  change the key and
				 * reprocess as a PRINTORDER command
				 */
				if (ch == KEY_TAB || ch == KEY_NEXT || ch == '\n'
						|| ch == KEY_ENTER) {
					ch = KEY_PRINTORDER;
					goto reprocess;
				} else
					status_set("Press ENTER to post order");
				break;
			} else if (curform == &cashinform) {
				/* this pops up after order post, to help with cashin/out */
				if (ch == KEY_TAB || ch == KEY_NEXT || ch == '\n'
						|| ch == KEY_ENTER) {
					ch = KEY_NEWORDER;
					goto reprocess;
				} else
					status_set("Press ENTER to leave");
				break;
			} else if (WINDOW_TEST(curform))
				curform = curform_butwindow;
			else
				break;
			a = form_driver(curform, FORM_ENTER_UP);
		} else if (a == FORM_ENTER_UP || a == FORM_ENTER_DOWN)
			a = form_driver(curform, a);
	} while ((a == FORM_EXIT_UP && curform != &addrform)
			|| (a == FORM_EXIT_DOWN && curform != &payform));

	display_form(curform);
}
