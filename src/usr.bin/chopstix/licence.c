/* $Gateweaver: licence.c,v 1.6 2007/08/19 05:09:53 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <err.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include "chopstix.h"
#include "licence.h"

RCSID("$Gateweaver: licence.c,v 1.6 2007/08/19 05:09:53 cmaxwell Exp $");

#define LICENCE_1_WEEK		1186545600		/* 20070801 + 1W = 20070808 */
#define LICENCE_25_YEARS	1974945600		/* 20070801 + 25Y = 20320801 */
#define LICENCE_PHONE_MIN	7				/* NPA-NXX# */

struct licence_x509_ctx {
	int depth;
};

time_t licence_time;
ASN1_TIME *licence_expiry = NULL;
char *licence_crlfile = REVOKEFILE;
int licence_crl_force = 0;

int licence_error_cb(int, X509_STORE_CTX *);
static const char * licence_mangle_text(const char *, size_t);
static const char * licence_mangle_phone(const char *, size_t);
int licence_verify_base64(const char *, X509 *,
		const char *, size_t);
static const char * licence_phone2str(const ChopstixPhone *);
static void licence_str2phone(const char *, ChopstixPhone *);
void licence_close(void);

int
licence_init(void)
{
	STACK_OF(X509) *chain = NULL;
	BIO *bio = NULL;
	X509 *cacert = NULL, *licence = NULL;
	X509_NAME *name;
	X509_NAME_ENTRY *ne;
	X509_CRL *crl = NULL;
	X509_STORE *store = NULL;
	X509_STORE_CTX *store_ctx = NULL;
	ASN1_TIME *expiry;
	struct licence_x509_ctx lvarg = {0};
	char *buf = NULL, *l_real = NULL, *l_name;
	FILE *fp = NULL;
	int i, nid, use_crl = 1, l_must = 0, l_have = 0;
	int NID_postalAddress, NID_telephoneNumber;

	/* Set licence authorization time to prevent system clock tomfoolery */
	licence_time = time(NULL);

	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();

	/* Load special licensing OIDs */
	if ((NID_postalAddress = OBJ_create("2.5.4.16", "postalAddress",
					"postalAddress")) == NID_undef) {
		warnx("cannot register licence postalAddress");
		goto fail;
	}
	if ((NID_telephoneNumber = OBJ_create("2.5.4.20", "telephoneNumber",
					"telephoneNumber")) == NID_undef) {
		warnx("cannot register licence telephoneNumber");
		goto fail;
	}

	if ((chain = sk_X509_new_null()) == NULL) {
		warnx("cannot allocate licence chain memory");
		goto fail;
	}

	/* Read CA Cert chain */
	for (i = 0; i < sizeof(licence_cacert_der)/sizeof(licence_cacert_der[0]);
			i++) {
		if ((bio = BIO_new_mem_buf(licence_cacert_der[i].cert,
						licence_cacert_der[i].certlen)) == NULL) {
			warnx("cannot allocate licence authority %d", i + 1);
			goto fail;
		}
		if (d2i_X509_bio(bio, &cacert) == NULL) {
			warnx("cannot read licence authority %d: %s", i + 1,
					ERR_reason_error_string(ERR_get_error()));
			goto fail;
		}
		BIO_free(bio);
		bio = NULL;

		/* Print subject names */
		if ((name = X509_get_subject_name(cacert)) == NULL ||
				(buf = X509_NAME_oneline(name, 0, 0)) == NULL) {
			warnx("cannot name licence authority %d: %s", i + 1,
					ERR_reason_error_string(ERR_get_error()));
			goto fail;
		}
		if (verbose)
			printf("Licence CA %d: %s\n", i + 1, buf);
		free(buf);
		buf = NULL;

		if (sk_X509_push(chain, cacert) == 0) {
			warnx("cannot stack licence authority %d: %s", i + 1,
					ERR_reason_error_string(ERR_get_error()));
			goto fail;
		}

		/* chain now owns cacert */
		cacert = NULL;
	}

	/*
	 * CRL
	 */
	if ((fp = fopen(licence_crlfile, "r")) == NULL && licence_crl_force) {
		warn("cannot open revocation list");
		goto fail;
	}
	if (fp && (crl = PEM_read_X509_CRL(fp, 0, 0, 0)) == NULL) {
		warnx("cannot read revocation list: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}
	if (fp)
		fclose(fp);
	fp = NULL;

	/*
	 * Licence Certificate
	 */
	if (strlen(config.licence) == 0)
		strlcpy(config.licence, LICENCEFILE, sizeof(LICENCEFILE));
	if ((fp = fopen(config.licence, "r")) == NULL) {
		warn("cannot open licence file \"%s\"", config.licence);
		goto fail;
	}
	if ((licence = PEM_read_X509(fp, 0, 0, 0)) == NULL) {
		warnx("cannot read licence: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}
	fclose(fp);
	fp = NULL;

	if ((store = X509_STORE_new()) == NULL ||
			(store_ctx = X509_STORE_CTX_new()) == NULL) {
		warnx("cannot allocate licence store: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}
	if (X509_STORE_CTX_init(store_ctx, store, licence, NULL) == NULL) {
		warnx("cannot setup licence store: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}

	X509_STORE_CTX_set_app_data(store_ctx, &lvarg); 
	X509_STORE_CTX_set_verify_cb(store_ctx, licence_error_cb); 
	X509_STORE_CTX_trusted_stack(store_ctx, chain); 

	/*
	 * If anyone is still using this version past 2032 they deserve to have the
	 * licence validate forever.
	 */
	if (sizeof(time_t) == 4 && time(NULL) > LICENCE_25_YEARS) {
		X509_STORE_CTX_set_time(store_ctx, 0, LICENCE_25_YEARS);
		use_crl = 0;
	}

	/* Use CRL when present or required */
	if (use_crl && (crl || licence_crl_force)) {
		if (X509_STORE_add_crl(store_ctx->ctx, crl) <= 0) {
			warnx("cannot store revocation list: %s",
					ERR_reason_error_string(ERR_get_error()));
			goto fail;
		}
		X509_STORE_CTX_set_flags(store_ctx, X509_V_FLAG_CRL_CHECK);
	}

	if ((i = X509_verify_cert(store_ctx)) <= 0) {
		if (ERR_get_error())
			warnx("cannot verify licence: %s",
					ERR_reason_error_string(ERR_get_error()));
		else
			warnx("cannot verify licence");
		goto fail;
	}

	/* Preserve Expiry */
	if ((expiry = X509_get_notAfter(licence)) == NULL)
		warnx("certificate does not expire");
	if (licence_expiry)
		M_ASN1_TIME_free(licence_expiry);
	licence_expiry = M_ASN1_TIME_dup(expiry);

	/*
	 * Licence test
	 */
	if ((name = X509_get_subject_name(licence)) == NULL) {
		warnx("cannot get licence name: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}
	for (i = 0; i < sk_X509_NAME_ENTRY_num(name->entries); i++) {
		if ((ne = sk_X509_NAME_ENTRY_value(name->entries, i)) == NULL) {
			warnx("cannot extract licencee name: %s",
					ERR_reason_error_string(ERR_get_error()));
			goto fail;
		}
		nid = OBJ_obj2nid(ne->object);
		if (nid == NID_undef ||
				((const char *)l_name = OBJ_nid2sn(nid)) == NULL)
			continue;

		if (verbose)
			fprintf(stderr, "Licence Component %d: [%d/%d] %s=%.*s\n",
					ne->set, nid, ne->value->type, l_name,
					ne->value->length, ne->value->data);

		/*
		 * C=CA, ST=Ontario, L=Waterdown, O=Manorsoft,
		 * OU=1 Main St
		 * CN=New Cuisine/emailAddress=9056890000@chopstix.manorsoft.ca
		 *
		 * CN must match licencee name
		 * OU must match address
		 * emailAddress must match telephone number
		 */

		l_real = NULL;
		if (nid == NID_countryName ||
				nid == NID_stateOrProvinceName) {
			/* XXX Lookup tables required */
		} else if (nid == NID_organizationName) {
			/* Company Name does not have to match */
		} else if (nid == NID_pkcs9_emailAddress) {
			/* Email address is KRB username */
		} else if (nid == NID_localityName ||
				nid == NID_commonName ||
				nid == NID_postalAddress) {
			l_must++;
			l_real = strdup(licence_mangle_text(ne->value->data,
						ne->value->length));
		} else if (nid == NID_telephoneNumber) {
			l_must++;
			l_real = strdup(licence_mangle_phone(ne->value->data,
						ne->value->length));
		} else {
			fprintf(stderr, "licence contains unknown object \"%s\"\n",
					l_name);
		}

		/* Required field did not mangle correctly, FAIL */
		if (l_real == NULL)
			continue;

		if (nid == NID_commonName) {
			if (header.company == NULL) {
				if ((header.company = calloc(1, ne->value->length + 1))
						== NULL)
					goto fail;
				bcopy(ne->value->data, header.company, ne->value->length);
			}
			if (strcmp(l_real, licence_mangle_text(header.company,
							strlen(header.company))) != 0)
				warnx("licence is only for use by \"%.*s\"",
						ne->value->length, ne->value->data);
			else
				l_have++;
		} else if (nid == NID_postalAddress) {
			if (header.addr.addr == NULL) {
				if ((header.addr.addr = calloc(1, ne->value->length + 1))
						== NULL)
					goto fail;
				bcopy(ne->value->data, header.addr.addr, ne->value->length);
			}
			if (strcmp(l_real, licence_mangle_text(header.addr.addr,
							strlen(header.addr.addr))) != 0)
				warnx("licence is only for use at \"%.*s\"",
						ne->value->length, ne->value->data);
			else
				l_have++;
		} else if (nid == NID_localityName) {
			if (header.city == NULL) {
				if ((header.city = calloc(1, ne->value->length + 1))
						== NULL)
					goto fail;
				bcopy(ne->value->data, header.city, ne->value->length);
			}
			if (strcmp(l_real, licence_mangle_text(header.city,
							strlen(header.city))) != 0)
				warnx("licence is only for use in the city of \"%.*s\"",
						ne->value->length, ne->value->data);
			else
				l_have++;
		} else if (nid == NID_telephoneNumber) {
			if (header.phone.npa == 0 && header.phone.nxx == 0 &&
					header.phone.num == 0)
				licence_str2phone(l_real, &header.phone);
			if (strstr(l_real, licence_phone2str(&header.phone)) == NULL)
				warnx("licence is only for use with phone \"%s\"", l_real);
			else
				l_have++;
		} else if (nid == NID_countryName) {
			if (header.country == NULL) {
				if ((header.country = calloc(1, ne->value->length + 1))
						== NULL)
					goto fail;
				bcopy(ne->value->data, header.country, ne->value->length);
			}
			/* just update the config */
		} else if (nid == NID_stateOrProvinceName) {
			if (header.province == NULL) {
				if ((header.province = calloc(1, ne->value->length + 1))
						== NULL)
					goto fail;
				bcopy(ne->value->data, header.province, ne->value->length);
			}
			/* just update the config */
		}
		free(l_real);
		l_real = NULL;
	}

	if (l_have != l_must) {
		warnx("licence verification failed %d/%d", l_must - l_have, l_must);
		goto fail;
	}

	/*
	 * Cleanup
	 */
	X509_STORE_CTX_cleanup(store_ctx);
	X509_STORE_free(store);
	if (crl)
		X509_CRL_free(crl);
	X509_free(licence);
	sk_X509_free(chain);
	return 0;

fail:
	if (l_real)
		free(l_real);
	if (store_ctx)
		X509_STORE_CTX_cleanup(store_ctx);
	if (store)
		X509_STORE_free(store);
	if (licence)
		X509_free(licence);
	if (crl)
		X509_CRL_free(crl);
	if (fp)
		fclose(fp);
	if (buf)
		free(buf);
	if (cacert)
		X509_free(cacert);
	if (bio)
		BIO_free(bio);
	if (chain)
		sk_X509_free(chain);
	return -1;
}

int
licence_error_cb(int ok, X509_STORE_CTX *context)
{
	struct licence_x509_ctx *ctx = X509_STORE_CTX_get_app_data(context);
	X509_NAME *name = X509_get_subject_name(context->current_cert); 
	char *buffer = X509_NAME_oneline(name, 0, 0);
	ctx->depth += 1;
	if (!ok) {
		warnx("Licence verification FAILED: %s",
				X509_verify_cert_error_string(context->error));
		warnx("Licence failure details: %s", buffer);
	}
	free(buffer);
	return ok;
}

/*
 * Verify that (str/len) matches the base64 encoded signature in (filename) was
 * signed by (cert).
 */
int
licence_verify_base64(const char *filename, X509 *cert,
		const char *str, size_t len)
{
	BIO *bio = NULL, *bio64 = NULL;
	EVP_PKEY *pubkey;
	EVP_MD_CTX evp_ctx = {0};
	size_t siglen, n;
	char *buf = NULL;

	if ((pubkey = X509_get_pubkey(cert)) == NULL) {
		warnx("cannot retrieve licence public key: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}

	if ((bio = BIO_new_file(filename, "r")) == NULL ||
			(bio64 = BIO_new(BIO_f_base64())) == NULL || 
			BIO_push(bio64, bio) == NULL) {
		warnx("cannot retrieve licence signature: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}

	siglen = EVP_PKEY_size(pubkey);
	if ((buf = calloc(1, siglen)) == NULL)
		goto fail;
	if ((n = BIO_read(bio64, buf, siglen)) != siglen) {
		warnx("cannot read licence signature %zd/%zd: %s", n, siglen,
				ERR_reason_error_string(ERR_get_error()));
		ERR_print_errors_fp(stderr);
		goto fail;
	}
	BIO_free(bio64);
	bio64 = NULL;
	BIO_free(bio);
	bio = NULL;

	EVP_MD_CTX_init(&evp_ctx);
	if (EVP_VerifyInit(&evp_ctx, EVP_sha1()) <= 0) {
		warnx("cannot initialize verification context: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}
	if (EVP_VerifyUpdate(&evp_ctx, str, len) <= 0) {
		warnx("cannot update verification context: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}
	if (EVP_VerifyFinal(&evp_ctx, buf, siglen, pubkey) <= 0) {
		warnx("cannot verify licence data: %s",
				ERR_reason_error_string(ERR_get_error()));
		goto fail;
	}
	EVP_MD_CTX_cleanup(&evp_ctx);

	return 0;

fail:
	EVP_MD_CTX_cleanup(&evp_ctx);
	if (buf)
		free(buf);
	if (bio64)
		BIO_free(bio64);
	if (bio)
		BIO_free(bio);
	return -1;
}

static const char *
licence_mangle_text(const char *str, size_t len)
{
	static string sa;
	char *s;

	if (!stracpy(&sa, ""))
		return NULL;
	for ((const char *)s = str; *s && s < str + len; s++)
		if (isalnum(*s))
			if (!strainsc(&sa, tolower(*s), sa.len))
				return NULL;

	return sa.s;
}

static const char *
licence_mangle_phone(const char *str, size_t len)
{
	static string sa;
	char *s;

	if (!stracpy(&sa, ""))
		return NULL;
	for ((const char *)s = str; *s && s < str + len; s++) {
		if (isdigit(*s))
			if (!strainsc(&sa, tolower(*s), sa.len))
				return NULL;
		if (*s == '@')
			break;
	}

	return sa.s;
}

static const char *
licence_phone2str(const ChopstixPhone *phone)
{
	static char buf[sizeof("NPANXXNMBR")];
	int len;

	if (phone == NULL) {
		strlcpy(buf, "0000000000", sizeof(buf));
		return buf;
	}

	if (phone->npa) {
		if ((len = snprintf(buf, sizeof(buf), "%03d%03d%04d",
						phone->npa, phone->nxx, phone->num)) == -1)
			return NULL;
	} else {
		if ((len = snprintf(buf, sizeof(buf), "%03d%04d",
						phone->nxx, phone->num)) == -1)
			return NULL;
	}

	return buf;
}

static void
licence_str2phone(const char *str, ChopstixPhone *phone)
{
	long long val;
	val = strtonum(str, 0, 19999999999LL, NULL);
	/* strip leading one */
	if (val >= 10000000000LL)
		val -= 10000000000LL;
	phone->num = val % 10000;
	val -= phone->num;
	phone->nxx = (val / 10000) % 1000;
	val -= phone->nxx * 10000;
	phone->npa = (val / 10000000);
}

int
licence_valid(void)
{
	time_t now = time(NULL);
	if (now < (licence_time - LICENCE_SKEW)) {
		status_warn("licence invalidated by system clock skew");
		return 0;
	}

	if (now <= LICENCE_25_YEARS && X509_cmp_time(licence_expiry, &now) < 0) {
		status_warn("licence has expired");
		return 0;
	}

	return 1;
}

void
licence_close(void)
{
	ERR_free_strings();
}
