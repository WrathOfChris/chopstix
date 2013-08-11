/* $Gateweaver: input.c,v 1.12 2005/12/05 22:19:17 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <sys/time.h>
#include <curses.h>
#include <event.h>
#include <unistd.h>
#include "chopstix.h"

RCSID("$Gateweaver: input.c,v 1.12 2005/12/05 22:19:17 cmaxwell Exp $");

static struct event ev_tty;
static struct input input;

static void stdin_cb(int, short, void *);

void
input_init(void)
{
	/* hook an event onto the input file descriptor */
	event_set(&ev_tty, STDIN_FILENO, EV_READ|EV_PERSIST, stdin_cb, NULL);
	event_add(&ev_tty, NULL);
}

void
input_set_keycb(void (*func)(int))
{
	input.keycb = func;
}

/*
 * 0	ctrl-`,2	10	ctrl-j,m	20	ctrl-t		30	ctrl-~,6
 * 1	ctrl-a		11	ctrl-k		21	ctrl-u		31	^V^/ ctrl-7,-
 * 2	ctrl-b		12	ctrl-l		22	^V^V
 * 3	^V^C		13	^V^M		23	ctrl-w
 * 4	ctrl-d		14	ctrl-n		24	ctrl-x
 * 5	ctrl-e		15	^V^O		25	^V^Y
 * 6	ctrl-f		16	ctrl-p		26	^V^Z
 * 7	ctrl-g		17	^V^Q		27	ESC, ctrl-3
 * 8	ctrl-h,BS	18	ctrl-r		28	ctrl-[
 * 9	ctrl-i		19	^V^S		29	ctrl-],5
 *
 * ^O	discard		^Y	dsusp		^D	eof
 * ^?	erase		^C	intr		^U	kill
 * ^V	lnext		^\	quit		^R	reprint
 * ^Q	start		^@	status		^S	stop
 * ^Z	susp		^W	werase
 *
 * HOME		1b,5b,37,7e
 * END		1b,5b,38,7e
 * C-LEFT	1b,4f,64
 * C-RIGHT	1b,4f,63
 */
void
input_handlekey(int ch, void *arg)
{
	static int last = 0, llast = 0, lllast = 0, llllast = 0;
	static int inesc = 0;
	static int in4f = 0, in5b = 0;
	static int showescapekeys = 0;

	/* escape sequenced codes */
	if (inesc) {
		switch (ch) {
			case 0x5b:
				in5b = 1;
				break;
			case 0x4f:
				in4f = 1;
				break;
			default:
				if (showescapekeys)
					status_warn("RAWKEYS: %o, %o, %o, %o, %o",
							llllast, lllast, llast, last, ch);
				break;
		}
		inesc = 0;
		return;
	}

	if (in4f) {
		switch (ch) {
			case 0x63:	/* CTRL-RIGHT */
				ch = KEY_NEXT;
				break;
			case 0x64:	/* CTRL-DOWN */
				ch = KEY_PREVIOUS;
				break;
			case 0x61:	/* CTRL-UP */
			case 0x62:	/* CTRL-DOWN */
			default:
				in4f = 0;
				return;
		}
		in4f = 0;
	}

	if (in5b) {
		if (in5b != 1 && ch == 0x7e)
			ch = in5b;
		else {
			switch (ch) {
				case 0x37:	/* HOME */
					in5b = KEY_HOME;
					return;
				case 0x38:	/* END */
					in5b = KEY_END;
					return;
				default:
					in5b = 0;
					return;
			}
		}
		in5b = 0;
	}

	switch (ch) {
		case 0x1b:	/* esc */
			inesc = 1;
			break;

		case 0x0b:	/* ^K */
			showescapekeys = !showescapekeys;
			if (showescapekeys)
				status_set("RAWKEY mode enabled");
			else
				status_clear();
			display_refresh();
			break;

		case 0x18:	/* ^X */
			event_loopexit(NULL);
			return;
		case 0x0c:	/* ^L */
			clear();
			status_clear();
			display_refresh();
			break;

		case 0x05:	/* CTRL-E */
			/* redisplay error message at top, overwriting */
			mvprintw(0, 0, "%s", status.status);
			break;

		default:
			/* call the registered keyboard callback */
			if (showescapekeys)
				status_set("RAWKEYS: %o, %o, %o, %o, %o",
						llllast, lllast, llast, last, ch);
			if (input.keycb)
				input.keycb(ch);
			display_refresh();
			break;
	}
	llllast = lllast;
	lllast = llast;
	llast = last;
	last = ch;
}

void
input_exit(void)
{
}

/*
 * Respond to keyboard events
 */
static void
stdin_cb(int fd, short which, void *arg)
{
	int ch, chs = 0;

	while ((ch = getch()) != ERR) {
		chs++;

		input_handlekey(ch, arg);
	}

	/* EOF */
	if (chs == 0)
		event_loopexit(NULL);
}
