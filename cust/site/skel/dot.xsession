#!/bin/sh
# $Gateweaver: dot.xsession,v 1.5 2007/10/15 17:26:37 cmaxwell Exp $
PATH=$HOME/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/X11R6/bin:/usr/local/bin:/usr/local/sbin:/usr/games:.
PKG_PATH=ftp://download.manorsoft.ca/pub/OpenBSD/%%OSREV%%/packages/i386
PS1='$USER@`hostname -s`:$PWD$ '
EDITOR=nano
export PATH HOME TERM PKG_PATH PS1 EDITOR

xset fp+ /usr/local/lib/X11/fonts/jmk
xset fp+ /usr/local/lib/X11/fonts/terminus
xset fp+ /usr/local/lib/X11/fonts/ghostscript
xset fp rehash

matchbox-panel &
matchbox-desktop &
matchbox-window-manager || xterm
