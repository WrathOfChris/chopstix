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
encode_ChopstixCustomer(unsigned char *p, size_t len, const ChopstixCustomer *data, size_t *size)
{
size_t ret = 0;
size_t l;
int i, e;

i = 0;
if((data)->credit)
{
int oldret = ret;
ret = 0;
e = encode_ChopstixCredits(p, len, (data)->credit, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
BACK;
ret += oldret;
}
if((data)->special)
{
int oldret = ret;
ret = 0;
e = encode_ChopstixSpecial(p, len, (data)->special, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_integer(p, len, &(data)->reps, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixIntersect(p, len, &(data)->isect, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixAddress(p, len, &(data)->addr, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixName(p, len, &(data)->name, &l);
BACK;
e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
BACK;
ret += oldret;
}
{
int oldret = ret;
ret = 0;
e = encode_ChopstixPhone(p, len, &(data)->phone, &l);
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
decode_ChopstixCustomer(const unsigned char *p, size_t len, ChopstixCustomer *data, size_t *size)
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
e = decode_ChopstixPhone(p, len, &(data)->phone, &l);
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
e = decode_ChopstixName(p, len, &(data)->name, &l);
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
e = decode_ChopstixAddress(p, len, &(data)->addr, &l);
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
e = decode_ChopstixIntersect(p, len, &(data)->isect, &l);
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
e = decode_integer(p, len, &(data)->reps, &l);
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

e = der_match_tag (p, len, ASN1_C_CONTEXT, CONS, 6, &l);
if (e)
(data)->credit = NULL;
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
(data)->credit = malloc(sizeof(*(data)->credit));
if((data)->credit == NULL) return ENOMEM;
e = decode_ChopstixCredits(p, len, (data)->credit, &l);
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
free_ChopstixCustomer(data);
return e;
}

void
free_ChopstixCustomer(ChopstixCustomer *data)
{
free_ChopstixPhone(&(data)->phone);
free_ChopstixName(&(data)->name);
free_ChopstixAddress(&(data)->addr);
free_ChopstixIntersect(&(data)->isect);
if((data)->special) {
free_ChopstixSpecial((data)->special);
free((data)->special);
(data)->special = NULL;
}
if((data)->credit) {
free_ChopstixCredits((data)->credit);
free((data)->credit);
(data)->credit = NULL;
}
}

size_t
length_ChopstixCustomer(const ChopstixCustomer *data)
{
size_t ret = 0;
{
int oldret = ret;
ret = 0;
ret += length_ChopstixPhone(&(data)->phone);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixName(&(data)->name);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixAddress(&(data)->addr);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_ChopstixIntersect(&(data)->isect);
ret += 1 + length_len(ret) + oldret;
}
{
int oldret = ret;
ret = 0;
ret += length_integer(&(data)->reps);
ret += 1 + length_len(ret) + oldret;
}
if((data)->special){
int oldret = ret;
ret = 0;
ret += length_ChopstixSpecial((data)->special);
ret += 1 + length_len(ret) + oldret;
}
if((data)->credit){
int oldret = ret;
ret = 0;
ret += length_ChopstixCredits((data)->credit);
ret += 1 + length_len(ret) + oldret;
}
ret += 1 + length_len(ret);
return ret;
}

int
copy_ChopstixCustomer(const ChopstixCustomer *from, ChopstixCustomer *to)
{
if(copy_ChopstixPhone(&(from)->phone, &(to)->phone)) return ENOMEM;
if(copy_ChopstixName(&(from)->name, &(to)->name)) return ENOMEM;
if(copy_ChopstixAddress(&(from)->addr, &(to)->addr)) return ENOMEM;
if(copy_ChopstixIntersect(&(from)->isect, &(to)->isect)) return ENOMEM;
*(&(to)->reps) = *(&(from)->reps);
if((from)->special) {
(to)->special = malloc(sizeof(*(to)->special));
if((to)->special == NULL) return ENOMEM;
if(copy_ChopstixSpecial((from)->special, (to)->special)) return ENOMEM;
}else
(to)->special = NULL;
if((from)->credit) {
(to)->credit = malloc(sizeof(*(to)->credit));
if((to)->credit == NULL) return ENOMEM;
if(copy_ChopstixCredits((from)->credit, (to)->credit)) return ENOMEM;
}else
(to)->credit = NULL;
return 0;
}

