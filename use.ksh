#!/bin/ksh

filename="`/usr/local/bin/use.bin -s KSH $*`"
. $filename
rc=$?
rm -f $filename
