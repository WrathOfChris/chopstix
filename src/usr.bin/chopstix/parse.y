/* $Gateweaver: parse.y,v 1.16 2007/08/20 17:07:53 cmaxwell Exp $ */
/*
 * Copyright (c) 2004 Christopher Maxwell.  All rights reserved.
 * 
 * Originally from:
 * 	usr.sbin/gfsd/parse.y,v 1.2 2004/09/13 22:30:20 cmaxwell Exp $ 
 * From src/usr.sbin/ntpd/parse.y
 * $OpenBSD: parse.y,v 1.11 2004/07/09 19:28:03 otto Exp $
 */

%{
#include <sys/param.h>
#include <sys/queue.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "chopstix.h"

static FILE *fin = NULL;
static int lineno = 1;
static int errors = 0;
static int pdebug = 1;
char *infile;

int yyerror(const char *, ...);
int yyparse(void);
int kw_cmp(const void *, const void *);
int lookup(char *);
int lgetc(FILE *);
int lungetc(int);
int findeol(void);
int yylex(void);
int cmdline_symset(char *);

TAILQ_HEAD(symhead, sym)	 symhead = TAILQ_HEAD_INITIALIZER(symhead);
struct sym {
	TAILQ_ENTRY(sym) entries;
	int used;
	int persist;
	char *nam;
	char *val;
};

int symset(const char *, const char *, int);
char *symget(const char *);
int atoul(char *, u_long *);

typedef struct {
	union {
		int64_t number;
		char *string;
		ChopstixPhone phone;
	} v;
	int lineno;
} YYSTYPE;

%}

%token	COMPANY NAME ADDRESS PHONE CITY PROVINCE STATE COUNTRY
%token	TAX REGISTRATION
%token	PREFIX
%token	LICENSE LICENCE
%token	DATABASE RULE MODULE CUSTOMER MENU ORDER ALL
%token	PRINTER QUEUE COLUMNS FEED TITLE BOLD SHOW
%token	CREDIT HIDE KITCHEN SIGNATURE NO
%token	ERROR
%token	<v.string>		STRING
%type	<v.number>		number
%type	<v.string>		string
%type	<v.phone>		phone
%%

grammar		: /* empty */
			| grammar '\n'
			| grammar conf_main '\n'
			| grammar varset '\n'
			| grammar error '\n'		{ errors++; }
			;

number		: STRING			{
				int64_t ulval;
				int64_t mult = 1;
				int slen = strlen($1);
				const char *errstr = NULL;

				/*
				 * tera, giga, mega, kilo
				 * (tibi, gibi, mibi, kibi) really
				 * ...and % for fixedpoint math
				 */
				switch ($1[slen - 1]) {
					case 't':
					case 'T':
						mult *= 1024;
						/* FALLTHROUGH */
					case 'g':
					case 'G':
						mult *= 1024;
						/* FALLTHROUGH */
					case 'm':
					case 'M':
						mult *= 1024;
						/* FALLTHROUGH */
					case 'k':
					case 'K':
						mult *= 1024;
						/* FALLTHROUGH */
						$1[slen - 1] = '\0';
						break;
					case '%':
						mult = 1;
						$1[slen - 1] = '\0';
						break;
				}
				
				ulval = strtonum($1, 0, LLONG_MAX, &errstr);
				if (errstr) {
					yyerror("\"%s\" is not a number (%s)", $1, errstr);
					free($1);
					YYERROR;
				} else
					$$ = ulval * mult;
				free($1);
			}
			;

string		: string STRING				{
				if (asprintf(&$$, "%s %s", $1, $2) == -1)
					err(1, "string: asprintf");
				free($1);
				free($2);
			}
			| STRING
			;

varset		: STRING '=' string		{
#if 0
				if (conf->opts & WREND_OPT_VERBOSE)
					printf("%s = \"%s\"\n", $1, $3);
#endif
				if (symset($1, $3, 0) == -1)
					err(1, "cannot store variable");
				free($1);
				free($3);
			}
			;

phone		: STRING {
				size_t len, state;
				char *p, *dash;
				const char *errstr = NULL;

				len = strlen($1);
				p = $1;
				state = 0;

				while (p < ($1 + len)) {
					if ((dash = strchr(p, '-')))
						*dash = '\0';
					switch (state) {
						case 0:
							$$.npa = strtonum(p, 0, PHONE_NPA_MAX, &errstr);
							break;
						case 1:
							$$.nxx = strtonum(p, 0, PHONE_NXX_MAX, &errstr);
							break;
						case 2:
							$$.num = strtonum(p, 0, PHONE_NUM_MAX, &errstr);
							break;
						case 3:
							if (($$.ext = calloc(1, sizeof(*$$.ext))) == NULL)
								err(1, "cannot allocate phone extension");
							*$$.ext = strtonum(p, 0, PHONE_EXT_MAX, &errstr);
							break;
						case 4:
							break;
					}
					if (errstr) {
						yyerror("\"%s\" is not a valid phone (%s)", $1, errstr);
						free($1);
						YYERROR;
					}
					state++;
					if (dash) {
						*dash = '-';
						p = dash + 1;
					} else
						break;
				}
				free($1);
			}

conf_main	: COMPANY NAME string {
				if (header.company)
					free(header.company);
				header.company = $3;
			}
			| COMPANY ADDRESS string {
				if (header.addr.addr)
					free(header.addr.addr);
				header.addr.addr = $3;
			}
			| COMPANY CITY string {
				if (header.city)
					free(header.city);
				header.city = $3;
			}
			| COMPANY PROVINCE string {
				if (header.province)
					free(header.province);
				header.province = $3;
			}
			| COMPANY STATE string {
				if (header.province)
					free(header.province);
				header.province = $3;
			}
			| COMPANY COUNTRY string {
				if (header.province)
					free(header.province);
				header.province = $3;
			}
			| COMPANY PHONE phone {
				int ret;

				free_ChopstixPhone(&header.phone);
				if ((ret = copy_ChopstixPhone(&$3, &header.phone)))
					err(1, "cannot set company phone (%d)", ret);
				free_ChopstixPhone(&$3);
			}
			| TAX number string number {
				if ($2 == 1) {
					config.tax1name = $3;
					config.tax1rate = $4;
				} else if ($2 == 2) {
					config.tax2name = $3;
					config.tax2rate = $4;
				} else {
					yyerror("TAX %lld not supported", $2);
					free($3);
					YYERROR;
				}
			}
			| TAX number REGISTRATION string {
				if ($2 == 1) {
					config.tax1reg = $4;
				} else if ($2 == 2) {
					config.tax2reg = $4;
				} else {
					yyerror("TAX %lld not supported", $2);
					free($4);
					YYERROR;
				}
			}
			| LICENSE string {
				if (strlcpy(config.licence, $2, sizeof(config.licence))
						>= sizeof(config.licence))
					err(1, "cannot store licence path");
				free($2);
			}
			| LICENCE string {
				if (strlcpy(config.licence, $2, sizeof(config.licence))
						>= sizeof(config.licence))
					err(1, "cannot store licence path");
				free($2);
			}
			| PHONE PREFIX number {
				config.phoneprefix = $3;
			}
			| DATABASE MODULE string {
				if (strlcpy(config.module, $3, sizeof(config.module))
						>= sizeof(config.module))
					err(1, "cannot store config module path");
				free($3);
			}
			| DATABASE MENU string {
				if (config.database.menudb)
					free(config.database.menudb);
				config.database.menudb = $3;
			}
			| DATABASE CUSTOMER string {
				if (config.database.custdb)
					free(config.database.custdb);
				config.database.custdb = $3;
			}
			| DATABASE ORDER string {
				if (config.database.orderdb)
					free(config.database.orderdb);	
				config.database.orderdb = $3;
			}
			| DATABASE ALL string {
				if (config.database.alldb)
					free(config.database.alldb);
				config.database.alldb = $3;
			}
			| RULE MODULE string {
				if (strlcpy(config.rulemodule, $3, sizeof(config.rulemodule))
						>= sizeof(config.rulemodule))
					err(1, "cannot store rule module path");
				free($3);
			}
			| PRINTER string {
				if (strlcpy(config.print.cmd, $2, sizeof(config.print.cmd))
						>= sizeof(config.print.cmd))
					err(1, "cannot store print command");
				free($2);
			}
			| PRINTER QUEUE string {
				if (strlcpy(config.print.queue, $3, sizeof(config.print.queue))
						>= sizeof(config.print.queue))
					err(1, "cannot store print queue");
				free($3);
			}
			| PRINTER COLUMNS number { config.print.columns = $3; }
			| PRINTER FEED number	{ config.print.feedlines = $3; }
			| PRINTER BOLD TITLE	{ config.print.title_bold = 1; }
			| PRINTER BOLD ORDER	{ config.print.order_bold = 1; }
			| PRINTER SHOW CITY		{ config.print.show_city = 1; }
			| PRINTER SHOW PROVINCE { config.print.show_province = 1; }
			| PRINTER SHOW COUNTRY	{ config.print.show_country = 1; }
			| CREDIT HIDE CUSTOMER	{ config.print.cchide_customer = 1; }
			| CREDIT HIDE KITCHEN	{ config.print.cchide_kitchen = 1; }
			| CREDIT SIGNATURE CUSTOMER	{ config.print.ccsig_customer = 1; }
			| CREDIT SIGNATURE KITCHEN	{ config.print.ccsig_kitchen = 1; }
			| NO CREDIT HIDE CUSTOMER	{ config.print.cchide_customer = 0; }
			| NO CREDIT HIDE KITCHEN	{ config.print.cchide_kitchen = 0; }
			| NO CREDIT SIGNATURE CUSTOMER	{ config.print.ccsig_customer = 0; }
			| NO CREDIT SIGNATURE KITCHEN	{ config.print.ccsig_kitchen = 0; }
			;

%%

struct keywords {
	const char *k_name;
	int k_val;
};

int
yyerror(const char *fmt, ...)
{
	va_list ap;
	char *nfmt;

	errors = 1;
	va_start(ap, fmt);
	if (asprintf(&nfmt, "%s:%d: %s", infile, yylval.lineno, fmt) == -1)
		errx(1, "yyerror asprintf");
	vwarnx(nfmt, ap);
	va_end(ap);
	free(nfmt);
	return (0);
}

int
kw_cmp(const void *k, const void *e)
{
	return (strcmp(k, ((const struct keywords *)e)->k_name));
}

int
lookup(char *s)
{
	/* this has to be sorted always */
	static const struct keywords keywords[] = {
		{ "address",	ADDRESS},
		{ "all",		ALL},
		{ "bold",		BOLD},
		{ "city",		CITY},
		{ "columns",	COLUMNS},
		{ "company",	COMPANY},
		{ "country",	COUNTRY},
		{ "credit",		CREDIT},
		{ "customer",	CUSTOMER},
		{ "database",	DATABASE},
		{ "feed",		FEED},
		{ "hide",		HIDE},
		{ "kitchen",	KITCHEN},
		{ "licence",	LICENCE},
		{ "license",	LICENSE},
		{ "module", 	MODULE},
		{ "name",		NAME},
		{ "no",			NO},
		{ "order",		ORDER},
		{ "phone",		PHONE},
		{ "prefix",		PREFIX},
		{ "printer",	PRINTER},
		{ "province",	PROVINCE},
		{ "queue",		QUEUE},
		{ "registration", REGISTRATION},
		{ "rule",		RULE},
		{ "show",		SHOW},
		{ "signature",	SIGNATURE},
		{ "state",		STATE},
		{ "tax",		TAX},
		{ "title",		TITLE}
	};
	const struct keywords	*p;

	p = bsearch(s, keywords, sizeof(keywords)/sizeof(keywords[0]),
	    sizeof(keywords[0]), kw_cmp);

	if (p) {
		if (pdebug > 1)
			fprintf(stderr, "%s: %d\n", s, p->k_val);
		return (p->k_val);
	} else {
		if (pdebug > 1)
			fprintf(stderr, "string: %s\n", s);
		return (STRING);
	}
}

#define MAXPUSHBACK	128

char *parsebuf;
int parseindex;
char pushback_buffer[MAXPUSHBACK];
int pushback_index = 0;

int
lgetc(FILE *f)
{
	int	c, next;

	if (parsebuf) {
		/* Read character from the parsebuffer instead of input. */
		if (parseindex >= 0) {
			c = parsebuf[parseindex++];
			if (c != '\0')
				return (c);
			parsebuf = NULL;
		} else
			parseindex++;
	}

	if (pushback_index)
		return (pushback_buffer[--pushback_index]);

	while ((c = getc(f)) == '\\') {
		next = getc(f);
		if (next != '\n') {
			if (isspace(next))
				yyerror("whitespace after \\");
			ungetc(next, f);
			break;
		}
		yylval.lineno = lineno;
		lineno++;
	}
	if (c == '\t' || c == ' ') {
		/* Compress blanks to a single space. */
		do {
			c = getc(f);
		} while (c == '\t' || c == ' ');
		ungetc(c, f);
		c = ' ';
	}

	return (c);
}

int
lungetc(int c)
{
	if (c == EOF)
		return (EOF);
	if (parsebuf) {
		parseindex--;
		if (parseindex >= 0)
			return (c);
	}
	if (pushback_index < MAXPUSHBACK-1)
		return (pushback_buffer[pushback_index++] = c);
	else
		return (EOF);
}

int
findeol(void)
{
	int	c;

	parsebuf = NULL;
	pushback_index = 0;

	/* skip to either EOF or the first real EOL */
	while (1) {
		c = lgetc(fin);
		if (c == '\n') {
			lineno++;
			break;
		}
		if (c == EOF)
			break;
	}
	return (ERROR);
}

int
yylex(void)
{
	char buf[8096];
	char *p, *val;
	int endc, c;
	int token;

top:
	p = buf;
	while ((c = lgetc(fin)) == ' ')
		; /* nothing */

	yylval.lineno = lineno;
	if (c == '#')
		while ((c = lgetc(fin)) != '\n' && c != EOF)
			; /* nothing */
	if (c == '$' && parsebuf == NULL) {
		while (1) {
			if ((c = lgetc(fin)) == EOF)
				return (0);

			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			if (isalnum(c) || c == '_') {
				*p++ = (char)c;
				continue;
			}
			*p = '\0';
			lungetc(c);
			break;
		}
		val = symget(buf);
		if (val == NULL) {
			yyerror("macro \"%s\" not defined", buf);
			return (findeol());
		}
		parsebuf = val;
		parseindex = 0;
		goto top;
	}

	switch (c) {
	case '\'':
	case '"':
		endc = c;
		while (1) {
			if ((c = lgetc(fin)) == EOF)
				return (0);
			if (c == endc) {
				*p = '\0';
				break;
			}
			if (c == '\n') {
				lineno++;
				continue;
			}
			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			*p++ = (char)c;
		}
		yylval.v.string = strdup(buf);
		if (yylval.v.string == NULL)
			err(1, "yylex: strdup");
		return (STRING);
	}

#define allowed_in_string(x) \
	(isalnum(x) || (ispunct(x) && x != '(' && x != ')' && \
	x != '{' && x != '}' && x != '<' && x != '>' && \
	x != '!' && x != '=' /*&& x != '/'*/ && x != '#' && \
	x != ','))

	if (isalnum(c) || c == ':' || c == '_' || c == '*' || c == '/') {
		do {
			*p++ = c;
			if ((unsigned)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(fin)) != EOF && (allowed_in_string(c)));
		lungetc(c);
		*p = '\0';
		if ((token = lookup(buf)) == STRING)
			if ((yylval.v.string = strdup(buf)) == NULL)
				err(1, "yylex: strdup");
		return (token);
	}
	if (c == '\n') {
		yylval.lineno = lineno;
		lineno++;
	}
	if (c == EOF)
		return (0);
	return (c);
}

int
parse_config(const char *filename)
{
	struct sym *sym, *next;

	lineno = 1;
	errors = 0;

	if ((fin = fopen(filename, "r")) == NULL) {
		warn("%s", filename);
		return (-1);
	}
	(const char *)infile = filename;

	yyparse();

	fclose(fin);

	/* Free macros and check which have not been used. */
	for (sym = TAILQ_FIRST(&symhead); sym != NULL; sym = next) {
		next = TAILQ_NEXT(sym, entries);
#if 0
		if ((conf->opts & WREND_OPT_VERBOSE2) && !sym->used)
			fprintf(stderr, "warning: macro \"%s\" not "
			    "used\n", sym->nam);
#endif
		if (!sym->persist) {
			free(sym->nam);
			free(sym->val);
			TAILQ_REMOVE(&symhead, sym, entries);
			free(sym);
		}
	}

	return (errors ? -1 : 0);
}

int
symset(const char *nam, const char *val, int persist)
{
	struct sym *sym;

	for (sym = TAILQ_FIRST(&symhead); sym && strcmp(nam, sym->nam);
	    sym = TAILQ_NEXT(sym, entries))
		;	/* nothing */

	if (sym != NULL) {
		if (sym->persist == 1)
			return (0);
		else {
			free(sym->nam);
			free(sym->val);
			TAILQ_REMOVE(&symhead, sym, entries);
			free(sym);
		}
	}
	if ((sym = calloc(1, sizeof(*sym))) == NULL)
		return (-1);

	sym->nam = strdup(nam);
	if (sym->nam == NULL) {
		free(sym);
		return (-1);
	}
	sym->val = strdup(val);
	if (sym->val == NULL) {
		free(sym->nam);
		free(sym);
		return (-1);
	}
	sym->used = 0;
	sym->persist = persist;
	TAILQ_INSERT_TAIL(&symhead, sym, entries);
	return (0);
}

int
cmdline_symset(char *s)
{
	char *sym, *val;
	int ret;
	size_t len;

	if ((val = strrchr(s, '=')) == NULL)
		return (-1);

	len = strlen(s) - strlen(val) + 1;
	if ((sym = malloc(len)) == NULL)
		err(1, "cmdline_symset: malloc");

	strlcpy(sym, s, len);

	ret = symset(sym, val + 1, 1);
	free(sym);

	return (ret);
}

char *
symget(const char *nam)
{
	struct sym *sym;

	TAILQ_FOREACH(sym, &symhead, entries)
		if (strcmp(nam, sym->nam) == 0) {
			sym->used = 1;
			return (sym->val);
		}
	return (NULL);
}

int
atoul(char *s, u_long *ulvalp)
{
	u_long ulval;
	char *ep;

	errno = 0;
	ulval = strtoul(s, &ep, 0);
	if (s[0] == '\0' || *ep != '\0')
		return (-1);
	if (errno == ERANGE && ulval == ULONG_MAX)
		return (-1);
	*ulvalp = ulval;
	return (0);
}
