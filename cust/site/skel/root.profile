# $Gateweaver: root.profile,v 1.3 2007/10/15 17:26:37 cmaxwell Exp $
# $OpenBSD: dot.profile,v 1.5 2005/03/30 21:18:33 millert Exp $
#
# sh/ksh initialization

PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/X11R6/bin:/usr/local/sbin:/usr/local/bin
export PATH
: ${HOME='/root'}
export HOME
PKG_PATH=ftp://download.manorsoft.ca/pub/OpenBSD/%%OSREV%%/packages/i386
PS1='$USER@`hostname -s`:$PWD# '
EDITOR=nano
export PKG_PATH PS1 EDITOR
umask 022

if [ -x /usr/bin/tset ]; then
	eval `/usr/bin/tset -sQ \?$TERM`
fi
