# $Gateweaver: Makefile,v 1.24 2007/08/09 16:54:12 cmaxwell Exp $

PROG=	chopstix
SRCS=	${PROG}.c display.c input.c order.c menu.c status.c customer.c parse.y \
		form.c special.c payment.c module.c str.c print.c window.c rule.c \
		licence.c
NOMAN=1

GENERATED=${.CURDIR}/generated
CFLAGS+=-I${.CURDIR} -I${GENERATED}
CFLAGS+=-I${.CURDIR}/../../lib/libchopstix/generated
CFLAGS+=-Wall -Werror
CFLAGS+=-Wpointer-arith -Wcast-align -Wcast-qual
CFLAGS+=-Waggregate-return
CFLAGS+=-Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
CFLAGS+=-Wbounded -Wformat
#-Wshadow -Wsign-compare

LDADD+=	-lcurses -levent -lcrypto
.if !defined(PROFILE)
LDADD+=	${.CURDIR}/../../lib/libchopstix/${__objdir}/libchopstix.a
.else
LDADD+=	${.CURDIR}/../../lib/libchopstix_sql/${__objdir}/libchopstix_sql_p.a
LDADD+=	${.CURDIR}/../../lib/libchopstix/${__objdir}/libchopstix_p.a
LDADD+= ${DESTDIR}/usr/local/lib/libsqlite3.a
CFLAGS+=-DPROFILE
.endif
CLEANFILES+= y.tab.h

regen_licence:
	cd ${GENERATED}; \
		DEBUG="${DEBUG}" \
			${.CURDIR}/licence_gen.sh \
			${.CURDIR}/../../share \
			>${GENERATED}/licence.h

regen: regen_licence

test:
	${.OBJDIR}/${PROG}

debug:
	gdb ${.OBJDIR}/${PROG}

crash:
	gdb ${.OBJDIR}/${PROG} ${.OBJDIR}/${PROG}.core

profile:
	gprof ${.OBJDIR}/${PROG} ${.OBJDIR}/gmon.out

start:
	@echo -n "START " >> ${.CURDIR}/TIMELOG
	@date >> ${.CURDIR}/TIMELOG
	@tail -n 1 ${.CURDIR}/TIMELOG

stop:
	@echo -n "STOP " >> ${.CURDIR}/TIMELOG
	@date >> ${.CURDIR}/TIMELOG
	@tail -n 2 ${.CURDIR}/TIMELOG

timecheck:
	@tail -n 2 ${.CURDIR}/TIMELOG

.include <bsd.prog.mk>
