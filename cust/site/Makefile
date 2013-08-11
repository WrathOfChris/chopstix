# $Gateweaver: Makefile,v 1.22 2007/10/15 17:26:37 cmaxwell Exp $

VERSION?=1.1
LICNAME?=
PASSWORD?=chopstix
HOSTDOMAIN?=chopstix.manorsoft.ca
PARSE_INSTALL=PASSWORD="${PASSWORD}"\
	LICENCE="${LICNAME}"\
	HOSTNAME="${LICNAME}.${HOSTDOMAIN}" \
	HOSTDOMAIN="${HOSTDOMAIN}" \
	OSrev="${OSrev}" OSREV="${OSREV}" \
	${.CURDIR}/../tools/install_parse.sh

USER_SERVICE=100
USER_CHOPSTIX=200
USER_MANAGER=201

.if empty(LICNAME)
all:
	@echo Must set LICNAME=

.else

all: common chopstix licence rules matchbox service ssl
	tar zcvf site${OSrev}-${LICNAME}.tgz \
		install.site upgrade.site etc/ root/ usr/ var/

common:
	${PARSE_INSTALL} ${INSTALL_COPY} -m 0755 -o root -g wheel \
		${.CURDIR}/install.site ${.OBJDIR}
	${INSTALL} ${INSTALL_COPY} -m 0755 -o root -g wheel \
		${.CURDIR}/upgrade.site ${.OBJDIR}
	${INSTALL} -d -o root -g ${USER_MANAGER} -m 0750 \
		${.OBJDIR}/var/db/chopstix/receipts
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/adduser.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o ${USER_MANAGER} -g ${USER_CHOPSTIX} -m 0644 \
		${.CURDIR}/chopstix.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/inetd.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/login.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/mk.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0664 \
		${.CURDIR}/motd ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/newsyslog.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/ntpd.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/printcap ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0775 \
		${.CURDIR}/printcap_copy ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/rc.conf.local ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/rc.shutdown ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0440 \
		${.CURDIR}/sudoers ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/syslog.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/ttys ${.OBJDIR}/etc
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/kerberosV
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/krb5.conf ${.OBJDIR}/etc/kerberosV
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/ppp
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/ppp.conf ${.OBJDIR}/etc/ppp
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/skel
	${PARSE_INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/skel/dot.profile ${.OBJDIR}/etc/skel/.profile
	${PARSE_INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/skel/dot.xsession ${.OBJDIR}/etc/skel/.xsession
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/skel/.kde/share/config
	${PARSE_INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/skel/konq-embedrc ${.OBJDIR}/etc/skel/.kde/share/config
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/skel/konq-cryptodefaults \
		${.OBJDIR}/etc/skel/.kde/share/config/cryptodefaults
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/ssl
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/ssl/ca.crt ${.OBJDIR}/etc/ssl
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/ssl/cert.pem ${.OBJDIR}/etc/ssl
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/X11
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/X11/xdm
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/X11/Xresources ${.OBJDIR}/etc/X11/xdm
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/X11/xdm/pixmaps
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/X11/Chopstix.xpm ${.OBJDIR}/etc/X11/xdm/pixmaps
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/root
	${PARSE_INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/skel/root.profile ${.OBJDIR}/root/.profile
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/usr
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/usr/bin
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/var/log
	${INSTALL} ${INSTALL_COPY} -m 0640 -o root -g ${USER_CHOPSTIX} \
		/dev/null ${.OBJDIR}/var/log/chopstix
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/var
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/var/db
	${INSTALL} -d -o ${USER_MANAGER} -g ${USER_CHOPSTIX} -m 0770 \
		${.OBJDIR}/var/db/chopstix
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/var/www
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/var/www/conf
	${PARSE_INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/httpd.conf ${.OBJDIR}/var/www/conf

licence:
	-${INSTALL} ${INSTALL_COPY} -o ${USER_MANAGER} -g ${USER_CHOPSTIX} \
		-m 0644 ${.CURDIR}/../${LICNAME}/chopstix.conf ${.OBJDIR}/etc
	${INSTALL} ${INSTALL_COPY} -o ${USER_MANAGER} -g ${USER_CHOPSTIX} \
		-m 0644 ${.CURDIR}/../${LICNAME}/chopstix.licence ${.OBJDIR}/etc

rules:
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/usr/local/lib
	-${INSTALL} ${INSTALL_COPY} -o ${USER_MANAGER} -g ${USER_CHOPSTIX} \
		-m 0644 \
		${.CURDIR}/../${LICNAME}/librules/obj/librules.so.${VERSION} \
		${.OBJDIR}/usr/local/lib/librules_${LICNAME}.so.${VERSION}

chopstix:
.if exists(${.CURDIR}/../${LICNAME}/chopstix.sql)
	echo ".quit" | sqlite3 \
		-init ${.CURDIR}/../${LICNAME}/chopstix.sql \
		${.OBJDIR}/var/db/chopstix/chopstix.db
.else
	echo ".quit" | sqlite3 \
		-init ${.CURDIR}/../../src/lib/libchopstix_sql/chopstix_db.sql \
		${.OBJDIR}/var/db/chopstix/chopstix.db
.endif #exists
	chmod 0660 ${.OBJDIR}/var/db/chopstix/chopstix.db
	chown ${USER_MANAGER}:${USER_CHOPSTIX} ${.OBJDIR}/var/db/chopstix/chopstix.db

service:
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/skel/dot.k5login ${.OBJDIR}/etc/skel/.k5login
	${INSTALL} -d -o root -g wheel -m 0700 ${.OBJDIR}/etc/skel/.ssh
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/../service/ssh_service_rsa.pub \
		${.OBJDIR}/etc/skel/.ssh/authorized_keys
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/ssh
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/../service/ssh_service_rsa.pub ${.OBJDIR}/etc/ssh
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/skel/dot.k5login ${.OBJDIR}/root/.k5login
	${INSTALL} -d -o root -g wheel -m 0700 ${.OBJDIR}/root/.ssh
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/../service/ssh_service_rsa.pub \
		${.OBJDIR}/root/.ssh/authorized_keys

matchbox:
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/skel/.matchbox
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/matchbox/kbdconfig ${.OBJDIR}/etc/skel/.matchbox
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/matchbox/mbdesktop_modules ${.OBJDIR}/etc/skel/.matchbox
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/skel/.matchbox/vfolders
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/matchbox/Root.order ${.OBJDIR}/etc/skel/.matchbox/vfolders
	for file in ${.CURDIR}/matchbox/*.directory ; do \
		${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		$$file ${.OBJDIR}/etc/skel/.matchbox/vfolders ;\
	done
	${INSTALL} -d -o root -g wheel ${.OBJDIR}/etc/skel/.applications
	for file in ${.CURDIR}/matchbox/*.desktop ; do \
		${PARSE_INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		$$file ${.OBJDIR}/etc/skel/.applications ;\
	done

ssl:
	${INSTALL} -d -o root -g wheel -m 700 ${.OBJDIR}/etc/ssl/private
.if exists(${.CURDIR}/../${LICNAME}/server.key)
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0600 \
		${.CURDIR}/../${LICNAME}/server.key \
		${.OBJDIR}/etc/ssl/private/server.key
.endif
.if exists(${.CURDIR}/../${LICNAME}/server.crt)
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 0644 \
		${.CURDIR}/../${LICNAME}/server.crt ${.OBJDIR}/etc/ssl/server.crt
.endif

.endif #LICNAME

.include <bsd.obj.mk>
.include <bsd.subdir.mk>
