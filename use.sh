#!/bin/sh

filename="`/usr/local/bin/use.bin -s SH $*`"
. $filename
rc=$?
rm -f $filename
