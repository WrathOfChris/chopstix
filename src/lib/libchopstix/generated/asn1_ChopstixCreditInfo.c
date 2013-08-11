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
encode_ChopstixCreditInfo(unsigned char *p, size_t len, const ChopstixCreditInfo *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
{
int oldret = ret;
ret = 0;
e = encode_ChopstixCreditExpiry(p, len, &(data)->expiry, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixCreditNumber(p, len, &(data)->number, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_CHOPSTIX_CARDTYPE(p, len, &(data)->type, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
BACK;
ret += oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
BACK;
*size = ret;
return 0;
}

#define FORW if(e) goto fail; p += l; len -= l; ret += l

int
decode_ChopstixCreditInfo(const unsigned char *p, size_t len, ChopstixCreditInfo *data, size_t *size)
{
size_t ret = 0, reallen;
size_t l;
int e;

memset(data, 0, sizeof(*data));
reallen = 0;
e = der_match_tag_and_length (p, len, ASN1_C_UNIV, CONS, UT_Sequence,&reallen, &l);
FORW;
{
int dce_fix;
if((dce_fix = fix_dce(reallen, &len)) < 0)
return ASN1_BAD_FORMAT;
{
size_t newlen, oldlen;

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 0, &l);
if (e)
return e;
else {
p += l;
len -= l;
ret += l;
e = der_get_length (p, len, &newlen, &l);
FORW;
{
int dce_fix;
oldlen = len;
if((dce_fix = fix_dce(newlen, &len)) < 0)return ASN1_BAD_FORMAT;
e = decode_CHOPSTIX_CARDTYPE(p, len, &(data)->type, &l);
FORW;
if(dce_fix){
e = der_match_tag_and_length (p, len, (Der_class)0, (Der_type)0, 0, &reallen, &l);
FORW;
}else 
len = oldlen - newlen;
}
}
}
{
size_t newlen, oldlen;

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 1, &l);
if (e)
return e;
else {
p += l;
len -= l;
ret += l;
e = der_get_length (p, len, &newlen, &l);
FORW;
{
int dce_fix;
oldlen = len;
if((dce_fix = fix_dce(newlen, &len)) < 0)return ASN1_BAD_FORMAT;
e = decode_ChopstixCreditNumber(p, len, &(data)->number, &l);
FORW;
if(dce_fix){
e = der_match_tag_and_length (p, len, (Der_class)0, (Der_type)0, 0, &reallen, &l);
FORW;
}else 
len = oldlen - newlen;
}
}
}
{
size_t newlen, oldlen;

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 2, &l);
if (e)
return e;
else {
p += l;
len -= l;
ret += l;
e = der_get_length (p, len, &newlen, &l);
FORW;
{
int dce_fix;
oldlen = len;
if((dce_fix = fix_dce(newlen, &len)) < 0)return ASN1_BAD_FORMAT;
e = decode_ChopstixCreditExpiry(p, len, &(data)->expiry, &l);
FORW;
if(dce_fix){
e = der_match_tag_and_length (p, len, (Der_class)0, (Der_type)0, 0, &reallen, &l);
FORW;
}else 
len = oldlen - newlen;
}
}
}
if(dce_fix){
e = der_match_tag_and_length (p, len, (Der_class)0, (Der_type)0, 0, &reallen, &l);
FORW;
}
}
if(size) *size = ret;
return 0;
fail:
free_ChopstixCreditInfo(data);
return e;
}

void
free_ChopstixCreditInfo(ChopstixCreditInfo *data)
{
free_CHOPSTIX_CARDTYPE(&(data)->type);
free_ChopstixCreditNumber(&(data)->number);
free_ChopstixCreditExpiry(&(data)->expiry);
}

size_t
length_ChopstixCreditInfo(const ChopstixCreditInfo *data)
{
size_t ret = 0;
{
int oldret = ret;
ret = 0;
ret += length_CHOPSTIX_CARDTYPE(&(data)->type);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixCreditNumber(&(data)->number);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixCreditExpiry(&(data)->expiry);
ret += 1 + length_len(ret) + oldret;
}
ret += 1 + length_len(ret);
return ret;
}

int
copy_ChopstixCreditInfo(const ChopstixCreditInfo *from, ChopstixCreditInfo *to)
{
if(copy_CHOPSTIX_CARDTYPE(&(from)->type, &(to)->type)) return ENOMEM;
if(copy_ChopstixCreditNumber(&(from)->number, &(to)->number)) return ENOMEM;
if(copy_ChopstixCreditExpiry(&(from)->expiry, &(to)->expiry)) return ENOMEM;
return 0;
}

