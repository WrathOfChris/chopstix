# $Gateweaver: Makefile,v 1.3 2007/08/08 17:59:38 cmaxwell Exp $
SRCDIR=${.CURDIR}/../../usr.bin/chopstix
GENDIR=${.CURDIR}/../../lib/libchopstix/generated

LIB=	rules
SRCS=	rules.c
NOMAN=	${LIB}.3

# Enable tracing of SQL to syslog/status display
#
#CFLAGS+=-DSQL_TRACE

CFLAGS+=-I${SRCDIR} -I${GENDIR}
CFLAGS+= -Wall
CFLAGS+= #-Wstrict-prototypes
CFLAGS+= -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations
CFLAGS+= -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
CFLAGS+= -Werror -Wshadow

.include <bsd.obj.mk>
LDADD+=${.CURDIR}/../libchopstix/${__objdir}/libchopstix.a

.include <bsd.lib.mk>
