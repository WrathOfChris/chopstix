# $Gateweaver: Makefile,v 1.4 2007/09/06 19:33:21 cmaxwell Exp $
#
# Chopstix Customer Makefile
#

OPENSSL=/usr/sbin/openssl
CADIR=${.CURDIR}/licence

LICNAME?=

KEYBITS?=4096
SSLSTARTDATE!=date +%y%m%d000000Z
SSLENDDATE=320801235900Z

# Hackery for password
SSLPASS?=
.if !empty(SSLPASS)
SSLENV="SSLPASS=\"${SSLPASS}\""
SSLKEYMODE=-aes256
SSLPASSIN=-passin env:SSLPASS
SSLPASSOUT=-passout env:SSLPASS
.else
SSLENV=
SSLKEYMODE=
SSLPASSIN=
SSLPASSOUT=
.endif

.if empty(LICNAME)
all:
	@echo "****************************************"
	@echo "* Chopstix Site Installation Generator *"
	@echo "****************************************"
	@echo UN/LOCODE: http://www.unece.org/cefact/locode/
	@echo
	@echo Must set LICNAME=

.else

all:
	@echo "*****************************************"
	@echo "* Chopstix Site Installation Generator  *"
	@echo "*****************************************"

custinit:
	mkdir -p -m 0771 ${.CURDIR}/${LICNAME}
	cvs add ${LICNAME}

lickey:
	env ${SSLENV} ${OPENSSL} genrsa ${SSLKEYMODE} ${SSLPASSOUT} \
		-out ${.CURDIR}/${LICNAME}/client.key \
		${KEYBITS}
	chmod 0640 ${.CURDIR}/${LICNAME}/client.key
	
licreq:
	env ${SSLENV} ${OPENSSL} req -new -keyform PEM -outform PEM ${SSLPASSIN} \
		-config ${CADIR}/openssl.cnf -sha1 \
		-key ${.CURDIR}/${LICNAME}/client.key \
		-out ${.CURDIR}/${LICNAME}/client.req
	chmod 0640 ${.CURDIR}/${LICNAME}/client.req

licsign:
	env ${SSLENV} ${OPENSSL} ca -batch -notext ${SSLPASSIN} -preserveDN \
		-config ${CADIR}/openssl.cnf -name client_ca -md sha1 \
		-startdate ${SSLSTARTDATE} -enddate ${SSLENDDATE} \
		-in ${.CURDIR}/${LICNAME}/client.req \
		-out ${.CURDIR}/${LICNAME}/client.crt

licmake:
	cat ${LICNAME}/client.key ${LICNAME}/client.crt \
		>${LICNAME}/chopstix.licence
	chmod 0640 ${.CURDIR}/${LICNAME}/chopstix.licence

licaddcvs:
	-cvs add ${LICNAME}/client.key ${LICNAME}/client.req ${LICNAME}/client.crt \
		${LICNAME}/chopstix.licence
	-cd ${CADIR} && \
		cvs add newcerts/`openssl x509 -noout -in \
			${.CURDIR}/${LICNAME}/client.crt -serial | sed 's/serial=//'`.pem

licshow:
	@openssl req -config ${CADIR}/openssl.cnf -noout \
		-in ${LICNAME}/client.req -subject

licence: custinit lickey licreq licsign licmake licaddcvs

.endif #LICNAME
