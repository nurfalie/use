#!/usr/bin/csh

set filename="`/usr/local/bin/use.bin -s CSH $*`"
source $filename
rm -f $filename
