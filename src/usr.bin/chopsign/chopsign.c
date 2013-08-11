/* $Gateweaver: chopsign.c,v 1.2 2007/08/10 19:06:14 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s <keyfile> <datafile>\n", __progname);
	exit(1);
}

int
main(int argc, char **argv)
{
	FILE *fp_key = NULL;
	BIO *bio = NULL, *b64 = NULL;
	EVP_MD_CTX evp_ctx = {0};
	EVP_PKEY *licence_key = NULL;
	char buf[1024], *sig;
	size_t n;
	int ch, siglen;

	while ((ch = getopt(argc, argv, "")) != -1) {
		switch (ch) {
			default:
				usage();
				/* NOTREACHED */
		}
	}
	argv += optind;
	argc -= optind;
	if (argc < 2)
		usage();

	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();

	if ((fp_key = fopen(argv[0], "r")) == NULL)
		err(1, "cannot open key \"%s\"", argv[0]);
	if (PEM_read_PrivateKey(fp_key, &licence_key, NULL, NULL) == NULL)
		errx(1, "cannot read key: %s",
				ERR_reason_error_string(ERR_get_error()));

	siglen = EVP_PKEY_size(licence_key);
	if ((sig = calloc(1, siglen)) == NULL)
		err(1, "calloc");

	if ((bio = BIO_new_file(argv[1], "r")) == NULL)
		errx(1, "cannot open data: %s",
				ERR_reason_error_string(ERR_get_error()));

	/*
	 * SIGN
	 */
	EVP_MD_CTX_init(&evp_ctx);
	if (EVP_SignInit(&evp_ctx, EVP_sha1()) <= 0)
		errx(1, "cannot sign init: %s",
				ERR_reason_error_string(ERR_get_error()));
	while ((n = BIO_read(bio, buf, sizeof(buf))) > 0)
		if (EVP_SignUpdate(&evp_ctx, buf, n) <= 0)
			errx(1, "cannot sign push: %s",
					ERR_reason_error_string(ERR_get_error()));
	if (EVP_SignFinal(&evp_ctx, sig, &siglen, licence_key) <= 0)
		errx(1, "cannot sign data: %s",
				ERR_reason_error_string(ERR_get_error()));
	BIO_free(bio);
	bio = NULL;

	/*
	 * OUTPUT: base64
	 */
	if ((b64 = BIO_new(BIO_f_base64())) == NULL)
		errx(1, "cannot init output encoding: %s",
				ERR_reason_error_string(ERR_get_error()));
	if ((bio = BIO_new_fp(stdout, BIO_NOCLOSE)) == NULL)
		errx(1, "cannot init output: %s",
				ERR_reason_error_string(ERR_get_error()));
	if ((bio = BIO_push(b64, bio)) == NULL)
		errx(1, "cannot push output: %s",
				ERR_reason_error_string(ERR_get_error()));
	if ((n = BIO_write(bio, sig, siglen)) != siglen) {
		errx(1, "cannot write output %zd/%d: %s", n, siglen,
				ERR_reason_error_string(ERR_get_error()));
	}
	BIO_flush(bio);
	BIO_free_all(bio);
	bio = NULL;

	exit(0);
}
