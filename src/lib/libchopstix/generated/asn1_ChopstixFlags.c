/* Generated from /home/cmaxwell/chopstix/src/lib/libchopstix/chopstix.asn1 */
/* Do not edit */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <chopstix_asn1.h>
#include <asn1_err.h>
#include <der.h>
#include <parse_units.h>

#define BACK if (e) return e; p -= l; len -= l; ret += l

int
encode_ChopstixFlags(unsigned char *p, size_t len, const ChopstixFlags *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
{
unsigned char c = 0;
*p-- = c; len--; ret++;
c = 0;
*p-- = c; len--; ret++;
c = 0;
*p-- = c; len--; ret++;
c = 0;
if(data->deleted) c |= 1<<7;
*p-- = c;
*p-- = 0;
len -= 2;
ret += 2;
}

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM,UT_BitString, &l);
BACK;
*size = ret;
return 0;
}

#define FORW if(e) goto fail; p += l; len -= l; ret += l

int
decode_ChopstixFlags(const unsigned char *p, size_t len, ChopstixFlags *data, size_t *size)
{
size_t ret = 0, reallen;
size_t l;
int e;

memset(data, 0, sizeof(*data));
reallen = 0;
e = der_match_tag_and_length (p, len, ASN1_C_UNIV, PRIM, UT_BitString,&reallen, &l);
FORW;
if(len < reallen)
return ASN1_OVERRUN;
p++;
len--;
reallen--;
ret++;
data->deleted = (*p >> 7) & 1;
p += reallen; len -= reallen; ret += reallen;
if(size) *size = ret;
return 0;
fail:
free_ChopstixFlags(data);
return e;
}

void
free_ChopstixFlags(ChopstixFlags *data)
{
}

size_t
length_ChopstixFlags(const ChopstixFlags *data)
{
size_t ret = 0;
ret += 7;
return ret;
}

int
copy_ChopstixFlags(const ChopstixFlags *from, ChopstixFlags *to)
{
*(to) = *(from);
return 0;
}

unsigned ChopstixFlags2int(ChopstixFlags f)
{
unsigned r = 0;
if(f.deleted) r |= (1U << 0);
return r;
}

ChopstixFlags int2ChopstixFlags(unsigned n)
{
	ChopstixFlags flags;

	flags.deleted = (n >> 0) & 1;
	return flags;
}

static struct units ChopstixFlags_units[] = {
	{"deleted",	1U << 0},
	{NULL,	0}
};

const struct units * asn1_ChopstixFlags_units(void){
return ChopstixFlags_units;
}

