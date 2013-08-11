# $Gateweaver: Makefile,v 1.11 2007/09/12 15:51:35 cmaxwell Exp $

PROG=	report.cgi
.PATH:	${.CURDIR}/../common
SRCS=	report.c cgi.c customers.c items.c menu.c orders.c payment.c render.c \
		time.c
NOMAN=	report.cgi.1
HTML=	report.css report_print.css header.html footer.html front.html \
		customer.html item.html order.html payment.html time.html \
		cust_credit.html cust_detail.html cust_info.html \
		item_info.html item_inforule.html item_infosub.html \
		order_detail.html order_item.html order_list.html order_total.html

CFLAGS+=-I${.CURDIR}/../../lib/libtoolbox
CFLAGS+=-I${.CURDIR}/../../lib/libtoolbox/generated
CFLAGS+=-I${.CURDIR}/../common

# Enable pause for debugger load
#CFLAGS+=-DDEBUG_GDB

.include <bsd.obj.mk>
LDADD+= ${LIBTOOLBOX}
LDADD+=	${MODDIR}/${__objdir}/libchopstix_sql.a
LDADD+=	${DESTDIR}/usr/local/lib/libsqlite3.a
LDADD+=	${CHOPSTIXDIR}/${__objdir}/libchopstix.a

afterinstall:
.for i in ${HTML}
	${INSTALL} ${INSTALL_COPY} ${.CURDIR}/html/${i} ${BINDIR}/
.endfor
	${INSTALL} ${INSTALL_COPY} ${.CURDIR}/../common/ChopstixLogo.gif ${BINDIR}/

.include <bsd.prog.mk>
