/*	$Gateweaver: cgi.h,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
/*	$UNDEADLY: cgi.h,v 1.4 2004/08/21 13:57:28 dhartmei Exp $ */

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

#ifndef _CGI_H_
#define _CGI_H_

#include <sys/types.h>
#include <sys/queue.h>
#include <toolbox.h>

#define CT_MULTIPART	"multipart/form-data"
#define CT_URLENCODED	"application/x-www-form-urlencoded"
#define CT_BOUNDARY		"boundary"
#define CT_DISPOSITION	"Content-Disposition"
#define CT_CONTENTTYPE	"Content-Type"
#define CT_FILENAME		"filename"

struct query_param;
LIST_HEAD(query_paramlist, query_param);

struct query {
	char			*request_method;
	char			*query_string;
	char			*remote_addr;
	char			*user_agent;
	struct query_paramlist get_params;
	struct query_paramlist post_params;
};

struct query_param {
	char *key;
	char *val;
	LIST_ENTRY(query_param) entry;
};

int url_dec(const char *, tbstring *);
int url_enc(const char *, tbstring *);
int url_enc_len(const char *, size_t, tbstring *);
int b64_enc(const char *, tbstring *);
int b64_dec(const char *, tbstring *);
int hex_dec(const char *, tbstring *);

int tokenize_query_params(char *, struct query_paramlist *);
int tokenize_params(char *, struct query_paramlist *);
struct query *get_query(void);
int read_post(struct query *, size_t,
		int (*)(const char *, const char *, size_t, void *), void *);
int tokenize_cb(const char *, const char *, size_t, void *);
const char * get_param(const struct query_paramlist *, const char *);
const char *get_query_param(const struct query *, const char *);
const char *get_post_param(const struct query *, const char *);
const char *get_cookie_val(const char *key);
void free_query_paramlist(struct query_paramlist *);
void free_query(struct query *q);
char * html_esc(const char *, tbstring *, int);
char * html_esc_len(const char *, size_t, tbstring *, int);
void dump_params(struct query_paramlist *);

#endif
