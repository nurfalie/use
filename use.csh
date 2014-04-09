#!/bin/csh

set filename="`/usr/local/bin/use.bin -s CSH $*`"
source $filename
set rc=$?
rm -f $filename
