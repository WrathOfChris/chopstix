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
encode_ChopstixPayment(unsigned char *p, size_t len, const ChopstixPayment *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
if((data)->ccinfo)
{
int oldret = ret;
ret = 0;
e = encode_ChopstixCreditInfo(p, len, (data)->ccinfo, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_CHOPSTIX_PAYMENTTYPE(p, len, &(data)->type, &l);
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
decode_ChopstixPayment(const unsigned char *p, size_t len, ChopstixPayment *data, size_t *size)
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
e = decode_CHOPSTIX_PAYMENTTYPE(p, len, &(data)->type, &l);
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
(data)->ccinfo = NULL;
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
(data)->ccinfo = malloc(sizeof(*(data)->ccinfo));
if((data)->ccinfo == NULL) return ENOMEM;
e = decode_ChopstixCreditInfo(p, len, (data)->ccinfo, &l);
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
free_ChopstixPayment(data);
return e;
}

void
free_ChopstixPayment(ChopstixPayment *data)
{
free_CHOPSTIX_PAYMENTTYPE(&(data)->type);
if((data)->ccinfo) {
free_ChopstixCreditInfo((data)->ccinfo);
free((data)->ccinfo);
(data)->ccinfo = NULL;
}
}

size_t
length_ChopstixPayment(const ChopstixPayment *data)
{
size_t ret = 0;
{
int oldret = ret;
ret = 0;
ret += length_CHOPSTIX_PAYMENTTYPE(&(data)->type);
ret += 1 + length_len(ret) + oldret;
}
if((data)->ccinfo){
int oldret = ret;
ret = 0;
ret += length_ChopstixCreditInfo((data)->ccinfo);
ret += 1 + length_len(ret) + oldret;
}
ret += 1 + length_len(ret);
return ret;
}

int
copy_ChopstixPayment(const ChopstixPayment *from, ChopstixPayment *to)
{
if(copy_CHOPSTIX_PAYMENTTYPE(&(from)->type, &(to)->type)) return ENOMEM;
if((from)->ccinfo) {
(to)->ccinfo = malloc(sizeof(*(to)->ccinfo));
if((to)->ccinfo == NULL) return ENOMEM;
if(copy_ChopstixCreditInfo((from)->ccinfo, (to)->ccinfo)) return ENOMEM;
}else
(to)->ccinfo = NULL;
return 0;
}

