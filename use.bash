#!/bin/bash

filename="`/usr/local/bin/use.bin -s BASH $*`"
. $filename
rc=$?
rm -f $filename
exit $rc
