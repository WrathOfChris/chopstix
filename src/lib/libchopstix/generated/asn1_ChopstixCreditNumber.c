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
encode_ChopstixCreditNumber(unsigned char *p, size_t len, const ChopstixCreditNumber *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
e = encode_general_string(p, len, data, &l);
BACK;
*size = ret;
return 0;
}

#define FORW if(e) goto fail; p += l; len -= l; ret += l

int
decode_ChopstixCreditNumber(const unsigned char *p, size_t len, ChopstixCreditNumber *data, size_t *size)
{
size_t ret = 0, reallen;
size_t l;
int e;

memset(data, 0, sizeof(*data));
reallen = 0;
e = decode_general_string(p, len, data, &l);
FORW;
if(size) *size = ret;
return 0;
fail:
free_ChopstixCreditNumber(data);
return e;
}

void
free_ChopstixCreditNumber(ChopstixCreditNumber *data)
{
free_general_string(data);
}

size_t
length_ChopstixCreditNumber(const ChopstixCreditNumber *data)
{
size_t ret = 0;
ret += length_general_string(data);
return ret;
}

int
copy_ChopstixCreditNumber(const ChopstixCreditNumber *from, ChopstixCreditNumber *to)
{
if(copy_general_string(from, to)) return ENOMEM;
return 0;
}

