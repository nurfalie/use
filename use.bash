#!/usr/bin/bash

filename="`/usr/local/bin/use.bin -s BASH $*`"
source $filename
rm -f $filename
