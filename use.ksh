#!/usr/bin/ksh

filename="`/usr/local/bin/use.bin -s KSH $*`"
. $filename
rm -f $filename
