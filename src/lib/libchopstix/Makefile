# $Gateweaver: Makefile,v 1.5 2007/08/08 17:59:28 cmaxwell Exp $

LIB=	chopstix
SRCS=	${ASN1FILES}
NOMAN=1

# asn1 generation
GENERATED=	${.CURDIR}/generated
.PATH:		${GENERATED}
ASN1FILES!=	cat ${GENERATED}/asn1_files | sed 's,\.x,\.c,g'
CFLAGS+=	-I${GENERATED}
CFLAGS+=	-I${DESTDIR}/usr/include/kerberosV
CFLAGS+=	-I${BSDSRCDIR}/kerberosV/src/lib/roken

.include <bsd.own.mk>

# asn1 requires parts of the krb5 lib
.PATH:		${BSDSRCDIR}/kerberosV/src/lib/asn1
CFLAGS+=	-I${BSDSRCDIR}/kerberosV/include
#CFLAGS+=	-DHAVE_CONFIG_H
CFLAGS+=	-D'RCSID(a)='
SRCS+=		der_get.c der_put.c der_free.c der_length.c der_copy.c der_cmp.c

regen_asn1:
	cd ${GENERATED}; \
		asn1_compile ${.CURDIR}/chopstix.asn1 chopstix_asn1; \
		for f in `cat asn1_files`; do \
			mv $$f `echo $$f | sed s,.x$$,.c,g` ;\
		done

regen: cleandir regen_asn1

.include <bsd.lib.mk>
