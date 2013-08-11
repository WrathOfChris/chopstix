# $Gateweaver: Makefile,v 1.10 2007/08/08 17:59:37 cmaxwell Exp $
SRCDIR=${.CURDIR}/../../usr.bin/chopstix
GENDIR=${.CURDIR}/../../lib/libchopstix/generated

LIB=	chopstix_sql
SRCS=	${LIB}.c customer.c menu.c order.c
MAN=	${LIB}.3

# Enable tracing of SQL to syslog/status display
#
#CFLAGS+=-DSQL_TRACE

# This is not finished yet, but could be if required
#
#CFLAGS+=-DPOST_EXTRAS

CFLAGS+=-I${SRCDIR} -I${GENDIR} -I${LOCALBASE}/include
CFLAGS+= -Wall
CFLAGS+= #-Wstrict-prototypes
CFLAGS+= -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations
CFLAGS+= -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
CFLAGS+= -Werror -Wshadow

.include <bsd.obj.mk>
LDADD+=${DESTDIR}/usr/local/lib/libsqlite3.a
LDADD+=${.CURDIR}/../libchopstix/${__objdir}/libchopstix.a

.include <bsd.lib.mk>
