#!/usr/local/bin/tcsh

set filename="`/usr/local/bin/use.bin -s TCSH $*`"
source $filename
rm -f $filename
