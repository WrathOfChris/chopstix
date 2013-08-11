# $Gateweaver: Makefile,v 1.4 2007/09/25 23:10:11 cmaxwell Exp $

PROG=	menu.cgi
.PATH:	${.CURDIR}/../common
SRCS=	menu.c cgi.c edit.c render.c
NOMAN=	menu.cgi.1
HTML=	menu.css menu_print.css header.html footer.html front.html list.html \
		info.html item.html item_extra.html item_style.html item_subitem.html \
		edit.html edit_extra.html edit_style.html edit_subitem.html

CFLAGS+=-I${.CURDIR}/../../lib/libtoolbox
CFLAGS+=-I${.CURDIR}/../../lib/libtoolbox/generated
CFLAGS+=-I${.CURDIR}/../common

# Enable pause for debugger load
#CFLAGS+=-DDEBUG_GDB
DEBUG=-g

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
