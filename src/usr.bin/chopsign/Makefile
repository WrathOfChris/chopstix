# $Gateweaver: Makefile,v 1.2 2007/08/10 19:06:14 cmaxwell Exp $

PROG=	chopsign
SRCS=	${PROG}.c
NOMAN=1

GENERATED=${.CURDIR}/generated
CFLAGS+=-Wall -Werror
CFLAGS+=-Wpointer-arith -Wcast-align -Wcast-qual
CFLAGS+=-Waggregate-return
CFLAGS+=-Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
CFLAGS+=-Wbounded -Wformat
#-Wshadow -Wsign-compare

LDADD+=	-lcrypto

.include <bsd.prog.mk>
