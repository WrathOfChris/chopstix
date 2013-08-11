#!/bin/sh
# $Gateweaver: install_parse.sh,v 1.3 2007/09/17 14:38:32 cmaxwell Exp $

INSTALL=/usr/bin/install
[[ -z $PASSWORD ]] && PASSWORD=
[[ -z $LICENCE ]] && LICENCE=

if [ -n $PASSWORD ]; then
	PWCRYPT=`encrypt -b 6 -- "$PASSWORD"`
else
	PWCRYPT='*'
fi

ORIGARGS=`getopt BbCcdf:g:m:o:pSs $*`
if [ $? -ne 0 ]; then
	echo `install`
	exit 1
fi
args=
dodir=0
set -- $ORIGARGS
for i ; do
	case "$i" in
		-C)		args="$args -C"; shift;;
		-B|-b)	args="$args -b"; shift;;
		-c)		shift;;
		-d)		args="$args -d"; dodir=1; shift;;
		-f)		args="$args -f $2"; shift; shift;;
		-g)		args="$args -g $2"; shift; shift;;
		-m)		args="$args -m $2"; shift; shift;;
		-o)		args="$args -o $2"; shift; shift;;
		-p)		args="$args -p"; shift;;
		-S)		args="$args -S"; shift;;
		-s)		args="$args -s"; shift;;
		--)		shift; break;;
	esac
done

[[ -z $1 ]] && {
	echo missing source
	exit 1
}

[[ $dodir == 1 ]] && {
	$INSTALL $args $@
	exit $?
}

[[ -z $2 ]] && {
	echo missing destination
	exit 1
}

eval last=\${$#}

for i ; do
	[[ $i == $last ]] && break
	TMP=`mktemp _install.XXXXXXXXXX`
	
	if [ -d $last ]; then
		dest=$last/`basename $i`
	else
		dest=$last
	fi

	sed -e "s,%%PASSWORD%%,$PWCRYPT," \
		-e "s,%%LICENCE%%,$LICENCE," \
		-e "s,%%HOSTNAME%%,$HOSTNAME," \
		-e "s,%%HOSTDOMAIN%%,$HOSTDOMAIN," \
		-e "s,%%OSrev%%,$OSrev," \
		-e "s,%%OSREV%%,$OSREV," \
		< $i > $TMP && \
		$INSTALL $args $TMP $dest
	rm -f $TMP
done
