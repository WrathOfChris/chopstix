# $Gateweaver: Makefile,v 1.1 2007/09/12 17:39:56 cmaxwell Exp $
#
# Chopstix Customer Makefile
#

OPENSSL=/usr/sbin/openssl
CADIR=${.CURDIR}/ca
CUSTDIR=${.CURDIR}/../cust

LICNAME?=

KEYBITS?=4096
SSLSTARTDATE!=date +%y%m%d000000Z
SSLDAYS?=365

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

sslkey:
	env ${SSLENV} ${OPENSSL} genrsa ${SSLKEYMODE} ${SSLPASSOUT} \
		-out ${CUSTDIR}/${LICNAME}/server.key \
		${KEYBITS}
	chmod 0640 ${CUSTDIR}/${LICNAME}/server.key
	
sslreq:
	env ${SSLENV} ${OPENSSL} req -new -keyform PEM -outform PEM ${SSLPASSIN} \
		-config ${CADIR}/openssl.cnf -sha1 \
		-key ${CUSTDIR}/${LICNAME}/server.key \
		-out ${CUSTDIR}/${LICNAME}/server.req
	chmod 0640 ${CUSTDIR}/${LICNAME}/server.req

sslsign:
	env ${SSLENV} ${OPENSSL} ca -batch -notext ${SSLPASSIN} -preserveDN \
		-config ${CADIR}/openssl.cnf -name server_ca -md sha1 \
		-startdate ${SSLSTARTDATE} -days ${SSLDAYS} \
		-in ${CUSTDIR}/${LICNAME}/server.req \
		-out ${CUSTDIR}/${LICNAME}/server.crt

ssladdcvs:
	-cd ${CUSTDIR} && \
		cvs add ${LICNAME}/server.key ${LICNAME}/server.req \
			${LICNAME}/server.crt
	-cd ${CADIR} && \
		cvs add newcerts/`openssl x509 -noout -in \
			${CUSTDIR}/${LICNAME}/server.crt -serial |\
				sed 's/serial=//'`.pem

sslshow:
	@openssl req -config ${CADIR}/openssl.cnf -noout \
		-in ${CUSTDIR}/${LICNAME}/server.req -subject

ssl: sslkey sslreq sslsign ssladdcvs

.endif #LICNAME
