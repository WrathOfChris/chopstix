/* $Gateweaver: rollsum.c,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
/*
 * Copyright (c) 2005-2007 Christopher Maxwell.  All rights reserved.
 */
#include <sys/types.h>
#include <stdlib.h>
#include "toolbox.h"

/*
 * s1	Simple adder.  This value can be combined.
 * 		s1(a) + s1(b) == s1(a+b)
 *
 * s2	Accumulator.  This value can be combined using s1 and len
 * 		s2(a) + (s1(a) * len) + s2(b) == s2(a+b)
 * 		s2(abcd) - (s1(a) * blen) - (s1(b) * (abcdlen - blen - cdlen)) - s2(b)
 * 			== s2(acd)
 */

#define ROLL32Normalize(ctx) do {	\
	(ctx)->s1 &= 0x0000ffff;		\
	(ctx)->s2 &= 0x0000ffff;		\
} while (0)

/*
 * 32-bit checksum based on the rsync checksum variation of Adler-32
 * Based on rsync-2.6.6 checksum.c:get_checksum1()
 */
void
ROLL32Update(ROLL32_CTX *ctx, const uint8_t *data, size_t len)
{
	uint32_t s1 = ctx->s1;
	uint32_t s2 = ctx->s2;

	ctx->len += len;

	/* this is just a speedup for procesing 4bytes at a time */
	while (len >= 4) {
		s2 += 4 * (s1 + data[0])
			+ 3 * data[1]
			+ 2 * data[2]
			+ data[3]
			+ 10 * ROLL32_CHAR_OFFSET;
		s1 += data[0]
			+ data[1]
			+ data[2]
			+ data[3]
			+ 4 * ROLL32_CHAR_OFFSET;
		data += 4;
		len -= 4;
	}

	/* this is the real algorithm */
	while (len != 0) {
		s1 += *data + ROLL32_CHAR_OFFSET;
		s2 += s1;
		data++;
		len--;
	}

	ctx->s1 = s1;
	ctx->s2 = s2;
	ROLL32Normalize(ctx);
}

void
ROLL32Final(uint8_t digest[ROLL32_DIGEST_LENGTH], const ROLL32_CTX *ctx)
{
	/* apply the sneaky moduli 65536 (2^16) */
	if (digest != NULL)
		*(uint32_t *)digest = (ctx->s1 & 0xffff) + (ctx->s2 << 16);
}

/*
 * Returns the rollsum of block (ab) where (a) and (b) are contiguous
 */
void
ROLL32Append(ROLL32_CTX *ctx, const ROLL32_CTX *b)
{
	ctx->len += b->len;
	ctx->s2 += (ctx->s1 * b->len) + b->s2;
	ctx->s1 += b->s1;

	ROLL32Normalize(ctx);
}

/* remove B from ABC */
void
ROLL32Remove(ROLL32_CTX *ctx, const ROLL32_CTX *a, const ROLL32_CTX *b)
{
	ctx->s1 -= b->s1;

	/* unwind (a) carry effect on (b) */
	ctx->s2 -= (a->s1 * b->len);

	/* unwind (b) carry effect on (c) */
	ctx->s2 -= (b->s1 * (ctx->len - b->len - a->len));

	/* remove (b) */
	ctx->s2 -= b->s2;

	ctx->len -= a->len;

	ROLL32Normalize(ctx);
}

/* insert B between AC to form ABC */
void
ROLL32Insert(ROLL32_CTX *ctx, const ROLL32_CTX *a, const ROLL32_CTX *b,
		const ROLL32_CTX *c)
{
	ctx->len = a->len + b->len + c->len;
	ctx->s1 = a->s1 + b->s1 + c->s1;
	ctx->s2 = a->s2 + b->s2 + c->s2;

	/* (a) carry effect on (b)+(c) */
	ctx->s2 += a->s1 * (b->len + c->len);

	/* (b) carry effect on (c) */
	ctx->s2 += b->s1 * c->len;

	ROLL32Normalize(ctx);
}

void
ROLL32Digest2ctx(const uint8_t digest[ROLL32_DIGEST_LENGTH], size_t len,
		ROLL32_CTX *ctx)
{
	ctx->s1 = *(const uint32_t *)digest & 0xffff;
	ctx->s2 = *(const uint32_t *)digest >> 16;
	ctx->len = len;
}
