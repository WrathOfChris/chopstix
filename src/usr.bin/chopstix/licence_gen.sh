#!/bin/sh
#
# Generate licence.h

FILE=$1

[[ -z $CURDIR ]] && CURDIR=`pwd`
[[ -z $FILE ]] && {
	echo >&2 "usage: $0 <cachain>"
	exit 1
}

#
# PREAMBLE
#
cat <<__EOT
/* \$Gateweaver\$ */
/* Generated from $FILE */
/* Do not edit */
#ifndef LICENSE_H
#define LICENSE_H
__EOT

# Stack type
cat <<__EOT

typedef struct licence_ent {
	unsigned char *name;
	size_t namelen;
	unsigned char *pubkey;
	size_t pubkeylen;
	unsigned char *cert;
	size_t certlen;
} licence_ent;
__EOT

# Licence CA Chain DER
i=1
for cert in $FILE/*.pem ; do
	echo ""
	openssl x509 -noout -C -in $cert |\
		sed -e "s/XXX/CAchain_$i/"
	i=$((i + 1))
done

echo ""
i=1
echo "struct licence_ent licence_cacert_der[] = {"
for cert in $FILE/*.pem ; do
	cat <<__EOT
	{ CAchain_${i}_subject_name, sizeof(CAchain_${i}_subject_name),
	  CAchain_${i}_public_key, sizeof(CAchain_${i}_public_key),
	  CAchain_${i}_certificate, sizeof(CAchain_${i}_certificate) },
__EOT
	i=$((i + 1))
done
echo "};"

#
# POSTAMBLE
#
cat <<__EOT

#endif
__EOT
