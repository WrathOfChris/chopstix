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
encode_ChopstixOrderKey(unsigned char *p, size_t len, const ChopstixOrderKey *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
e = encode_integer(p, len, data, &l);
BACK;
*size = ret;
return 0;
}

#define FORW if(e) goto fail; p += l; len -= l; ret += l

int
decode_ChopstixOrderKey(const unsigned char *p, size_t len, ChopstixOrderKey *data, size_t *size)
{
size_t ret = 0, reallen;
size_t l;
int e;

memset(data, 0, sizeof(*data));
reallen = 0;
e = decode_integer(p, len, data, &l);
FORW;
if(size) *size = ret;
return 0;
fail:
free_ChopstixOrderKey(data);
return e;
}

void
free_ChopstixOrderKey(ChopstixOrderKey *data)
{
}

size_t
length_ChopstixOrderKey(const ChopstixOrderKey *data)
{
size_t ret = 0;
ret += length_integer(data);
return ret;
}

int
copy_ChopstixOrderKey(const ChopstixOrderKey *from, ChopstixOrderKey *to)
{
*(to) = *(from);
return 0;
}

