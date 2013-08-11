# $Gateweaver: Makefile.inc,v 1.2 2007/09/04 14:54:35 cmaxwell Exp $

BINDIR=		/var/www/cgi-bin
BINMODE=	555
LDSTATIC=	${STATIC}

CFLAGS+=	-Wall -Werror
CFLAGS+=	-Wshadow -Wpointer-arith -Wcast-align -Wcast-qual
CFLAGS+=	-Waggregate-return
CFLAGS+=	-Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
CFLAGS+=	-Wsign-compare -Wbounded -Wformat

CHOPSTIXDIR=${.CURDIR}/../../lib/libchopstix
SQLDIR=		${.CURDIR}/../../lib/libsqlite3
MODDIR=		${.CURDIR}/../../lib/libchopstix_sql
APIDIR=		${.CURDIR}/../../usr.bin/chopstix
LIBTOOLBOX=	${.CURDIR}/../../lib/libtoolbox/${__objdir}/libtoolbox.a

CFLAGS+=	-I${CHOPSTIXDIR}/generated
CFLAGS+=	-I${APIDIR}
