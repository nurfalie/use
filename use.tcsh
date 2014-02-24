#!/bin/tcsh

set filename="`/usr/local/bin/use.bin -s TCSH $*`"
source $filename
set rc=$?
rm -f $filename
exit $rc
