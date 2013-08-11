# $Gateweaver: Makefile,v 1.10 2007/10/15 17:26:37 cmaxwell Exp $

COMMENT-main=		"Restaurant Take-Out and Delivery order entry"
COMMENT-desktop=	"Restaurant Take-Out and Delivery desktop accessories"
COMMENT-menu=		"Restaurant Take-Out and Delivery menu manager"
COMMENT-report=		"Restaurant Take-Out and Delivery report manager"
CVSUSER?=		cvs
CVSHOST?=		cvs.manorsoft.ca
CVSDEST?=		${CVSUSER}@${CVSHOST}
CVSPATH?=		/cvs/Manorsoft
CVSTAG?=		now
.if !empty(CVSDEST)
CVSREPO?=		${CVSDEST}:${CVSPATH}
.else
CVSREPO?=		${CVSPATH}
.endif

V=			1.1
DISTNAME=		chopstix-${V}
PKGNAME-main=		chopstix-${V}
PKGNAME-desktop=	chopstix-desktop-${V}
PKGNAME-menu=		chopstix-menu-${V}
PKGNAME-report=		chopstix-report-${V}
CATEGORIES=		misc

HOMEPAGE=		http://www.manorsoft.ca/products/chopstix/
MAINTAINER=		Christopher Maxwell <chris@manorsoft.ca>

# Commercial
PERMIT_PACKAGE_CDROM=	No
PERMIT_PACKAGE_FTP=	No
PERMIT_DISTFILES_CDROM=	No
PERMIT_DISTFILES_FTP=	No
WANTLIB=		c crypto curses event

MASTER_SITES=		http://download.manorsoft.ca/
MULTI_PACKAGES=		-main -desktop -menu -report

PREFIX-menu=		/var/www/chopstix/menu
WANTLIB-menu=

PREFIX-report=		/var/www/chopstix/report
WANTLIB-report=

BUILD_DEPENDS=		sqlite3-*:databases/sqlite3

MAKE_ENV=		BINDIR=${LOCALBASE}/bin \
			LIBDIR=${LOCALBASE}/lib
PROGDIRS=		lib/libchopstix lib/libchopstix_sql lib/librules \
			lib/libtoolbox \
			usr.bin/chopstix var.www/menu.cgi var.www/report.cgi

SEPARATE_BUILD=		simple
SHARED_LIBS=		chopstix ${V} \
			chopstix_sql ${V} \
			rules ${V}

NO_CHECKSUM=		Yes
NO_REGRESS=		Yes

do-fetch:
	@echo -n

pre-extract:
	mkdir -p ${WRKDIST}

do-extract:
	cd ${WRKDIR} && \
		cvs -d ${CVSREPO} export -d ${DISTNAME} -D ${CVSTAG} chopstix/src

post-configure:
.for dir in ${PROGDIRS}
	cd ${WRKSRC}/${dir} && \
		${SETENV} ${MAKE_ENV} ${MAKE_PROGRAM} ${MAKE_FLAGS} \
			-f ${WRKSRC}/${dir}/${MAKE_FILE} obj
.endfor

do-build:
.for dir in ${PROGDIRS}
	cd ${WRKSRC}/${dir} && \
		${SETENV} ${MAKE_ENV} ${MAKE_PROGRAM} ${MAKE_FLAGS} \
			-f ${WRKSRC}/${dir}/${MAKE_FILE} depend ${ALL_TARGET}
.endfor

do-install:
	${INSTALL_DATA} ${WRKSRC}/lib/libchopstix/obj/libchopstix.so.${V} \
		${PREFIX}/lib
	${INSTALL_DATA} ${WRKSRC}/lib/libchopstix_sql/obj/libchopstix_sql.so.${V} \
		${PREFIX}/lib
	${INSTALL_DATA} ${WRKSRC}/lib/libchopstix_sql/obj/chopstix_sql.cat3 \
		${PREFIX}/man/cat3/chopstix_sql.0
	${INSTALL_DATA} ${WRKSRC}/lib/librules/obj/librules.so.${V} ${PREFIX}/lib
	${INSTALL_PROGRAM} ${WRKSRC}/usr.bin/chopstix/obj/chopstix ${PREFIX}/bin
	${INSTALL_PROGRAM} ${.CURDIR}/files/mb-chopstix-exec ${PREFIX}/bin
	${INSTALL_PROGRAM} ${.CURDIR}/files/chopstix-backup ${PREFIX}/bin
	${INSTALL_DATA_DIR} ${WRKINST}/${PREFIX-menu}
	${INSTALL_PROGRAM} ${WRKSRC}/var.www/menu.cgi/obj/menu.cgi \
		${WRKINST}/${PREFIX-menu}
	${INSTALL_DATA} ${WRKSRC}/var.www/menu.cgi/menu.conf \
		${WRKINST}/${PREFIX-menu}
	${INSTALL_DATA} ${WRKSRC}/var.www/common/ChopstixLogo.gif \
		${WRKINST}/${PREFIX-menu}
	${INSTALL_DATA} ${WRKSRC}/var.www/menu.cgi/html/* \
		${WRKINST}/${PREFIX-menu}
	${INSTALL_DATA_DIR} ${WRKINST}/${PREFIX-report}
	${INSTALL_PROGRAM} ${WRKSRC}/var.www/report.cgi/obj/report.cgi \
		${WRKINST}/${PREFIX-report}
	${INSTALL_DATA} ${WRKSRC}/var.www/report.cgi/report.conf \
		${WRKINST}/${PREFIX-report}
	${INSTALL_DATA} ${WRKSRC}/var.www/common/ChopstixLogo.gif \
		${WRKINST}/${PREFIX-report}
	${INSTALL_DATA} ${WRKSRC}/var.www/report.cgi/html/* \
		${WRKINST}/${PREFIX-report}

.include <bsd.port.mk>
