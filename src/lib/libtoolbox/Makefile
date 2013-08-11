# $Gateweaver: Makefile,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $
#
# XXX This is a stripped down version of libtoolbox from 20070904
#
CFLAGS+=-D'RCSID(a)='

LIB=	toolbox
HDRS=	toolbox.h
SRCS=	init.c

# Logging
SRCS+=	log.c

# Config
SRCS+=	parse.c

# Common Error table support
SRCS+=	error.c toolbox_err.c
HDRS+=	generated/toolbox_err.h

# Rollsums
SRCS+=	rollsum.c

# String library
SRCS+=	str.c

# Symbol
SRCS+=	symbol.c

NOMAN=1
GENERATED=	${.CURDIR}/generated
.PATH:		${GENERATED}

CFLAGS+= -I${.CURDIR}
CFLAGS+= -I${GENERATED}
CFLAGS+= -Wall
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations
CFLAGS+= -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
CFLAGS+= -Werror #-Wshadow

BINDIR?=			/usr/bin
TBERROR_COMPILE?=	${BINDIR}/tberror_compile

includes:
	@cd ${.CURDIR}; for i in $(HDRS); do \
		b=`basename $$i`; \
	    j="cmp -s $$i ${DESTDIR}/usr/include/$$b || \
		${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} -m 444 \
			$$i ${DESTDIR}/usr/include" ; \
	    echo $$j; \
	    eval "$$j"; \
	done

regen_err:
	cd ${GENERATED}; \
		${TBERROR_COMPILE} ${.CURDIR}/toolbox_err.et

.include <bsd.lib.mk>
