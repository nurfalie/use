#!/usr/bin/bash

filename="`/usr/local/bin/use.bin -s BASH $*`"
. $filename
rm -f $filename
