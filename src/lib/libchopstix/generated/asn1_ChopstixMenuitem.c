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
encode_ChopstixMenuitem(unsigned char *p, size_t len, const ChopstixMenuitem *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
{
int oldret = ret;
ret = 0;
e = encode_ChopstixFlags(p, len, &(data)->flags, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
BACK;
ret += oldret;
}
if((data)->subitems)
{
int oldret = ret;
ret = 0;
e = encode_ChopstixItemExtras(p, len, (data)->subitems, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixItemExtras(p, len, &(data)->extras, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixItemStyles(p, len, &(data)->styles, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_integer(p, len, &(data)->price, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_general_string(p, len, &(data)->name, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixItemCode(p, len, &(data)->code, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_integer(p, len, &(data)->gen, &l);
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
decode_ChopstixMenuitem(const unsigned char *p, size_t len, ChopstixMenuitem *data, size_t *size)
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
e = decode_integer(p, len, &(data)->gen, &l);
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
e = decode_general_string(p, len, &(data)->name, &l);
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
e = decode_integer(p, len, &(data)->price, &l);
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
e = decode_ChopstixItemStyles(p, len, &(data)->styles, &l);
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
e = decode_ChopstixItemExtras(p, len, &(data)->extras, &l);
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
e = decode_ChopstixItemExtras(p, len, (data)->subitems, &l);
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
e = decode_ChopstixFlags(p, len, &(data)->flags, &l);
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
free_ChopstixMenuitem(data);
return e;
}

void
free_ChopstixMenuitem(ChopstixMenuitem *data)
{
free_ChopstixItemCode(&(data)->code);
free_general_string(&(data)->name);
free_ChopstixItemStyles(&(data)->styles);
free_ChopstixItemExtras(&(data)->extras);
if((data)->subitems) {
free_ChopstixItemExtras((data)->subitems);
free((data)->subitems);
(data)->subitems = NULL;
}
free_ChopstixFlags(&(data)->flags);
}

size_t
length_ChopstixMenuitem(const ChopstixMenuitem *data)
{
size_t ret = 0;
{
int oldret = ret;
ret = 0;
ret += length_integer(&(data)->gen);
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
ret += length_general_string(&(data)->name);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_integer(&(data)->price);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixItemStyles(&(data)->styles);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixItemExtras(&(data)->extras);
ret += 1 + length_len(ret) + oldret;
}
if((data)->subitems){
int oldret = ret;
ret = 0;
ret += length_ChopstixItemExtras((data)->subitems);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixFlags(&(data)->flags);
ret += 1 + length_len(ret) + oldret;
}
ret += 1 + length_len(ret);
return ret;
}

int
copy_ChopstixMenuitem(const ChopstixMenuitem *from, ChopstixMenuitem *to)
{
*(&(to)->gen) = *(&(from)->gen);
if(copy_ChopstixItemCode(&(from)->code, &(to)->code)) return ENOMEM;
if(copy_general_string(&(from)->name, &(to)->name)) return ENOMEM;
*(&(to)->price) = *(&(from)->price);
if(copy_ChopstixItemStyles(&(from)->styles, &(to)->styles)) return ENOMEM;
if(copy_ChopstixItemExtras(&(from)->extras, &(to)->extras)) return ENOMEM;
if((from)->subitems) {
(to)->subitems = malloc(sizeof(*(to)->subitems));
if((to)->subitems == NULL) return ENOMEM;
if(copy_ChopstixItemExtras((from)->subitems, (to)->subitems)) return ENOMEM;
}else
(to)->subitems = NULL;
if(copy_ChopstixFlags(&(from)->flags, &(to)->flags)) return ENOMEM;
return 0;
}

