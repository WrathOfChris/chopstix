/* $Gateweaver: str.c,v 1.4 2005/11/28 05:39:39 cmaxwell Exp $ */
/*
 * Copyright (c) 1997-2003 Christopher J. Maxwell.  All rights reserved.
 *
 * Originally from:
 * 	src/gateweaver/libtools/str.c,v 1.15 2004/02/18 19:55:43 cmaxwell
 */
#ifndef lint
static const char rcsid[] = "$Gateweaver: str.c,v 1.4 2005/11/28 05:39:39 cmaxwell Exp $";
#endif

#include <stdlib.h>
#include <string.h>

#include "str.h"
/*
 * string.s		-> pointer to character array
 * string.a		-> size of the buffer allocated
 * string.len	-> length of the string
 */

/*
 * TODO:
 * 	check all (int) usage since there may be a case of overflow when a string
 * 	is longer than 32768 characters.
 */
/* C-string functions */
/* {{{ int stracat(string *sa, const char *s)
 * Tacks *s onto *sa.
 *
 * Returns 0 on error.
 * Returns >0 from strlcat on success
 */
int stracat(string *sa, const char *s)
{
	int l;
	size_t n;

	if (!sa)	return 0;
	if (!s)		return 0;

	/* empty sa, so just copying */
	if (!sa->s)
		return stracpy(sa, s);

	/* grab the length of the string (no null) */
	l = strlen(s);

	/* ask for memory, old + new + null */
	if (!strachk(sa, (sa->len + l + 1)))
		return 0;

	/* cat the string, giving the size of our abuf */
	n = strlcat(sa->s, s, sa->a);

	/* truncated string */
	if (n > sa->a)
		return 0;

	/* increment length by what we add */
	sa->len += l;

	return n;
}
/* }}} */
/* {{{ int stracpy(string *sa, const char *s) 
 * Copies s over sa.   ex.   s = cow  sa = thepasture   cow(\0)asture
 *
 * Returns 0 on error
 * Returns length of strlcpy on success
 */
int stracpy(string *sa, const char *s)
{
	int l;
	size_t n;

	if (!sa)	return 0;
	if (!s)		return 0;

	/* grab the length of the string (no null) */
	l = strlen(s);

	/* ask for memory (new + null) */
	if (!strachk(sa, (l + 1)))
		return 0;

	/* copy the string, giving the size of our abuf */
	n = strlcpy(sa->s, s, sa->a);

	/* truncated string */
	if (n > sa->a)
		return 0;

	/* set to length of new string */
	sa->len = l;

	return 1;
}
/* }}} */
/* {{{ int stracats(string *sa, string *s)
 * the s != plural
 * tacks string s onto string sa.
 *
 * Returns 0 on error.
 * Returns >0 from strlcat on success
 */
int stracats(string *sa, string *s)
{
	if (!sa)	return 0;
	if (!s)		return 0;

	/* empty sa, so just copying */
	if (!sa->s)
		return stracpy(sa, (char *)s->s);

	/* ask for memory (old + new + null) */
	if (!strachk(sa, (sa->len + s->len + 1)))
		return 0;

	/* cat the string, giving the size of our abuf */
	memcpy(&sa->s[sa->len], s->s, s->len);

	/* increment by length of new string */
	sa->len += s->len;

	sa->s[sa->len] = 0;

	return 1;
}
/* }}} */
/* {{{ int stracpys(string *sa, string *s) 
 * Same as stracpy, only with strings not char.
 *
 * Returns 0 on error
 * Returns length of strlcpy on success
 */
int stracpys(string *sa, string *s)
{
	if (!sa)	return 0;
	if (!s)		return 0;

	/* ask for memory (new + null) */
	if (!strachk(sa, (s->len + 1)))
		return 0;

	/* copy the string, giving the size of our abuf */
	memcpy(sa->s, s->s, sa->a);

	/* stop string at the length */
	sa->len = s->len;

	return 1;
}
/* }}} */
/* {{{ char *stranow(const char *s)
 *
 * Quickly build a new character string with SAFE memory allocation and return
 * a pointer.  Onus on safe. 
 *
 * ptr = stranow("hello");
 */
char *stranow(const char *s)
{
	char *p;
	int i;

	if (!s)	return 0;

	i = strlen(s);

	p = (char *)malloc(sizeof(char) * (i+1));
	if (!p)
		return 0;

	if (!strlcpy(p, s, (i+1))) {
		if (p)
			free(p);
		return 0;
	}

	return p;
}
/* }}} */
/* {{{ int strapre(string *sa, const char *s)
 * Prefix a string sa with string s
 * sa=thepasture s=cow  sa==>cowthepasture
 */
int strapre(string *sa, const char *s)
{
	string	tstr = {0};

	if (!sa)	return 0;
	if (!s)		return 0;

	/* start our tempstring with prefix */
	if (!stracat(&tstr, s))
		return 0;

	/* cat our old string to tempstring */
	if (!stracats(&tstr, sa)) {
		strafree(&tstr);
		return 0;
	}

	/* copy temp string over string */
	if (!stracpys(sa, &tstr)) {
		strafree(&tstr);
		return 0;
	}

	strafree(&tstr);

	return 1;
}
/* }}} */
int strainsc(string *sa, int c, size_t pos)
{
	if (!sa)	return 0;
	if (!c)		return 0;

	/* ask for another byte of memory + NUL */
	if (!strachk(sa, (sa->len + 1 + 1)))
		return 0;

	/* oversize means append */
	if (pos > sa->len)
		pos = sa->len;

	/* move the afterpart */
	if (pos < sa->len)
		memmove(sa->s + pos + 1, sa->s + pos, sa->len - pos);

	/* increase the string len after movement */
	sa->len++;

	*(sa->s + pos) = c;
	*(sa->s + sa->len) = '\0';

	return sa->len;
}
int stradelc(string *sa, size_t pos)
{
	if (!sa)	return 0;

	if (sa->len == 0)
		return 0;
	if (pos > sa->len)
		pos = sa->len;

	if (pos < sa->len)
		memmove(sa->s + pos, sa->s + pos + 1, sa->len - pos);

	sa->len--;
	*(sa->s + sa->len) = '\0';

	if (!strachk(sa, (sa->len + 1)))
		return 0;

	return 1;
}
/* safe for both: Cstring and byte string */
/* {{{ void strafree(string *sa)
 * Frees allocated memory
 */
void strafree(string *sa)
{
	if (!sa)
		return;

	if (!sa->s)
		return;

	free(sa->s);

	sa->len = 0;
	sa->s = NULL;
	sa->a = 0;
}
/* }}} */
/* {{{ int strachk(string *sa, size_t size)
 * Allocates memory for strings.
 *
 * Returns 0 on error
 * Returns >0 on success
 *
 * sa->a	is our old length
 * size		is our requested length (should include null char)
 */
int strachk(string *sa, size_t size)
{
	int n = 0;
	char *ptr2;

	if (!sa)	return 0;

	if (size == 0)
		return 0;

	/* first time, no pointer */
	if (!sa->s) {
		sa->s = (char *)malloc(size * sizeof(char));

		/* throw back error */
		if (!sa->s)
			return 0;

		sa->a = size;
		return sa->a;
	}

	/* check if we need space */
	if (size > sa->a) {
		/* re-allocate the buffer */
		if ((ptr2 = (char *)realloc(sa->s, (size * sizeof(char)))) == NULL) {
			if (sa->s) free(sa->s);
			sa->s = NULL;
			return 0;
		}
		sa->s = ptr2;
		sa->a = size;
	} else
		n = sa->a;

	return 1;
}
/* }}} */
/* {{{ int strapos(string *sa, int c, int start)	XXX - change int to long for int-overflow protection
 * Finds first occurance of char c, starting at start, in string sa.
 * like say for searching for =  in   2 x 2 = your mom.
 */
int strapos(string *sa, int c, size_t start)
{
	size_t i;

	if (!sa)
		return 0;

	if (start > sa->len)
		return 0;

	for (i = start; i < sa->len; ++i) {
		if (sa->s[i] == c)
			return i;
	}

	return 0;
}
/* }}} */
/* byte only: */
/* {{{ int strbcat(string *sa, const char *s, int len)
 *
 * Here we keep a NULL on the end, just for safety. Because you might forget
 * that its a byte string, not a cstring (nub).
 *
 * Returns 0 on error.
 * Returns >0 from strlcat on success
 */
int strbcat(string *sa, const char *s, int len)
{
	if (!sa)	return 0;
	if (!s)
		return 0;

	/* empty sa, so just copying */
	if (!sa->s)
		return strbcpy(sa, s, len);

	/* ask for memory (old + new + hidden null)*/
	if (!strachk(sa, (sa->len + len + 1)))
		return 0;

	/* cat the string, starting at hidden null */
	memcpy((sa->s + sa->len), s, len);

	/* increment length by what we add */
	sa->len += len;

	/* add our safety null (the 0/1 shift gives correct offset) */
	sa->s[sa->len] = '\0'; 

	return 1;
}
/* }}} */
/* {{{ int strbcpy(string *sa, const char *s, int len)
 * Same as bcat but copys.
 *
 * Returns 0 on error
 * Returns length of strlcpy on success
 */
int strbcpy(string *sa, const char *s, int len)
{
	if (!sa)	return 0;
	if (!s)		return 0;

	/* ask for memory (new + hidden null) */
	if (!strachk(sa, (len + 1)))
		return 0;

	/* copy the string, starting at 0 */
	memcpy(sa->s, s, len);

	/* set to length of new string */
	sa->len = len;

	/* add our hidden null */
	sa->s[sa->len] = '\0';

	return 1;
}
/* }}} */
/* {{{ int strbzero(string *sa) 
 * NULLs the string.
 */
int strbzero(string *sa)
{
	char ch = '\0';
	
	if (!sa)	return 0;

	return strbcat(sa, &ch, 1);
}
/* }}} */
/* Complementary Functions */
/* {{{ int strpos(const char *s, char c) 
 * Your kernel is looking so nice today.
 * Same as strapos but on cstrings rather than safestrings.
 */
int strpos(const char *s, char c)
{
	size_t i;

	if (!s)	return 0;

	for (i = 0; i < strlen(s); ++i)
		if (s[i] == c)
			return i;
	return 0;
}
/* }}} */
/* {{{ int strarep(string *sa, char *s, char *old, char *new)
 * Looks for old in s and replaces with new but in sa. With safe 
 * memory allocation.
 * sa = null  s thecowisinthepasture  old cow new house
 * then sa = thehouseisinthepasture
 */
int strarep(string *sa, char *s, char *old, char *new)
{
	char	*ptr;

	/* check pointers */
	if (!sa)	return 0;
	if (!s)		return 0;
	if (!old)	return 0;
	if (!new)	return 0;

	if (strlen(s) == 0)
		return 0;
	if (strlen(old) == 0)
		return 0;
	/* new is allowed to be 0-length */

	/* search for old */
	ptr = strstr(s, old);
	if (ptr == NULL) {
		/* not found */
		return 0;
	}

	/* check that ptr is within s*/
	if ((size_t)(ptr - s) > strlen(s))
		return 0;

	/* copy the prefix */
	if (!strbcpy(sa, s, (ptr - s)))	return 0;

	/* copy the new text */
	if (!stracat(sa, new))	return 0;

	/* copy the end */
	if (!stracat(sa, (ptr + strlen(old))))	return 0;

	return 1;
}
/* }}} */
/* {{{ int strsplit2(const char *s, int c, string *left, string *right)
 * Split a string into "left" and "right" parts based on first position of
 * character c in string s.
 * If seperator is not found, loads entire string into (left)
 */
int strsplit2(const char *s, int c, string *left, string *right)
{
	int	pos;

	if (!s)	return 0;
	if (!left)	return 0;
	if (!right)	return 0;

	pos = strpos(s, c);
	if (pos <= 0) {
		/* not found, load everything into (left) */
		if (!stracpy(left, s))	return 0;
		if (!strbzero(right))	return 0;
		return 1;
	}

	/* copy the left portion, adding the NULL */
	if (!strbcpy(left, s, pos))	return 0;
	if (!strbcat(left, "\0", 1))	return 0;

	/* pos is the EXACT placement in the string of the (c) */
	if (!strbcpy(right, (s + pos + 1), (strlen(s) - pos - 1)))	return 0;
	if (!strbcat(right, "\0", 1))	return 0;

	return 1;
}
/* }}} */
/* {{{ int stracmp_host(const char *s1, const char *s2) (TODO)
 * special func for comparing hostnames
 *
 * returns 0 if success
 * returns 1 if really not alike
 * returns 2 if **could** be a match (host vs. fqdn compared ok)
 */
int stracmp_host(const char *s1, const char *s2)
{
	int ret;

	ret = strcasecmp(s1, s2);
	if (ret == 0)
		return 0;

	/* first host/fqdn */
	ret = strncasecmp(s1, s2, strpos(s1, '.'));
	if (ret == 0)
		return 2;

	/* second fqdn/host */
	ret = strncasecmp(s1, s2, strpos(s2, '.'));
	if (ret == 0)
		return 2;

	return 1;
}
/* }}} */
/* {{{ int straclean(const char *s, const char *charset, string *new)
 */
int straclean(const char *s, const char *charset, string *new)
{
	int	pos = 0;
	int	abs = 0;
	int len = strlen(s);
	char *ptr;

	if (!s)	return 0;
	if (!charset) return 0;
	if (!new)	return 0;

	/* increment pos past the character we stopped on */
	for (;abs < len; abs++) {

		/* increment absolute */
		abs += pos;

		/* set ptr to last position */
		(const char *)ptr = &s[abs];

		/* see how many characters we can copy */
		pos = strcspn(s, charset);

		if (pos <= 0) {
			/* two chars in a row, just keep looping */
			if (abs < len)
				continue;

			/* we are done */
			if (!strbcat(new, "\0", 1))	return 0;
			return 1;
		}

		/* copy the left portion, adding the NULL */
		if (!strbcpy(new, ptr, pos))	return 0;
	}

	/* another form of done */
	if (!strbcat(new, "\0", 1))	return 0;

	return 1;
}
/* }}} */
/* Integer functions */
/* {{{ int stracatul_p(string *sa, unsigned long ul, unsigned int n)
 * n is the minimum number of chars to use
 */
int stracatul_p(string *sa, unsigned long ul, unsigned int n)
{
	unsigned int len;
	unsigned long q;
	char *s;

	len = 1;
	q = ul;
	while (q > 9) {
		++len;
		q /= 10;
	}

	if (len < n)
		len = n;

	/* +NULL */
	if (!strachk(sa, sa->len + len + 1))	return 0;
	s = sa->s + sa->len;
	sa->len += len;

	/* this works backwards */
	while (len) {
		s[--len] = '0' + (ul % 10);
		ul /= 10;
	}

	return 1;
}
/* }}} */
/* {{{ int stracatl_p(string *sa, long i, unsigned int n)
 * n is the minimum number of chars to use
 */
int stracatl_p(string *sa, long l, unsigned int n)
{
	if (l < 0) {
		if (!stracat(sa, "-"))	return 0;
		l = -l;
	}
	return stracatul_p(sa, l, n);
}
/* }}} */
