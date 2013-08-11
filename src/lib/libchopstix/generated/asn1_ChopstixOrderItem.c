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
encode_ChopstixOrderItem(unsigned char *p, size_t len, const ChopstixOrderItem *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
if((data)->subitems)
{
int oldret = ret;
ret = 0;
e = encode_ChopstixSubItems(p, len, (data)->subitems, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
BACK;
ret += oldret;
}
if((data)->special)
{
int oldret = ret;
ret = 0;
e = encode_ChopstixSpecial(p, len, (data)->special, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_integer(p, len, &(data)->style, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixItemCode(p, len, &(data)->code, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_integer(p, len, &(data)->qty, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_CHOPSTIX_ITEMTYPE(p, len, &(data)->type, &l);
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
decode_ChopstixOrderItem(const unsigned char *p, size_t len, ChopstixOrderItem *data, size_t *size)
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
e = decode_CHOPSTIX_ITEMTYPE(p, len, &(data)->type, &l);
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
e = decode_integer(p, len, &(data)->qty, &l);
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
e = decode_ChopstixItemCode(p, len, &(data)->code, &l);
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
e = decode_integer(p, len, &(data)->style, &l);
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
(data)->special = NULL;
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
(data)->special = malloc(sizeof(*(data)->special));
if((data)->special == NULL) return ENOMEM;
e = decode_ChopstixSpecial(p, len, (data)->special, &l);
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
(data)->subitems = NULL;
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
(data)->subitems = malloc(sizeof(*(data)->subitems));
if((data)->subitems == NULL) return ENOMEM;
e = decode_ChopstixSubItems(p, len, (data)->subitems, &l);
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
free_ChopstixOrderItem(data);
return e;
}

void
free_ChopstixOrderItem(ChopstixOrderItem *data)
{
free_CHOPSTIX_ITEMTYPE(&(data)->type);
free_ChopstixItemCode(&(data)->code);
if((data)->special) {
free_ChopstixSpecial((data)->special);
free((data)->special);
(data)->special = NULL;
}
if((data)->subitems) {
free_ChopstixSubItems((data)->subitems);
free((data)->subitems);
(data)->subitems = NULL;
}
}

size_t
length_ChopstixOrderItem(const ChopstixOrderItem *data)
{
size_t ret = 0;
{
int oldret = ret;
ret = 0;
ret += length_CHOPSTIX_ITEMTYPE(&(data)->type);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_integer(&(data)->qty);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixItemCode(&(data)->code);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_integer(&(data)->style);
ret += 1 + length_len(ret) + oldret;
}
if((data)->special){
int oldret = ret;
ret = 0;
ret += length_ChopstixSpecial((data)->special);
ret += 1 + length_len(ret) + oldret;
}
if((data)->subitems){
int oldret = ret;
ret = 0;
ret += length_ChopstixSubItems((data)->subitems);
ret += 1 + length_len(ret) + oldret;
}
ret += 1 + length_len(ret);
return ret;
}

int
copy_ChopstixOrderItem(const ChopstixOrderItem *from, ChopstixOrderItem *to)
{
if(copy_CHOPSTIX_ITEMTYPE(&(from)->type, &(to)->type)) return ENOMEM;
*(&(to)->qty) = *(&(from)->qty);
if(copy_ChopstixItemCode(&(from)->code, &(to)->code)) return ENOMEM;
*(&(to)->style) = *(&(from)->style);
if((from)->special) {
(to)->special = malloc(sizeof(*(to)->special));
if((to)->special == NULL) return ENOMEM;
if(copy_ChopstixSpecial((from)->special, (to)->special)) return ENOMEM;
}else
(to)->special = NULL;
if((from)->subitems) {
(to)->subitems = malloc(sizeof(*(to)->subitems));
if((to)->subitems == NULL) return ENOMEM;
if(copy_ChopstixSubItems((from)->subitems, (to)->subitems)) return ENOMEM;
}else
(to)->subitems = NULL;
return 0;
}

