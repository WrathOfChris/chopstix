/*	$Gateweaver: cgi.c,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
/*	$UNDEADLY: cgi.c,v 1.5 2005/01/03 08:02:26 dhartmei Exp $ */

/*
 * Copyright (c) 2004 Daniel Hartmeier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

static const char rcsid[] = "$Gateweaver: cgi.c,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $";

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cgi.h"

/* steal these from resolv.h to avoid the rest of its junk */
int	__b64_ntop(unsigned char const *, size_t, char *, size_t);
int	__b64_pton(char const *, unsigned char *, size_t);

int
url_dec(const char *s, tbstring *sa)
{
	static const char *hex = "0123456789abcdef";
	int hd;

	if (tbstrcpy(sa, ""))
		return -1;

	while (*s) {
		if (*s == '%') {
			char *i;

			s++;
			if (*s && (i = strchr(hex, tolower(*s))) != NULL) {
				hd = (i - hex) * 16;
				s++;
				if (*s && (i = strchr(hex, tolower(*s))) != NULL) {
					hd += i - hex;
					s++;
					if (tbstrinsc(sa, hd, sa->len))
						return -1;
				} else {
					/* malformed percent escape */
					if (tbstrinsc(sa, '%', sa->len))
						return -1;
				}
			} else {
				/* malformed percent escape */
				if (tbstrinsc(sa, '%', sa->len))
					return -1;
			}
		} else {
			if (tbstrinsc(sa, *s == '+' ? ' ' : *s, sa->len))
				return -1;
			s++;
		}
	}

	return 0;
}

int
url_enc(const char *str, tbstring *sa)
{
	return url_enc_len(str, strlen(str), sa);
}

int
url_enc_len(const char *str, size_t len, tbstring *sa)
{
	static const char *res = "%;/?:@&=+$";
	static const char *hex = "0123456789abcdef";
	const char *s = str;
	char hd[4];

	if (tbstrcpy(sa, ""))
		return -1;

#define allowed_in_url(c) \
	(isalnum(c) || c == '-' || c == '_' || c == '.')

	while (*s && s < (str + len)) {
		if (*s == ' ') {
			if (tbstrinsc(sa, '+', sa->len))
				return -1;
		} else if (!allowed_in_url(*s) || strchr(res, *s) != NULL) {
			hd[0] = '%';
			hd[1] = hex[*s / 16];
			hd[2] = hex[*s % 16];
			hd[3] = '\0';
			if (tbstrcat(sa, hd))
				return -1;
		} else {
			if (tbstrinsc(sa, *s, sa->len))
				return -1;
		}
		s++;
	}

	return 0;
}

int
b64_enc(const char *s, tbstring *sa)
{
	if (tbstrchk(sa, strlen(s) * 4 / 3 + 4 + 1))
		return -1;
	if (__b64_ntop(s, strlen(s), sa->s, sa->a - 1) == -1) {
		log_warnx("b64_enc: failed len %zd salen %zd", strlen(s), sa->a);
		tbstrfree(sa);
		return -1;
	}

	return 0;
}

int
b64_dec(const char *s, tbstring *sa)
{
	if (tbstrchk(sa, strlen(s) * 3 / 4 + 3 + 1))
		return -1;
	if (__b64_pton(s, sa->s, sa->a - 1) == -1) {
		log_warnx("b64_dec: failed");
		tbstrfree(sa);
		return -1;
	}

	return 0;
}

int
hex_dec(const char *s, tbstring *sa)
{
	static const char *hex = "0123456789abcdef";
	char *i;
	int hd;

	if (tbstrcpy(sa, ""))
		return -1;

	while (*s) {
		if (*s && (i = strchr(hex, tolower(*s))) != NULL) {
			hd = (i - hex) * 16;
			s++;
			if (*s && (i = strchr(hex, tolower(*s))) != NULL) {
				hd += i - hex;
				s++;
				if (tbstrinsc(sa, hd, sa->len))
					return -1;
			}
		} else
			s++;
	}

	return 0;
}

int
tokenize_query_params(char *q, struct query_paramlist *p)
{
	char *r, *s;
	char t;
	struct query_param *n;
	tbstring sa = {0};

	while (*q) {
		if (*q == '&' || *q == ';' || *q == ' ' || *q == '\"') {
			q++;
			continue;
		}
		s = NULL;
		for (r = q + 1; *r; ++r) {
			if (*r == '&' || *r == ';')
				break;
			if (*r == '=' && s == NULL)
				s = r;
		}
		
		/* empty val */
		if (s == NULL && *r == ';')
			s = r;
		else if (s == NULL)
			break;

		if ((n = malloc(sizeof(*n))) == NULL)
			return -1;
		t = *s;
		*s = 0;

		/* decode key */
		bzero(&sa, sizeof(sa));
		if (url_dec(q, &sa) == -1) {
			free(n);
			return -1;
		}

		/* abuse the string to avoid mallocopy */
		n->key = sa.s;

		/* deal with empty val */
		if (s == r)
			*s = t;
		else
			*s++ = t;

		/* handle quotes in value exactly once */
		if (*s == '\"')
			s++;
		if (*(r - 1) == '\"')
			r--;

		t = *r;
		*r = 0;

		/* decode val */
		bzero(&sa, sizeof(sa));
		if (url_dec(s, &sa) == -1) {
			free(n->key);
			free(n);
			return -1;
		}

		/* abuse the string to avoid mallocopy */
		n->val = sa.s;

		LIST_INSERT_HEAD(p, n, entry);
		*r = t;
		q = r;
	}

	return 0;
}

/*
 * Tokenizer that does not assume the source is a query.
 */
int
tokenize_params(char *q, struct query_paramlist *p)
{
	char *r, *s;
	char t;
	struct query_param *n;
	tbstring sa = {0};

	while (*q) {
		if (*q == ';' || *q == ' ' || *q == '\"') {
			q++;
			continue;
		}
		s = NULL;
		for (r = q + 1; *r; ++r) {
			if (*r == ';')
				break;
			if (*r == '=' && s == NULL)
				s = r;
		}
		
		/* empty val */
		if (s == NULL && *r == ';')
			s = r;
		else if (s == NULL)
			break;

		if ((n = malloc(sizeof(*n))) == NULL)
			return -1;
		t = *s;
		*s = 0;

		/* decode key */
		bzero(&sa, sizeof(sa));
		if (url_dec(q, &sa) == -1) {
			free(n);
			return -1;
		}

		/* abuse the string to avoid mallocopy */
		n->key = sa.s;

		/* deal with empty val */
		if (s == r)
			*s = t;
		else
			*s++ = t;

		/* handle quotes in value exactly once */
		if (*s == '\"')
			s++;
		if (*(r - 1) == '\"')
			r--;

		t = *r;
		*r = 0;

		/* decode val */
		bzero(&sa, sizeof(sa));
		if (url_dec(s, &sa) == -1) {
			free(n->key);
			free(n);
			return -1;
		}

		/* abuse the string to avoid mallocopy */
		n->val = sa.s;

		LIST_INSERT_HEAD(p, n, entry);
		*r = t;
		q = r;
	}

	return 0;
}

struct query *
get_query(void)
{
	struct query *q;
	const char *e;

	if ((q = calloc(1, sizeof(*q))) == NULL)
		return NULL;
	LIST_INIT(&q->get_params);
	LIST_INIT(&q->post_params);

	if ((e = getenv("REQUEST_METHOD")) != NULL)
		q->request_method = strdup(e);
	if ((e = getenv("QUERY_STRING")) != NULL) {
		q->query_string = strdup(e);
		if (tokenize_query_params(q->query_string, &q->get_params) == -1) {
			free_query(q);
			return NULL;
		}
	}
	if ((e = getenv("REMOTE_ADDR")) != NULL)
		q->remote_addr = strdup(e);
	if ((e = getenv("HTTP_USER_AGENT")) != NULL)
		q->user_agent = strdup(e);

	return q;
}

/*
 * Simple tokenizer that decodes the name/val pair and acts just like the GET
 * version with the QUERY_STRING environment.  This ignores voff and vlen.
 */
int
tokenize_cb(const char *name, const char *val, size_t vlen,
		void *arg)
{
	struct query_paramlist *qp = arg;
	struct query_param *n;
	tbstring sa;

#ifdef DEBUG_CGI
	log_debug("TOKENIZE_CB: len %u name %s value %.*s", vlen, name, vlen, val);
#endif

	LIST_FOREACH(n, qp, entry)
		if (strcmp(n->key, name) == 0) {
			LIST_REMOVE(n, entry);
			break;
		}
	if (n == LIST_END(qp))
		if ((n = calloc(1, sizeof(*n))) == NULL)
			return -1;

	/* decode key */
	if (n->key == NULL) {
		bzero(&sa, sizeof(sa));
		if (url_dec(name, &sa) == -1) {
			free(n);
			return -1;
		}
		n->key = sa.s;
	}

	/* decode val */
	bzero(&sa, sizeof(sa));
	if (url_dec(val, &sa) == -1) {
		free(n->key);
		free(n->val);
		free(n);
		return -1;
	}
	if (n->val == NULL)
		n->val = sa.s;
	else {
		if (tbstrpre(&sa, n->val)) {
			free(n->key);
			free(n->val);
			free(n);
			return -1;
		}
		free(n->val);
		n->val = sa.s;
	}

	LIST_INSERT_HEAD(qp, n, entry);

	return 0;
}

/*
 * Same as strstr(3), but uses memcmp to deal with binary load lifters.
 * Based on: $OpenBSD: strstr.c,v 1.5 2005/08/08 08:05:37 espie Exp $
 */
static char *
memstr(const char *str, size_t slen, const char *find)
{
	char *s, c, sc;
	size_t len;

	(const char *)s = str;
	if ((c = *find++) != 0) {
		len = strlen(find);
		do {
			do {
				sc = *s++;
				if (s > (str + slen - len))
					return NULL;
			} while (sc != c);
		} while (memcmp(s, find, len) != 0);
		s--;
	}

	return s;
}

/*
 * nameval may specify an extra field that should be retrieved from the
 * headers.
 */
static int
parse_mime_headers(const char *str, size_t len, char **name, char **nameval)
{
	struct query_paramlist cdp;
	char *s, *e, *ename = NULL;
	int n;

	LIST_INIT(&cdp);
	*name = NULL;

	if (nameval) {
		if (*nameval)
			ename = *nameval;
		*nameval = NULL;
	}

	/* Ensure the headers are terminated correctly */
	if ((s = memstr(str, len, "\r\n\r\n")) == NULL)
		return -1;
	n = (s - str) + 4;

#ifdef DEBUG_CGI
	(const char *)s = str;
	while ((e = memstr(s, (str + len - s), "\r\n")) && e < (str + len)) {
		if ((e - s) <= 2)
			break;
		else
			log_debug("PARSE_MIME: %.*s", e - s, s);
		s = e + 2;
	}
#endif

	/* Content-Disposition */
	if ((s = memstr(str, len, CT_DISPOSITION ": ")) == NULL || s > (str + len))
		return -1;
	s += strlen(CT_DISPOSITION) + 2;
	if ((e = memstr(s, (str + len - s), "\r\n")) == NULL || e > (str + len))
		return -1;
	*e = '\0';
	if (tokenize_query_params(s, &cdp) == -1) {
		*e = '\r';
		goto fail;
	}
	*e = '\r';

	if (((const char *)s = get_param(&cdp, "name")) == NULL ||
			(*name = strdup(s)) == NULL)
		goto fail;
	if (ename)
		if (((const char *)s = get_param(&cdp, ename)) != NULL &&
				(*nameval = strdup(s)) == NULL)
			goto fail;

	free_query_paramlist(&cdp);
	return n;

fail:
	free_query_paramlist(&cdp);
	return -1;
}

/*
 * Read POST stream from stdin, running (callback)(name, value, voff, vlen) for
 * each multipart detected.  Large portions are broken up into (bsz) sized
 * blocks.
 */
int
read_post(struct query *q, size_t bsz,
		int (*callback)(const char *, const char *, size_t, void *), void *arg)
{
	struct query_paramlist ctp;
	struct query_param *qp;

	char *buf = NULL, *c_type, *bound;
	char *s, *b_beg, *b_end;
	char *name = NULL, *nameval = NULL;
	size_t buflen, n, b_off;
	int hn;
	tbstring sa = {0};

	if (bsz <= 0)
		bsz = BUFSIZ;

	/*
	 * Parse the content type of the POST data.
	 */
	LIST_INIT(&ctp);
	if ((c_type = getenv("CONTENT_TYPE")) == NULL)
		return -1;

	/* application/x-www-form-urlencoded */
	if (strcmp(c_type, CT_URLENCODED) == 0) {
		if (tbstrcpy(&sa, ""))
			goto fail;
		if ((buf = malloc(BUFSIZ)) == NULL)
			goto fail;
		while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0)
			if (tbstrbcat(&sa, buf, n))
				goto fail;
		free_query_paramlist(&ctp);
		if (tokenize_query_params(sa.s, &ctp) == -1)
			goto fail;
		LIST_FOREACH(qp, &ctp, entry)
			if (callback(qp->key, qp->val, strlen(qp->val), arg) == -1)
				goto fail;
		goto out;
	}

	/* only support "multipart/form-data" for now */
	if (tokenize_query_params(c_type, &ctp) == -1)
		goto fail;
	if (((const char *)bound = get_param(&ctp, CT_MULTIPART)) == NULL) {
		log_warnx("unsupported CONTENT_TYPE \"%s\"", c_type);
		goto fail;
	}

	/* Get the boundary separator */
	if (((const char *)bound = get_param(&ctp, CT_BOUNDARY)) == NULL) {
		log_warnx("CONTENT_TYPE has no \"%s\"", CT_BOUNDARY);
		goto fail;
	}

	/* Allocate (bsz) plus two boundaries including the extra '--'s */
	buflen = bsz + 2 * (2 + strlen(bound));
	if ((buf = calloc(1, buflen + 1)) == NULL)
		goto fail;

	b_off = 0;
	b_beg = b_end = NULL;
	while ((n = fread(buf + b_off, 1, buflen - b_off, stdin)) > 0) {
#ifdef DEBUG_CGI
		log_debug("CGI READ boff %u blen %u", b_off, buflen - b_off);
#endif

		s = buf + b_off;
		while (s < (buf + n)) {
#ifdef DEBUG_CGI
			log_debug("CGI BND boff %u", (s - buf));
#endif
			if ((s = memstr(s, buflen - (s - buf), "--")) == NULL)
				break;
			s += 2;
			if (strncmp(s, bound, strlen(bound)))
				continue;

			/* NEXT run, shift boundaries */
			if (b_beg && b_end) {
				/* "\r\n--" + "\r\n" */
				b_beg = b_end + 4 + strlen(bound) + 2;
				b_end = NULL;
#ifdef DEBUG_CGI
				log_debug("CGI BEG NXT BOUNDARY off %d", b_beg - buf);
#endif
			}

			/* FIRST run, locate end of this boundary */
			if (b_beg == NULL) {
				s += strlen(bound);
				while (*s == '\r' || *s == '\n')
					s++;
				b_beg = s;
#ifdef DEBUG_CGI
				log_debug("CGI BEG 1ST BOUNDARY off %d", b_beg - buf);
#endif
				continue;
			}

			/* locate beginning of next boundary */
			if (b_end == NULL) {
				b_end = s - 4;	/* "\r\n--" */
				if (memcmp(b_end, "\r\n--", 4)) {
					log_warnx("invalid mime end boundary");
					goto fail;
				}
				s += strlen(bound);
#ifdef DEBUG_CGI
				log_debug("CGI END BOUNDARY off %d", b_end - buf);
#endif

				/* parse headers if not yet done */
				if (name == NULL) {
					nameval = CT_FILENAME;
					if ((hn = parse_mime_headers(b_beg, b_end - b_beg, &name,
									&nameval)) == -1) {
						log_warnx("invalid mime header");
						goto fail;
					}

					/* shift to the beginning of data */
					b_beg += hn;

#ifdef DEBUG_CGI
					log_debug("CGI HDR BOUNDARY off %d", b_beg - buf);
#endif
				}
				*b_end = '\0';
				if (nameval)
					if (callback(CT_FILENAME, nameval, strlen(nameval), arg)
							== -1) {
						*b_end = '\r';
						goto fail;
					}
				if (callback(nameval ? nameval : name, b_beg, b_end - b_beg,
							arg) == -1) {
					*b_end = '\r';
					goto fail;
				}
				*b_end = '\r';

				/* onto the next deal */
				free(name);
				free(nameval);
				name = nameval = NULL;

				/* "\r\n--" + "\r\n" */
				b_beg = b_end + 4 + strlen(bound) + 2;
				b_end = NULL;
#ifdef DEBUG_CGI
				log_debug("CGI BEG BOUNDARY off %d", b_beg - buf);
#endif

				continue;
			}
		}
		b_off += n;

		/*
		 * END boundary was not found within the message.  
		 */

		/* Pull the name and value out of the content type */
		if ((b_end == NULL || b_end > buf + bsz) && name == NULL) {
			nameval = CT_FILENAME;
			if ((hn = parse_mime_headers(b_beg, buflen - (b_beg - buf), &name,
							&nameval)) == -1) {
				/* headers do not fit, nor will they */
				if (b_beg == buf) {
					log_warnx("header will overflow block buffer %zd bytes",
							bsz);
					goto fail;
				}

				/* do a shift */
#ifdef DEBUG_CGI
				log_debug("CGI NOHEADER SHIFT from %u+%u", (b_beg - buf),
						buflen - (b_beg - buf));
#endif
				memmove(buf, b_beg, buflen - (b_beg - buf));
				b_off = buflen - (b_beg - buf);
				b_beg = buf;
				free(name);
				free(nameval);
				name = nameval = NULL;
				continue;
			}
			if (nameval)
				if (callback(CT_FILENAME, nameval, strlen(nameval), arg) == -1)
					goto fail;

			/* Header is still in the buffer, lets do the shift */
#if DEBUG_CGI
			log_debug("CGI HEADER SHIFT from %u+%u", (b_beg + hn - buf),
					buflen - (b_beg + hn - buf));
#endif
			memmove(buf, b_beg + hn, buflen - (b_beg + hn - buf));
			b_off = buflen - (b_beg + hn - buf);
			b_beg = buf;
		}

		/* Only run callbacks on full buffers */
		if (b_beg == buf && (b_end == NULL || b_end >= buf + bsz)) {
			if (b_off >= bsz) {
				char t = *(buf + bsz);
				*(buf + bsz) = '\0';
				if (callback(nameval ? nameval : name, buf, bsz, arg) == -1) {
					*(buf + bsz) = t;
					goto fail;
				}
				*(buf + bsz) = t;
#ifdef DEBUG_CGI
				log_debug("CGI DATA SHIFT from %u+%u", bsz, buflen - bsz);
#endif
				memmove(buf, buf + bsz, buflen - bsz);
				b_off = buflen - bsz;
				b_beg = buf;
			}
			/* not enough in buffer yet */
			continue;
		}
	}

out:
	tbstrfree(&sa);
	free(name);
	free(nameval);
	free_query_paramlist(&ctp);
	free(buf);
	return 0;

fail:
	tbstrfree(&sa);
	free(name);
	free(nameval);
	free_query_paramlist(&ctp);
	free(buf);
	return -1;
}

void
free_query_paramlist(struct query_paramlist *qp)
{
	struct query_param *p;

	while ((p = LIST_FIRST(qp)) != NULL) {
		LIST_REMOVE(p, entry);
		free(p->key);
		free(p->val);
		free(p);
	}
}

void
free_query(struct query *q)
{
	if (q->request_method != NULL)
		free(q->request_method);
	if (q->query_string != NULL)
		free(q->query_string);
	if (q->remote_addr != NULL)
		free(q->remote_addr);
	if (q->user_agent != NULL)
		free(q->user_agent);
	free_query_paramlist(&q->get_params);
	free_query_paramlist(&q->post_params);
	free(q);
}

const char *
get_param(const struct query_paramlist *qp, const char *key)
{
	const struct query_param *p;

	LIST_FOREACH(p, qp, entry)
		if (!strcasecmp(p->key, key))
			return p->val;

	return NULL;
}

const char *
get_query_param(const struct query *q, const char *key)
{
	return get_param(&q->get_params, key);
}

const char *
get_post_param(const struct query *q, const char *key)
{
	return get_param(&q->post_params, key);
}

const char *
get_cookie_val(const char *qkey)
{
	char *c, *d, *e;
	char key[64];
	static char val[4096];

	if (snprintf(key, sizeof(key), "%s=", qkey) == -1)
		return NULL;

	if ((c = getenv("HTTP_COOKIE")) == NULL || (d = strdup(c)) == NULL)
		return NULL;

	c = d;
	while (*c == ' ')
		c++;
	while (c && *c && strncmp(c, key, strlen(key))) {
		c = strchr(c, ';');
		while (c && (*c == ';' || *c == ' '))
			c++;
	}
	if (!c || !*c) {
		free(d);
		return NULL;
	}
	c += strlen(key);

	e = c;
	while (*e && *e != ';')
		e++;
	*e = '\0';

	if (strlcpy(val, c, sizeof(val)) >= sizeof(val)) {
		free(d);
		return NULL;
	}
	free(d);

	return val;
}

/*
 * Safely escape HTML.  (s) is source, (d) is destination.
 * Set (allownl) to transform \n|\r to <br>
 */
char *
html_esc(const char *s, tbstring *sa, int allownl)
{
	return html_esc_len(s, strlen(s), sa, allownl);
}

char *
html_esc_len(const char *str, size_t len, tbstring *sa, int allownl)
{
	const char *s = str;
	if (tbstrcpy(sa, ""))
		goto overflow;

	for (; *s && s < (str + len); ++s)
		switch (*s) {
			case '&':
				if (tbstrcat(sa, "&amp;"))
					goto overflow;
				break;
			case '\"':
				if (tbstrcat(sa, "&quot;"))
					goto overflow;
				break;
			case '<':
				if (tbstrcat(sa, "&lt;"))
					goto overflow;
				break;
			case '>':
				if (tbstrcat(sa, "&gt;"))
					goto overflow;
				break;
			case '\r':
			case '\n':
				if (!allownl) {
					/* skip */
					break;
				} else if (allownl > 1 && *s == '\r') {
					if (tbstrcat(sa, "<br>"))
						goto overflow;
					break;
				}
				/* FALLTHROUGH */
			default:
				if (tbstrinsc(sa, *s, sa->len))
					goto overflow;
		}

	return sa->s;

overflow:
	return "(internal overflow)";
}

void
dump_params(struct query_paramlist *qp)
{
	struct query_param *p;
	LIST_FOREACH(p, qp, entry)
		printf("NAME \"%s\" VALUE \"%s\"\n",
				p->key, p->val);
}
