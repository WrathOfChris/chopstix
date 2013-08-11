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
encode_ChopstixOrder(unsigned char *p, size_t len, const ChopstixOrder *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
{
int oldret = ret;
ret = 0;
e = encode_ChopstixTotal(p, len, &(data)->total, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixOrderItems(p, len, &(data)->ruleitems, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixSpecial(p, len, &(data)->special, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixOrderItems(p, len, &(data)->items, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixPayment(p, len, &(data)->payment, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_CHOPSTIX_ORDERTYPE(p, len, &(data)->type, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixCustomer(p, len, &(data)->customer, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixTime(p, len, &(data)->date, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixOrderKey(p, len, &(data)->key, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
BACK;
ret += oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 1, &l);
BACK;
*size = ret;
return 0;
}

#define FORW if(e) goto fail; p += l; len -= l; ret += l

int
decode_ChopstixOrder(const unsigned char *p, size_t len, ChopstixOrder *data, size_t *size)
{
size_t ret = 0, reallen;
size_t l;
int e;

memset(data, 0, sizeof(*data));
reallen = 0;
e = der_match_tag_and_length (p, len, ASN1_C_APPL, CONS, 1, &reallen, &l);
FORW;
{
int dce_fix;
if((dce_fix = fix_dce(reallen, &len)) < 0)
return ASN1_BAD_FORMAT;
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
e = decode_ChopstixOrderKey(p, len, &(data)->key, &l);
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
e = decode_ChopstixTime(p, len, &(data)->date, &l);
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
e = decode_ChopstixCustomer(p, len, &(data)->customer, &l);
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

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 3, &l);
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
e = decode_CHOPSTIX_ORDERTYPE(p, len, &(data)->type, &l);
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

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 4, &l);
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
e = decode_ChopstixPayment(p, len, &(data)->payment, &l);
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

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 5, &l);
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
e = decode_ChopstixOrderItems(p, len, &(data)->items, &l);
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

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 6, &l);
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
e = decode_ChopstixSpecial(p, len, &(data)->special, &l);
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

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 7, &l);
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
e = decode_ChopstixOrderItems(p, len, &(data)->ruleitems, &l);
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

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 8, &l);
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
e = decode_ChopstixTotal(p, len, &(data)->total, &l);
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
if(dce_fix){
e = der_match_tag_and_length (p, len, (Der_class)0, (Der_type)0, 0, &reallen, &l);
FORW;
}
}
if(size) *size = ret;
return 0;
fail:
free_ChopstixOrder(data);
return e;
}

void
free_ChopstixOrder(ChopstixOrder *data)
{
free_ChopstixOrderKey(&(data)->key);
free_ChopstixTime(&(data)->date);
free_ChopstixCustomer(&(data)->customer);
free_CHOPSTIX_ORDERTYPE(&(data)->type);
free_ChopstixPayment(&(data)->payment);
free_ChopstixOrderItems(&(data)->items);
free_ChopstixSpecial(&(data)->special);
free_ChopstixOrderItems(&(data)->ruleitems);
free_ChopstixTotal(&(data)->total);
}

size_t
length_ChopstixOrder(const ChopstixOrder *data)
{
size_t ret = 0;
{
int oldret = ret;
ret = 0;
ret += length_ChopstixOrderKey(&(data)->key);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixTime(&(data)->date);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixCustomer(&(data)->customer);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_CHOPSTIX_ORDERTYPE(&(data)->type);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixPayment(&(data)->payment);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixOrderItems(&(data)->items);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixSpecial(&(data)->special);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixOrderItems(&(data)->ruleitems);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixTotal(&(data)->total);
ret += 1 + length_len(ret) + oldret;
}
ret += 1 + length_len(ret);
ret += 1 + length_len (ret);
return ret;
}

int
copy_ChopstixOrder(const ChopstixOrder *from, ChopstixOrder *to)
{
if(copy_ChopstixOrderKey(&(from)->key, &(to)->key)) return ENOMEM;
if(copy_ChopstixTime(&(from)->date, &(to)->date)) return ENOMEM;
if(copy_ChopstixCustomer(&(from)->customer, &(to)->customer)) return ENOMEM;
if(copy_CHOPSTIX_ORDERTYPE(&(from)->type, &(to)->type)) return ENOMEM;
if(copy_ChopstixPayment(&(from)->payment, &(to)->payment)) return ENOMEM;
if(copy_ChopstixOrderItems(&(from)->items, &(to)->items)) return ENOMEM;
if(copy_ChopstixSpecial(&(from)->special, &(to)->special)) return ENOMEM;
if(copy_ChopstixOrderItems(&(from)->ruleitems, &(to)->ruleitems)) return ENOMEM;
if(copy_ChopstixTotal(&(from)->total, &(to)->total)) return ENOMEM;
return 0;
}

