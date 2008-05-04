#!/usr/bin/sh

filename="`/usr/local/bin/use.bin -s SH $*`"
. $filename
rm -f $filename
