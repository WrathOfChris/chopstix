/* $Gateweaver: str.h,v 1.2 2005/11/23 23:04:17 cmaxwell Exp $ */
/*
 * Copyright (c) 1997-2003 Christopher J. Maxwell.  All rights reserved.
 */
#ifndef STR_H
#define STR_H

#include <sys/types.h>

#define CONF_BUFSIZE 16384

struct string_s {
	char	*s;				/* Pointer to the string */
	unsigned int	a;		/* Amount of memory allocated */
	unsigned int	len;	/* length for bstrings */
};

typedef struct string_s string;

extern int	stracat(string *, const char *);
extern int	stracpy(string *, const char *);
extern int	stracats(string *, string *);
extern int	stracpys(string *, string *);
extern char	*stranow(const char *);
extern int	strapre(string *, const char *);
extern int	strainsc(string *, int, size_t);
extern int	stradelc(string *, size_t);

extern void	strafree(string *);
extern int	strachk(string *, size_t);
extern int	strapos(string *, int, size_t);

extern int	strbcat(string *, const char *, int);
extern int	strbcpy(string *, const char *, int);
extern int	strbzero(string *);

extern int	strpos(const char *, char);
extern int	strarep(string *, char *, char *, char *);
extern int	strsplit2(const char *, int, string *, string *);
extern int	stracmp_host(const char *, const char *);
extern int	straclean(const char *, const char *, string *);

extern int	stracatul_p(string *, unsigned long, unsigned int);
extern int	stracatl_p(string *, long, unsigned int);
#define stracati_p(sa, i, n) stracatl_p(sa, (long)i, n)
#define stracati(sa, i) stracatl_p(sa, (long)i, 0)
#define stracatl(sa, l) stracatl_p(sa, l, 0)
#define stracatul(sa, ul) stracatul_p(sa, ul, 0)

#endif
