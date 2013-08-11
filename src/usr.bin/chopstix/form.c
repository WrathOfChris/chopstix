/* $Gateweaver: form.c,v 1.25 2007/09/24 14:31:27 cmaxwell Exp $ */
/*
 * Copyright (c) 2005 Christopher Maxwell.  All rights reserved.
 */
#include <sys/param.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chopstix.h"

RCSID("$Gateweaver: form.c,v 1.25 2007/09/24 14:31:27 cmaxwell Exp $");

static ChopstixFormAction form_field_putdata_default(const char *, void *);

void
form_init(void)
{
}

/*
 * Wipe all input from the form
 */
void
form_wipe_input(ChopstixForm *form)
{
	ChopstixFormField *ff;

	form->cur = 0;
	TAILQ_FOREACH(ff, &form->fields, entry) {
		ff->cpos = 0;
		stracpy(&ff->input, "");
	}
}

ChopstixFormField *
form_field_new(const char *label)
{
	ChopstixFormField *ff;

	if ((ff = calloc(1, sizeof(ChopstixFormField))) == NULL)
		return NULL;

	if (label)
		if ((ff->label = strdup(label)) == NULL) {
			free(ff);
			return NULL;
		}

	/* one line and only one */
	ff->l = 1;

	/* allocate a single \0 byte */
	if (!stracpy(&ff->input, "")) {
		if (ff->label)
			free(ff->label);
		free(ff);
		return NULL;
	}

	ff->validate = &form_field_validate_default;
	ff->getcpos = &form_field_getcpos_default;
	ff->getrpos = &form_field_getrpos_default;
	ff->getstr = &form_field_getstr_default;
	ff->putdata = &form_field_putdata_default;

	return ff;
}

void
form_field_add(ChopstixForm *form, ChopstixFormField *ff)
{
	TAILQ_INSERT_TAIL(&form->fields, ff, entry);
	++form->len;
}

void
form_field_del(ChopstixForm *form, ChopstixFormField *ff)
{
	ChopstixFormField *ffc;

	TAILQ_REMOVE(&form->fields, ff, entry);
	--form->len;

	if (form->len < 0) {
		status_warn("internal form reference count error (corrected)");
		form->len = 0;
		TAILQ_FOREACH(ffc, &form->fields, entry)
			++form->len;
	}

	/* fix the current pointer */
	if (form->cur > form->len)
		form->cur = form->len;
	if (form->cur < 0)
		form->cur = 0;
}

void
form_field_insafter(ChopstixForm *form, ChopstixFormField *field,
		ChopstixFormField *ff)
{
	TAILQ_INSERT_AFTER(&form->fields, field, ff, entry);
	++form->len;
}

void
form_field_free(ChopstixFormField *ff)
{
	if (ff) {
		if (ff->label)
			free(ff->label);
		strafree(&ff->input);
	}
}

/* -1 does not set the value */
void
form_field_setyx(ChopstixFormField *ff, int y, int x)
{
	if (y > -1)
		ff->y = y;
	if (x > -1)
		ff->x = x;
}

/* -1 does not set the value */
void
form_field_setwl(ChopstixFormField *ff, int w, int l)
{
	if (w > -1)
		ff->w = w;
	if (l > -1)
		ff->l = l;
}

void
form_field_setiw(ChopstixFormField *ff, int iw)
{
	if (iw > -1)
		ff->iw = iw;
}

void
form_field_setarg(ChopstixFormField *ff, void *arg)
{
	ff->arg = arg;
}

void
form_field_setdisplayonly(ChopstixFormField *ff, int val)
{
	ff->displayonly = val;
}

void
form_field_setrightalign(ChopstixFormField *ff, int val)
{
	ff->rightalign = val;
}

int
form_field_llen(ChopstixFormField *ff)
{
	return strlen(ff->label);
}

int
form_field_flen(ChopstixFormField *ff)
{
	if (ff->input.s == NULL)
		return 0;
	return strlen(ff->getstr(ff->input.s, ff->arg));
}

const char *
form_field_getstr(ChopstixFormField *ff)
{
	return ff->getstr(ff->input.s, ff->arg);
}

/*
 * Get the cursor position within the field
 */
int
form_field_getcpos(ChopstixFormField *ff)
{
	return ff->getcpos(ff->input.s, ff->cpos, FIELD_POS_NONE, ff->arg);
}

/*
 * Get the real input position, based on the cursor position
 */
int
form_field_getrpos(ChopstixFormField *ff)
{
	return ff->getrpos(ff->input.s, ff->cpos, ff->arg);
}

int
form_getalign(ChopstixForm *form)
{
	ChopstixFormField *ff;
	int lwidth = 0;

	if (!form->align)
		return 0;

	TAILQ_FOREACH(ff, &form->fields, entry)
		lwidth = MAX(form_field_llen(ff), lwidth);

	lwidth += strlen(form->labelsep);

	return lwidth;
}

/*
 * return (1) if alignment should be done
 * return (0) if not
 */
int
form_doalign(ChopstixForm *form, ChopstixFormField *ff)
{
	ChopstixFormField *ffirst;

	if ((ffirst = TAILQ_FIRST(&form->fields)) == NULL)
		return form->align;

	if (ff->x == ffirst->x)
		return 1;

	return 0;
}

ChopstixFormField *
getfield_cur(ChopstixForm *form)
{
	ChopstixFormField *ff;
	int this = 0;

	TAILQ_FOREACH(ff, &form->fields, entry)
		if (this++ == form->cur)
			return ff;

	return NULL;
}

void
form_setfield_cur(ChopstixForm *form, ChopstixFormField *field)
{
	ChopstixFormField *ff;
	int this = 0;

	TAILQ_FOREACH(ff, &form->fields, entry) {
		if (ff == field)
			form->cur = this;
		this++;
	}
}

int
form_field_active(ChopstixForm *form, ChopstixFormField *field)
{
	ChopstixFormField *ff;
	int this = 0;

	TAILQ_FOREACH(ff, &form->fields, entry)
		if (this++ == form->cur)
			if (ff == field)
				return 1;

	return 0;
}

static int
getfield_max(ChopstixForm *form)
{
	ChopstixFormField *ff;
	int this = 0;

	TAILQ_FOREACH(ff, &form->fields, entry)
		++this;

	return this;
}

/*
 * Deal with the form internals.  When an action is taken that would leave the
 * form, return a FORM_EXIT_* 
 */
ChopstixFormAction
form_driver(ChopstixForm *form, ChopstixFormAction action)
{
	ChopstixFormField *ff, *ffp = NULL;

reprocess:
	ff = getfield_cur(form);

	/* deal with empty form */
	if (ff == NULL) {
		if (action == FIELD_POS_RIGHT
				|| action == FIELD_POS_WRIGHT
				|| action == FIELD_POS_END)
			return FORM_EXIT_RIGHT;

		if (action == FIELD_POS_LEFT
				|| action == FIELD_POS_WLEFT
				|| action == FIELD_POS_HOME)
			return FORM_EXIT_LEFT;

		if (action == FIELD_POS_UP
				|| action == FIELD_PREV)
			return FORM_EXIT_UP;

		if (action == FIELD_POS_DOWN
				|| action == FIELD_NEXT)
			return FORM_EXIT_DOWN;

		return FORM_NONE;
	}

	/* normal form processing */
	switch (action) {
		case FIELD_POS_NONE:		/* nop */
			return FORM_NONE;

		/* pass these to field driver */
		case FIELD_POS_RIGHT:		/* move 1 char right */
		case FIELD_POS_LEFT:		/* move 1 char left */
		case FIELD_POS_WLEFT:		/* move 1 word left */
		case FIELD_POS_WRIGHT:		/* move 1 word right */
		case FIELD_POS_HOME:		/* move to beginning */
		case FIELD_POS_END:			/* move to end */
			ff->cpos = ff->getcpos(ff->input.s, ff->cpos, action, ff->arg);
			ff->overwrite = 0;
			return FORM_NONE;

		/* check with field driver, if no change then switch fields */
		case FIELD_POS_UP:
			if (ff->cpos == ff->getcpos(ff->input.s, ff->cpos, action,
						ff->arg)) {
				do {
					if (ff == TAILQ_FIRST(&form->fields))
						return FORM_EXIT_UP;
					else
						--form->cur;
				} while ((ff = getfield_cur(form)) && ff->displayonly == 1);
				ff->overwrite = 1;
				ff->cpos = ff->getcpos(ff->input.s, 0, FIELD_POS_NONE, ff->arg);
			}
			return FORM_NONE;
		case FIELD_POS_DOWN:
			if (ff->cpos == ff->getcpos(ff->input.s, ff->cpos, action,
						ff->arg)) {
				do {
					if (TAILQ_NEXT(ff, entry) == TAILQ_END(&form->fields))
						return FORM_EXIT_DOWN;
					else
						++form->cur;
				} while ((ff = getfield_cur(form)) && ff->displayonly == 1);
			}
				ff->overwrite = 1;
				ff->cpos = ff->getcpos(ff->input.s, 0, FIELD_POS_NONE, ff->arg);
			return FORM_NONE;

		case FIELD_HOTSTORE:
			/*
			 * Characters are coming in HOT.  If we get a FIELD_HOTSTORE return,
			 * it means the character was ambiguous, so treat it as FORM_NONE
			 */
			if (ff->putdata) {
				action = ff->putdata(ff->input.s, ff->arg);
				if (action == FIELD_HOTSTORE)
					action = FORM_NONE;
				if (action != FIELD_STORE)
					goto reprocess;
			}
			/* fall through to processing */
			action = FIELD_NEXT;
			goto reprocess;
			break;

		case FIELD_STORE:
			/*
			 * If the (putdata) function exists, it is allowed to reset the
			 * action value.  The only thing it is not allowed to do is set it
			 * back to FIELD_STORE, which would cause an infinite loop.
			 *
			 * If no (putdata) function exists, fallthrough to FIELD_NEXT
			 */
			if (ff->putdata) {
				action = ff->putdata(ff->input.s, ff->arg);
				if (action == FIELD_HOTSTORE)
					action = FIELD_NEXT;
				if (action != FIELD_STORE)
					goto reprocess;
			}
			/* FALLTHROUGH */
		case FIELD_NEXT:
			do {
				if (TAILQ_NEXT(ff, entry) == TAILQ_END(&form->fields))
					return FORM_EXIT_DOWN;
				else
					++form->cur;
			} while ((ff = getfield_cur(form)) && ff->displayonly == 1);
			ff->overwrite = 1;
			ff->cpos = ff->getcpos(ff->input.s, 0, FIELD_POS_NONE, ff->arg);
			return FORM_NONE;
		case FIELD_PREV:
			do {
				if (ff == TAILQ_FIRST(&form->fields))
					return FORM_EXIT_UP;
				else
					--form->cur;
			} while ((ff = getfield_cur(form)) && ff->displayonly == 1);
			ff->overwrite = 1;
			ff->cpos = ff->getcpos(ff->input.s, 0, FIELD_POS_NONE, ff->arg);
			return FORM_NONE;

		case FIELD_BEGIN:
			/* Iterate back through fields on the same y-line */
			do {
				if (ffp == TAILQ_FIRST(&form->fields))
					break;
				else
					--form->cur;
			} while ((ffp = getfield_cur(form)) && ffp->y == ff->y);
			/* Now go forward, looking for first displayable */
			do {
				if (TAILQ_NEXT(ffp, entry) == TAILQ_END(&form->fields))
					break;
				else
					++form->cur;
			} while ((ffp = getfield_cur(form)) && ffp->displayonly == 1);
			break;

		/* handle entry from the TOP down */
		case FORM_ENTER_UP:
			form->cur = 0;
			while ((ff = getfield_cur(form)) && ff->displayonly == 1) {
				if (TAILQ_NEXT(ff, entry) == TAILQ_END(&form->fields)
						&& ff->displayonly == 1)
					return FORM_EXIT_DOWN;
				else
					++form->cur;
			}
			return FORM_NONE;

		/* handle entry from the BOTTOM up */
		case FORM_ENTER_DOWN:
			form->cur = getfield_max(form);
			do {
				if (ff == TAILQ_FIRST(&form->fields) && ff->displayonly == 1)
					return FORM_EXIT_UP;
				else
					--form->cur;
			} while ((ff = getfield_cur(form)) && ff->displayonly == 1);
			return FORM_NONE;

		/* do nothing for these */
		case FORM_ENTER_RIGHT:
		case FORM_ENTER_LEFT:
		case FORM_EXIT_UP:
		case FORM_EXIT_DOWN:
		case FORM_EXIT_RIGHT:
		case FORM_EXIT_LEFT:
		case FORM_NONE:
			return FORM_NONE;
	}

	return FORM_NONE;
}

/*
 * Deal with input to the form field
 */
ChopstixFormAction
form_input(ChopstixForm *form, int ch)
{
	ChopstixFormField *ff;
	int rpos;
	ChopstixFormAction a = FIELD_POS_RIGHT;

	if ((ff = getfield_cur(form)) == NULL)
		return FORM_NONE;

	if (ff->overwrite)
		if (!stracpy(&ff->input, ""))
			status_warn("could not overwrite field");
	ff->overwrite = 0;

	rpos = form_field_getrpos(ff);

	if (ch == KEY_BSPACE) {
		if (strlen(ff->input.s))
			if (!stradelc(&ff->input, rpos-1)) {
				status_warn("cannot backspace here");
				return FORM_NONE;
			}
		return form_driver(form, FIELD_POS_LEFT);
	}

	if (!strainsc(&ff->input, ch, rpos)) {
		status_warn("Key \"%c\" not allowed here", ch);
		return FORM_NONE;
	}

	if (ff->validate(ff->input.s, &rpos, ff->arg) == -1) {
		if (!stradelc(&ff->input, rpos))
			status_warn("Corrupted input, key \"%c\"", ch);
		return FORM_NONE;
	}

	if (ff->hotfield)
		a = form_driver(form, FIELD_HOTSTORE);
	else
		a = form_driver(form, FIELD_POS_RIGHT);

	return a;
}

void
form_getyx_int(ChopstixForm *form, int *y, int *x)
{
	ChopstixFormField *ff;
	int lwidth = 0, fwidth;

	if ((ff = getfield_cur(form)) == NULL)
		return;

	*y = ff->y;

	if (form->align && form_doalign(form, ff))
		lwidth = form_getalign(form);
	else {
		if (ff->label)
			lwidth = strlen(ff->label) + strlen(form->labelsep);
	}

	*x = ff->x + lwidth + form_field_getcpos(ff);

	if (ff->w > 0)
		fwidth = ff->w;
	else if (ff->iw > 0)
		fwidth = lwidth + ff->iw;
	else
		fwidth = 0;

	/* if multiline, reduce to support wrapped lines */
	if (ff->l > 1 && fwidth > 0)
		while (*x > (ff->x + fwidth) && *y < (ff->y + ff->l - 1)) {
			*x -= fwidth;
			*y += 1;
		}

	/* fixup overline */
	if (fwidth > 0 && *x > (ff->x + fwidth))
		*x = ff->x + fwidth;
}

void
form_exit(void)
{
}

/*
 * If validation fails, return the position in (pos)
 */
int
form_field_validate_default(const char *str, int *pos, void *arg)
{
	const char *s;

	for (s = str; *s; *s++)
		if (!(isgraph(*s) || *s == ' ')) {
			*pos = s - str;
			return -1;
		}
	return 0;
}

int
form_field_validate_number(const char *str, int *pos, void *arg)
{
	const char *s;
	
	for (s = str; *s; *s++)
		if (!isdigit(*s) && *s != '-') {
			*pos = s - str;
			return -1;
		}
	return 0;
}

int
form_field_validate_money(const char *str, int *pos, void *arg)
{
	const char *s;

	int cnt_dolla = 0, cnt_dot = 0, cnt_dash = 0;

	for (s = str; *s; *s++) {
		if ((!isdigit(*s) && *s != '$' && *s != '.' && *s != '-')
				|| (s - str) > (ALIGN_MONEYWIDTH - 1)) {
			*pos = s - str;
			return -1;
		}
		if ((*s == '$' && ++cnt_dolla > 1)
				|| (*s == '.' && ++cnt_dot > 1)
				|| (*s == '-' && ++cnt_dash > 1)) {
			*pos = s - str;
			return -1;
		}
	}
	return 0;
}

int
form_field_validate_phone(const char *str, int *pos, void *arg)
{
	const char *s;

	for (s = str; *s; *s++)
		if (!isdigit(*s) && *s != 'X' && *s != 'x'
				&& (s - str) < CHOPSTIX_PHONE_RAWSIZE) {
			*pos = s - str;
			return -1;
		}
	return 0;
}

int
form_field_validate_discount(const char *str, int *pos, void *arg)
{
	const char *s;
	int cnt_dolla = 0, cnt_dot = 0, cnt_dash = 0, cnt_perc = 0;

	for (s = str; *s; *s++) {
		if ((!isdigit(*s) && *s != '$' && *s != '.' && *s != '-' && *s != '%')
				|| (s - str) > (ALIGN_MONEYWIDTH - 1)) {
			*pos = s - str;
			return -1;
		}
		if ((*s == '$' && ++cnt_dolla > 1)
				|| (*s == '.' && ++cnt_dot > 1)
				|| (*s == '-' && ++cnt_dash > 1)
				|| (*s == '%' && ++cnt_perc > 1)) {
			*pos = s - str;
			return -1;
		}
	}
	return 0;
}

/*
 * GETSTR
 * ------
 *
 * Return the displayable string.  This may be different than the input buffer.
 */

const char *
form_field_getstr_default(const char *str, void *arg)
{
	return str;
}

const char *
form_field_getstr_phone(const char *raw, void *arg)
{
	static char str[CHOPSTIX_PHONE_SIZE];
	const char *r;
	int len;

	if (raw == NULL || *raw == '\0') {
		strlcpy(str, "(___) ___-____", sizeof(str));
		return str;
	}

	r = raw;
	len = 0;
	bzero(str, sizeof(str));
	do {
		/* 
		 * (___) ___-____ x_____
		 * 012345678901234567890
		 */
		switch (len) {
			case 0:
				str[len++] = '(';
				break;
			case 4:
				str[len++] = ')';
				/* FALLTHROUGH */
			case 5:
				str[len++] = ' ';
				break;
			case 9:
				str[len++] = '-';
				break;
			case 14:
				if (*r != '\0')
					str[len++] = ' ';
				/* FALLTHROUGH */
			case 15:
				if (*r != '\0')
					str[len++] = 'x';
				break;
		}

		if (isdigit(*r))
			str[len++] = *r++;
		else if (tolower(*r) == 'x') {
			if (len < 14)
				str[len++] = '_';
			else
				*r++;
		} else if (*r == '\0' && len < 14)
			str[len++] = '_';
		else
			len++;
	} while (len < (CHOPSTIX_PHONE_SIZE - 1));

	return str;
}

const char *
form_field_getstr_money(const char *raw, void *arg)
{
	static char str[ALIGN_MONEYWIDTH + 1];
	int neg = 1;		/* set to -1 to negate the value */
	int64_t val = 0;
	int money;

	/* a negative sign is permitted anywhere, including inline */
	if (strchr(raw, '-') != NULL)
		neg = -1;

	while (*raw) {
		if (!isdigit(*raw) && *raw != '$' && *raw != '.' && *raw != '-')
			break;

		if (isdigit(*raw)) {
			val *= 10;
			val += (*raw - '0');
		}

		*raw++;
	}
	money = ROLLOVER(val * neg);

	if (money == 0)
		str[0] = '\0';
	else if (money >= INT_MAX || money <= INT_MIN)
		snprintf(str, sizeof(str), "$%s########.##",
				money < 0 ? "-" : "");
	else
		snprintf(str, sizeof(str), "$%s%0d.%02d",
				NEGSIGN(money), DOLLARS(money), CENTS(money));

	return str;
}

const char *
form_field_getstr_discount(const char *raw, void *arg)
{
	static char str[ALIGN_MONEYWIDTH + 1];
	int neg = 1;		/* set to -1 to negate the value */
	int64_t val = 0;
	int money, discount = 0;

	/* a negative sign is permitted anywhere, including inline */
	if (strchr(raw, '-') != NULL)
		neg = -1;

	while (*raw) {
		if (!isdigit(*raw) && *raw != '$' && *raw != '.' && *raw != '-'
				&& *raw != '%')
			break;

		if (isdigit(*raw)) {
			val *= 10;
			val += (*raw - '0');
		} else if (*raw == '%')
			discount = 1;

		*raw++;
	}
	money = ROLLOVER(val * neg);

	if (money == 0)
		str[0] = '\0';
	else if (money >= INT_MAX || money <= INT_MIN)
		snprintf(str, sizeof(str), "$%s########.##",
				money < 0 ? "-" : "");
	else if (discount)
		snprintf(str, sizeof(str), "%s%d%%",
				NEGSIGN(money), money);
	else
		snprintf(str, sizeof(str), "$%s%0d.%02d",
				NEGSIGN(money), DOLLARS(money), CENTS(money));

	return str;
}

/*
 * GETCPOS
 * -------
 *
 * Get the cursor position within the displayed field.
 */
int
form_field_getcpos_default(const char *str, int cpos, ChopstixFormAction action,
		void *arg)
{
	int len = strlen(str) + 1;
	int pos = cpos;
	char *spc;

	if (cpos > len)
		cpos = len;

	switch (action) {
		case FIELD_POS_NONE:
		case FIELD_POS_UP:
		case FIELD_POS_DOWN:
			break;
		case FIELD_POS_RIGHT:
			pos = MIN(len, cpos + 1);
			break;
		case FIELD_POS_LEFT:
			pos = MAX(0, cpos - 1);
			break;
		case FIELD_POS_WLEFT:
			if ((spc = strrchr(str + len, ' ')) == NULL)
				cpos = 0;
			else
				cpos = spc - str;
			pos = MIN(0, cpos);
			break;
		case FIELD_POS_WRIGHT:
			if ((spc = strchr(str, ' ')) == NULL)
				cpos = len;
			else
				cpos = spc - str;
			pos = MAX(len, cpos);
			break;
		case FIELD_POS_HOME:
			return 0;
		case FIELD_POS_END:
			pos = len;
			break;

		default:
			/* no form control here */
			break;
	}

	return MIN(pos, strlen(str));
}

int
form_field_getcpos_phone(const char *str, int cpos, ChopstixFormAction action,
		void *arg)
{
	int pos;

	pos = form_field_getrpos_phone(str, cpos, arg);

	switch (action) {
		case FIELD_POS_NONE:
			break;
		case FIELD_POS_RIGHT:
			pos = MIN(pos + 1, strlen(str));
			break;
		case FIELD_POS_LEFT:
			pos = MAX(pos - 1, 0);
			break;
		case FIELD_POS_UP:
		case FIELD_POS_DOWN:
			break;
		case FIELD_POS_WLEFT:
			if (pos < 3)
				pos = 0;
			else if (pos < 6)
				pos = 3;
			else if (pos < 10)
				pos = 6;
			else
				pos = 10;
			break;
		case FIELD_POS_WRIGHT:
			if (pos < 3)
				pos = 3;
			else if (pos < 6)
				pos = 6;
			else if (pos < 10)
				pos = 10;
			else
				pos = 15;
			break;
		case FIELD_POS_HOME:
			pos = 0;
			break;
		case FIELD_POS_END:
			pos = MIN(15, strlen(str));
			break;

		default:
			/* no form control here */
			break;
	}

	/*
	 * remap pos->cpos
	 *           1         2
	 * 012345678901234567890
	 * (NPA) NXX-NMBR xEXTN5
	 *  012  345 6789 012345
	 */
	if (pos < 0)
		pos = 0;
	if (pos < 3)
		return 1 + pos;
	else if (pos < 6)
		return 3 + pos;
	else if (pos < 10)
		return 4 + pos;
	else if (pos < 15)
		return 5 + pos;
	else
		return 20;
}

int
form_field_getcpos_money(const char *str, int cpos, ChopstixFormAction action,
		void *arg)
{
	return ALIGN_MONEY_MAX;
}

int
form_field_getcpos_discount(const char *str, int cpos, ChopstixFormAction action,
		void *arg)
{
	return ALIGN_MONEY_MAX;
}

/*
 * GETRPOS
 * -------
 *
 * Get the 'real' position of the cursor within the input buffer.
 */

int
form_field_getrpos_default(const char *str, int cpos, void *arg)
{
	return MIN(cpos, strlen(str));
}

int
form_field_getrpos_phone(const char *str, int cpos, void *arg)
{
	int pos;

	/*
	 * unmap cpos->digit
	 *           1         2
	 * 012345678901234567890
	 * (NPA) NXX-NMBR xEXTN5
	 *  012  345 6789 012345
	 */
	if (cpos == 0)
		pos = 0;
	else if (cpos >= 1 && cpos <= 3)	/* NPA */
		pos = cpos - 1;
	else if (cpos == 4 || cpos == 5)
		pos = 3;
	else if (cpos >= 6 && cpos <= 8)	/* NXX */
	   pos = cpos - 3;
	else if (cpos == 9)
		pos = 6;
	else if (cpos >= 10 && cpos <= 13)	/* NUM */
		pos = cpos - 4;
	else if (cpos == 14 || cpos == 15)
		pos = 10;
	else if (cpos >= 16 && cpos <= 20)
		pos = cpos - 5;
	else
		pos = 15;

	return pos;
}

static ChopstixFormAction
form_field_putdata_default(const char *str, void *arg)
{
	status_warn("no handler for field, cannot store \"%s\"", str);
	return FIELD_NEXT;
}
